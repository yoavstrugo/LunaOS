#pragma once

#include <stdint.h>
#include <types.hpp>

struct k_gdt_descriptor {
    uint16_t size;
    uint64_t offset;
}__attribute__((packed)); 

struct k_gdt_entry {
    uint16_t limit0;
    uint16_t base0;
    uint8_t base1;
    uint8_t accessByte;
    uint8_t limit1_flags;
    uint8_t base2;

    k_gdt_entry(uint64_t base, uint32_t limit, uint8_t accessByte, uint8_t flags);
}__attribute__((packed));

void gdtInitiallize(linear_address gdtAddress);

/**
 * @brief Loads the GDT into the lgdt register, defined with assembly
 * 
 */
extern "C" void _loadGDT(k_gdt_descriptor *gdtDescriptorAddress);