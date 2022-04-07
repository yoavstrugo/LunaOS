#include <interrupts/exceptions.hpp>

#include <kernel.hpp>

__attribute__((interrupt)) void exceptionPageFault(interrupt_frame *frame) {
    kernelPanic("%! A page fault has occured!", "Memory Exception:");
}