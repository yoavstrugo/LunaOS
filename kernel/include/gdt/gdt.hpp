#pragma once

#include <stdint.h>
#include <types.hpp>

#define K_GDT_SIZE 6
#define K_TSS_LOCATION  0xFFFF800150200000

/**
 * @brief Reprsents the descriptor for the GDT.
 *
 */
struct k_gdt_descriptor
{
    uint16_t size;
    uint64_t offset;
} __attribute__((packed));

/**
 * @brief Represnts a GDT entry (the segment descriptor).
 *
 */
struct k_gdt_entry
{
    uint16_t limitLow;
    uint16_t baseLow;
    uint8_t baseMid;
    uint8_t accessByte;
    uint8_t limitHighFlags;
    uint8_t baseHigh;
} __attribute__((packed));

struct k_gdt_tss_entry {
    uint16_t limitLow;
    uint16_t baseLow;
    uint8_t baseMid0;
    uint8_t accessByte;
    uint8_t limitHighFlags;
    uint8_t baseMid1;
    uint32_t baseHigh;
    uint32_t reserved;
} __attribute__((packed));

struct k_tss {
    uint32_t reserved0;
    uint32_t rsp0Low;
    uint32_t rsp0High;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t ioMapBaseAddress;
}__attribute__((packed));

struct k_gdt {
    k_gdt_entry entries[K_GDT_SIZE];
    k_gdt_tss_entry tss;
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
 * @brief Creates a TSS entry in the GDT
 * 
 */
void gdtCreateTSSEntry();

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
extern "C" void _flushTSS();