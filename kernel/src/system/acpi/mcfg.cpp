#include <system/acpi/mcfg.hpp>

#include <system/pci/pci.hpp>
#include <memory/memory.hpp>
#include <logger/logger.hpp>

// void mcfgParse(k_acpi_sdt_hdr *sdtHeader)
// {
//     k_mcfg_hdr *mcfgHeader = (k_mcfg_hdr *)sdtHeader;

//     int entryCount = ((mcfgHeader->header.length) - sizeof(k_mcfg_hdr)) / sizeof(k_mcfg_entry);

//     for (int i = 0; i < entryCount; i++)
//     {
//         k_mcfg_entry *entry = (k_mcfg_entry *)((uint64_t)mcfgHeader + sizeof(k_mcfg_hdr) + i * sizeof(k_mcfg_entry));
//         for (uint8_t bus = entry->startBusNumber; bus < entry->endBusNumber; bus++)
//         {
//             pciParseBus(entry->baseAddress, bus - entry->startBusNumber);
//         }
//     }

//     logInfon("%! MCFG has been parsed", "[MCFG]");
// }