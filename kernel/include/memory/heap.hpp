#pragma once

#include <types.hpp>

#define K_HEAP_EXPANSION_STEP 0x100000

extern bool heapVerbose;

/**
 * @brief Initializes the kernel's heap at the given memory address'
 *
 * @param start The start of the heap
 * @param end   The end of the heap
 */
void heapInitialize(virtual_address_t start, virtual_address_t end);

/**
 * @brief Expands the heap (downwards)
 *
 */
bool heapExpand();

/**
 * @brief Allocate some memory on the heap
 *
 * @param size  How much memory to allocate
 * @return      The allocated memory address
 */
void *heapAllocate(uint64_t size);

/**
 * @brief Free allocated memory on the heap
 *
 * @param mem   The pointer to the memory
 */
void heapFree(void *mem);
