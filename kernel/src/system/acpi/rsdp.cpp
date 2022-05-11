#include <system/acpi/rsdp.hpp>

#include <logger/logger.hpp>
#include <stivale2/stivale2_tools.hpp>
#include <stddef.h>

bool rsdpValidate(k_rsdp_descriptor *rsdp) {
    uint8_t *rsdpBytes = (uint8_t *)rsdp;
    uint8_t sum = 0;

    logDebugn("%! Checking first part.", "RSDP");
    uint16_t i;
    for (i = 0; i < sizeof(k_rsdp_descriptor); i++)
        sum += rsdpBytes[i];
    
    if (sum != 0)
        return false;

    logDebugn("%! First part checks up!", "RSDP");

    // Check if its above version 2.0
    if (rsdp->revision > 0) {
        logDebugn("%! version above 2.0, checking second part.", "RSDP");
        for (; i < sizeof(k_rsdp_descriptor_20); i++)
            sum += rsdpBytes[i];
        
        if (sum != 0)
            return false;
        
        logDebugn("%! Second part checks up as well!", "RSDP");
    }
    
    return true;
}

k_rsdp_descriptor *rsdpGet(stivale2_struct *bootInfo) {
    // Get the struct from sitvale
    stivale2_struct_tag_rsdp *rsdpStruct = 
        (stivale2_struct_tag_rsdp *)stivale2_get_tag(bootInfo, STIVALE2_STRUCT_TAG_RSDP_ID);

    if (!rsdpStruct) return NULL;

    // Validate the RSDP
    k_rsdp_descriptor *rsdp = (k_rsdp_descriptor *)rsdpStruct->rsdp;
    if (rsdpValidate(rsdp)) return rsdp;

    return NULL;
}