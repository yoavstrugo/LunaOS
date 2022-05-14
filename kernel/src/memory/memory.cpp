#include <memory/memory.hpp>

#include <memory/paging.hpp>
#include <kernel.hpp>
#include <stivale2/stivale2_tools.hpp>
#include <memory/heap.hpp>
#include <logger/logger.hpp>

BitmapAllocator memoryPhysicalAllocator;
k_virtual_address_range_allocator virtualAddressRangeAllocator;

void memoryInitialize(stivale2_struct *stivale2Info)
{
    stivale2_struct_tag_memmap *memMap = (stivale2_struct_tag_memmap *)
        stivale2_get_tag(stivale2Info, STIVALE2_STRUCT_TAG_MEMMAP_ID);

    if (!memMap)
        kernelPanic("%! Couldn't find Memory Map tag!", "[Memory]");

    memoryPhysicalAllocator.initialize(memMap);

    stivale2_struct_tag_hhdm *hhdm = (stivale2_struct_tag_hhdm *)
        stivale2_get_tag(stivale2Info, STIVALE2_STRUCT_TAG_HHDM_ID);

    if (!hhdm)
        kernelPanic("%! Couldn't find HHDM tag!", "[Memory]");

    logDebugn("%! HHDM Address: 0x%64x","[Memory]", hhdm->addr);

    stivale2_struct_tag_kernel_base_address *kernelBase = (stivale2_struct_tag_kernel_base_address *)
        stivale2_get_tag(stivale2Info, STIVALE2_STRUCT_TAG_KERNEL_BASE_ADDRESS_ID);

    if (!kernelBase)
        kernelPanic("%! Couldn't find Kernel Base tag!", "[Memory]");

    logDebugn("%! Kernel Loaded \ 
            \n\t- Kernel Physical Base: 0x%64x \
            \n\t- Kernel Virtual Base:  0x%64x ",
              "[Memory]",
              kernelBase->physical_base_address, kernelBase->virtual_base_address);

    pagingDeepDebug = false;
    // Initialize paging
    pagingInitialize(kernelBase->physical_base_address, hhdm->addr);

    // Map the heap's memory
    for (virtual_address_t virt = K_CONST_HEAP_DEFAULT_START; virt < K_CONST_HEAP_DEFAULT_END; virt += PAGE_SIZE)
    {
        physical_address_t phys = memoryPhysicalAllocator.allocatePage();
        pagingMapPage(virt, phys);   
    }

    // Initialize the kernel's heap
    heapInitialize(K_CONST_HEAP_DEFAULT_START, K_CONST_HEAP_DEFAULT_END);

    // Initialize the kernel's virtual address ranges allocator
    virtualAddressRangeAllocator.addRange(K_CONST_KERNEL_VIRTUAL_RANGES_START, K_CONST_KERNEL_VIRTUAL_RANGES_END);

    logDebugn("%! Kernel memory has been initialized successfully.", "[Memory]");
}
