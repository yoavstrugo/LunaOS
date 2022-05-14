#pragma once

#include <stdint.h>
#include <stivale2/stivale2.h>
/**
 * @brief Represnts the descriptor of the RSDP for ACPI version 1.0
 *
 */
struct k_rsdp_descriptor
{
    char signature[8];
    uint8_t checksum;
    char oemId[6];
    uint8_t revision;
    uint32_t rsdtAddress;
} __attribute__((packed));

/**
 * @brief Represnts the descriptor of the RSDP for ACPI version 2.0 and above
 *
 */
struct k_rsdp_descriptor_20
{
    k_rsdp_descriptor firstPart;

    uint32_t length;
    uint64_t xsdtAddress;
    uint8_t extendedChecksum;
    uint8_t reserved[3];
} __attribute__((packed));

/**
 * @brief Uses the stivale2 boot info to retrieve the RSDP and then validates it.
 *
 * @param bootInfo  The stivale2 bootinfo
 * @return Pointer to the RSDP decriptor, NULL if not found or invalid.
 */
k_rsdp_descriptor *rsdpGet(stivale2_struct *bootInfo);

/**
 * @brief Validate the checksum of the rsdp descriptor
 *
 * @param rsdp      The rsdp descriptor
 * @return true     If the checksum is correct
 * @return false    If the checksum is incorrect
 */
bool rsdpValidate(k_rsdp_descriptor *rsdp);