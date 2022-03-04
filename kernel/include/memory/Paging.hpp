#pragma once

#include <stdint.h>
#include <stivale2.h>
#include <types.hpp>
#include <memory/PhysicalMemoryAllocator.hpp>
#include <strings.hpp>

// Macros to calculate the relevant address parts
#define PT_indexer(add)        (add >> 12) // Get the address of the Page Table
#define PD_indexer(add)        (add >> 21) // Get the address of the Page Directory
#define PDP_indexer(add)       (add >> 30) // Get the address of the Page Directory Pointer
#define PML4_indexer(add)      (add >> 39) // Get the address of the Page Map Level 4

#define PAGE_TABLE_SIZE 512

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

class PageManager {
    public:
        PageManager(PhysicalMemoryAllocator *physicalMemoryAllocator);
        physical_address getPML4();

        void mapPage(linear_address pageAddress, physical_address physicalAddress);

        void freePage(linear_address pageAddr);
        void lockPage(linear_address pageAddr);
        linear_address requestPage();

    private:
        PhysicalMemoryAllocator *_physicalMemoryAllocator;
        PageTable  *_PML4;
};
