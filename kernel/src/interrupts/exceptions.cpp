#include <interrupts/exceptions.hpp>

#include <kernel.hpp>

void exceptionDoubleFault(interrupt_frame *frame) {
    kernelPanic("%! A double fault has occured!", "Memory Exception:");
}

void exceptionGPFault(interrupt_frame *frame) {
    kernelPanic("%! A general protection fault has occured!", "Memory Exception:");
}

void exceptionPageFault(interrupt_frame *frame) {
    kernelPanic("%! A page fault has occured!", "Memory Exception:");
}

