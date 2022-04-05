#include <memory/memory.hpp>

BitmapAllocator memoryPhysicalAllocator;

void memoryInitiallize(stivale2_struct_tag_memmap *memoryMap) {
    memoryPhysicalAllocator.initiallize(memoryMap);
}
