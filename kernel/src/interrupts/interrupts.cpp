#include <interrupts/interrupts.hpp>

#include <interrupts/idt.hpp>
#include <logger/logger.hpp>
#include <interrupts/exceptions.hpp>
#include <kernel.hpp>

void interruptsInitiallize() {
    idtInitiallize();
    idtLoad();
    interruptsInstallRoutines();
}

void interruptsEnable() {
    asm ("cli");
}

void interruptsDisable() {
    asm ("cli");
}

void interruptsInstallRoutines() {
    idtCreateEntry(0x0E, (uint64_t)exceptionPageFault, 0x08, 0x00, K_IDT_TA_INTERRUPT);
}