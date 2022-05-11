#include <system/acpi/acpi.hpp>

#include <logger/logger.hpp>
#include <system/acpi/rsdp.hpp>

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