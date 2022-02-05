#pragma once

#include <stdint.h>
#include <stivale2.h>
#include <Bitmap.hpp>
#include <constants.hpp>
#include <memory/Paging.hpp>
#include <strings.hpp>

class PhysicalMemoryAllocator {
    public:

        /**
         * @brief                   Construct a new Physical Memory Allocator object
         * 
         */
        PhysicalMemoryAllocator();

        /**
         * @brief                   Map the memory from the stivale2 memmap structure
         * 
         * @param memmapStruct      The memmap structure from stivale2
         */
        void mapMemory(stivale2_struct_tag_memmap *memmapStruct);

        /**
         * @brief                   Get the total memory of the system
         * 
         * @return uint64_t         The total memory of the system
         */
        uint64_t totalMemory();

        /**
         * @brief                   Get the free memory in the system
         * 
         * @return uint64_t         The free memory in the system
         */
        uint64_t freeMemory();

        /**
         * @brief                   Get the used memory in the system
         * 
         * @return uint64_t         The used memory in the system
         */
        uint64_t usedMemory();

        /**
         * @brief                   Get the reserved memory in the system
         * 
         * @return uint64_t         The reserved memory in the system
         */
        uint64_t reservedMemory();

    private:
        uint64_t _freeMemory;
        uint64_t _usedMemory;
        uint64_t _reservedMemory;

        bool _isInitiallized;

        Bitmap _memoryBitmap;

        physic_address _findNextEmptyPage();
};