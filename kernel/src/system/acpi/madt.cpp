#include <system/acpi/madt.hpp>

#include <logger/logger.hpp>

void madtParse(k_acpi_sdt_hdr *madtHeader)
{
    k_madt_hdr *madt = (k_madt_hdr *)madtHeader;

    uint8_t *madtData = (uint8_t *)((uint64_t)madt + sizeof(k_madt_hdr));
    uint32_t madtLength = madtHeader->length - sizeof(k_madt_hdr);

    logDebugn("%! Table Entries:", "[MADT]");
    uint32_t i = 0;
    while (i < madtLength)
    {
        k_madt_entry_header *entryHeader = (k_madt_entry_header *)&madtData[i];

        switch ((enum MADT_ENTRY_TYPE)entryHeader->entryType)
        {
        case P_LAPIC:
            // TODO: handle processor lapic
            {
                k_madt_lapic_entry *entry = (k_madt_lapic_entry *)(entryHeader);
                logDebugn("\t- Found Processor Local APIC, ApicID: %d, ProcessorId: %d.",
                          entry->apicId,
                          entry->processorId);
                break;
            }
        case IOAPIC:
            // TODO: handle ioapic
            {
                k_madt_ioapic_entry *ioentry = (k_madt_ioapic_entry *)(entryHeader);
                logDebugn("\t- Found IO APIC, ApicID: %d, Address: 0x%64x, GSI Base: 0x%64x.",
                          ioentry->ioapicId,
                          ioentry->ioapicAddress,
                          ioentry->globalSystemInterruptBase);
                break;
            }
        default:
            logInfon("\t- Unsupported device type %d.", entryHeader->entryType);
            break;
        }

        i += entryHeader->recordLength;
    }
}