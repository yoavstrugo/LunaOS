#include <syscalls/syscalls_tasking.hpp>

#include <tasking/scheduler.hpp>

#include <logger/logger.hpp>

void syscallExit(k_thread *thread, syscall_data_exit *data) {
    thread->status = DEAD;
    taskingSwitch();
}

void syscallFork(k_thread *thread, syscall_data_fork *data) {
    logInfon("Fork systemcall not implemented");
}