#include <interrupts/requests.hpp>

#include <logger/logger.hpp>
#include <interrupts/pic.hpp>
#include <interrupts/lapic.hpp>
#include <io.hpp>
#include <tasking/scheduler.hpp>

void (*requestHandlers[224])();

void requestKeyboardInt()
{
    logInfon("Pressed!");
    ioInByte(0x60);
}

void requestTimer()
{
    schedulerTime();
    taskingSwitch();
}