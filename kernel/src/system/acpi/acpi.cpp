#include <system/acpi/acpi.hpp>

#include <logger/logger.hpp>
#include <system/acpi/rsdp.hpp>
#include <memory/memory.hpp>

void acpiInitialize(stivale2_struct *bootInfo) {
     rsdpGet(bootInfo);
}

bool acpiValidateHeader(k_acpi_sdt_hdr *acpiSdtHdr) {
    uint8_t *acpiSdtHdrBytes = (uint8_t *)acpiSdtHdr;

    uint8_t sum = 0;

    logDebugn("%! Checking SDT header.", "ACPI");
    for (uint32_t i = 0; i < acpiSdtHdr->length; i++)
        sum += acpiSdtHdrBytes[i];
    

    return sum == 0;    
}

k_acpi_sdt_hdr *acpiMapSDT(physical_address_t sdtAddr) {
    // first map just a single page to read the length of the SDT header
    virtual_address_t temporaryPage = virtualAddressRangeAllocator.allocateRange(1);
    pagingMapPage(temporaryPage, PAGING_ALIGN_PAGE_DOWN(sdtAddr));

    // Now we can access the SDT from the temporary page
    k_acpi_sdt_hdr *sdtHeader = (k_acpi_sdt_hdr *)(temporaryPage + (sdtAddr - PAGING_ALIGN_PAGE_DOWN(sdtAddr)));
    uint32_t sdtLength = sdtHeader->length;

    pagingUnmapPage(temporaryPage);
    virtualAddressRangeAllocator.freeRange(temporaryPage);

    
}