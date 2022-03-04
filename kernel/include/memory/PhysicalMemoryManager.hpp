#pragma once

#include <types.hpp>
#include <stivale2.h>

class PhysicalMemoryManager {
    public:
        /**
         * @brief                   Allocate a page-sized block in the physical memory
         * 
         * @return physical_address The physical address of the allocated block it will
         *                          be page-aligned.
         */
        virtual physical_address allocateBlock() = 0;

        /**
         * @brief                   Free an allocated block in the physical memory,
         *                          If block is not allocated it won't do anything.
         * 
         * @param blockAddr         The address of the block to free. Must be page-aligned.
         */
        virtual void freeBlock(physical_address blockAddr) = 0;

        /**
         * @brief                   Get the total memory (in bytes) in the system. 
         * 
         * @return uint64_t         How the total amount of memory (in bytes).
         */
        virtual uint64_t totalMemory() = 0;

        /**
         * @brief                   Get the free memory (in bytes) in the system. 
         * 
         * @return uint64_t         How much memory is free (in bytes).
         */
        virtual uint64_t freeMemory() = 0;

        /**
         * @brief                   Get the used memory (in bytes) in the system. 
         * 
         * @return uint64_t         How much memory is used (in bytes).
         */
        virtual uint64_t usedMemory() = 0;

        /**
         * @brief                   Get the reserved memory (in bytes) in the system. 
         * 
         * @return uint64_t         How much memory is reserved (in bytes).
         */
        virtual uint64_t reservedMemory() = 0;
};