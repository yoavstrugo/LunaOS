#include <interrupts/interrupts.hpp>

#include <interrupts/idt.hpp>
#include <logger/logger.hpp>
#include <interrupts/exceptions.hpp>
#include <interrupts/requests.hpp>
#include <kernel.hpp>

void interruptsInitiallize() {
    idtInitiallize();
    idtLoad();
    interruptsInstallRoutines();
    interruptsEnable();
}

void interruptsEnable() {
    asm ("sti");
}

void interruptsDisable() {
    asm ("cli");
}

void interruptsInstallRoutines() {
    idtCreateEntry(0x08, (uint64_t)exceptionDoubleFault, 0x08, 0x00, K_IDT_TA_INTERRUPT);
    idtCreateEntry(0x0D, (uint64_t)exceptionGPFault, 0x08, 0x00, K_IDT_TA_INTERRUPT);
    idtCreateEntry(0x0E, (uint64_t)exceptionPageFault, 0x08, 0x00, K_IDT_TA_INTERRUPT);
    idtCreateEntry(0x21, (uint64_t)requestKeyboardInt, 0x08, 0x00, K_IDT_TA_INTERRUPT);
}