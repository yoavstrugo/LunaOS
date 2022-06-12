#include <memory/heap.hpp>

#include <memory/freelist_allocator.hpp>
#include <memory/memory.hpp>
#include <memory/paging.hpp>
#include <kernel.hpp>
#include <logger/logger.hpp>

virtual_address_t heapStart;
virtual_address_t heapEnd;
bool heapInitialized = false;
k_freelist_allocator heapAllocator;
uint64_t heapUsed;

bool heapVerbose = false;

void heapInitialize(virtual_address_t start, virtual_address_t end)
{
    heapAllocator.init(start, end);
    heapStart = start;
    heapEnd = end;

    heapInitialized = true;
    heapUsed = 0;

    // Lock the pages in the heap
    uint64_t pageCount = ((uint64_t)(end - start)) / PAGE_SIZE + 1;
    for (uint64_t page = 0; page < pageCount; page++)
    {
        physical_address_t pAddr = memoryPhysicalAllocator.allocatePage();
        pagingMapPage(start + PAGE_SIZE * page, pAddr);
    }

    #ifdef VERBOSE_HEAP
        logDebugn("%! Heap has been initialized. \
                \n\t- Starting at: 0x%64x, \
                \n\t- Ending At: 0x%64x",
                  "[Kernel Heap]", start, end);
    #endif
}

void *heapAllocate(uint64_t size)
{
    if (size == 0)
        return NULL;

    if (!heapInitialized)
        kernelPanic("%! An attempt to allocate memory with uninitialized heap has occurred.", "[Kernel Heap]");

    // Allocate memory
    void *ptr = heapAllocator.allocate(size);

    // Handle the case where heap has ran out of memory
    if (!ptr)
    {
        if (heapExpand())
        {
            return heapAllocate(size);
        }

        kernelPanic("%! failed to allocate kernel memory.", "[Kernel Heap]");
        return 0;
    }

    #ifdef VERBOSE_HEAP
        logDebugn("%! Successfully allocated %d %s.", "[Kernel Heap]", K_MEMORY_SIZE(size), K_MEMORY_UNIT(size));
    #endif
    heapUsed += size;
    return ptr;
}

bool heapExpand()
{
    if (!heapInitialized)
        kernelPanic("%! An attempt to expand an uninitialized heap has occurred.", "[Kernel Heap]");

    if (heapEnd + K_HEAP_EXPANSION_STEP > K_CONST_HEAP_MAX_EXPANSION)
    {
        logWarnn("%! Kernel heap has ran out of memory", "[Kernel Heap]");
        return false;
    }

    for (virtual_address_t virt = heapStart; virt < heapEnd + K_HEAP_EXPANSION_STEP; virt += PAGE_SIZE)
    {
        physical_address_t phys = memoryPhysicalAllocator.allocatePage();
        if (phys == 0)
        {
            logWarnn("%! Failed to expand heap, out of physical memory.", "[Kernel Heap]");
            return false;
        }

        pagingMapPage(virt, phys);
    }

    heapAllocator.expand(K_HEAP_EXPANSION_STEP);
    heapEnd += K_HEAP_EXPANSION_STEP;

    #ifdef VERBOSE_HEAP
        logDebugn("%! Heap has expanded by %d %s, up to 0x%64x (%d %s in use)",
                  "[Kernel Heap]",
                  K_MEMORY_SIZE(K_HEAP_EXPANSION_STEP),
                  K_MEMORY_UNIT(K_HEAP_EXPANSION_STEP),
                  heapEnd,
                  K_MEMORY_SIZE(heapUsed),
                  K_MEMORY_UNIT(heapUsed));
    #endif
    return true;
}

void heapFree(void *mem)
{
    if (!heapInitialized)
        kernelPanic("%! An attempt to free memory with uninitialized heap has occurred.", "[Kernel Heap]");

    heapUsed -= heapAllocator.free(mem);
}

void *operator new(size_t count)
{
    return heapAllocate(count);
}

void *operator new[](size_t count)
{
    return heapAllocate(count);
}

void operator delete(void *ptr)
{
    heapFree(ptr);
}

void operator delete[](void *ptr)
{
    heapFree(ptr);
}