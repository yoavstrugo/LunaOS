#include <gdt/gdt.hpp>

#include <logger/logger.hpp>
#include <memory/memory.hpp>
#include <kernel.hpp>

k_gdt_descriptor gdtDescriptor;
k_gdt gdt;
k_tss tss;

void gdtCreateEntry(uint8_t idx, uint32_t base, uint32_t limit, uint8_t accessByte, uint8_t flags)
{
    gdt.entries[idx].limitLow = (uint16_t)(limit & 0xFFFF);     // first 16 bits of limit
    gdt.entries[idx].baseLow = (uint16_t)(base & 0xFFFF);       // first 16 bits of base
    gdt.entries[idx].baseMid = (uint16_t)((base >> 16) & 0xFF); // next 16 bits of base
    gdt.entries[idx].accessByte = accessByte;
    gdt.entries[idx].limitHighFlags = (uint8_t)(((limit >> 16) & 0xF) | (flags << 4)); // 4bits of limit and 4 bits of flags
    gdt.entries[idx].baseHigh = (uint8_t)((base >> 24) & 0xFF);                        // last 8 bits of base
}

void gdtCreateTSSEntry() {
    uint64_t base = ((uint64_t)&tss);

    gdt.tss.limitLow = (uint16_t)(sizeof(k_tss) & 0xFFFF);
    gdt.tss.limitHighFlags = (uint8_t)(((sizeof(k_tss) >> 16) & 0xF));
    gdt.tss.accessByte = 0x89;
    gdt.tss.baseLow = (uint16_t)(base & 0xFFFF);
    gdt.tss.baseMid0 = (uint16_t)((base >> 16) & 0xFF);
    gdt.tss.baseMid1 = (uint8_t)((base >> 24) & 0xFF);
    gdt.tss.baseHigh = (uint32_t)(base >> 32);
}

void gdtInitialize()
{
    // Create the table entries
    gdtCreateEntry(0, (uint32_t)0x0, 0x00000, 0x00, 0x0); // null
    gdtCreateEntry(1, (uint32_t)0x0, 0xFFFFF, 0x9A, 0xA); // kernel code
    gdtCreateEntry(2, (uint32_t)0x0, 0xFFFFF, 0x92, 0xC); // kernel data
    gdtCreateEntry(3, (uint32_t)0x0, 0xFFFFF, 0xFA, 0xA); // user code
    gdtCreateEntry(4, (uint32_t)0x0, 0xFFFFF, 0xF2, 0xC); // user data

    // TSS
    gdtCreateTSSEntry();
    tss.ist1 = 0x8;
    virtual_address_t stackPtr = kernelGetStack();
    tss.rsp0Low = (uint32_t)(stackPtr && 0xffffffff);
    tss.rsp0High = (uint32_t)(stackPtr >> 32);

    // Set the descriptor
    gdtDescriptor.size = sizeof(gdt) - 1;
    gdtDescriptor.offset = (uint64_t)&gdt;

    // Inform everything and load with assembly
    logDebugn("%! GDT has been created at 0x%64x.", "[GDT]", (uint64_t)&gdt);
    logDebugn("\t- Size %d \n\t- Offset 0x%64x \n\t- Created at 0x%64x", gdtDescriptor.size, gdtDescriptor.offset, &gdtDescriptor);

    _loadGDT(&gdtDescriptor);
    logDebugn("%! has been loaded into GDT register.", "[GDT]");
    // _flushTSS();
    // logDebugn("%! flushed TSS.", "[GDT]");
}
