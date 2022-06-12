#pragma once

#include <stdint.h>

#include <syscalls/syscalls_data.hpp>

struct k_thread;

namespace Syscall
{
    typedef void (*SyscallHandler_t)(k_thread *thread, SyscallData *);

    /**
     * @brief Handles the system call for the given thread
     *
     * @param thread The thread that initiated the system call
     */
    void handle(k_thread *thread);

    /**
     * @brief Register a system call
     *
     * @param id The id for the system call
     * @param handler The handler for the system call
     */
    void registerHandler(uint16_t id, SyscallHandler_t handler);

    /**
     * @brief Registers all system calls
     *
     */
    void initialize();

    /**
     * @brief Run the syscall for the thread
     *
     * @param thread    The thread
     * @param syscall   The syscall
     * @param data      The data for the system call
     */
    void runHandler(k_thread *thread, SyscallHandler_t syscall, void *data);
}
