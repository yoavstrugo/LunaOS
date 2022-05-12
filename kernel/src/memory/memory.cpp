#include <memory/memory.hpp>

#include <memory/paging.hpp>
#include <kernel.hpp>
#include <stivale2/stivale2_tools.hpp>
#include <memory/heap.hpp>

BitmapAllocator memoryPhysicalAllocator;
k_virtual_address_range_allocator virtualAddressRangeAllocator;

void memoryInitialize(stivale2_struct *stivale2Info) {
    stivale2_struct_tag_memmap *memMap = (stivale2_struct_tag_memmap *)
                                            stivale2_get_tag(stivale2Info, STIVALE2_STRUCT_TAG_MEMMAP_ID);
    if (memMap == NULL) kernelPanic("Memory map was not found!");
    memoryPhysicalAllocator.initialize(memMap);

    stivale2_struct_tag_hhdm *hhdm = (stivale2_struct_tag_hhdm *)
                                        stivale2_get_tag(stivale2Info, STIVALE2_STRUCT_TAG_HHDM_ID);
    
    stivale2_struct_tag_kernel_base_address *kernelBase 
        = (stivale2_struct_tag_kernel_base_address *)  
            stivale2_get_tag(stivale2Info, STIVALE2_STRUCT_TAG_KERNEL_BASE_ADDRESS_ID);

    // Initialize paging
    pagingInitialize(kernelBase->physical_base_address, hhdm->addr);

    // Initialize the kernel's heap
    heapInitialize(K_CONST_HEAP_DEFAULT_START, K_CONST_HEAP_DEFAULT_END);

    // Intialize the kernel's virtual address ranges allocator
    virtualAddressRangeAllocator.addRange(K_CONST_KERNEL_VIRTUAL_RANGES_START, K_CONST_KERNEL_VIRTUAL_RANGES_END);
}
