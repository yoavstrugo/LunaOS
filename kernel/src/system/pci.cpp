#include <system/pci/pci.hpp>

#include <memory/paging.hpp>
#include <memory/memory.hpp>
#include <memory/heap.hpp>
#include <logger/logger.hpp>

#include <system/acpi/mcfg.hpp>

#include <system/pci/pci_devices.hpp>

static k_pci_device_entry *devices;

void pciParseBus(physical_address_t base, uint64_t bus)
{
    uint64_t offset = bus << 20;

    physical_address_t addr = base + offset;
    physical_address_t physBusAddr = PAGING_ALIGN_PAGE_DOWN(base + offset);
    virtual_address_t busAddrAligned = virtualAddressRangeAllocator.allocateRange(1);
    pagingMapPage(busAddrAligned, physBusAddr);

    virtual_address_t busAddr = busAddrAligned + ((base + offset) - physBusAddr);

    k_pci_device_hdr *deviceHeader = (k_pci_device_hdr *)busAddr;

    if (deviceHeader->deviceId == 0)
        return; // Invalid device
    if (deviceHeader->deviceId == 0xFFFF)
        return;

    pagingUnmapPage(busAddrAligned);

    // Each bus has up to 32 devices
    for (uint64_t device = 0; device < 32; device++)
    {
        pciParseDevice(addr, device);
    }
}

void pciParseDevice(physical_address_t bus, uint64_t device)
{
    uint64_t offset = device << 15;

    physical_address_t addr = bus + offset;
    physical_address_t physDriveAddr = PAGING_ALIGN_PAGE_DOWN(addr);
    virtual_address_t deviceAddrAligned = virtualAddressRangeAllocator.allocateRange(1);
    pagingMapPage(deviceAddrAligned, physDriveAddr);

    virtual_address_t deviceAddr = deviceAddrAligned + ((bus + offset) - physDriveAddr);

    k_pci_device_hdr *deviceHeader = (k_pci_device_hdr *)deviceAddr;

    if (deviceHeader->deviceId == 0)
        return; // Invalid device
    if (deviceHeader->deviceId == 0xFFFF)
        return;

    pagingUnmapPage(deviceAddrAligned);

    // Each device has 8 functions
    for (uint64_t function = 0; function < 8; function++)
    {
        pciParseFunction(bus + offset, function);
    }
}

void pciParseFunction(virtual_address_t deviceAddr, uint64_t function)
{
    uint64_t offset = function << 12;

    physical_address_t physFuncAddr = PAGING_ALIGN_PAGE_DOWN(deviceAddr + offset);
    virtual_address_t funcAddrAligned = virtualAddressRangeAllocator.allocateRange(1);
    pagingMapPage(funcAddrAligned, physFuncAddr);

    virtual_address_t funcAddr = funcAddrAligned + ((deviceAddr + offset) - physFuncAddr);

    k_pci_device_hdr *deviceHeader = (k_pci_device_hdr *)funcAddrAligned;

    if (deviceHeader->deviceId == 0)
        return; // Invalid device
    if (deviceHeader->deviceId == 0xFFFF)
        return;

    // Add it to the list
    k_pci_device_entry *deviceEntry = new k_pci_device_entry();
    deviceEntry->device = deviceHeader;
    deviceEntry->next = devices;
    devices = deviceEntry;

    // Print
    k_pci_class_mapping device = pciGetMapping(deviceHeader->mainClass, deviceHeader->subclass, deviceHeader->progIf);
    logDebugn("%! Device: %s/%s", "[PCI]", device.className, device.subclassName);
}

k_pci_device_hdr *pciGetDevice(uint8_t classCode,
                               uint8_t subclassCode,
                               uint8_t progIf)
{
    k_pci_device_entry *curr = devices;

    while (curr)
    {

        if (curr->device->mainClass == classCode &&
            curr->device->subclass == subclassCode &&
            curr->device->progIf == progIf)
            return curr->device;

        curr = curr->next;
    }

    return NULL;
}