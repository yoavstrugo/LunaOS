#include <interrupts/idt.hpp>

#include <logger/logger.hpp>
#include <types.hpp>
#include <interrupts/exceptions.hpp>
#include <interrupts/requests.hpp>

k_idt_descriptor idtDescriptor;
k_idt_entry idt[K_IDT_SIZE];

void idtCreateEntry(uint8_t idx, uint64_t offset, void (*handler)(uint64_t), uint16_t segmentSelector, uint8_t ist, uint8_t flags)
{
    idt[idx].offsetLow = (uint16_t)(offset & 0xFFFF);
    idt[idx].segmentSelector = segmentSelector;
    idt[idx].ist = ist & 0b00000111;
    idt[idx].flags = flags;
    idt[idx].offsetMid = (uint16_t)((offset >> 16) & 0xFFFF);
    idt[idx].offsetHigh = (uint32_t)((offset >> 32) & 0xFFFFFFFF);
    idt[idx].zero = 0;

    if (idx < 0x20) {
        exceptionHandlers[idx] = handler;
    } else {
        requestHandlers[idx - 0x20] = handler;
    }
}

void idtInitialize()
{
    idtDescriptor.offset = (uint64_t)&idt;
    idtDescriptor.size = (sizeof(k_idt_entry) * K_IDT_SIZE) - 1;
}

void idtLoad()
{
    logDebugn("%! is at 0x%64x", "[IDT]", &idt);
    logDebugn("%! is at 0x%64x, \n\t\tsize: %d, \n\t\toffset: 0x%64x", "[IDT]",
              &idtDescriptor, idtDescriptor.size, idtDescriptor.offset);
    _loadIDT(&idtDescriptor);
    logDebugn("%! has been loaded into lidt", "[IDT]");
}