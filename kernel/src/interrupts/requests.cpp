#include <interrupts/requests.hpp>

#include <logger/logger.hpp>
#include <interrupts/pic.hpp>
#include <io.hpp>

void requestKeyboardInt(interrupt_frame *frame) {
    logInfon("Pressed!");
    ioInByte(0x60);
    picSendEOI(0x21);
}