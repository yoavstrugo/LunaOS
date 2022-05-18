#pragma once

#include <system/acpi/rsdp.hpp>
#include <stivale2/stivale2.h>
#include <memory/paging.hpp>

#define K_XSDT_ENTRIES(xsdt) (xsdt->length - sizeof(k_acpi_sdt_hdr)) / 8

#define K_ACPI_SIGNATURE_BYTES 4

struct k_acpi_sdt_hdr
{
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oemId[6];
  char oemTableId[8];
  uint32_t oemRevision;
  uint32_t creatorID;
  uint32_t creatorRevision;
} __attribute__((packed));

struct k_xsdt
{
  k_acpi_sdt_hdr h;
  uint64_t *pointerToOtherSDT;
};

struct k_acpi_entry
{
  k_acpi_sdt_hdr *header;
  k_acpi_entry *next;
};

/**
 * @brief Initializes the ACPI and parse some tables
 *
 * @param bootInfo The stivale2 boot info
 */
void acpiInitialize(stivale2_struct *bootInfo);

/**
 * @brief Prepares the root STD and detrmain whether it's xsdt or rsdt
 *
 * @param rsdp  The pointer to rsdp.
 */
void acpiPrepareXSDT(k_rsdp_descriptor *rsdp);

/**
 * @brief Maps a virtual address to the physical address of the SDT.
 *
 * @param sdtAddr  The physical address of the SDT
 * @return k_acpi_sdt_hdr* Pointer to the mapped virtual address of the SDT header.
 */
k_acpi_sdt_hdr *acpiMapSDT(physical_address_t sdtAddr);

/**
 * @brief Validates the SDT by checking its checksum
 *
 * @param sdt The SDT to check
 * @return true If the SDT is valid
 * @return false If the SDT is invalid
 */
bool acpiValidateSDT(k_acpi_sdt_hdr *sdt);

/**
 * @brief Get the i'th SDT in the XSDT
 *
 * @param i   The index of the SDT
 * @return physical_address_t The physical address of the SDT
 */
physical_address_t acpiGetEntry(uint32_t i);

/**
 * @brief Get the ACPI entry with the provided signature
 *
 * @param signature The signature of the SDT
 * @return k_acpi_sdt_hdr* The pointer to the entry
 */
k_acpi_sdt_hdr *acpiGetEntryWithSignature(const char *signature);