#include <tasking/tasking.hpp>

#include <memory/memory.hpp>
#include <logger/logger.hpp>

#include <tasking/scheduler.hpp>
#include <memory/heap.hpp>
#include <interrupts/interrupts.hpp>

#include <gdt/gdt.hpp>

static k_processor_tasking *processorTaskingArray;

uint64_t k_processor_tasking::getNextID()
{
    return this->nextPID++;
}

void _idleThread()
{
    interruptsEnable();
    for (;;)
        asm("hlt");
}

void taskingInitialize(uint8_t numOfCPUs)
{
    processorTaskingArray = new k_processor_tasking[numOfCPUs];
    for (int i = 0; i < numOfCPUs; i++)
        memset((char *)processorTaskingArray, 0, numOfCPUs * sizeof(k_processor_tasking));

    // Start with idle thread
    k_process *proc = taskingCreateProcess();
    k_thread *thread = taskingCreateThread((uint64_t)_idleThread, proc, KERNEL);

    taskingGetProcessor()->currentThread = thread;
}

void taskingAddCPU(uint8_t id)
{
    processorTaskingArray[id].currentThread = NULL;
    processorTaskingArray[id].processes = 0;
    processorTaskingArray[id].processorId = id;
}

k_process *taskingCreateProcess()
{
    k_process *process = new k_process();

    // 2 MiB for code, should change to ELF size
    uint64_t codeSize = 2 * MiB_unit;
    process->processAllocator = new k_userspace_allocator();
    // process->processAllocator->allocateUserspaceCode(codeSize);
    process->processAllocator->allocateUserspaceHeap();
    process->addressSpace = process->processAllocator->getSpace();

    process->pid = processorTaskingArray[0].getNextID();

    k_process_entry *entry = new k_process_entry();
    entry->process = process;
    entry->next = taskingGetProcessor()->processes;
    taskingGetProcessor()->processes = entry;

    return process;
}

void taskingInitializeThreadMemory(k_thread *thread, THREAD_PRIVILEGE privilege)
{
    if (privilege == KERNEL)
    {
        thread->stack.start = thread->process->processAllocator->allocateStack(USERSPACE_STACK_SIZE, true);
        thread->stack.end = thread->stack.start + USERSPACE_STACK_SIZE;
        thread->interruptStack.start = 0;
        thread->interruptStack.end = 0;
    }
    else
    {
        thread->stack.start = thread->process->processAllocator->allocateStack(USERSPACE_STACK_SIZE, false);
        thread->stack.end = thread->stack.start + USERSPACE_STACK_SIZE;
        thread->interruptStack.start = thread->process->processAllocator->allocateInterruptStack(INTERRUPT_STACK_SIZE);
        thread->interruptStack.end = thread->interruptStack.start + INTERRUPT_STACK_SIZE;
    }

    // The context of the thread is on the top of the stack (stack is going downwards - end is the top)
    thread->context = (k_thread_state *)(thread->stack.end - sizeof(k_thread_state));
    // Stack always start at the topmost address and goes down
    thread->context->rbp = thread->stack.end; // TODO: this might cause a problem
    // Set the stack pointer of the thread
    thread->context->rsp = (register_t)thread->context;
}

void taskingAddThreadToProcess(k_thread *thread, k_process *process)
{
    // Add it to the process' list
    k_thread_entry *entry = new k_thread_entry();
    entry->thread = thread;
    entry->next = process->threads;
    process->threads = entry;

    // Assign to the process and an select an id id
    if (process->mainThread == NULL)
    {
        process->mainThread = thread;
        thread->id = process->pid;
    }
    else
    {
        thread->id = taskingGetProcessor()->getNextID();
    }
}

k_thread *taskingCreateThread(virtual_address_t entryPoint, k_process *process, THREAD_PRIVILEGE privilege)
{
    k_thread *thread = new k_thread();
    thread->process = process;

    pagingSwitchSpace(thread->process->addressSpace);

    taskingInitializeThreadMemory(thread, privilege);
    // Set the IP for the thread
    thread->context->rip = entryPoint;

    // Set segments according to privileges
    thread->privilege = privilege;
    taskingSetupPrivileges(thread);

    // Add the thread to it's parent process and assign an id to it
    taskingAddThreadToProcess(thread, process);

    thread->status = READY;
    schedulerNewJob(thread);
    return thread;
}

void taskingSwitch()
{
    k_thread *previousThread = taskingGetRunningThread();
    k_thread *threadToRun = schedulerSchedule();

    if (threadToRun != NULL)
    {

        if (threadToRun->process->addressSpace != pagingGetCurrentSpace())
            pagingSwitchSpace(threadToRun->process->addressSpace);

        if (threadToRun->privilege != KERNEL)
            gdtSetActiveStack(threadToRun->interruptStack.end);

        taskingGetProcessor()->currentThread = threadToRun;
        threadToRun->status = RUNNING;

        // The previouse thread should be ready to execute again
        if (previousThread != NULL)
            if (previousThread->status == RUNNING)
                previousThread->status = READY;
    }
}

void taskingDumpProcesses()
{
    k_processor_tasking *processorTasking = processorTaskingArray;
    k_process_entry *curr = processorTasking->processes;

    logInfon("%! Dumping processes:", "[Tasking]");
    while (curr)
    {
        logInfon("\t- PID: %d", curr->process->pid);
        curr = curr->next;
    }
}

k_thread *taskingGetRunningThread()
{
    return taskingGetProcessor()->currentThread;
}

k_processor_tasking *taskingGetProcessor()
{
    return &processorTaskingArray[0];
}

void taskingSetupPrivileges(k_thread *thread)
{
    if (thread->privilege == KERNEL)
    {
        thread->context->cs = 0x08 | 0;
        thread->context->ss = 0x10 | 0;
        thread->context->gs = 0x10 | 0;
        thread->context->fs = 0x10 | 0;
        thread->context->ds = 0x10 | 0;
        thread->context->rflags = RFLAGS_IOPL | RFLAGS_ALWAYS_ON;
    }
    else if (thread->privilege == USER)
    {
        thread->context->cs = 0x18 | 3;
        thread->context->ss = 0x20 | 3;
        thread->context->gs = 0x20 | 3;
        thread->context->fs = 0x20 | 3;
        thread->context->ds = 0x20 | 3;
        thread->context->rflags = RFLAGS_ALWAYS_ON;
    }
}