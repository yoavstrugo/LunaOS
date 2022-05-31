#include <system/pci/pci.hpp>

#include <memory/paging.hpp>
#include <memory/memory.hpp>
#include <memory/heap.hpp>
#include <logger/logger.hpp>

#include <system/acpi/mcfg.hpp>

#include <system/pci/pci_devices.hpp>

static k_pci_device_entry *devices;
static int PCI_REQUESTS = 0;

PCICommonConfig *pciGetDevice(uint8_t classCode,
                              uint8_t subclassCode,
                              uint8_t progIf)
{
    k_pci_device_entry *curr = devices;

    while (curr)
    {

        if (curr->device->baseClass == classCode &&
            curr->device->subClass == subclassCode &&
            curr->device->progIf == progIf)
            return curr->device;

        curr = curr->next;
    }

    return NULL;
}

void pciEnumerateDevices(k_mcfg_hdr *mcfgHeader)
{
    int entryCount = ((mcfgHeader->header.length) - sizeof(k_mcfg_hdr)) / sizeof(k_mcfg_entry);

    for (int i = 0; i < entryCount; i++)
    {
        k_mcfg_entry *entry = (k_mcfg_entry *)((uint64_t)mcfgHeader + sizeof(k_mcfg_hdr) + i * sizeof(k_mcfg_entry));

        physical_address_t physicalBase = entry->baseAddress;
        virtual_address_t virtualBase = virtualAddressRangeAllocator.allocateRange(256 * 32 * 8, "pci\0");

        for (int offset = 0; offset < (256 * 32 * 8) * PAGE_SIZE; offset += PAGE_SIZE)
            pagingMapPage(virtualBase + offset, physicalBase + offset);

        for (uint8_t bus = entry->startBusNumber; i < entry->endBusNumber; i++)
        {
            uint64_t offset = (bus - entry->startBusNumber) << 20;

            virtual_address_t virtAddr = virtualBase + offset;

            // Now virtAddr should point to the configuration space
            PCICommonConfig *commonConfig = (PCICommonConfig *)virtAddr;
            if (commonConfig->deviceID == 0xFFFF ||
                commonConfig->deviceID == 0x0000)
                continue;

            // 32 devices per bus
            for (uint8_t device = 0; device < 32; device++)
            {
                offset =
                    (((bus - entry->startBusNumber) << 20) | (device << 15));

                virtAddr = virtualBase + offset;

                // Now virtAddr should point to the configuration space
                commonConfig = (PCICommonConfig *)virtAddr;
                if (commonConfig->deviceID == 0xFFFF ||
                    commonConfig->deviceID == 0x0000)
                    continue;

                // 8 functions per device
                for (uint8_t function = 0; function < 8; function++)
                {
                    uint64_t offset =
                        (((bus - entry->startBusNumber) << 20) | (device << 15) | (function << 12));

                    virtual_address_t virtAddr = virtualBase + offset;

                    // Now virtAddr should point to the configuration space
                    PCICommonConfig *commonConfig = (PCICommonConfig *)virtAddr;
                    if (commonConfig->deviceID == 0xFFFF ||
                        commonConfig->deviceID == 0x0000)
                        continue;

                    k_pci_device_entry *device = new k_pci_device_entry();
                    device->physicalAddress = physicalBase + offset;
                    device->next = devices;
                    devices = device;
                }
            }
        }

        virtualAddressRangeAllocator.freeRange(virtualBase);
        for (int offset = 0; offset < (256 * 32 * 8) * PAGE_SIZE; offset += PAGE_SIZE)
            pagingUnmapPage(virtualBase + offset);
    }

    k_pci_device_entry *device = devices;
    while (device)
    {
        device->device = (PCICommonConfig *)virtualAddressRangeAllocator.allocateRange(1, "pci\0");
        pagingMapPage((virtual_address_t)device->device, device->physicalAddress);

        device = device->next;
    }
}