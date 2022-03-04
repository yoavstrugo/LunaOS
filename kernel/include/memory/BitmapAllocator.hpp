#pragma once

#include <memory/PhysicalMemoryManager.hpp>
#include <types.hpp>
#include <stivale2.h>
#include <stdint.h>
#include <Bitmap.hpp>
#include <strings.hpp>

class BitmapAllocator : public PhysicalMemoryManager {
    public:
        /**
         * @brief               Construct a new Bitmap Allocator object
         * 
         * @param memmapStruct  The memory map given by stivale
         */
        BitmapAllocator(stivale2_struct_tag_memmap *memmapStruct);

        physical_address allocateBlock();
        void freeBlock(physical_address blockAddr);
        
        uint64_t totalMemory();
        uint64_t freeMemory();
        uint64_t usedMemory();
        uint64_t reservedMemory();

    private:        /**
         * @brief                   Count how much memory there is in the memmap
         * 
         * @param memmapStruct      The memory map from stivale
         * @return uint64_t         The total memory present in the memmap
         */
        static uint64_t _countMemory(stivale2_struct_tag_memmap *memmapStruct);
        /**
         * @brief                   Iterate through the memory map and, lock reserved pages
         *                          and count memory.
         * 
         * @param memmapStruct      The memory map given by stivale
         */
        void _mapMemory(stivale2_struct_tag_memmap *memmapStruct);

        /**
         * @brief                   Lock a block, if the block is already locked,
         *                          it won't do anything.
         * 
         * @param blockAddr         The address of the block, page-aligned.
         */
        void _lockBlock(physical_address blockAddr);

        /**
         * @brief                   Free a block, if the block is already free,
         *                          it won't do anything.
         * 
         * @param blockAddr         The address of the block, page-aligned.
         */
        void _freeBlock(physical_address blockAddr);

        /**
         * @brief                   Get the address of the next free block.
         * 
         * @return physical_address The address of the block, page-aligned.
         */
        physical_address _nextBlock();

        uint64_t _freeMemory;
        uint64_t _usedMemory;
        uint64_t _reservedMemory;

        bool _isInitiallized;

        Bitmap _memoryBitmap;
};

/**
 * @brief                   Count how much memory there is in the memmap
 * 
 * @param memmapStruct      The memory map from stivale
 * @return uint64_t         The total memory present in the memmap
 */
uint64_t countMemory(stivale2_struct_tag_memmap *memmapStruct);