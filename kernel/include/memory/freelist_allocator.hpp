#pragma once

#include <types.hpp>
#include <stddef.h>

struct k_chunk_header
{
    k_chunk_header *next;
    uint8_t used : 1;
    size_t size;
} __attribute__((packed));

struct k_freelist_allocator
{
    k_chunk_header *head;

    /**
     * @brief Construct a new freelist allocator
     *
     * @param start The start of the allocator
     * @param end   The end of the allocator
     */
    void init(virtual_address_t start, virtual_address_t end);

    /**
     * @brief Allocates memory
     *
     * @param How much memory to allocate
     *
     * @return void* ptr to the allocated memory, 0 if no memory was allocated
     */
    void *allocate(uint64_t size);

    /**
     * @brief Free memory
     *
     * @param mem ptr to the memory
     * @return The size of the free'd memory
     */
    uint64_t free(void *mem);

    /**
     * @brief Expand the allocator's working area
     *
     * @param amount How much to expand
     */
    void expand(uint64_t amount);

private:
    /**
     * @brief Loop over the list and merge unused chunks
     *
     */
    void mergeChunks();
};
