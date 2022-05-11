#pragma once

#include <stivale2.h>
#include <memory/bitmap_allocator.hpp>

#define K_HEAP_DEFAULT_START    0xFFFF800000100000
#define K_HEAP_DEFAULT_END      0xFFFF800000200000
#define K_HEAP_MAX_EXPANSION    0xFFFF800100000000

extern BitmapAllocator memoryPhysicalAllocator;

void memoryInitialize(stivale2_struct *stivale2Info);
