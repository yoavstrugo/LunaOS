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
}

k_thread_state *interruptHandler(k_thread_state *rsp)
{
    k_thread *thread = taskingGetRunningThread();

    if (thread)
    {
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

        rsp = thread->context;
    }
    else
    {
        // First thread
        taskingSwitch();
        thread = taskingGetRunningThread();
    }

    if (thread != NULL)
        rsp = thread->context;

    // TODO: remove
    // rsp->gs =       thread->context->gs;
    // rsp->fs =       thread->context->fs    ;
    // rsp->rax =      thread->context->rax  ;
    // rsp->rbx =      thread->context->rbx ;
    // rsp->rcx =      thread->context->rcx;
    // rsp->rdx =      thread->context->rdx;
    // rsp->rsi =      thread->context->rsi;
    // rsp->rdi =      thread->context->rdi;

    // rsp->r8 =       thread->context->r8;
    // rsp->r9 =       thread->context->r9;
    // rsp->r10 =      thread->context->r10;
    // rsp->r11 =      thread->context->r11;
    // rsp->r12 =      thread->context->r12;
    // rsp->r13 =      thread->context->r13;
    // rsp->r14 =      thread->context->r14;
    // rsp->r15 =      thread->context->r15;
    // rsp->rbp =      thread->context->rbp;

    // rsp->interruptCode = thread->context->interruptCode;
    // rsp->errorCode =     thread->context->errorCode;

    // rsp->rip  =     thread->context->rip;
    // rsp->cs  =      thread->context->cs;
    // rsp->rflags  =  thread->context->rflags;
    // rsp->rsp  =     thread->context->rsp;
    // rsp->ss  =      thread->context->ss;

    // logDebugn("TIMER");
    // TODO: tss?
    lapicSendEOI();
    return rsp;
}