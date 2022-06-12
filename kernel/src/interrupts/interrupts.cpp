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
#include <system/processor/processor.hpp>
#include <ps2/ps2.hpp>

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
    idtCreateEntry(0x21, (uint64_t)_iReq33, PS2::keyboardHandler, 0x08, 0x00, K_IDT_TA_INTERRUPT);
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
            Syscall::handle(thread);
        }
        else
        {
            requestHandlers[thread->context->interruptCode - 0x20](thread->context->errorCode);
        }
    }
    else
    {
        // First thread
        taskingSwitch();
        if (rsp->interruptCode < 0x20)
        {
            exceptionHandlers[rsp->interruptCode](rsp->errorCode);
        }
        else if (rsp->interruptCode == 0x80)
        {
        }
        else
        {
            requestHandlers[rsp->interruptCode - 0x20](rsp->errorCode);
        }
    }

    thread = taskingGetRunningThread();

    if (thread != NULL)
        rsp = thread->context;

    if (thread->privilege == USER) {
        // asm volatile("movq %0, %%rax\n\
        //             wrfsbase %%rax" ::"r"(thread->tls.start | 0xFFFF000000000000));
        processorSetMSR(0xC0000100, thread->tls.userThread);
        thread->context->fs_base = thread->tls.userThread;
    }

    lapicSendEOI();
    return rsp;
}