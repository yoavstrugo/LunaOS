#pragma once

#include <stdint.h>
#include <types.hpp>

#define K_GDT_SIZE  5

/**
 * @brief Reprsents the descriptor for the GDT.
 * 
 */
struct k_gdt_descriptor {
    uint16_t size;
    uint64_t offset;
}__attribute__((packed)); 

/**
 * @brief Represnts a GDT entry (the segment descriptor).
 * 
 */
struct k_gdt_entry {
    uint16_t limitLow;
    uint16_t baseLow;
    uint8_t baseMid;
    uint8_t accessByte;
    uint8_t limitHighFlags;
    uint8_t baseHigh;
}__attribute__((packed));

/**
 * @brief Creates a GDT entry (Segment Descriptor) 
 * 
 * @param idx           The index to insert it in. 
 * @param base          Where the segment starts.
 * @param limit         How long the segment is.
 * @param accessByte    The access byte.
 * @param flags         The flags.
 */
void gdtCreateEntry(uint8_t idx, uint32_t base, uint32_t limit, uint8_t accessByte, uint8_t flags);

/**
 * @brief Initializes the GDT, and loads it into gdtr.
 * 
 */
void gdtInitialize();

/**
 * @brief Loads the GDT into the lgdt register, defined with assembly
 * 
 */
extern "C" void _loadGDT(k_gdt_descriptor *gdtDescriptorAddress);