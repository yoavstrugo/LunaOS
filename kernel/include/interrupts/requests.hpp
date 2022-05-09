#pragma once

struct interrupt_frame;
__attribute__((interrupt)) void requestKeyboardInt(interrupt_frame *frame);