#include <tasking/tasking.hpp>

#include <memory/memory.hpp>
#include <memory/heap.hpp>
#include <logger/logger.hpp>

static k_processor_tasking *processorTaskingArray;

uint64_t k_processor_tasking::getNextPID()
{
    return this->nextPID++;
}

void taskingInitialize(uint8_t numOfCPUs)
{
    processorTaskingArray = new k_processor_tasking[numOfCPUs];
    for (int i = 0; i < numOfCPUs; i++)
        memset((char *)processorTaskingArray, 0, numOfCPUs * sizeof(k_processor_tasking));
}

void taskingAddCPU(uint8_t id)
{
    processorTaskingArray[id].currentThread = NULL;
    processorTaskingArray[id].processes = 0;
    processorTaskingArray[id].processorId = id;
    processorTaskingArray[id].threads = NULL;
}

k_process *taskingCreateProcess()
{
    k_process *process = (k_process *)heapAllocate(sizeof(k_process));
    
    // 2 MiB for code, should change to ELF size
    uint64_t codeSize = 2 * MiB;
    process->processAllocator.allocateUserspaceCode(codeSize);
    process->processAllocator.allocateUserspaceHeap();

    process->pid = processorTaskingArray[0].getNextPID();

    k_process_entry *entry = new k_process_entry();
    entry->process = process;
    entry->next = processorTaskingArray[0].processes;
    processorTaskingArray[0].processes = entry;

    return process;
}

k_thread *taskingCreateThread(virtual_address_t entryPoint, k_process *process) {
    k_thread *thread = new k_thread();
    thread->process = process;
    thread->stack.start = process->processAllocator.allocateStack(USERSPACE_STACK_SIZE);
    thread->stack.end = thread->stack.start + USERSPACE_STACK_SIZE;
    thread->status = READY;

    k_thread_entry *entry = new k_thread_entry();
}

void taskingDumpProcesses()
{
    k_processor_tasking *processorTasking = processorTaskingArray;
    k_process_entry *curr = processorTasking->processes;

    logInfon("%! Dumpinf processes:", "[Tasking]");
    while (curr)
    {
        logInfon("\t- PID: %d", curr->process->pid);
        curr = curr->next;
    }
}