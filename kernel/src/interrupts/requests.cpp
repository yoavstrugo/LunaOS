#include <interrupts/requests.hpp>

#include <logger/logger.hpp>
#include <interrupts/pic.hpp>
#include <interrupts/lapic.hpp>
#include <io.hpp>
#include <tasking/scheduler.hpp>

void (*requestHandlers[224])(uint64_t);

void requestKeyboardInt(uint64_t)
{
    logInfon("Pressed!");
    ioInByte(0x60);
}

void requestTimer(uint64_t)
{
    schedulerTime();
    taskingSwitch();
}