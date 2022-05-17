#include <tasking/scheduler.hpp>

#include <memory/heap.hpp>
#include <interrupts/lapic.hpp>

k_jobs_queue jobsQueues[K_CONST_SCHEDULER_QUEUES];
k_scheduler_job *runningJob;
uint64_t priorityBoostTime;

void schedulerInit()
{
    for (job_priority_t priority = 0; priority < K_CONST_SCHEDULER_QUEUES; priority++)
    {
        jobsQueues[priority].head = NULL;
        jobsQueues[priority].tail = NULL;
        jobsQueues[priority].isEmpty = true;
        jobsQueues[priority].timeAllotment = schedulerGetTimeAllotment(priority);
    }

    priorityBoostTime = 0;
    runningJob = NULL;
}

uint64_t schedulerGetTimeAllotment(uint8_t priority)
{
    return (priority * (K_CONST_MAXIMUM_TIMESLICE - K_CONST_MINIMUM_TIMESLICE) /
            K_CONST_SCHEDULER_QUEUES) +
           K_CONST_MINIMUM_TIMESLICE;
}

k_thread *schedulerSchedule()
{
    if (runningJob != NULL)
        runningJob->timeInPriority += APIC_TIMER_TIMESLOT_MS;
    priorityBoostTime += APIC_TIMER_TIMESLOT_MS;
    if (priorityBoostTime > K_CONST_PRIORITY_BOOST)
        schedulerPriorityBoost();

    if (runningJob == NULL ||
        runningJob->thread->status == THREAD_STATUS::WAITING ||
        runningJob->thread->status == THREAD_STATUS::STOPPED ||
        runningJob->timeInPriority > jobsQueues[runningJob->priority].timeAllotment)
    {
        if (runningJob != NULL)
        {
            if (runningJob->thread->status == THREAD_STATUS::STOPPED)
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
                // Now add it to one lower priority
                schedulerAddJob(runningJob, runningJob->priority + 1);
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

            // Round-robin on the highest priority queue
            // Therefore the current job to run is the first job
            runningJob = jobsQueues[priority].head;

            // Move it to the queue's end
            schedulerRemoveJob(runningJob);
            schedulerAddJob(runningJob, priority);
        }
    }

    if (runningJob == NULL)
        return NULL;
    return runningJob->thread;
}

void schedulerPriorityBoost()
{
    for (job_priority_t priority = 0; priority < K_CONST_SCHEDULER_QUEUES - 1; priority++)
    {
        k_scheduler_job *job = jobsQueues[priority].head;
        while (job->next)
        {
            // If it's already in priority 0, just set the time to 0
            job->timeInPriority = 0;
            if (priority != 0 && job != runningJob)
            {
                schedulerRemoveJob(job);
                schedulerAddJob(job, 0);
            }
        }
    }
}

void schedulerRemoveJob(k_scheduler_job *job)
{
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
}

void schedulerAddJob(k_scheduler_job *job, job_priority_t priority)
{
    // Set it's priority
    job->priority = priority;

    // Add it to the end of the list
    job->prev = jobsQueues->tail;
    job->next = NULL;
    if (jobsQueues[priority].tail != NULL)
        jobsQueues[priority].tail->next = job;
    jobsQueues[priority].tail = job;
}

void schedulerNewJob(k_thread *thread)
{
    k_scheduler_job *job = new k_scheduler_job();

    job->thread = thread;

    // All new jobs starts with priority 0
    job->timeInPriority = 0;
    schedulerAddJob(job, 0);
}