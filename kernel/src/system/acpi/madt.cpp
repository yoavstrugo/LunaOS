#include <system/acpi/madt.hpp>

#include <logger/logger.hpp>
#include <interrupts/lapic.hpp>
#include <interrupts/ioapic.hpp>
#include <stddef.h>
#include <memory/heap.hpp>

k_madt_entry_header_node<k_madt_lapic_entry> *lapicList = NULL;
k_madt_entry_header_node<k_madt_ioapic_entry> *ioapicList = NULL;
k_madt_entry_header_node<k_madt_ioapic_interrupt_src_override_entry> *ioapicInterruptSrcOverrideList = NULL;

void madtParse(k_acpi_sdt_hdr *madtHeader)
{
    k_madt_hdr *madt = (k_madt_hdr *)madtHeader;

    physical_address_t lapicAddress = madt->localAPICAddr;

    uint8_t *madtData = (uint8_t *)((uint64_t)madt + sizeof(k_madt_hdr));
    uint32_t madtLength = madtHeader->length - sizeof(k_madt_hdr);

    // Parse the table
    logDebugn("%! Table Entries:", "[MADT]");

    #ifdef K_LOG_LEVEL_DEBUG
    heapVerbose = false;
    #endif

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

                // Add it to the linked list
                k_madt_entry_header_node<k_madt_lapic_entry> *listEntry = new k_madt_entry_header_node<k_madt_lapic_entry>();
                listEntry->next = lapicList;
                listEntry->header = entry;
                lapicList = listEntry;

                break;
            }
        case IOAPIC:
            // TODO: handle ioapic
            {
                k_madt_ioapic_entry *entry = (k_madt_ioapic_entry *)(entryHeader);
                logDebugn("\t- Found IO APIC, ApicID: %d, Address: 0x%64x, GSI Base: 0x%64x.",
                          entry->ioapicId,
                          entry->ioapicAddress,
                          entry->globalSystemInterruptBase);

                // Add it to the linked list
                k_madt_entry_header_node<k_madt_ioapic_entry> *listEntry = new k_madt_entry_header_node<k_madt_ioapic_entry>();
                listEntry->next = ioapicList;
                listEntry->header = entry;
                ioapicList = listEntry;

                break;
            }
        case LAPIC_AO:
        {
            k_madt_lapic_address_override_entry *entry = (k_madt_lapic_address_override_entry *)(entryHeader);
            logDebugn("\t- Found Local APIC Address Override, overriding lapic address to 0x%64x", entry->localAPICAddr);
            break;
        }
        case IOAPIC_ISO:
        {
            k_madt_ioapic_interrupt_src_override_entry *entry = (k_madt_ioapic_interrupt_src_override_entry *)entryHeader;
            logDebugn("\t- Found an IO/APIC Interrupt Source Override entry, \
                            \n\t\t* Bus Source: %d \
                            \n\t\t* IRQ Source: %d \
                            \n\t\t* Global System Interrupt: %d",
                      entry->busSource, entry->irqSource, entry->globalSystemInterrupt);

            // Add it to the linked list
            k_madt_entry_header_node<k_madt_ioapic_interrupt_src_override_entry> *listEntry =
                new k_madt_entry_header_node<k_madt_ioapic_interrupt_src_override_entry>();
            listEntry->next = ioapicInterruptSrcOverrideList;
            listEntry->header = entry;
            ioapicInterruptSrcOverrideList = listEntry;

            break;
        }
        default:
            logInfon("\t- Unsupported device type %d.", entryHeader->entryType);
            break;
        }

        i += entryHeader->recordLength;
    }

    #ifdef K_LOG_LEVEL_DEBUG
    heapVerbose = true;
    #endif

    // Handle the entries
    // LAPIC
    k_madt_entry_header_node<k_madt_lapic_entry> *lapicCurr = lapicList;
    while (lapicCurr)
    {
        // TODO: handle it
        lapicCurr = lapicCurr->next;
    }

    // IOAPIC
    k_madt_entry_header_node<k_madt_ioapic_entry> *ioapicCurr = ioapicList;
    while (ioapicCurr)
    {
        ioapicAdd(ioapicCurr->header->ioapicId, ioapicCurr->header->ioapicAddress, ioapicCurr->header->globalSystemInterruptBase);
        ioapicCurr = ioapicCurr->next;
    }

    // IOAPIC Interrupt Source Override
    k_madt_entry_header_node<k_madt_ioapic_interrupt_src_override_entry> *ioapicInterruptSrcOverrideCurr =
        ioapicInterruptSrcOverrideList;
    while (ioapicInterruptSrcOverrideCurr)
    {
        // TODO: handle it
        ioapicInterruptSrcOverrideCurr = ioapicInterruptSrcOverrideCurr->next;
    }

    lapicPrepare(lapicAddress);
}