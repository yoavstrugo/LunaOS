#include <memory/userspace_allocator.hpp>

#include <stddef.h>
#include <memory/heap.hpp>
#include <memory/paging.hpp>
#include <memory/memory.hpp>
#include <kernel.hpp>
#include <logger/logger.hpp>

k_userspace_allocator::k_userspace_allocator()
{
    this->userspaceCodeStart = NULL;
    this->userspaceHeapStart = NULL;
    this->threadsStackAllocator = new k_virtual_address_range_allocator();
    this->threadsStackAllocator->addRange(USERSPACE_MEMORY_START + 0x1000, USERSPACE_STACK_MAX);

    // Map the memory for the process
    this->pml4Physical = memoryPhysicalAllocator.allocatePage();

    // Copy the kernel mappings
    pagingCopyKernelMappings(this->pml4Physical);
    #ifdef VERBOSE_USERSPACEALLOCATOR
    logDebugn("%! Mapped kernel to process' space.", "[Userspace Allocator]");
    #endif
}

physical_address_t k_userspace_allocator::getSpace()
{
    return this->pml4Physical;
}

void k_userspace_allocator::allocateUserspaceCode(uint64_t userspaceCodeSize)
{
    // Calculate the starting page for the userspace code
    this->userspaceCodeStart = PAGING_ALIGN_PAGE_DOWN(USERSPACE_MEMORY_END - userspaceCodeSize);

    // Allocate the space physically
    uint64_t pages = (USERSPACE_MEMORY_END - this->userspaceCodeStart) / PAGE_SIZE;
    for (uint64_t page = 0; page < pages; page++)
    {
        physical_address_t phys = memoryPhysicalAllocator.allocatePage();
        virtual_address_t virt = this->userspaceCodeStart + PAGE_SIZE * page;

        // Map the page
        pagingMapPageInSpace(virt, phys, this->pml4Physical, USERSPACE_DEFAULT_PAGING_FLAGS);
    }
    #ifdef VERBOSE_USERSPACEALLOCATOR
    logDebugn("%! Mapped code area from 0x%64x with size %m.", "[Userspace Allocator]", this->userspaceCodeStart, userspaceCodeSize);
    #endif
}

void k_userspace_allocator::allocateUserspaceHeap()
{
    if (!this->userspaceCodeStart)
        kernelPanic("%! Cannot allocate heap before code was allocated.", "[Userspace Allocator]");

    uint64_t heapEnd = this->userspaceCodeStart;
    uint64_t heapStart = PAGING_ALIGN_PAGE_DOWN(heapEnd - USERSPACE_HEAP_INITIAL_SIZE);

    for (uint64_t virt = heapStart; virt < heapEnd; virt += PAGE_SIZE)
    {
        physical_address_t phys = memoryPhysicalAllocator.allocatePage();
        pagingMapPageInSpace(virt, phys, this->pml4Physical, USERSPACE_DEFAULT_PAGING_FLAGS);
    }

    #ifdef VERBOSE_USERSPACEALLOCATOR
    logDebugn("%! Allocated heap at 0x%64x-0x%64x.", "[Userspace Allocator]", heapStart, heapEnd);
    #endif
}

void k_userspace_allocator::expandUserspaceHeap()
{
    if (this->userspaceHeapStart - USERSPACE_HEAP_EXPANSION < USERSPACE_STACK_MAX)
    {
        logWarnn("%! Cannot grow heap any larger.", "[Userspace Allocator]");
        // TODO: do something about this
        return;
    }

    virtual_address_t oldStart = this->userspaceHeapStart;
    this->userspaceHeapStart -= USERSPACE_HEAP_EXPANSION;

    // Allocate physical memory and map the pages
    for (uint64_t virt = this->userspaceHeapStart; virt < oldStart; virt += PAGE_SIZE)
    {
        physical_address_t phys = memoryPhysicalAllocator.allocatePage();
        pagingMapPageInSpace(virt, phys, this->pml4Physical, USERSPACE_DEFAULT_PAGING_FLAGS);
    }

    #ifdef VERBOSE_USERSPACEALLOCATOR
    logDebugn("%! Heap expanded now at starts at 0x%64x (instead of 0x%64x)", "[Userspace Allocator]", this->userspaceHeapStart, oldStart);
    #endif
}

