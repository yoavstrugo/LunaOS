#include <syscalls/syscalls_tasking.hpp>

#include <tasking/scheduler.hpp>

void syscallExit(k_thread *thread, syscall_data_exit *data) {
    thread->status = DEAD;
    taskingSwitch();
}