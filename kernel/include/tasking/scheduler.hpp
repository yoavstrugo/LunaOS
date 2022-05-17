#pragma once

#include <tasking/tasking.hpp>
#include <stddef.h>

/**
 * @brief An implementation of the MLFQ scheduler
 * 
 */

#define K_CONST_SCHEDULER_QUEUES 10

#define K_CONST_MINIMUM_TIMESLICE 20
#define K_CONST_MAXIMUM_TIMESLICE 100
#define K_CONST_PRIORITY_BOOST 1000

typedef uint8_t job_priority_t;

struct k_scheduler_job
{
    k_thread *thread;
    job_priority_t priority;
    uint64_t timeInPriority;

    k_scheduler_job *prev;
    k_scheduler_job *next;
};

struct k_jobs_queue
{
    k_scheduler_job *head = NULL;
    k_scheduler_job *tail = NULL;
    uint64_t timeAllotment;
    bool isEmpty = false;
};

/**
 * @brief Initialize the scheduler
 */
void schedulerInit();

/**
 * @briefGet the time allotment that corresponds to the priority
 *
 * @param job_priority_t The priority
 * @return uint64_t The time allotment
 */
uint64_t schedulerGetTimeAllotment(job_priority_t priority);

/**
 * @brief Decide the next thread to run
 *
 * @return k_thread The selected thread to run
 */
k_thread *schedulerSchedule();

/**
 * @brief Boost all the jobs to priority 0
 */
void schedulerPriorityBoost();

/**
 * @brief Add the job to the end of the queue with the priority
 *
 * @param job The job to add
 * @param priority  The queue's priority
 */
void schedulerAddJob(k_scheduler_job *job, job_priority_t priority);

/**
 * @brief Removes the job from it's current priority
 *
 * @param job The job to remove
 */
void schedulerRemoveJob(k_scheduler_job *job);

/**
 * @brief Adds a new job to the scheduler
 */
void schedulerNewJob(k_thread *thread);