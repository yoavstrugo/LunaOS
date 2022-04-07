#pragma once

#include <stdint.h>

#define K_IDT_SIZE  256

#define K_IDT_TA_INTERRUPT  0x8E
#define K_IDT_TA_TRAP       0x8F

struct k_idt_descriptor {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed));

struct k_idt_entry {
    uint16_t offsetLow;
    uint16_t segmentSelector;
    uint8_t ist;
    uint8_t flags;
    uint16_t offsetMid;
    uint32_t offsetHigh;
    uint32_t zero;
}__attribute__((packed));

/**
 * @brief Create an IDT entry (gate descriptor)
 * 
 * @param idx               The index in the IDT to put the entry in
 * @param offset            The address of the Interrupt Service Routine 
 * @param segmentSelector   The segment offset of the Interrupt Service Routine segment in the GDT
 * @param ist               Offset to the Interrupt Stack Table
 * @param flags             Flags.
 */
void idtCreateEntry(uint8_t idx, uint64_t offset, uint16_t segmentSelector, uint8_t ist, uint8_t flags);

/**
 * @brief Create the IDT descriptor
 * 
 */
void idtInitiallize();

/**
 * @brief Print all the information about the IDT and loads it into gdtr.
 * 
 */
void idtLoad();

/**
 * @brief Loads the contents of idtDescriptorAddress (Value it points at) to gdtr
 * 
 * @param idtDescriptorAddress The pointer to the IDT descriptor.
 * 
 */
extern "C" void _loadIDT(k_idt_descriptor *idtDescriptorAddress);