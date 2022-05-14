#pragma once

#include <stivale2.h>
#include <memory/bitmap_allocator.hpp>
#include <memory/virtual_address_range_allocator.hpp>

#define K_CONST_HEAP_DEFAULT_START              0xFFFF800100100000
#define K_CONST_HEAP_DEFAULT_END                0xFFFF800100200000
#define K_CONST_HEAP_MAX_EXPANSION              0xFFFF800150100000

#define K_CONST_KERNEL_VIRTUAL_RANGES_START     0xFFFF800150100000
#define K_CONST_KERNEL_VIRTUAL_RANGES_END       0xFFFF8100A0100000

#define BYTE    (1)
#define KiB     (1024 * BYTE)
#define MiB     (1024 * KiB)
#define GiB     (1024 * MiB)

#define K_MEMORY_UNIT(mem)  ((mem >= GiB) ? "GiB" : ((mem >= MiB) ? "MiB" : ((mem >= KiB) ? "KiB" : "Bytes")))
#define K_MEMORY_SIZE(mem)  ((mem >= GiB) ? mem/GiB : ((mem >= MiB) ? mem/MiB : ((mem >= KiB) ? mem/KiB : mem)))

extern BitmapAllocator memoryPhysicalAllocator;
extern k_virtual_address_range_allocator virtualAddressRangeAllocator;

void memoryInitialize(stivale2_struct *stivale2Info);
