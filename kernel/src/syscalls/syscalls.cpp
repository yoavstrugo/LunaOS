#include <syscalls/syscalls.hpp>

#include <kernel.hpp>
#include <stddef.h>
#include <logger/logger.hpp>
#include <strings.hpp>

#include <syscalls/syscalls_tasking.hpp>

static syscall_handler_t systemCalls[K_SYSTEM_CALL_COUNT];

void syscallHandle(k_thread *thread)
{
    // The system call id is saved on rax
    uint16_t syscallId = thread->context->rax;
    // The data for the system call will be stored on rbx
    void *syscallData = (void *)thread->context->rbx;

    syscallRun(thread, systemCalls[thread->context->rax], syscallData);
}

void syscallRun(k_thread *caller, syscall_handler_t syscall, void *data)
{
    // The thread is waiting for the system call to end
    caller->status = WAITING;

    k_thread *syscallThread =
        taskingCreateThread((virtual_address_t)syscallThread, caller->process, KERNEL);

    syscallThread->syscall.handler = syscall;
    syscallThread->syscall.data = data;
    syscallThread->syscall.targetThread = caller;
    // TODO: ?syscallThread->context.rip = (register_t)syscallThread;
}

void syscallThread()
{
    k_processor_tasking *processor = taskingGetProcessor();
    k_thread *syscallThread = processor->currentThread;
    void *syscallData = syscallThread->syscall.data;
    k_thread *targetThread = syscallThread->syscall.targetThread;

    syscallThread->syscall.handler(targetThread, syscallData);

    targetThread->status = READY;
    syscallThread->status = DEAD;
    taskingSwitch();
}

void syscallRegister(uint16_t id, syscall_handler_t handler)
{
    if (id > K_SYSTEM_CALL_COUNT)
    {
        kernelPanic("%! Tried to register system call with id %d but maximum is %d", "[System Calls]", id, K_SYSTEM_CALL_COUNT);
    }

    if (systemCalls[id] != NULL)
        logDebugn("%! Overriding an existing system call with id %d.", "[System Calls]", id);

    // Set the system call
    systemCalls[id] = handler;
}

void syscallInitialize()
{
    // Set everything to null
    memset((char *)systemCalls, 0, sizeof(systemCalls));

    syscallRegister(SYSTEMCALL_EXIT, (syscall_handler_t)syscallExit);
}