#include <memory/VirtualMemoryManager.hpp>

#include <memory/paging.hpp>

VirtualMemoryManager::VirtualMemoryManager(physical_address_t PML4, PhysicalMemoryManager *PMM) {
    this->_PML4 = (PageTable *)PML4;
    this->_PMM = PMM;
}

/**
 * @brief Get the page table in the parnet page table entry (or create it if does
 *        not exist) with PRESENT and READWRITE flags.
 * 
 * @param parentEntry The parent page table entry
 * @param PMM         The Physical Memory Manager
 * @return PageTable * The physical address of the created page table
 */
PageTable *createOrGetPagetable(PageTableEntry *parentEntry, PhysicalMemoryManager *PMM) {
    PageTable *pageTable;
    if (!parentEntry->present) {
        // PDP is not present, lets create one
        pageTable = (PageTable *)PMM->allocateBlock();
        memset((char *)pageTable, 0, PAGE_SIZE);

        // Modify properties
        parentEntry->address = (physical_address_t)pageTable >> 12;
        parentEntry->present = true;
        parentEntry->readWrite = true;
    } else {
        pageTable = (PageTable *)((physical_address_t)parentEntry->address << 12);
    }

    return pageTable;
}

/**
 * @brief Check whether the page the page table is empty
 * 
 * @param pageTable The page table
 * @return true     If the page table is empty
 * @return false    If the page table is not empty
 */
bool isPagetableEmpty(PageTable *pageTable) {
    for (int entryIdx = 0; entryIdx < PAGE_TABLE_SIZE; entryIdx++)
        if (pageTable->entries[entryIdx].present)
            return false; 
    return true;    
}

// TODO: Add flags parameter
void VirtualMemoryManager::mapPage(virtual_address_t pageAddr, physical_address_t physicAddr) {
    PageTable *PDP, *PD, *PT;
    uint64_t offset;

    // PML4
    offset = PML4_indexer(pageAddr);
    PDP = createOrGetPagetable(&this->_PML4->entries[offset], this->_PMM);

    // PDP
    offset = PDP_indexer(pageAddr);
    PD = createOrGetPagetable(&PDP->entries[offset], this->_PMM);

    // Page Directory
    offset = PD_indexer(pageAddr);
    PT = createOrGetPagetable(&PD->entries[offset], this->_PMM);

    // Page Table
    // TODO: should I lock this slot?
    offset = PT_indexer(pageAddr);
    PageTableEntry *PTe = &PT->entries[offset];
    PTe->address = physicAddr >> 12;
    PTe->present = true;
    PTe->readWrite = true;
}

void VirtualMemoryManager::unmapPage(virtual_address_t pageAddr) {
    PageTableEntry *PML4e = &this->_PML4->entries[PML4_indexer(pageAddr)];
    if (PML4e->present) {
        PageTable *PDP = (PageTable *)(PML4e->address << 12);
        PageTableEntry *PDPe = &PDP->entries[PDP_indexer(pageAddr)];
        if (PDPe->present) {
            PageTable *PD = (PageTable *)(PDPe->address << 12);
            PageTableEntry *PDe = &PD->entries[PD_indexer(pageAddr)];
            if (PDe->present) {
                PageTable *PT = (PageTable *)(PDe->address << 12);
                PageTableEntry *PTe = &PT->entries[PT_indexer(pageAddr)];

                if (PTe->present) {
                    // Remove the page
                    PTe->address = 0x0;
                    PTe->present = false;
                }

                // Check if the table is empty
                if (isPagetableEmpty(PT)) {
                    // It's empty, remove the table
                    this->_PMM->freeBlock((physical_address_t)PT);
                    PDe->present = false;
                }
            }

            // Check if the table is empty
            if (isPagetableEmpty(PD)) {
                this->_PMM->freeBlock((physical_address_t)PD);
                PDPe->present = false;
            }  
        }

        // Check if the table is empty
        if (isPagetableEmpty(PDP)) {
            this->_PMM->freeBlock((physical_address_t)PDP);
            PML4e->present = false;
        }
    }
}

void VirtualMemoryManager::remapPage(virtual_address_t pageAddr, physical_address_t physicAddr) {
    this->unmapPage(pageAddr);
    this->mapPage(pageAddr, physicAddr);
}