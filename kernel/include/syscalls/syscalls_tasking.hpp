#pragma once

#include <tasking/tasking.hpp>

struct syscall_data_exit {
    // The exit code
    int code;
};
void syscallExit(k_thread *thread, syscall_data_exit *data);

struct syscall_data_fork;
void syscallFork(k_thread *thread, syscall_data_fork *data);