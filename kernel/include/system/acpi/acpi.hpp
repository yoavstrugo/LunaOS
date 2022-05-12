#pragma once

#include <system/acpi/rsdp.hpp>
#include <stivale2/stivale2.h>
#include <paging.hpp>

struct k_acpi_sdt_hdr {
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oemId[6];
  char oemTableId[8];
  uint32_t oemRevision;
  uint32_t creatorID;
  uint32_t creatorRevision;
}__attribute__((packed));

struct k_xsdt {
  k_acpi_sdt_hdr h;
  uint64_t *pointerToOtherSDT;
};

struct k_sdt_entry {
  k_acpi_sdt_hdr *header;
  k_sdt_entry *next;
};

/**
 * @brief Initializes the ACPI and parse some tables
 * 
 * @param bootInfo The stivale2 boot info
 */
void acpiInitialize(stivale2_struct *bootInfo);

/**
 * @brief Validate the header of the ACPI SDT
 * 
 * @param acpi_sdt_hdr  The ACPI SDT header
 * @return true         If the checksum matches the header
 * @return false        If the checksum doesn't matches the header
 */
bool acpiValidateHeader(k_acpi_sdt_hdr *acpiSDTHeader);

/**
 * @brief Maps a virtual address to the physical address of the SDT. 
 * 
 * @param sdtAddr  The physical address of the SDT
 * @return k_acpi_sdt_hdr* Pointer to the mapped virtual address of the SDT header.
 */
k_acpi_sdt_hdr *acpiMapSDT(physical_address_t sdtAddr);