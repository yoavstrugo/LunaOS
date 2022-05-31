#include <interrupts/interrupts.hpp>

#include <interrupts/idt.hpp>
#include <logger/logger.hpp>
#include <interrupts/exceptions.hpp>
#include <interrupts/requests.hpp>
#include <kernel.hpp>
#include <syscalls/syscalls.hpp>
#include <tasking/tasking.hpp>
#include <interrupts/lapic.hpp>
#include <interrupts/ioapic.hpp>
#include <interrupts/pic.hpp>
#include <stddef.h>

void interruptsInitialize()
{
    idtInitialize();
    idtLoad();
    interruptsInstallRoutines();

    picRemapIRQs();

    ioapicCreateISARedirection(1, 1, 0);
    ioapicCreateISARedirection(0x80, 0x80, 0);
    lapicInitialize();
    interruptsEnable();
}

void interruptsEnable()
{
    asm("sti");
}

void interruptsDisable()
{
    asm("cli");
}

void interruptsInstallRoutines()
{
    idtCreateEntry(0x08, (uint64_t)_iExc8, exceptionDoubleFault, 0x08, 0x00, K_IDT_TA_INTERRUPT);
    idtCreateEntry(0x0D, (uint64_t)_iExc13, exceptionGPFault, 0x08, 0x00, K_IDT_TA_INTERRUPT);
    idtCreateEntry(0x0E, (uint64_t)_iExc14, exceptionPageFault, 0x08, 0x00, K_IDT_TA_INTERRUPT);
    idtCreateEntry(0x20, (uint64_t)_iReq32, requestTimer, 0x08, 0x00, K_IDT_TA_INTERRUPT);
    idtCreateEntry(0x21, (uint64_t)_iReq33, requestKeyboardInt, 0x08, 0x00, K_IDT_TA_INTERRUPT);
    idtCreateEntry(0x80, (uint64_t)_iReq128, requestTimer, 0x08, 0x00, K_IDT_TA_INTERRUPT_USER);
}

k_thread_state *interruptHandler(k_thread_state *rsp)
{
    k_thread *thread = taskingGetRunningThread();

    if (thread)
    {
        // If there was a thread running before, store its context
        thread->context = rsp;

        if (thread->context->interruptCode < 0x20)
        {
            exceptionHandlers[thread->context->interruptCode](thread->context->errorCode);
        }
        else if (thread->context->interruptCode == 0x80)
        {
            // System call request
            syscallHandle(thread);
        }
        else
        {
            requestHandlers[thread->context->interruptCode - 0x20](thread->context->errorCode);
        }

    } else {
        // First thread
        taskingSwitch();
    }

    thread = taskingGetRunningThread();

    if (thread != NULL)
        rsp = thread->context;
    
    lapicSendEOI();
    return rsp;
}