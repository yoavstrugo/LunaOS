#pragma once

#include <stdint.h>
#include <types.hpp>

#define K_GDT_SIZE 5
#define K_TSS_LOCATION 0xFFFF800150200000

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

enum DESCRIPTOR_ACCESS
{
    ACCESS_BIT8 = 0x01,
    ACCESS_BIT9 = 0x02,
    ACCESS_BIT10 = 0x04,
    ACCESS_BIT11 = 0x08,
    ACCESS_TYPE = 0x10,
    ACCESS_DPL = 0x60,
    ACCESS_PRESENT = 0x80
};

#define KERNEL_PRIVILEGE (0)
#define USER_PRIVILEGE (3)

#define DESCRIPTOR_TYPE_TSS (ACCESS_BIT8 | ACCESS_BIT11)

#define DPL_KERNEL_ACCESS (ACCESS_DPL & KERNEL_PRIVILEGE)
#define DPL_USER_ACCESS (ACCESS_DPL & USER_PRIVILEGE)

enum DESCRIPTOR_FLAGS
{
    SEGMENT_AVAILABLE = 0x10,
    SEGMENT_64BIT_CODE = 0x20,
    SEGMENT_DEFAULT = 0x40,
    SEGMENT_GRANULARITY = 0x80
};

struct TSSDescriptor
{
    uint16_t limitLow;
    uint16_t baseLow;

    uint8_t baseMid0;
    uint8_t accessByte;
    uint8_t limitHighFlags;
    uint8_t baseMid1;

    uint32_t baseHigh;

    uint32_t reserved;

    void set(uint64_t base, uint64_t limit, uint8_t flags, uint8_t access)
    {
        limitLow = (uint16_t)(limit & 0xFFFF);
        baseLow = (uint16_t)(base & 0xFFFF);
        baseMid0 = (uint8_t)((base >> 16) & 0xFFFF);
        accessByte = (uint8_t)((access | DESCRIPTOR_TYPE_TSS | ACCESS_PRESENT));
        limitHighFlags = (uint8_t)(((limit >> 16) && 0xF) | (flags & 0xF0));
        baseMid1 = (uint8_t)((base >> 24) & 0xFF);
        baseHigh = (uint32_t)((base >> 32) & 0xFFFFFFFF);
        reserved = 0;
    }

} __attribute__((packed));

struct TSS
{
    uint32_t reserved0;

    uint32_t rsp0Low;
    uint32_t rsp0High;
    uint64_t rsp[2];

    uint64_t reserved1;

    uint64_t ist[7];

    uint64_t reserved2;
    uint16_t reserved3;

    uint16_t iopbOffset;
} __attribute__((packed));

struct k_gdt
{
    k_gdt_entry entries[K_GDT_SIZE];
    TSSDescriptor tss;
} __attribute__((packed));

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
 * @brief Sets the active "privileged" stack
 *
 * @param stackPtr The pointer to the stack (stack end)
 */
void gdtSetActiveStack(virtual_address_t stackPtr);

/**
 * @brief Loads the GDT into the lgdt register, defined with assembly
 *
 */
extern "C" void _loadGDT(k_gdt_descriptor *gdtDescriptorAddress);
extern "C" void _loadTSS(uint16_t tssIndex);