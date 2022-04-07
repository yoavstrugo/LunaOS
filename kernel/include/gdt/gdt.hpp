#pragma once

#include <stdint.h>
#include <types.hpp>

#define K_GDT_SIZE  5

struct k_gdt_descriptor {
    uint16_t size;
    uint64_t offset;
}__attribute__((packed)); 

struct k_gdt_entry {
    uint16_t limitLow;
    uint16_t baseLow;
    uint8_t baseMid;
    uint8_t accessByte;
    uint8_t limitHighFlags;
    uint8_t baseHigh;
}__attribute__((packed));

k_gdt_entry gdtCreateEntry(uint64_t base, uint32_t limit, uint8_t accessByte, uint8_t flags);

void gdtInitiallize();

/**
 * @brief Loads the GDT into the lgdt register, defined with assembly
 * 
 */
extern "C" void _loadGDT(k_gdt_descriptor *gdtDescriptorAddress);