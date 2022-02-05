#pragma once

#include <stdint.h>
#include <stivale2.h>
#include <types.h>

// Some constants to be use
#define PAGE_SIZE   4096        // The size of a page (in bytes)

// Macros to calculate the relevant address parts
#define PTi(add)    (add >> 12) // Get the address of the Page Table
#define PDi(add)    (add >>  9) // Get the address of the Page Directory
#define PDPi(add)   (add >>  9) // Get the address of the Page Directory Pointer
#define PML4i(add)  (add >>  9) // Get the address of the Page Map Level 4


// typedefs for address
typedef address     linear_address;
typedef address     physic_address;



