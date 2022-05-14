#include <system/acpi/acpi.hpp>

#include <logger/logger.hpp>
#include <system/acpi/rsdp.hpp>
#include <memory/memory.hpp>
#include <stddef.h>
#include <kernel.hpp>
#include <strings.hpp>

k_acpi_sdt_hdr *xsdt = NULL;
k_acpi_entry *acpiTables = NULL;

void acpiInitialize(stivale2_struct *bootInfo)
{
    k_rsdp_descriptor *rsdp = rsdpGet(bootInfo);
    if (!rsdp)
        kernelPanic("%! Couldn't find RSDP", "[ACPI]");

    pagingDeepDebug = true;
    acpiPrepareXSDT(rsdp);
    pagingDeepDebug = false;

    for (uint32_t i = 0; i < K_XSDT_ENTRIES(xsdt); i++)
    {
        physical_address_t sdtPhysical = acpiGetEntry(i);

        if (!sdtPhysical)
            continue;

        k_acpi_sdt_hdr *sdt = acpiMapSDT(sdtPhysical);
        if (!sdt)
            continue;

        k_acpi_entry *entry = new k_acpi_entry();
        entry->header = sdt;
        entry->next = acpiTables;
        acpiTables = entry;
    }

    logDebugn("%! Initialization completed!", "[ACPI]");
}

uint32_t acpiGetUnmappedSDTLength(physical_address_t sdtAddr)
{
    // map just a single page to read the length of the SDT header
    virtual_address_t temporaryPage = virtualAddressRangeAllocator.allocateRange(1);

    if (!temporaryPage)
    {
        logWarnn("%! Couldn't allocate virtual range of 1 page while calculating the length of a SDT.", "[ACPI]");
        return 0;
    }

    pagingMapPage(temporaryPage, PAGING_ALIGN_PAGE_DOWN(sdtAddr));

    // Now we can access the SDT from the temporary page
    k_acpi_sdt_hdr *sdtHeader = (k_acpi_sdt_hdr *)(temporaryPage + (sdtAddr - PAGING_ALIGN_PAGE_DOWN(sdtAddr)));
    uint32_t sdtLength = sdtHeader->length;

    pagingUnmapPage(temporaryPage);
    virtualAddressRangeAllocator.freeRange(temporaryPage);

    return sdtLength;
}

k_acpi_sdt_hdr *acpiMapSDT(physical_address_t sdtPhysAddr)
{
    if (!sdtPhysAddr)
    {
        logWarnn("%! Tried to map an SDT with a NULL pointer.", "[ACPI]");
        return NULL;
    }

    uint32_t length = acpiGetUnmappedSDTLength(sdtPhysAddr);

    if (length == 0)
    {
        logWarnn("%! Invalid length 0 for a SDT.", "[ACPI]");
        return NULL;
    }

    // First, determane how many pages does the range need
    physical_address_t rangeStart = PAGING_ALIGN_PAGE_DOWN(sdtPhysAddr);
    physical_address_t rangeEnd = PAGING_ALIGN_PAGE_UP(sdtPhysAddr + length);

    uint32_t rangePages = (rangeEnd - rangeStart) / PAGE_SIZE;

    // Allocate the pages on the virtual address ranges
    virtual_address_t sdtVirtualAddr = virtualAddressRangeAllocator.allocateRange(rangePages);

    if (!sdtVirtualAddr)
    {
        logWarnn("%! Couldn't allocate virtual range of size %d pages while mapping a SDT.", "[ACPI]", rangePages);
        return NULL;
    }

    // Map the pages to the physical address
    for (uint64_t page = 0; page < rangePages; page += PAGE_SIZE)
        pagingMapPage(sdtVirtualAddr + page, rangeStart + page);

    // Calculate the offset inside the first page to the start of the header
    uint64_t headerOffset = sdtPhysAddr - rangeStart;

    return (k_acpi_sdt_hdr *)(sdtVirtualAddr + headerOffset);
}

bool acpiValidateSDT(k_acpi_sdt_hdr *sdt)
{
    if (!sdt)
    {
        logWarnn("%! Tried to validate an SDT with a NULL pointer.", "[ACPI]");
        return false;
    }

    uint8_t *acpiSdtHdrBytes = (uint8_t *)sdt;

    uint8_t sum = 0;

    logDebugn("%! Checking SDT header.", "[ACPI]");
    for (uint32_t i = 0; i < sdt->length; i++)
        sum += acpiSdtHdrBytes[i];

    return sum == 0;
}

void acpiPrepareXSDT(k_rsdp_descriptor *rsdp)
{
    physical_address_t rootPhysAddress = NULL;

    if (rsdp->revision > 0)
    {
        // It's the new format, get the address from the bigger table
        k_rsdp_descriptor_20 *rsdp20 = (k_rsdp_descriptor_20 *)rsdp;
        if (rsdp20->xsdtAddress)
        {
            rootPhysAddress = rsdp20->xsdtAddress;
            logDebugn("%! System is using XSDT which is located at 0x%64x (physical).", "[ACPI]", rootPhysAddress);
        }
        else
        {
            kernelPanic("%! ACPI version is above 2.0 but XSDT is not used.", "[ACPI]");
            return;
        }
    }

    xsdt = acpiMapSDT(rootPhysAddress);
    logDebugn("%! XSDT is mapped to virtual address 0x%64x", "[ACPI]", xsdt);
}

physical_address_t acpiGetEntry(uint32_t i)
{
    return ((physical_address_t *)((virtual_address_t)xsdt + sizeof(k_acpi_sdt_hdr)))[i];
}


k_acpi_sdt_hdr *acpiGetEntryWithSignature(const char *signature)
{
    k_acpi_entry *curr = acpiTables;

    while (curr)
    {
        if (!strncmp(curr->header->signature, signature, K_ACPI_SIGNATURE_BYTES))
            return curr->header;

        curr = curr->next;
    }

    return NULL;
}