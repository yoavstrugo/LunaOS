#pragma once

#include <stdint.h>
#include <../../lunaapi/calls.hpp>

struct k_thread;

typedef void (*syscall_handler_t)(k_thread *thread, void*);

/**
 * @brief Handles the system call for the given thread
 * 
 * @param thread The thread that initiated the system call
 */
void syscallHandle(k_thread *thread);

/**
 * @brief Register a system call
 * 
 * @param id The id for the system call
 * @param handler The handler for the system call
 */
void syscallRegister(uint16_t id, syscall_handler_t handler);

/**
 * @brief Registers all system calls
 * 
 */
void syscallInitialize();

/**
 * @brief Run the syscall for the thread
 * 
 * @param thread    The thread
 * @param syscall   The syscall
 * @param data      The data for the system call
 */
void syscallRun(k_thread *thread, syscall_handler_t syscall, void *data);