#include <tasking/scheduler.hpp>

#include <memory/heap.hpp>
#include <interrupts/lapic.hpp>

#include <logger/logger.hpp>

// #define VERBOSE_SCHEDULER

k_jobs_queue jobsQueues[K_CONST_SCHEDULER_QUEUES];
k_scheduler_job *runningJob;
uint64_t priorityBoostTime;
static bool initialized = false;

void schedulerInit()
{
    for (job_priority_t priority = 0; priority < K_CONST_SCHEDULER_QUEUES; priority++)
    {
        jobsQueues[priority].head = NULL;
        jobsQueues[priority].tail = NULL;
        jobsQueues[priority].isEmpty = true;
        jobsQueues[priority].timeAllotment = 50;//schedulerGetTimeAllotment(priority);
    }

    priorityBoostTime = 0;
    runningJob = NULL;
    initialized = true;
}

uint64_t schedulerGetTimeAllotment(uint8_t priority)
{
    if (!initialized)
        return 0;

    return ((priority + 1) * (K_CONST_MAXIMUM_TIMESLICE - K_CONST_MINIMUM_TIMESLICE) /
            K_CONST_SCHEDULER_QUEUES) +
           K_CONST_MINIMUM_TIMESLICE;
}

static uint64_t i = 0;

void schedulerTime()
{
    if (!initialized)
        return;

    if (runningJob != NULL)
        runningJob->timeInPriority += APIC_TIMER_TIMESLOT_MS;
    priorityBoostTime += APIC_TIMER_TIMESLOT_MS;
    if (priorityBoostTime > K_CONST_PRIORITY_BOOST) {
        schedulerPriorityBoost();
        priorityBoostTime = 0;
    }
}



k_thread *schedulerSchedule()
{
    if (!initialized)
        return NULL;

    if (runningJob == NULL ||
        runningJob->thread->status == WAITING ||
        runningJob->thread->status == DEAD ||
        runningJob->timeInPriority > jobsQueues[runningJob->priority].timeAllotment)
    {
        if (runningJob != NULL)
        {
            if (runningJob->thread->status == DEAD)
            {
                // The thread had stopped, remove it
                schedulerRemoveJob(runningJob);
                runningJob = NULL;
            }
            // Job has used all of it's time in the priority, lower it
            else if (runningJob->timeInPriority > jobsQueues[runningJob->priority].timeAllotment &&
                     runningJob->priority < K_CONST_SCHEDULER_QUEUES)
            {
                // First remove it from the current priority
                schedulerRemoveJob(runningJob);
                runningJob->thread->status = READY;

                // Kernel jobs shouldn't be demoted, but only go to the "back of the line"
                if (runningJob->thread->privilege == USER && runningJob->priority < K_CONST_SCHEDULER_QUEUES - 1)
                    // Now add it to one lower priority
                    schedulerAddJob(runningJob, runningJob->priority + 1);
                else 
                    schedulerAddJob(runningJob, runningJob->priority);

                // Reset it's time
                runningJob->timeInPriority = 0;
            }
        }
        // Select the next job to run
        for (job_priority_t priority = 0; priority < K_CONST_SCHEDULER_QUEUES; priority++)
        {
            // We want to select job with highest priority
            if (jobsQueues[priority].isEmpty)
                continue;

            k_scheduler_job *job = jobsQueues[priority].head;

            // Get the next ready to run job
            while (job && job->thread->status != READY)
            {
                // Remove the job since it's not ready to be executed
                schedulerRemoveJob(runningJob);

                // If the job isn't dead move it to the queue's end
                if (job->thread->status != DEAD)
                    schedulerAddJob(runningJob, priority);
            }
            i++;
#ifdef VERBOSE_SCHEDULER
            logDebugn("%d) Selected job to run PID: %d TID: %d", i,job->thread->process->pid, job->thread->id);
#endif

            // Round-robin on the highest priority queue
            // Therefore the current job to run is the first job
            runningJob = job;
            break;
        }
    }

    if (runningJob == NULL)
        return NULL;
    return runningJob->thread;
}

void schedulerPriorityBoost()
{
    if (!initialized)
        return;

    for (job_priority_t priority = 0; priority < K_CONST_SCHEDULER_QUEUES - 1; priority++)
    {
        k_scheduler_job *job = jobsQueues[priority].head;
        while (job)
        {
            // If it's already in priority 0, just set the time to 0
            job->timeInPriority = 0;
            if (priority != 0 && job != runningJob)
            {
                schedulerRemoveJob(job);
                schedulerAddJob(job, 0);
            }

            job = job->next;
        }
    }
}

void schedulerRemoveJob(k_scheduler_job *job)
{
    if (!initialized)
        return;

    k_scheduler_job *prev = job->prev;
    k_scheduler_job *next = job->next;
    job_priority_t priority = job->priority;

    if (prev != NULL)
        prev->next = job->next;
    else
        jobsQueues[priority].head = job->next;

    if (next != NULL)
        next->prev = job->prev;
    else
        jobsQueues[priority].tail = job->prev;

    if (jobsQueues[priority].head == NULL)
        jobsQueues[priority].isEmpty = true;
}

void schedulerAddJob(k_scheduler_job *job, job_priority_t priority)
{
    if (!initialized)
        return;

    // Set it's priority
    job->priority = priority;

    // Add it to the end of the list
    job->prev = jobsQueues->tail;
    job->next = NULL;
    if (jobsQueues[priority].tail != NULL)
        jobsQueues[priority].tail->next = job;
    if (jobsQueues[priority].head == NULL)
        jobsQueues[priority].head = job;
    jobsQueues[priority].tail = job;
    jobsQueues[priority].isEmpty = false;
}

void schedulerNewJob(k_thread *thread)
{
    if (!initialized)
        return;

    k_scheduler_job *job = new k_scheduler_job();

    job->thread = thread;

    // All new jobs starts with priority 0
    job->timeInPriority = 0;
    schedulerAddJob(job, 0);
}