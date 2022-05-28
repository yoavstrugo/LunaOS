#pragma once

#include <system/acpi/acpi.hpp>

#include <stdint.h>

struct k_mcfg_entry {
    uint64_t baseAddress;
    uint16_t pciSegmentGroupNumber;
    uint8_t startBusNumber;
    uint8_t endBusNumber;
    uint32_t reserved;
}__attribute__((packed));

struct k_mcfg_hdr
{
    k_acpi_sdt_hdr header;
    uint64_t reserved;
} __attribute__((packed));



/**
 * @brief Parses the MCFG table
 * 
 * @param mcfgHeader The pointer to the header
 */
void mcfgParse(k_acpi_sdt_hdr *mcfgHeader);