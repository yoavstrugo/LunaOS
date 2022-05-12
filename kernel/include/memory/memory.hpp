#pragma once

#include <stivale2.h>
#include <memory/bitmap_allocator.hpp>
#include <memory/virtual_address_range_allocator.hpp>

#define K_CONST_HEAP_DEFAULT_START              0xFFFF800000100000
#define K_CONST_HEAP_DEFAULT_END                0xFFFF800000200000
#define K_CONST_HEAP_MAX_EXPANSION              0xFFFF800050100000

#define K_CONST_KERNEL_VIRTUAL_RANGES_START     0xFFFF800050100000
#define K_CONST_KERNEL_VIRTUAL_RANGES_END       0xFFFF8000A0100000

extern BitmapAllocator memoryPhysicalAllocator;
extern k_virtual_address_range_allocator virtualAddressRangeAllocator;

void memoryInitialize(stivale2_struct *stivale2Info);
