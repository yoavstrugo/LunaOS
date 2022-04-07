#include <gdt/gdt.hpp>

#include <logger/logger.hpp>

k_gdt_entry::k_gdt_entry(uint64_t base, uint32_t limit, uint8_t accessByte, uint8_t flags) {
    this->limit0 = (uint16_t)(limit & 0xFFFF); // first 16 bits of limit
    this->base0 = (uint16_t)(base & 0xFFFF); // first 16 bits of base
    this->base1 = (uint16_t)((base >> 16) & 0xFF); // next 8 bits
    this->accessByte = accessByte;
    this->limit1_flags = (uint8_t)(((limit >> 16) & 0xF) | (flags << 4));
    this->base2 = (uint8_t)((base >> 0x18) & 0xFF);
};

void gdtInitiallize(linear_address gdtPage) {
    linear_address gdtAddress = gdtPage + sizeof(k_gdt_descriptor);
    
    k_gdt_entry *gdt = (k_gdt_entry *)gdtAddress;

    gdt[0] = k_gdt_entry(0x0, 0x00000, 0x00, 0x0); // null
    gdt[1] = k_gdt_entry(0x0, 0xFFFFF, 0x9A, 0xA); // kernel code
    gdt[2] = k_gdt_entry(0x0, 0xFFFFF, 0x92, 0xC); // kernel data
    gdt[3] = k_gdt_entry(0x0, 0xFFFFF, 0xFA, 0xA); // user code
    gdt[4] = k_gdt_entry(0x0, 0xFFFFF, 0xF2, 0xC); // user data

    k_gdt_descriptor *gdtDesc = (k_gdt_descriptor *)gdtPage;
    gdtDesc->size = (sizeof(k_gdt_entry) * 5) - 1;
    gdtDesc->offset = gdtAddress;

    logDebugn("%! has been created at 0x%64x.", "GDT", gdtAddress);
    logDebugn("%! with \n\tsize %d \n\toffset 0x%64x \n\tcreated at 0x%64x", "GDT descriptor", gdtDesc->size, gdtDesc->offset, gdtDesc);

    _loadGDT(gdtDesc);
    logDebugn("Loaded GDT at 0x%64x into lgdt register,", gdtDesc);
}

