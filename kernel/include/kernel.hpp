#pragma once

#include <stivale2.h>

/**
 * @brief The entry point of our kernel
 * 
 * @param stivale2_struct The structre provided by the bootloader, following the stivale2 boot protocol.
 */
extern "C" void kernelMain(stivale2_struct *stivaleInfo);

void kernelInitiallize(stivale2_struct *stivaleInfo);

void kernelPanic(const char *msg, ...);

void kernelHalt();