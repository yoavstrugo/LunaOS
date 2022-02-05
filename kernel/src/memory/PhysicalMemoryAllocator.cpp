#include <memory/PhysicalMemoryAllocator.hpp>

PhysicalMemoryAllocator::PhysicalMemoryAllocator() {
    this->_freeMemory = 0;
    this->_usedMemory = 0;
    this->_reservedMemory = 0;
    this->_isInitiallized = false;
}

void PhysicalMemoryAllocator::mapMemory(stivale2_struct_tag_memmap *memmapStruct)
{

    if (this->_isInitiallized)
        return;
    this->_isInitiallized = true;

    // Before we start mapping the memory we have to calculate the memory size
    uint64_t memorySize = 0;
    {
        for (uint64_t i = 0; i < memmapStruct->entries; i++)
        {
            stivale2_mmap_entry entry = memmapStruct->memmap[i];
            memorySize += entry.length;
        }
    }

    uint64_t bitmapSize = ((memorySize) / PAGE_SIZE + 1) / BYTE_SIZE;
    uint8_t tempArr[bitmapSize]; // Create a temporary array for the bitmap,
                                 // we will change it late

    memset((char *)tempArr, 0, bitmapSize);

    this->_memoryBitmap.setBuffer(tempArr);

    // Map the memory to the temporary array
    for (uint64_t entryIdx = 0; entryIdx < memmapStruct->entries; entryIdx++)
    {
        stivale2_mmap_entry entry = memmapStruct->memmap[entryIdx];

        bool memStatus = (entry.type == STIVALE2_MMAP_USABLE);

        this->_freeMemory += memStatus ? entry.length : 0;
        this->_reservedMemory += !memStatus ? entry.length : 0;

        for (uint64_t pageAddr = 0; pageAddr < entry.length; pageAddr += PAGE_SIZE)
        {
            // Calculate the absolute address of the page, get the page index
            // (in bytes) and divide by the bit size
            uint64_t pageBit = (entry.base + pageAddr) / PAGE_SIZE;

            this->_memoryBitmap.set(pageBit, memStatus);
        }
    }

    uint64_t bitmapPage;
    // Find the first place to put the bit map in
    for (uint64_t pageIndex = 0; pageIndex < bitmapSize; pageIndex++)
    {
        if (this->_memoryBitmap[pageIndex])
        {
            // Found an empty page, check if this section has enough space for
            // the bitmap
            bool found = true;
            for (uint64_t i = 0; i < bitmapSize / PAGE_SIZE + 1; i++)
            {
                if (!this->_memoryBitmap[pageIndex + i])
                {
                    found = false;
                    break;
                }
            }

            if (found)
            {
                bitmapPage = pageIndex;
                break;
            }
        }
    }

    // TODO: Do something in case there's no place for the bitmap

    // Calculate the address
    physic_address bitmapAddress = bitmapPage * PAGE_SIZE;
    memset((char *)bitmapAddress, 0, bitmapSize);

    // Copy the temp array to the location
    memcpy((void *)bitmapAddress, (void *)tempArr, bitmapSize);

    // Change the buffer of the bitmap
    this->_memoryBitmap.setBuffer((uint8_t *)bitmapAddress);

    // TODO: Lock the pages of the bitmap
}

uint64_t PhysicalMemoryAllocator::totalMemory()
{
    return this->_freeMemory + this->_usedMemory + this->_reservedMemory;
}

uint64_t PhysicalMemoryAllocator::freeMemory()
{
    return this->_freeMemory;
}

uint64_t PhysicalMemoryAllocator::usedMemory()
{
    return this->_usedMemory;
}

uint64_t PhysicalMemoryAllocator::reservedMemory()
{
    return this->_reservedMemory;
}