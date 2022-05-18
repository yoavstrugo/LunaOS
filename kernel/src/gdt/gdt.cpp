#include <gdt/gdt.hpp>

#include <logger/logger.hpp>

k_gdt_descriptor gdtDescriptor;
k_gdt_entry gdt[K_GDT_SIZE];

void gdtCreateEntry(uint8_t idx, uint32_t base, uint32_t limit, uint8_t accessByte, uint8_t flags)
{
    gdt[idx].limitLow = (uint16_t)(limit & 0xFFFF);     // first 16 bits of limit
    gdt[idx].baseLow = (uint16_t)(base & 0xFFFF);       // first 16 bits of base
    gdt[idx].baseMid = (uint16_t)((base >> 16) & 0xFF); // next 16 bits of base
    gdt[idx].accessByte = accessByte;
    gdt[idx].limitHighFlags = (uint8_t)(((limit >> 16) & 0xF) | (flags << 4)); // 4bits of limit and 4 bits of flags
    gdt[idx].baseHigh = (uint8_t)((base >> 24) & 0xFF);                        // last 8 bits of base
}

void gdtInitialize()
{
    // Create the table entries
    gdtCreateEntry(0, 0x0, 0x00000, 0x00, 0x0); // null
    gdtCreateEntry(1, 0x0, 0xFFFFF, 0x9A, 0xA); // kernel code
    gdtCreateEntry(2, 0x0, 0xFFFFF, 0x92, 0xC); // kernel data
    gdtCreateEntry(3, 0x0, 0xFFFFF, 0xFA, 0xA); // user code
    gdtCreateEntry(4, 0x0, 0xFFFFF, 0xF2, 0xC); // user data

    // Set the descriptor
    gdtDescriptor.size = (sizeof(k_gdt_entry) * K_GDT_SIZE) - 1;
    gdtDescriptor.offset = (uint64_t)gdt;

    // Inform everythin and load with assembly
    logDebugn("%! has been created at 0x%64x.", "GDT", (uint64_t)gdt);
    logDebugn("%! with \n\tsize %d \n\toffset 0x%64x \n\tcreated at 0x%64x", "GDT Descriptor",
              gdtDescriptor.size, gdtDescriptor.offset, &gdtDescriptor);

    _loadGDT(&gdtDescriptor);
    logDebugn("%! has been loadded into gdtr.", "GDT");
}
