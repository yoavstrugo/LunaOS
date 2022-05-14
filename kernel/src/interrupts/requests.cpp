#include <interrupts/requests.hpp>

#include <logger/logger.hpp>
#include <interrupts/pic.hpp>
#include <interrupts/lapic.hpp>
#include <io.hpp>

void requestKeyboardInt(interrupt_frame *frame) {
    logInfon("Pressed!");
    ioInByte(0x60);
    lapicSendEOI();
}

uint64_t count = 0;
uint64_t seconds = 0;

void requestTimer(interrupt_frame *frame) {
    count++;
    if (count >= 1000) {
        count = 0;
        seconds++;
        logDebugn("%! %ds", "[APIC Timer]", seconds);
    }
        lapicSendEOI();
}