#pragma once

#include <stivale2.h>
#include <memory/bitmap_allocator.hpp>
#include <memory/virtual_address_range_allocator.hpp>

#define K_CONST_HEAP_DEFAULT_START 0xFFFF800100100000
#define K_CONST_HEAP_DEFAULT_END 0xFFFF800100200000
#define K_CONST_HEAP_MAX_EXPANSION 0xFFFF800150100000

#define K_CONST_KERNEL_VIRTUAL_RANGES_START 0xFFFF800150100000
#define K_CONST_KERNEL_VIRTUAL_RANGES_END 0xFFFF8100A0100000

#define BYTE_unit (1)
#define KiB_unit (1024 * BYTE_unit)
#define MiB_unit (1024 * KiB_unit)
#define GiB_unit (1024 * MiB_unit)

#define K_MEMORY_UNIT(mem) ((mem >= GiB_unit) ? "GiB" : ((mem >= MiB_unit) ? "MiB" : ((mem >= KiB_unit) ? "KiB" : "Bytes")))
#define K_MEMORY_SIZE(mem) ((mem >= GiB_unit) ? mem / GiB_unit : ((mem >= MiB_unit) ? mem / MiB_unit : ((mem >= KiB_unit) ? mem / KiB_unit : mem)))

#define ALIGN_UP(val, align)    (((val) % (align)) ? ((val) - ((val) % (align)) + (align)) : (val))

extern BitmapAllocator memoryPhysicalAllocator;
extern k_virtual_address_range_allocator virtualAddressRangeAllocator;

void memoryInitialize(stivale2_struct *stivale2Info);
