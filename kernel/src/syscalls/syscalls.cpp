#include <syscalls/syscalls.hpp>

#include <kernel.hpp>
#include <stddef.h>
#include <logger/logger.hpp>
#include <strings.hpp>

#include <tasking/tasking.hpp>

#include <syscalls/syscalls_calls.hpp>
#include <syscalls/syscalls_data.hpp>

namespace Syscall
{
    static SyscallHandler_t systemCalls[100];

    void handle(k_thread *thread)
    {
        // The system call id is saved on rax
        uint16_t syscallId = thread->context->rax;
        // The data for the system call will be stored on rbx
        SyscallData *syscallData = (SyscallData *)thread->context->rbx;

        // runHandler(thread, systemCalls[thread->context->rax], syscallData);
        pagingSwitchSpace(thread->process->addressSpace);
        systemCalls[syscallId](thread, syscallData);
    }

    void runHandler(k_thread *caller, SyscallHandler_t syscall, SyscallData *data)
    {
        // The thread is waiting for the system call to end
        caller->status = WAITING;

        k_thread *syscallThread =
            taskingCreateThread((virtual_address_t)syscallThread, caller->process, KERNEL);

        syscallThread->syscall.handler = syscall;
        syscallThread->syscall.data = data;
        syscallThread->syscall.targetThread = caller;
        syscallThread->context->rip = (register_t)syscallThread;
    }

    void syscallThread()
    {
        k_processor_tasking *processor = taskingGetProcessor();
        k_thread *syscallThread = processor->currentThread;
        SyscallData *syscallData = syscallThread->syscall.data;
        k_thread *targetThread = syscallThread->syscall.targetThread;

        syscallThread->syscall.handler(targetThread, syscallData);

        targetThread->status = READY;
        syscallThread->status = DEAD;
        taskingSwitch();
    }

    void registerHandler(uint16_t id, SyscallHandler_t handler)
    {
        if (id > 255)
        {
            kernelPanic("%! Tried to register system call with id %d but maximum is %d", "[System Calls]", id, 100);
        }

        if (systemCalls[id] != NULL)
            logDebugn("%! Overriding an existing system call with id %d.", "[System Calls]", id);

        // Set the system call
        systemCalls[id] = handler;
    }

    void syscallPrintString(k_thread *thread, char *data)
    {
        logInfo("%s", data);
    }

    void initialize()
    {
        // Set everything to null
        memset((char *)systemCalls, 0, sizeof(systemCalls));

        registerHandler(SYS_DEBUG, (SyscallHandler_t)Calls::debug);
        registerHandler(SYS_FUTEX_TID, (SyscallHandler_t)Calls::futexTID);
        registerHandler(SYS_FUTEX_WAIT, (SyscallHandler_t)Calls::futexWait);
        registerHandler(SYS_FUTEX_WAKE, (SyscallHandler_t)Calls::futexWake);
        registerHandler(SYS_TCB_SET, (SyscallHandler_t)Calls::tcbSet);
        registerHandler(SYS_VM_MAP, (SyscallHandler_t)Calls::vmMap);
        registerHandler(SYS_VM_UNMAP, (SyscallHandler_t)Calls::vmUnmap);
        registerHandler(SYS_EXIT, (SyscallHandler_t)Calls::exit);
        registerHandler(SYS_GETPID, (SyscallHandler_t)Calls::getpid);
        registerHandler(SYS_GETPPID, (SyscallHandler_t)Calls::getppid);
        registerHandler(SYS_CLOCK_GET, (SyscallHandler_t)Calls::clockGet);
        registerHandler(SYS_GETCWD, (SyscallHandler_t)Calls::getcwd);
        registerHandler(SYS_CHDIR, (SyscallHandler_t)Calls::chdir);
        registerHandler(SYS_SLEEP, (SyscallHandler_t)Calls::sleep);
        registerHandler(SYS_GETUID, (SyscallHandler_t)Calls::getuid);
        registerHandler(SYS_GETEUID, (SyscallHandler_t)Calls::geteuid);
        registerHandler(SYS_SETUID, (SyscallHandler_t)Calls::setuid);
        registerHandler(SYS_SETEUID, (SyscallHandler_t)Calls::seteuid);
        registerHandler(SYS_GETGID, (SyscallHandler_t)Calls::getgid);
        registerHandler(SYS_GETEGID, (SyscallHandler_t)Calls::getegid);
        registerHandler(SYS_SETGID, (SyscallHandler_t)Calls::setgid);
        registerHandler(SYS_SETEGID, (SyscallHandler_t)Calls::setegid);
        registerHandler(SYS_YIELD, (SyscallHandler_t)Calls::yield);
        registerHandler(SYS_CLONE, (SyscallHandler_t)Calls::clone);
        registerHandler(SYS_THREAD_EXIT, (SyscallHandler_t)Calls::threadExit);
        registerHandler(SYS_WAITPID, (SyscallHandler_t)Calls::waitpid);
        registerHandler(SYS_FORK, (SyscallHandler_t)Calls::fork);
        registerHandler(SYS_EXECVE, (SyscallHandler_t)Calls::execve);
        registerHandler(SYS_OPEN, (SyscallHandler_t)Calls::open);
        registerHandler(SYS_CLOSE, (SyscallHandler_t)Calls::close);
        registerHandler(SYS_READ, (SyscallHandler_t)Calls::read);
        registerHandler(SYS_WRITE, (SyscallHandler_t)Calls::write);
    }
}
