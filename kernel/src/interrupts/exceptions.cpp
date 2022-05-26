#include <interrupts/exceptions.hpp>

#include <kernel.hpp>

void (*exceptionHandlers[32])();

void exceptionDoubleFault()
{
    kernelPanic("%! A double fault has occurred!", "Memory Exception:");
}

void exceptionGPFault()
{
    kernelPanic("%! A general protection fault has occurred!", "Memory Exception:");
}

void exceptionPageFault()
{
    kernelPanic("%! A page fault has occurred!", "Memory Exception:");
}
