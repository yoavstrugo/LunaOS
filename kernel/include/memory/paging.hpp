#pragma once

#include <types.hpp>

typedef uint64_t    pagetable_entry_t;

typedef uint64_t    pagetable_flags_t;
typedef uint64_t    page_flags_t;

#define PAGETABLE_SIZE 512
#define PAGE_SIZE       4096

const uint64_t HHDM  =  0xffff800000000000;

// MACROS
#define ADDRESS_EXCLUDE(add)    (add & 0xffffffffffff000)
#define OFFSET_EXCLUDE(add)     (add & 0xfff)
#define PT_INDEXER(add)         ((add >> 12) & 0x01ff) 
#define PD_INDEXER(add)         ((add >> 21) & 0x01ff) 
#define PDPT_INDEXER(add)       ((add >> 30) & 0x01ff) 
#define PML4_INDEXER(add)       ((add >> 39) & 0x01ff)

// TODO: merge flags
// FLAGS FOR A PAGE TABLE

// Present; must be 1 to reference another page table.
const uint64_t PAGETABLE_PRESENT      = 1;
// Read/write; if 0, writes may not be allowed to the memory region controlled by this entry.  
const uint64_t PAGETABLE_READWRITE    = 1 << 1;
// User/supervisor; if 0, user-mode accesses are not allowed to the memory region controlled by this entry.
const uint64_t PAGETABLE_USERSUPER    = 1 << 2;
// Page-level write-through; indirectly determines the memory type used to access the pagetable referenced by this entry.
const uint64_t PAGETABLE_WRITETHROUGH = 1 << 3;
// Page-level cache disable; indirectly determines the memory type used to access the pagetable referenced by this entry.
const uint64_t PAGETABLE_CACHE        = 1 << 4;
// Accessed; indicates whether this entry has been used for linear-address translation
const uint64_t PAGETABLE_ACCESSED     = 1 << 5;
const uint64_t PAGETABLE_PAGE_SIZE    = 1 << 7;
// For ordinary paging, ignored; for HLAT paging, restart (if 1, linear-address translation is restarted with ordinary paging)
const uint64_t PAGETABLE_ORDINARY     = (uint64_t)1 << 11;
// Execution disabled; if 1, instruction fetches are not allowed from the memory region controlled by this entry.
const uint64_t PAGETABLE_EXEC         = (uint64_t)1 << 63;

// FLAGS FOR A PAGE   
const uint64_t PAGE_PRESENT      = 1;  
const uint64_t PAGE_READWRITE    = 1 << 1;
const uint64_t PAGE_USERSUPER    = 1 << 2;
const uint64_t PAGE_WRITETHROUGH = 1 << 3;
const uint64_t PAGE_CACHE        = 1 << 4;
const uint64_t PAGE_ACCESSED     = 1 << 5;
const uint64_t PAGE_DIRTY        = 1 << 6;
const uint64_t PAGE_PAT          = 1 << 7;
const uint64_t PAGE_GLOBAL       = (uint64_t)1 << 8;
const uint64_t PAGE_ORDINARY     = (uint64_t)1 << 11;
const uint64_t PAGE_EXEC         = (uint64_t)1 << 63;

struct k_paging_flags {
    pagetable_flags_t   pml4Flags;
    pagetable_entry_t   pdptFlags;
    pagetable_flags_t   pdFlags;
    page_flags_t        ptFlags;
};

#define PAGING_PAGETABLE_DEFAULT_FLAGS  (PAGETABLE_PRESENT | PAGETABLE_READWRITE)
#define PAGING_PAGE_DEFAULT_FLAGS       (PAGE_PRESENT | PAGE_READWRITE)

const k_paging_flags PAGING_DEFAULT_FLAGS = {
    .pml4Flags  = PAGING_PAGETABLE_DEFAULT_FLAGS,
    .pdptFlags  = PAGING_PAGETABLE_DEFAULT_FLAGS,
    .pdFlags    = PAGING_PAGE_DEFAULT_FLAGS,
    .ptFlags    = PAGING_PAGE_DEFAULT_FLAGS
};


// TODO: add user space presets 

// FLAG MACROS
#define SET_FLAG(addr, flag)    (addr |= flag)
#define UNSET_FLAG(addr, flag)  (addr &= ~flag)
#define CHECK_FLAG(addr, flag)  (addr & flag)


/**
 * @brief Map the the virtual address to the physical address in the current space.
 * 
 * @param virt  The virtual address 
 * @param phys  The physical address
 * @param pml4eFlags    Flags for the Page Map Level 4 entry.
 * @param pdpteFlags    Flags for the Page Directory Pointer Table entry.
 * @param pdeFlags      Flags for the Page Directory entry.
 * @param pteFlags      Flage for the Page Table entry.
 * @param override Whether or not to override if the virtual address is already mapped.
 */
void pagingMapPage  ( 
                    virtual_address_t virt, physical_address_t phys, 
                    k_paging_flags flags = PAGING_DEFAULT_FLAGS,
                    bool override = false
                    );

/**
 * @brief Unmap the virtual address if it is mapped, otherwise does nothing.
 * 
 * @param virt  The virtual address.
 */
void pagingUnmapPage(virtual_address_t virt);

/**
 * @brief Initialize paging with some mappings.
 * 
 */
void pagingInitialize(physical_address_t kernelBase, virtual_address_t hhdm);

/**
 * @brief Switch the address space, by replacing the PML4 with another one.
 * 
 * @param phys  The physical address to the new PML4.
 */
void pagingSwitchSpace(physical_address_t phys);

/**
 * @brief Get the physical address of the current PML4.
 * 
 * @return physical_address_t The physical address of the PML4.
 */
physical_address_t pagingGetCurrentSpace();

/**
 * @brief Find the physical address that the virtual address is mapped to in the current space.
 * 
 * @param virt  The virtual address.
 * @return physical_address_t   The physical address mapped by the virtual address, NULL if it's not mapped
 */
physical_address_t pagingVirtualToPhysical(virtual_address_t virt);