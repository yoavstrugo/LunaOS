#pragma once

#include <types.hpp>
#include <memory/PhysicalMemoryManager.hpp>
#include <strings.hpp>
#include <constants.hpp>

#define PAGE_TABLE_SIZE 512

// Macros to calculate the relevant address parts
#define PT_indexer(add)        (add >> 12) & 0x01ff // Get the address of the Page Table
#define PD_indexer(add)        (add >> 21) & 0x01ff // Get the address of the Page Directory
#define PDP_indexer(add)       (add >> 30) & 0x01ff // Get the address of the Page Directory Pointer
#define PML4_indexer(add)      (add >> 39) & 0x01ff // Get the address of the Page Map Level 4

struct PageTableEntry {
    bool present:       1;
    bool readWrite:     1;
    bool userSuper:     1;
    bool writeThrough:  1;
    bool cacheDisable:  1;
    bool accessed:      1;
    bool dirty:         1;
    bool pat:           1;
    bool global:        1;
    uint8_t _ignored0:  2;
    bool ordinary:      1;
    uint64_t address:   36;
    uint8_t _reserved0: 4;
    uint8_t protKey:    4;  
    bool execDisable:   1;  
};

struct PageTable {
    PageTableEntry entries[PAGE_TABLE_SIZE];
}__attribute__((aligned(0x1000)));

class VirtualMemoryManager {
    public:

        /**
         * @brief       Construct a new Virtual Memory Manager object
         * 
         * @param PML4  The address to the PML4 
         * @param PMM   The Physical Memory Manager
         */
        VirtualMemoryManager(physical_address PML4, PhysicalMemoryManager *PMM);

        /**
         * @brief               Map the page to the physical address
         * 
         * @param pageAddr      The page address 
         * @param physicAddr    The physical adress
         */
        void mapPage(linear_address pageAddr, physical_address physicAddr);

        /**
         * @brief               Unmap the page
         * 
         * @param pageAddr      The page address 
         */
        void unmapPage(linear_address pageAddr);

        /**
         * @brief               Remap the page to the physical address
         * 
         * @param pageAddr      The page address 
         * @param physicAddr    The physical adress
         */
        void remapPage(linear_address pageAddr, physical_address physicAddr);

    private:
        PageTable *_PML4;
        PhysicalMemoryManager *_PMM;
};