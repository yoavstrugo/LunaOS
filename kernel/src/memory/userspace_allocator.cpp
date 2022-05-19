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
    this->threadsStackAllocator.addRange(USERSPACE_MEMORY_START, USERSPACE_STACK_MAX);
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
        virtual_address_t  virt = this->userspaceCodeStart + PAGE_SIZE * page;

        // Map the page
        pagingMapPage(virt, phys, USERSPACE_DEFAULT_PAGING_FLAGS);
    }
}

void k_userspace_allocator::allocateUserspaceHeap() {
    if (!this->userspaceCodeStart)
        kernelPanic("%! Cannot allocate heap before code was allocated.", "[Userspace Allocator]");

    uint64_t heapEnd = this->userspaceCodeStart;
    uint64_t heapStart = PAGING_ALIGN_PAGE_DOWN(heapEnd - USERSPACE_HEAP_INITIAL_SIZE);

    for (uint64_t virt = heapStart; virt < heapEnd; virt += PAGE_SIZE)
    {
        physical_address_t phys = memoryPhysicalAllocator.allocatePage();
        pagingMapPage(virt, phys, USERSPACE_DEFAULT_PAGING_FLAGS);
    }
}

void k_userspace_allocator::expandUserspaceHeap() {
    if (this->userspaceHeapStart - USERSPACE_HEAP_EXPANSION < USERSPACE_STACK_MAX) {
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
        pagingMapPage(virt, phys, USERSPACE_DEFAULT_PAGING_FLAGS);
    }
}

void k_userspace_allocator::free() {
    // Free userspace code
    for (uint64_t virt = this->getUserspaceCodeStart(); virt < this->getUserspaceCodeEnd(); virt += PAGE_SIZE)
    {
        physical_address_t phys = pagingUnmapPage(virt);
        memoryPhysicalAllocator.freePage(phys);
    }
    

    // Free userspace heap
    for (uint64_t virt = this->getUserspaceHeapStart(); virt < this->getUserspaceHeapEnd(); virt += PAGE_SIZE)
    {
        physical_address_t phys = pagingUnmapPage(virt);
        memoryPhysicalAllocator.freePage(phys);
    }

    // Free stacks
    k_address_range_header *range = this->threadsStackAllocator.getRanges();
    while (range) {
        if (range->used) {
            virtual_address_t virt = range->base;
            uint64_t pages = range->pages;

            // Unmap and free each page
            for (uint64_t page = 0; page < pages; page++) {
                physical_address_t phys = pagingUnmapPage(virt + PAGE_SIZE * page);
                memoryPhysicalAllocator.freePage(phys);
            }
        }
        range = range->next;
    }
}

virtual_address_t k_userspace_allocator::allocateStack(uint64_t stackSize) {
    uint64_t pages = PAGING_ALIGN_PAGE_UP(stackSize) / PAGE_SIZE;
    virtual_address_t stackPtr = this->threadsStackAllocator.allocateRange(pages);
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
        pagingMapPage(stackPtr + PAGE_SIZE * page, phys, USERSPACE_DEFAULT_PAGING_FLAGS);
    }
    
    return stackPtr;
}

void k_userspace_allocator::freeStack(virtual_address_t stackPtr) {
    this->threadsStackAllocator.freeRange(stackPtr);
}

virtual_address_t k_userspace_allocator::getUserspaceCodeStart() {
    return this->userspaceCodeStart;
}

virtual_address_t k_userspace_allocator::getUserspaceCodeEnd() {
    return 0xFFFF800000000000;
}

virtual_address_t k_userspace_allocator::getUserspaceHeapStart() {
    return this->userspaceHeapStart;
}

virtual_address_t k_userspace_allocator::getUserspaceHeapEnd() {
    return this->userspaceCodeStart;
}