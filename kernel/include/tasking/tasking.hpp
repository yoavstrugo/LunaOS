#pragma once

#include <types.hpp>
#include <stdint.h>
#include <memory/userspace_allocator.hpp>

#define USERSPACE_IMAGE_END 0xFFFF800000000000
#define USERSPACE_HEAP_INITIAL_SIZE 1 * GiB

struct k_process;
struct k_thread;

typedef uint64_t register_t;

enum THREAD_STATUS
{
    // If the thread is running
    RUNNING,
    // If the thread is ready to run
    READY,
    // If the thread is waiting for something (IO for example)
    WAITING,
    // If the thread had stopped running
    STOPPED
};

struct k_thread
{
    /**
     * @brief The process that this thread belongs to
     */
    k_process *process;

    // The thread's status
    THREAD_STATUS status;

    /**
     * @brief The thread's stack
     */
    struct
    {
        // Where it starts
        virtual_address_t start;
        // Where it ends
        virtual_address_t end;
    } stack;

    /**
     * @brief The state of the thread, the general-purpose registers
     */
    struct
    {
        register_t rax;
        register_t rbx;
        register_t rcx;
        register_t rdx;

        register_t rsi;
        register_t rdi;

        register_t rsp;
        register_t rbp;

        register_t r8;
        register_t r9;
        register_t r10;
        register_t r11;
        register_t r12;
        register_t r13;
        register_t r14;
        register_t r15;
    } state;
};

/**
 * @brief A linked list for threads
 */
struct k_thread_entry
{
    k_thread *thread;
    k_thread_entry *next;
};

struct k_process
{
    // The process' id
    uint64_t pid;

    // The address space of the process
    physical_address_t addressSpace;

    // This is the current thread of the process
    k_thread *currentThread;
    // The list of the process' threads
    k_thread_entry *threads;

    k_userspace_allocator processAllocator;
};

/**
 * @brief A linked list for processes
 */
struct k_process_entry
{
    k_process *process;
    k_process_entry *next;
};

/**
 * @brief This structures holds all the tasking information for a single processor
 *
 */
struct k_processor_tasking
{
    /**
     * @brief The processor's id
     */
    uint8_t processorId;

    /**
     * @brief The current running thread
     */
    k_thread *currentThread;

    /**
     * @brief A list of all the processor's threads
     */
    k_thread_entry *threads;

    /**
     * @brief A list of all the processor's processes
     */
    k_process_entry *processes;

    /**
     * @brief Returns the next PID for process
     * 
     * @return uint64_t The PID
     */
    uint64_t getNextPID();

    private:
        /**
         * @brief The next process' id
         */
        uint64_t nextPID = 0;
};

void taskingDumpProcesses();

/**
 * @brief Initialize tasking
 * 
 * @param numOfCPUs How many cpus aer in the PC
 */
void taskingInitialize(uint8_t numOfCPUs);

/**
 * @brief Add a CPU to the list
 * 
 * @param id The id of the CPU  
 */
void taskingAddCPU(uint8_t id);

/**
 * @brief Creates an "empty" process
 *
 * @return k_process* The process has been created
 */
k_process *taskingCreateProcess();

/**
 * @brief Creates a thread
 *
 * @param entryPoint The entry point to the thread
 * @param process The process which the thread belong to
 * @return k_thread* The created thread
 */
k_thread *taskingCreateThread(virtual_address_t entryPoint, k_process *process);