void k_userspace_allocator::free()
{

    #ifdef VERBOSE_USERSPACEALLOCATOR
    logDebugn("%! Freeing process' memory", "[Userspace Allocator]");
    #endif

    // Free userspace code
    for (uint64_t virt = this->getUserspaceCodeStart(); virt < this->getUserspaceCodeEnd(); virt += PAGE_SIZE)
    {
        physical_address_t phys = pagingUnmapPageInSpace(virt, this->pml4Physical);
        memoryPhysicalAllocator.freePage(phys);
    }

    #ifdef VERBOSE_USERSPACEALLOCATOR
    logDebugn("\t- Code has been freed");
    #endif

    // Free userspace heap
    for (uint64_t virt = this->getUserspaceHeapStart(); virt < this->getUserspaceHeapEnd(); virt += PAGE_SIZE)
    {
        physical_address_t phys = pagingUnmapPageInSpace(virt, this->pml4Physical);
        memoryPhysicalAllocator.freePage(phys);
    }

    #ifdef VERBOSE_USERSPACEALLOCATOR
    logDebugn("\t- Heap has been freed");
    #endif

    // Free userspace stacks
    k_address_range_header *range = this->threadsStackAllocator->getRanges();
    while (range)
    {
        if (range->used)
        {
            virtual_address_t virt = range->base;
            uint64_t pages = range->pages;

            // Unmap and free each page
            for (uint64_t page = 0; page < pages; page++)
            {
                physical_address_t phys = pagingUnmapPageInSpace(virt + PAGE_SIZE * page, this->pml4Physical);
                memoryPhysicalAllocator.freePage(phys);
            }
        }
        range = range->next;
    }

    #ifdef VERBOSE_USERSPACEALLOCATOR
    logDebugn("\t- Userspace stacks has been freed");
    #endif

    // Free kernelspace stacks and interrupt stacks
    range = this->kernelspaceRanges;
    while (range)
    {
        if (range->used)
        {
            virtual_address_t virt = range->base;
            uint64_t pages = range->pages;

            // Unmap and free each page
            for (uint64_t page = 0; page < pages; page++)
            {
                physical_address_t phys = pagingUnmapPageInSpace(virt + PAGE_SIZE * page, this->pml4Physical);
                memoryPhysicalAllocator.freePage(phys);
            }
        }
        range = range->next;
    }

    #ifdef VERBOSE_USERSPACEALLOCATOR
    logDebugn("\t- Kernelspace stacks and interrupt stacks has been freed");
    #endif
}

virtual_address_t k_userspace_allocator::allocateStack(uint64_t stackSize, bool kernelStack)
{
    uint64_t pages = PAGING_ALIGN_PAGE_UP(stackSize) / PAGE_SIZE;

    virtual_address_t stackPtr;
    k_paging_flags flags;
    if (!kernelStack)
    {
        stackPtr = this->threadsStackAllocator->allocateRange(pages);
        flags = USERSPACE_DEFAULT_PAGING_FLAGS;
    }
    else
    {
        stackPtr = virtualAddressRangeAllocator.allocateRange(pages);
        k_address_range_header *range = new k_address_range_header();
        range->base = stackPtr;
        range->pages = pages;
        range->next = this->kernelspaceRanges;
        this->kernelspaceRanges = range->next;
        flags = PAGING_DEFAULT_FLAGS;
    }

    if (!stackPtr)
    {
        logWarnn("%! Couldn't allocate %d pages for a thread stack", "[Userspace Allocator]", pages);
        // TODO: do something about it
        return NULL;
    }

    // Allocated on physical memory
    for (uint64_t page = 0; page < pages; page++)
    {
        physical_address_t phys = memoryPhysicalAllocator.allocatePage();
        pagingMapPageInSpace(stackPtr + PAGE_SIZE * page, phys, this->pml4Physical, flags);
    }

    #ifdef VERBOSE_USERSPACEALLOCATOR
    logDebugn("%! Allocated stack starting at 0x%64x with size %m", "[Userspace Allocator]", stackPtr, PAGING_ALIGN_PAGE_UP(stackSize));
    #endif

    return stackPtr;
}

virtual_address_t k_userspace_allocator::allocateInterruptStack(uint64_t stackSize)
{
    uint64_t pages = PAGING_ALIGN_PAGE_UP(stackSize) / PAGE_SIZE;

    virtual_address_t stackPtr;

    stackPtr = virtualAddressRangeAllocator.allocateRange(pages);
    k_address_range_header *range = new k_address_range_header();
    range->base = stackPtr;
    range->pages = pages;
    range->next = this->kernelspaceRanges;
    this->kernelspaceRanges = range->next;

    if (!stackPtr)
    {
        logWarnn("%! Couldn't allocate %d pages for a thread interrupt stack", "[Userspace Allocator]", pages);
        // TODO: do something about it
        return NULL;
    }

    // Allocated on physical memory
    for (uint64_t page = 0; page < pages; page++)
    {
        physical_address_t phys = memoryPhysicalAllocator.allocatePage();
        pagingMapPageInSpace(stackPtr + PAGE_SIZE * page, phys, this->pml4Physical, PAGING_DEFAULT_FLAGS);
    }

    #ifdef VERBOSE_USERSPACEALLOCATOR
    logDebugn("%! Allocated interrupt stack starting at 0x%64x with size %m", "[Userspace Allocator]", stackPtr, PAGING_ALIGN_PAGE_UP(stackSize));
    #endif

    return stackPtr;
}

void k_userspace_allocator::freeStack(virtual_address_t stackPtr)
{
    this->threadsStackAllocator->freeRange(stackPtr);
}

virtual_address_t k_userspace_allocator::getUserspaceCodeStart()
{
    return this->userspaceCodeStart;
}

virtual_address_t k_userspace_allocator::getUserspaceCodeEnd()
{
    return 0xFFFF800000000000;
}

virtual_address_t k_userspace_allocator::getUserspaceHeapStart()
{
    return this->userspaceHeapStart;
}

virtual_address_t k_userspace_allocator::getUserspaceHeapEnd()
{
    return this->userspaceCodeStart;
}