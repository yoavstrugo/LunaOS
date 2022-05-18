#include <interrupts/exceptions.hpp>

#include <kernel.hpp>

void exceptionDoubleFault(interrupt_frame *frame)
{
    kernelPanic("%! A double fault has occurred!", "Memory Exception:");
}

void exceptionGPFault(interrupt_frame *frame)
{
    kernelPanic("%! A general protection fault has occurred!", "Memory Exception:");
}

void exceptionPageFault(interrupt_frame *frame)
{
    kernelPanic("%! A page fault has occurred!", "Memory Exception:");
}
