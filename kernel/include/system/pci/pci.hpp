#pragma once

#include <stdint.h>
#include <types.hpp>

struct k_pci_device_hdr
{
    uint16_t vendorId;
    uint16_t deviceId;
    uint16_t command;
    uint16_t status;
    uint8_t revisionId;
    uint8_t progIf;
    uint8_t subclass;
    uint8_t mainClass;
    uint8_t cacheLineSize;
    uint8_t latencyTimer;
    uint8_t headerType;
    uint8_t bist;
}__attribute__((packed));

struct k_pci_header0 {
    k_pci_device_hdr header;
    uint32_t BAR0;
    uint32_t BAR1;
    uint32_t BAR2;
    uint32_t BAR3;
    uint32_t BAR4;
    uint32_t BAR5;
    uint32_t cardbusCISPointer;
    uint16_t subsystemVendorID;
    uint16_t subsystemID;
    uint32_t expansionROMBaseAddress;
    uint8_t capabilitesPointer;
    uint8_t reserved[7];
    uint8_t interruptLine;
    uint8_t interruptPIN;
    uint8_t minGrant;
    uint8_t maxLatency;
}__attribute__((packed));

struct k_pci_device_entry {
    k_pci_device_hdr *device;
    k_pci_device_entry *next;
};

void pciParseBus(physical_address_t base, uint64_t bus);

void pciParseDevice(physical_address_t base, uint64_t device);

void pciParseFunction(virtual_address_t deviceAddr, uint64_t function);

k_pci_device_hdr *pciGetDevice(uint8_t classCode,
                               uint8_t subclassCode,
                               uint8_t progIf);