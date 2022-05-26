#include <tasking/tasking.hpp>

#include <memory/memory.hpp>
#include <logger/logger.hpp>

#include <tasking/scheduler.hpp>
#include <memory/heap.hpp>

static k_processor_tasking *processorTaskingArray;

uint64_t k_processor_tasking::getNextPID()
{
    return this->nextPID++;
}

void _idleThread() {
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
    k_thread *thread = taskingCreateThread((uint64_t)_idleThread, proc);
    
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
    uint64_t codeSize = 2 * MiB;
    process->processAllocator = new k_userspace_allocator();
    process->processAllocator->allocateUserspaceCode(codeSize);
    process->processAllocator->allocateUserspaceHeap();
    process->addressSpace = process->processAllocator->getSpace();

    process->pid = processorTaskingArray[0].getNextPID();

    k_process_entry *entry = new k_process_entry();
    entry->process = process;
    entry->next = taskingGetProcessor()->processes;
    taskingGetProcessor()->processes = entry;

    return process;
}

k_thread *taskingCreateThread(virtual_address_t entryPoint, k_process *process, THREAD_PRIVILEGE privilege)
{
    k_thread *thread = new k_thread();
    thread->process = process;

    thread->stack.start = process->processAllocator->allocateStack(USERSPACE_STACK_SIZE);
    thread->stack.end = thread->stack.start + USERSPACE_STACK_SIZE;
    // Stack always start at the topmost address and goes down
    thread->context->rbp = thread->stack.end;
    // The context of the thread is on the top of the stack (stack is going downwards - end is the top)
    thread->context = (k_thread_state *)(thread->stack.end - sizeof(k_thread_state));
    // Set the stack pointer of the thread
    thread->context->rsp = (register_t)thread->context;

    // Add it to the process' list
    k_thread_entry *entry = new k_thread_entry();
    entry->thread = thread;
    entry->next = process->threads;
    process->threads = entry;

    thread->privilege = privilege;

    // Set the IP for the thread
    thread->context->rip = entryPoint;

    // Set segments
    taskingSetupPrivileges(thread);

    thread->status = READY;
    schedulerNewJob(thread);
    return thread;
}

void taskingSwitch()
{
    k_thread *threadToRun = schedulerSchedule();
    taskingGetProcessor()->currentThread = threadToRun;

    if (threadToRun == NULL)
    {
        // TODO: idle thread
        return;
    }

    if (threadToRun->process->addressSpace != pagingGetCurrentSpace())
    {
        pagingSwitchSpace(threadToRun->process->addressSpace);
    }

    // Switch context

    threadToRun->status = RUNNING;
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

void taskingSetupPrivileges(k_thread *thread) {
    if (thread->privilege == KERNEL) {
        thread->context->cs = 0x10 | 0;
        thread->context->ds = 0x08 | 0;
        thread->context->gs = 0x08 | 0;
        thread->context->fs = 0x08 | 0;
        thread->context->rflags |= 0x3000;
    } else if (thread->privilege == USER) {
        thread->context->cs = 0x20 | 3;
        thread->context->ds = 0x18 | 3;
        thread->context->gs = 0x18 | 3;
        thread->context->fs = 0x18 | 3;
    }
}