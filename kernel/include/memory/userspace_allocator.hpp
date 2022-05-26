#pragma once

#include <types.hpp>
#include <memory/virtual_address_range_allocator.hpp>
#include <memory/paging.hpp>

#define USERSPACE_MEMORY_START          0x0000000000000000
#define USERSPACE_MEMORY_END            0xFFFF800000000000
#define USERSPACE_HEAP_INITIAL_SIZE     2 * MiB
#define USERSPACE_HEAP_EXPANSION        2 * MiB
#define USERSPACE_STACK_SIZE            10 * PAGE_SIZE
#define USERSPACE_STACK_MAX             0xFFFF400000000000

struct k_userspace_allocator
{
    /**
     * @brief Construct a new userspace allocator
     */
    k_userspace_allocator();

    /**
     * @brief Allocates memory on the userspace for the process' code
     *
     * @param userspaceCodeSize The size of the code
     */
    void allocateUserspaceCode(uint64_t userspaceCodeSize);

    /**
     * @brief Allocates memory on the userspace for the process' heap
     *
     * @return virtual_address_t The start of the heap
     */
    void allocateUserspaceHeap();

    /**
     * @brief Returns the address to the PML4 of this process space
     * 
     * @return physical_address_t The physical address of the PML4
     */
    physical_address_t getSpace();

    /**
     * @brief Frees all the allocated memory in this allocator
     * 
     */
    void free();

    /**
     * @brief Expands the userspace heap by a default amount
     *
     */
    void expandUserspaceHeap();

    /**
     * @brief Allocated a new stack for a thread
     *
     * @param stackSize The size of the stack
     *
     * @return virtual_address_t The start of the stack
     */
    virtual_address_t allocateStack(uint64_t stackSize);

    /**
     * @brief Frees an allocated stack
     *
     * @param stackPtr The pointer to the stack start
     */
    void freeStack(virtual_address_t stackPtr);

    /**
     * @brief Get the start of the usespace code
     *
     * @return virtual_address_t The address to the start of the code
     */
    virtual_address_t getUserspaceCodeStart();

    /**
     * @brief Get the end of the usespace code
     *
     * @return virtual_address_t The address to the end of the code
     */
    virtual_address_t getUserspaceCodeEnd();

    /**
     * @brief Get the start of the usespace heap
     *
     * @return virtual_address_t The address to the start of the heap
     */
    virtual_address_t getUserspaceHeapStart();

    /**
     * @brief Get the end of the usespace heap
     *
     * @return virtual_address_t The address to the end of the heap
     */
    virtual_address_t getUserspaceHeapEnd();

private:
    virtual_address_t userspaceCodeStart;
    virtual_address_t userspaceHeapStart;
    physical_address_t pml4Physical;
    virtual_address_t pml4Virtual;
    k_virtual_address_range_allocator *threadsStackAllocator;
};