#pragma once

struct interrupt_frame;
__attribute__((interrupt)) void exceptionPageFault(interrupt_frame *frame);