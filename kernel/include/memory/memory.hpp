#pragma once

#include <stivale2.h>
#include <memory/BitmapAllocator.hpp>

extern BitmapAllocator memoryPhysicalAllocator;

void memoryInitiallize(stivale2_struct_tag_memmap *memoryMap);
