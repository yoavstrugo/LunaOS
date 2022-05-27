#include <interrupts/lapic.hpp>

#include <stddef.h>
#include <types.hpp>
#include <logger/logger.hpp>
#include <memory/memory.hpp>
#include <kernel.hpp>
#include <memory/paging.hpp>
#include <system/pit.hpp>
#include <interrupts/interrupts.hpp>

virtual_address_t lapicGlobalAddress = NULL;
bool isPrepared = false;

void lapicPrepare(physical_address_t lapicAddress)
{
    if (lapicAddress != K_LAPIC_EXPECTED_ADDRESS)
        logWarnn("%! Local APIC address 0x%64x different from expected 0x%64x.",
                 "[LAPIC]",
                 lapicAddress,
                 K_LAPIC_EXPECTED_ADDRESS);

    // Map a virtual address to the physical address
    virtual_address_t lapicVirtAddress = virtualAddressRangeAllocator.allocateRange(1);
    if (!lapicVirtAddress)
        kernelPanic("%! Couldn't get a virtual address for the global Local APIC physical address.", "[LAPIC]");

    pagingMapPage(lapicVirtAddress, lapicAddress, LAPIC_MEMORY_FLAGS);
    lapicGlobalAddress = lapicVirtAddress;
    isPrepared = true;
    logDebugn("%! Local APIC has been prepared, Global LAPIC Address: 0x%64x.", "[LAPIC]", lapicGlobalAddress);
}

void lapicInitialize()
{
    lapicWrite(APIC_REGISTER_DEST_FORMAT, 0xFFFFFFFF);
    lapicWrite(APIC_REGISTER_LOGICAL_DEST, (lapicRead(APIC_REGISTER_LOGICAL_DEST) & 0x00FFFFFF) | 1);
    lapicWrite(APIC_REGISTER_LVT_TIMER, APIC_LVT_INT_MASKED);
    lapicWrite(APIC_REGISTER_LVT_PERFMON, APIC_LVT_DELIVERY_MODE_NMI);
    lapicWrite(APIC_REGISTER_LVT_LINT0, APIC_LVT_INT_MASKED);
    lapicWrite(APIC_REGISTER_LVT_LINT1, APIC_LVT_INT_MASKED);
    lapicWrite(APIC_REGISTER_TASK_PRIO, 0);

    lapicWrite(APIC_REGISTER_SPURIOUS_IVT, 0xFF | APIC_SPURIOUS_IVT_SOFTWARE_ENABLE);
    logDebugn("%! Local APIC has been initialized and enabled.", "[LAPIC]");
}

void lapicWrite(uint32_t reg, uint32_t value)
{
    if (!isPrepared)
        kernelPanic("%! Tried to perform an action but LAPIC is not prepared.", "[LAPIC]");

    *((volatile uint32_t *)(lapicGlobalAddress + reg)) = value;
}

uint32_t lapicRead(uint32_t reg)
{
    return *((volatile uint32_t *)(lapicGlobalAddress + reg));
}

void lapicStartTimer()
{
    interruptsDisable();
    // Tell APIC timer to use divider 16
    lapicWrite(APIC_REGISTER_TIMER_DIV, 0x3);

    // Prepare the PIT to sleep for 10ms (10000µs)
    pitPrepareSleep(10000);

    // Set APIC init counter to -1
    lapicWrite(APIC_REGISTER_TIMER_INITCNT, 0xFFFFFFFF);

    // Perform PIT-supported sleep
    pitPerformSleep();

    // Stop the APIC timer
    lapicWrite(APIC_REGISTER_LVT_TIMER, APIC_LVT_INT_MASKED);

    // Now we know how often the APIC timer has ticked in 10ms
    uint32_t ticksIn10ms = 0xFFFFFFFF - lapicRead(APIC_REGISTER_TIMER_CURRCNT);

    logDebugn("%! Detected %d ticks in 10ms of APIC Timer", "[LAPIC]", ticksIn10ms);

    // Start timer as periodic on IRQ 0, divider 16, with the number of ticks we counted
    lapicWrite(APIC_REGISTER_LVT_TIMER, 32 | APIC_LVT_TIMER_MODE_PERIODIC);
    lapicWrite(APIC_REGISTER_TIMER_DIV, 0x3);
    lapicWrite(APIC_REGISTER_TIMER_INITCNT, ticksIn10ms / 10 * APIC_TIMER_TIMESLOT_MS);

    logDebugn("%! APIC Timer has started with freqency 1khz (tick every 1ms)", "[LAPIC]");
    interruptsEnable();
}

void lapicSendEOI()
{
    lapicWrite(APIC_REGISTER_EOI, 0);
}