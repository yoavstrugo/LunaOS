#pragma once

#include <stdint.h>
#include <types.hpp>

// Memory Mapped Registers for Accessing IOAPIC Registers

/**
 * @brief This register selects the IOAPIC Register to be read/written.
 * The data is then read from or written to the selected register through the IOWIN Register.
 */
#define IOAPIC_REGSEL 0x00
/**
 * @brief This register is used to write to and read from the register selected by the
 * IOREGSEL Register. Readability/writability is determined by the IOAPIC register that is
 * currently selected.
 */
#define IOAPIC_REGWIN 0x10

// IOAPIC Registers

/**
 * @brief This register contains the 4-bit APIC ID
 */
#define IOAPIC_ID 0x00

/**
 * @brief The IOAPIC Version Register identifies the APIC hardware version
 */
#define IOAPIC_VER 0x01

/**
 * @brief The APICARB Register contains the bus arbitration priority for the IOAPIC
 */
#define IOAPIC_ARB 0x02

#define IOAPIC_REDTBL 0x10

/**
 * Values for REDTBL entries
 */
#define IOAPIC_REDTBL_INTVEC_MAKE(i) ((i)&0xFF)

#define IOAPIC_REDTBL_DELMOD_FIXED (0x0 << 8)  // 000
#define IOAPIC_REDTBL_DELMOD_LOWEST (0x1 << 8) // 001
#define IOAPIC_REDTBL_DELMOD_SMI (0x2 << 8)    // 010
#define IOAPIC_REDTBL_DELMOD_NMI (0x4 << 8)    // 100
#define IOAPIC_REDTBL_DELMOD_INIT (0x5 << 8)   // 101
#define IOAPIC_REDTBL_DELMOD_EXTINT (0x7 << 8) // 111

#define IOAPIC_REDTBL_DESTMOD_PHYSICAL (0 << 10)
#define IOAPIC_REDTBL_DESTMOD_LOGICAL (1 << 10)

#define IOAPIC_REDTBL_DELIVS_IDLE (0 << 11)
#define IOAPIC_REDTBL_DELIVS_SEND_PENDING (1 << 11)

#define IOAPIC_REDTBL_INTPOL_HIGH_ACTIVE (0 << 12)
#define IOAPIC_REDTBL_INTPOL_LOW_ACTIVE (1 << 12)

#define IOAPIC_REDTBL_REMOTEIRR_REC_EOI (0 << 13)
#define IOAPIC_REDTBL_REMOTEIRR_ACCEPTING (1 << 13)

#define IOAPIC_REDTBL_TRIGGERMOD_EDGE (0 << 14)
#define IOAPIC_REDTBL_TRIGGERMOD_LEVEL (1 << 14)

#define IOAPIC_REDTBL_INTMASK_UNMASKED (0 << 15)
#define IOAPIC_REDTBL_INTMASK_MASKED (1 << 15)

#define IOAPIC_REDTBL_DESTINATION_MAKE(i, f) ((((uint64_t)i & 0xFF) | (uint64_t)f) << 56)
#define IOAPIC_REDTBL_DESTINATION_FLAG_PHYSICAL (0 << 10)
#define IOAPIC_REDTBL_DESTINATION_FLAG_LOGICAL (1 << 10)

/**
 * Masks for each entry
 */
#define IOAPIC_REDTBL_MASK_INTVEC (0xFF)
#define IOAPIC_REDTBL_MASK_DELMOD (7 << 8)
#define IOAPIC_REDTBL_MASK_DESTMOD (1 << 10)
#define IOAPIC_REDTBL_MASK_DELIVS (1 << 11)
#define IOAPIC_REDTBL_MASK_INTPOL (1 << 12)
#define IOAPIC_REDTBL_MASK_REMOTEIRR (1 << 13)
#define IOAPIC_REDTBL_MASK_TRIGGERMOD (1 << 14)
#define IOAPIC_REDTBL_MASK_INTMASK (1 << 15)
#define IOAPIC_REDTBL_MASK_RESERVED (0xFFFFFFFFFFC << 16)
#define IOAPIC_REDTBL_MASK_DESTINATION (0xFF << 55)

struct k_ioapic_entry
{
    uint8_t id;
    virtual_address_t address;

    uint32_t globalSystemInterruptBase;
    uint32_t redtblSize;

    k_ioapic_entry *next;
};

/**
 * @brief Adds a new IOAPIC
 *
 * @param id The id of the IOAPIC
 * @param physcialAddress The physical address of the IOAPIC
 * @param globalSystemInterruptBase The GSI base of the IOAPIC
 */
void ioapicAdd(uint8_t id, physical_address_t physcialAddress, uint32_t globalSystemInterruptBase);

/**
 * @brief Writes a value to the register of the IOAPIC
 *
 * @param ioapic The IOAPIC the register belongs to
 * @param reg The register to write to
 * @param value The value
 */
void ioapicWrite(k_ioapic_entry *ioapic, uint32_t reg, uint32_t value);

/**
 * @brief Read the value of a IOAPIC's register
 *
 * @param ioapic The IOAPIC the register belongs to
 * @param reg The register
 */
uint32_t ioapicRead(k_ioapic_entry *ioapic, uint32_t reg);

/**
 * @brief Get the IOAPIC responsible for the source
 *
 * @param source The source
 * @return k_ioapic_entry* The IOAPIC responsible for the source
 */
k_ioapic_entry *ioapicGetResponsible(uint32_t source);

/**
 * @brief Sets the redirection on the IOAPIC
 *
 * @param ioapic The IOAPIC
 * @param index The index of the redirection
 * @param value The redirection entry
 */
void ioapicSetRedirection(k_ioapic_entry *ioapic, uint32_t index, uint64_t value);

/**
 * @brief Create an ISA redirection to the IRQ
 *
 * @param source The source interrupt
 * @param irq The destination IRQ
 * @param lapicId The LAPIC id
 * @return true If it was successfull
 * @return false If it wasn't successfull
 */
bool ioapicCreateISARedirection(uint32_t source, uint32_t irq, uint32_t lapicId);