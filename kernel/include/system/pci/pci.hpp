#pragma once

#include <stdint.h>
#include <types.hpp>
#include <system/acpi/mcfg.hpp>

#define PCI_TYPE0_ADDRESSES 6
#define PCI_TYPE1_ADDRESSES 2

struct PCICommonConfig
{
    uint16_t vendorID;
    uint16_t deviceID;
    uint16_t command;
    uint16_t status;
    uint8_t revisionID;
    uint8_t progIf;
    uint8_t subClass;
    uint8_t baseClass;
    uint8_t cacheLineSize;
    uint8_t latencyTimer;
    uint8_t headerType;
    uint8_t BIST;

    union
    {
        struct PCIHeaderType0
        {
            uint32_t baseAddresses[PCI_TYPE0_ADDRESSES];
            uint32_t CIS;
            uint16_t subVendorID;
            uint16_t subSystemID;
            uint32_t romBaseAddress;
            uint32_t reserved2[2];

            uint8_t interruptLine;
            uint8_t interruptPin;
            uint8_t minimumGrant;
            uint8_t maximumLatency;
        } type0;

        struct PCIHeaderType1
        {
            uint32_t baseAddresses[PCI_TYPE1_ADDRESSES];
            uint8_t primaryBusNumber;
            uint8_t secondaryBusNumber;
            uint8_t subordinateBusNumber;
            uint8_t secondaryLatencyTimer;
            uint8_t IOBase;
            uint8_t IOLimit;
            uint16_t secondaryStatus;
            uint16_t memoryBase;
            uint16_t memoryLimit;
            uint16_t prefetchableMemoryBase;
            uint16_t prefetchableMemoryLimit;
            uint32_t prefetchableMemoryBaseUpper32;
            uint32_t prefetchableMemoryLimitUpper32;
            uint16_t IOBaseUpper;
            uint16_t IOLimitUpper;
            uint32_t reserved2;
            uint32_t expansionROMBase;
            uint8_t interruptLine;
            uint8_t interruptPin;
            uint16_t bridgeControl;
        } type1;

        struct PCIHeaderType2
        {
            uint32_t baseAddress;
            uint8_t capabilitiesPtr;
            uint8_t reserved2;
            uint16_t secondaryStatus;
            uint8_t primaryBusNumber;
            uint8_t cardbusBusNumber;
            uint8_t subordinateBusNumber;
            uint8_t cardbusLatencyTimer;
            uint32_t memoryBase0;
            uint32_t memoryLimit0;
            uint32_t memoryBase1;
            uint32_t memoryLimit1;
            uint16_t IOBase0_LO;
            uint16_t IOBase0_HI;
            uint16_t IOLimit0_LO;
            uint16_t IOLimit0_HI;
            uint16_t IOBase1_LO;
            uint16_t IOBase1_HI;
            uint16_t IOLimit1_LO;
            uint16_t IOLimit1_HI;
            uint8_t interruptLine;
            uint8_t interruptPin;
            uint16_t bridgeControl;
            uint16_t subVendorID;
            uint16_t subSystemID;
            uint32_t legacyBaseAddress;
            uint8_t reserved3[56];
            uint32_t systemControl;
            uint8_t multiMediaControl;
            uint8_t generalStatus;
            uint8_t reserved4[2];
            uint8_t GPIO0Control;
            uint8_t GPIO1Control;
            uint8_t GPIO2Control;
            uint8_t GPIO3Control;
            uint32_t IRQMuxRouting;
            uint8_t retryStatus;
            uint8_t cardControl;
            uint8_t deviceControl;
            uint8_t diagnostic;
        } type2;

    } u;

    uint8_t deviceSpecific[108];
};

struct k_pci_device_hdr
{
    volatile uint16_t vendorId;
    volatile uint16_t deviceId;
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
} __attribute__((packed));

struct k_pci_header0
{
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
} __attribute__((packed));

struct k_pci_device_entry
{
    PCICommonConfig *device;
    physical_address_t physicalAddress;
    k_pci_device_entry *next;
};

void pciParseBus(physical_address_t base, uint64_t bus);

void pciParseDevice(physical_address_t base, uint64_t device);

void pciParseFunction(virtual_address_t deviceAddr, uint64_t function);

void pciEnumerateDevices(k_mcfg_hdr *mcfgHeader);

PCICommonConfig *pciGetDevice(uint8_t classCode,
                              uint8_t subclassCode,
                              uint8_t progIf);