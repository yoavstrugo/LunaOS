#pragma once

#include <stdint.h>
#include <stddef.h>
#include <types.hpp>

#define LINKED_LIST_IS_EMPTY(link) ((link)->linkf == link)

struct k_buddyallocator_block_hdr
{
    // Whether this block is available or not
    uint8_t tag : 1;
    // Forward link in the list
    k_buddyallocator_block_hdr *linkf;
    // Backward link in the list
    k_buddyallocator_block_hdr *linkb;
    // The order of the block
    uint8_t kval;
} __attribute__((packed));

struct k_buddyallocator
{
private:
    // list of the available blocks, has a size of 48, this means we can
    // use a maximum of 48 different block sizes, which should be enough
    k_buddyallocator_block_hdr avail[48];

    // The maximum order of a block
    uint8_t maxOrder;

    // The minimum size for a block, which is set to be of order 0
    uint8_t minBlockSize;

    // The base address of the allocator
    virtual_address_t base;

    // The size of the allocator
    size_t size;

public:
    /**
     * @brief Creates a new buddy allocator, starting at base with the given size
     *
     * @param base The base of the allocator, must be aligned
     * @param size The size of memory the allocator will work on, must be a power of 2
     * @param minBlockSize The minimum block size
     */
    k_buddyallocator(virtual_address_t base, size_t size, uint8_t minBlockSize = 1);

    /**
     * @brief Allocate some memory with the buddy allocator
     *
     * @param size How much memory to allocate
     * @return virtual_address_t The address of the allocated memory
     */
    virtual_address_t allocate(size_t size);

    /**
     * @brief Deallocates allocated memory
     *
     * @param addr The address of the memory to deallocate.
     */
    void deallocate(virtual_address_t addr);
};

#define BUDDYALLOCATOR_GET_BUDDY(block) ((k_buddyallocator_block_hdr *)((uint64_t)block ^ (1L << block->kval)))

uint8_t buddyallocatorFindOrder(size_t size, uint8_t min);