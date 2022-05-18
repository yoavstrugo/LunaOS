#include <interrupts/ioapic.hpp>

#include <memory/heap.hpp>
#include <memory/memory.hpp>
#include <kernel.hpp>
#include <stddef.h>
#include <types.hpp>
#include <logger/logger.hpp>

k_ioapic_entry *ioapicHead = NULL;

void ioapicAdd(uint8_t id, physical_address_t physcialAddress, uint32_t globalSystemInterruptBase)
{
    k_ioapic_entry *ioapic = new k_ioapic_entry();

    // Create a mapping for the ioapic entry
    virtual_address_t virt = virtualAddressRangeAllocator.allocateRange(2);
    if (!virt)
        kernelPanic("%! Couldn't allocate virtual range for mapping", "[IOAPIC]");

    for (int i = 0; i < 2; i++)
        pagingMapPage(virt + i * PAGE_SIZE, PAGING_ALIGN_PAGE_DOWN(physcialAddress) + i * PAGE_SIZE);

    uint64_t offset = physcialAddress - PAGING_ALIGN_PAGE_DOWN(physcialAddress);

    ioapic->address = virt + offset;

    // Validate the ID and change it if needed
    if (id != ioapicRead(ioapic, IOAPIC_ID))
    {
        logDebugn("%! IOAPIC has wrong ID %d, changing...", "[IOAPIC]", ioapicRead(ioapic, IOAPIC_ID));
        // Change the id
        ioapicWrite(ioapic, IOAPIC_ID, id);
    }

    ioapic->globalSystemInterruptBase = globalSystemInterruptBase;
    ioapic->id = id;

    // Read the maximum redirections
    uint32_t verValue = ioapicRead(ioapic, IOAPIC_VER);
    ioapic->redtblSize = (verValue >> 16) & 0xFF;

    ioapic->next = ioapicHead;
    ioapicHead = ioapic;

    logDebugn("%! Added a new IOAPIC (Virtual Address: 0x%64x) with: \
                \n\t- Id: %d \
                \n\t- GIS Base: %d, \
                \n\t- Redirections: %d",
              "[IOAPIC]", ioapic->address, ioapic->id, ioapic->globalSystemInterruptBase, ioapic->redtblSize);
}

void ioapicWrite(k_ioapic_entry *ioapic, uint32_t reg, uint32_t value)
{
    *((volatile uint32_t *)(ioapic->address + IOAPIC_REGSEL)) = reg;
    *((volatile uint32_t *)(ioapic->address + IOAPIC_REGWIN)) = value;
}

uint32_t ioapicRead(k_ioapic_entry *ioapic, uint32_t reg)
{
    *((volatile uint32_t *)(ioapic->address + IOAPIC_REGSEL)) = reg;
    return *((volatile uint32_t *)(ioapic->address + IOAPIC_REGWIN));
}

k_ioapic_entry *ioapicGetResponsible(uint32_t source)
{
    k_ioapic_entry *curr = ioapicHead;

    while (curr)
    {
        if (source >= curr->globalSystemInterruptBase && source <= (curr->globalSystemInterruptBase + curr->redtblSize))
            return curr;
        curr = curr->next;
    }

    return NULL;
}

void ioapicSetRedirection(k_ioapic_entry *ioapic, uint32_t index, uint64_t value)
{
    ioapicWrite(ioapic, IOAPIC_REDTBL + index * 2, value & 0xFFFFFFFF);
    ioapicWrite(ioapic, IOAPIC_REDTBL + index * 2 + 1, value >> 32);
}

bool ioapicCreateISARedirection(uint32_t source, uint32_t irq, uint32_t lapicId)
{
    k_ioapic_entry *ioapic = ioapicGetResponsible(source);
    if (!ioapic)
    {
        logWarnn("%! Found no responsible I/O APIC for interrupt %d", "[IOAPIC]", source);
        return false;
    }

    uint64_t redirectionTableEntry = 0;
    redirectionTableEntry |= IOAPIC_REDTBL_INTVEC_MAKE(0x20 + irq);
    redirectionTableEntry |= IOAPIC_REDTBL_DELMOD_FIXED;
    redirectionTableEntry |= IOAPIC_REDTBL_DESTMOD_PHYSICAL;
    redirectionTableEntry |= IOAPIC_REDTBL_INTPOL_HIGH_ACTIVE;
    redirectionTableEntry |= IOAPIC_REDTBL_TRIGGERMOD_EDGE;
    redirectionTableEntry |= IOAPIC_REDTBL_INTMASK_UNMASKED;
    redirectionTableEntry |= IOAPIC_REDTBL_DESTINATION_MAKE(lapicId, IOAPIC_REDTBL_DESTINATION_FLAG_PHYSICAL);

    ioapicSetRedirection(ioapic, source, redirectionTableEntry);

    logDebugn("%! Created ISA redirection entry %d -> %d", "[IOAPIC]", source, irq);
    return true;
}