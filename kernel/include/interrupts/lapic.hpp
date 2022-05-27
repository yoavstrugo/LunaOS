#pragma once

#include <types.hpp>
#include <memory/paging.hpp>

#define K_LAPIC_EXPECTED_ADDRESS 0xFEE00000

// APIC registers
#define APIC_REGISTER_ID 0x20
#define APIC_REGISTER_VERSION 0x30
#define APIC_REGISTER_TASK_PRIO 0x80
#define APIC_REGISTER_ARBITRATION_PRIO 0x90
#define APIC_REGISTER_EOI 0xB0
#define APIC_REGISTER_LOGICAL_DEST 0xD0
#define APIC_REGISTER_DEST_FORMAT 0xE0
#define APIC_REGISTER_SPURIOUS_IVT 0xF0
#define APIC_REGISTER_ERROR_STATUS 0x280
#define APIC_REGISTER_LVT_CMCI 0x2F0
#define APIC_REGISTER_INT_COMMAND_LOW 0x300
#define APIC_REGISTER_INT_COMMAND_HIGH 0x310
#define APIC_REGISTER_LVT_TIMER 0x320
#define APIC_REGISTER_LVT_THERMAL_SENSOR 0x320
#define APIC_REGISTER_LVT_PERFMON 0x340
#define APIC_REGISTER_LVT_LINT0 0x350
#define APIC_REGISTER_LVT_LINT1 0x360
#define APIC_REGISTER_LVT_ERROR 0x370
#define APIC_REGISTER_TIMER_INITCNT 0x380
#define APIC_REGISTER_TIMER_CURRCNT 0x390
#define APIC_REGISTER_TIMER_DIV 0x3E0

/**
 * Spurious interrupt entry flags
 */
#define APIC_SPURIOUS_IVT_SOFTWARE_ENABLE 0x100

/**
 * LVT flags
 */
#define APIC_LVT_DELIVERY_MODE_FIXED (0 << 8)  // 000........
#define APIC_LVT_DELIVERY_MODE_SMI (2 << 8)    // 010........
#define APIC_LVT_DELIVERY_MODE_NMI (4 << 8)    // 100........
#define APIC_LVT_DELIVERY_MODE_INIT (5 << 8)   // 101........
#define APIC_LVT_DELIVERY_MODE_EXTINT (7 << 8) // 111........

#define APIC_LVT_GET_DELIVERY_STATUS(i) ((i >> 12) & 1)

#define APIC_LVT_INT_POLARITY_HIGH (1 << 13)
#define APIC_LVT_INT_POLARITY_LOW (0 << 13)

#define APIC_LVT_GET_REMOTE_IRR(i) ((i << 14) & 1)

#define APIC_LVT_TRIGGER_MODE_EDGE (0 << 15)
#define APIC_LVT_TRIGGER_MODE_LEVEL (1 << 15)

#define APIC_LVT_INT_UNMASKED (0 << 16)
#define APIC_LVT_INT_MASKED (1 << 16)

#define APIC_LVT_TIMER_MODE_ONESHOT (0 << 17)      // 00
#define APIC_LVT_TIMER_MODE_PERIODIC (1 << 17)     // 01
#define APIC_LVT_TIMER_MODE_TSC_DEADLINE (2 << 17) // 10

#define APIC_TIMER_TIMESLOT_MS 2

const k_paging_flags LAPIC_MEMORY_FLAGS = {
    .pml4Flags = PAGETABLE_PRESENT | PAGETABLE_READWRITE,
    .pdptFlags = PAGETABLE_PRESENT | PAGETABLE_READWRITE,
    .pdFlags = PAGETABLE_PRESENT | PAGETABLE_READWRITE,
    .ptFlags = PAGE_PRESENT | PAGE_READWRITE | PAGE_CACHE};

/**
 * @brief Prepares the Local APIC for initialization
 *
 * @param lapicAddress The address of the global lapic registers
 */
void lapicPrepare(physical_address_t lapicAddress);

/**
 * @brief Initializes the Local APIC, must be prepared beforehand.
 *
 */
void lapicInitialize();

/**
 * @brief Read the contents of a Local APIC register
 *
 * @param reg The register's offset
 * @return uint32_t The value of the register
 */
uint32_t lapicRead(uint32_t reg);

/**
 * @brief Write to a Local APIC register
 *
 * @param reg The offset of the register to write to
 * @param value The value to write
 */
void lapicWrite(uint32_t reg, uint32_t value);

/**
 * @brief Starts the ACPI timer on frequency of 1000hz (interrupt every 1ms)
 *
 */
void lapicStartTimer();

/**
 * @brief Send an End Of Interrupt to the Local APIC
 *
 */
void lapicSendEOI();