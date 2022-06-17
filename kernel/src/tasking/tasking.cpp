#include <tasking/tasking.hpp>

#include <memory/memory.hpp>
#include <logger/logger.hpp>

#include <tasking/scheduler.hpp>
#include <memory/heap.hpp>
#include <interrupts/interrupts.hpp>

#include <gdt/gdt.hpp>

#include <logger/printf.hpp>
#include <strings.hpp>

static k_processor_tasking *processorTaskingArray;
static uint8_t processorCout;
static k_process *focusedProcess;

uint64_t k_processor_tasking::getNextID()
{
    return this->nextPID++;
}

void _idleThread()
{
    interruptsEnable();
    for (;;)
        ;
}

void taskingInitialize(uint8_t numOfCPUs)
{
    processorTaskingArray = new k_processor_tasking[numOfCPUs];
    processorCout = numOfCPUs;
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

    // Initialize file descriptors hash map
    process->fileDescriptors = new List<FIL *>(3);

    

    f_chdir("/root");
    f_mkdir("proc");
    f_chdir("proc");
    char pidStr[10];
    sprintf(pidStr, "%d", process->pid);
    f_mkdir(pidStr);
    f_chdir(pidStr);
    f_mkdir("fd");
    f_chdir("fd");
    
    if (f_open(&process->stdin, "0", FA_READ | FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
        return NULL;

    process->fileDescriptors->add(&process->stdin); // stdin
    process->stdinWritePtr = 0;

    if (f_open(&process->stdout, "1", FA_READ | FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
        return NULL;
    process->fileDescriptors->add(&process->stdout); // stdin

    if (f_open(&process->stderr, "2", FA_READ | FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
        return NULL;
    process->fileDescriptors->add(&process->stderr); // stdin


    f_chdir("/");


    return process;
}

void taskingInitializeThreadMemory(k_thread *thread, THREAD_PRIVILEGE privilege)
{
    char *stackStr;
    if (privilege == KERNEL)
    {
        thread->stack.start = thread->process->processAllocator->allocateStack(USERSPACE_STACK_SIZE, true);
        thread->stack.end = thread->stack.start + USERSPACE_STACK_SIZE;
        thread->interruptStack.start = 0;
        thread->interruptStack.end = 0;

        // The context of the thread is on the top of the stack (stack is going downwards - end is the top)
        thread->context = (k_thread_state *)(thread->stack.end - sizeof(k_thread_state));
    }
    else
    {
        thread->stack.start = thread->process->processAllocator->allocateStack(USERSPACE_STACK_SIZE, false);
        thread->stack.end = thread->stack.start + USERSPACE_STACK_SIZE;
        thread->interruptStack.start = thread->process->processAllocator->allocateInterruptStack(INTERRUPT_STACK_SIZE);
        thread->interruptStack.end = thread->interruptStack.start + INTERRUPT_STACK_SIZE;

        // Copy the Master TLS to this thread
        k_process *process = thread->process;
        size_t alignment = process->masterTLS.alignment; // process->masterTLS.alignment > alignof(UserThread) ? process->masterTLS.alignment : alignof(UserThread);
        size_t totalSizeAligned = ALIGN_UP(process->masterTLS.totalSize, alignment) + sizeof(UserThread);
        thread->tls.start = process->processAllocator->allocateStack(totalSizeAligned, false);
        thread->tls.end = thread->tls.start + totalSizeAligned;

        memset((char *)thread->tls.start, 0, totalSizeAligned);
        memcpy((void *)thread->tls.start, (void *)process->masterTLS.location, process->masterTLS.actualSize);

        UserThread *userThread = (UserThread *)(thread->tls.start +
                                                ALIGN_UP(process->masterTLS.totalSize, alignment));
        userThread->self = userThread;
        thread->tls.userThread = (virtual_address_t)userThread;

        virtual_address_t argv = process->processAllocator->allocateStack(4096, false);

        const char *name = "heyo";
        char *stackStr = (char *)thread->stack.end;

        // envp
        stackStr -= sizeof(char *);
        *stackStr = NULL;

        // argv
        memcpy((void *)argv, name, strlen(name) + 1);
        stackStr -= sizeof(char *);
        *(char **)stackStr = (char *)argv;
        argv += strlen(name) + 1;

        // argc
        stackStr -= sizeof(char *);
        *(int *)stackStr = 1;

        // The context of the thread is on the top of the stack (stack is going downwards - end is the top)
        thread->context = (k_thread_state *)((virtual_address_t)stackStr - sizeof(k_thread_state));
    }

    // Stack always start at the topmost address and goes down
    thread->context->rbp = thread->stack.end; // TODO: this might cause a problem
    // Set the stack pointer of the thread
    thread->context->rsp = (register_t)thread->context + sizeof(k_thread_state);
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
    thread->context->rflags |= RFLAGS_IF | RFLAGS_ALWAYS_ON;

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
        if (previousThread != NULL && previousThread != threadToRun)
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
        thread->context->rflags = RFLAGS_IOPL;
    }
    else if (thread->privilege == USER)
    {
        thread->context->cs = 0x18 | 3;
        thread->context->ss = 0x20 | 3;
        thread->context->gs = 0x20 | 3;
        thread->context->fs = 0x20 | 3;
        thread->context->ds = 0x20 | 3;
    }
}

k_process *taskingGetFocusedProcess()
{
    return focusedProcess;
}

void taskingSetFocusedProcess(k_process *proc) {
    focusedProcess = proc;
}

void taskingFinalize() {
    for (uint8_t processorI = 0; processorI < processorCout; processorI++)
    {
        k_processor_tasking processor = processorTaskingArray[processorI];
        k_process_entry *entry = processor.processes;
        while (entry)
        {
            k_process *process = entry->process;
            uint8_t fd;
            for (uint8_t fd = 0; fd < process->fileDescriptors->size(); fd++)
                f_close(process->fileDescriptors->get(fd));
            
            entry = entry->next;
        }
    }
    
}