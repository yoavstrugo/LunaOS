#include <syscalls/syscalls_calls.hpp>

#include <logger/logger.hpp>
#include <syscalls/errno.h>
#include <fatfs/ff.h>
#include <memory/heap.hpp>
#include <system/processor/processor.hpp>
#include <strings.hpp>

namespace Syscall::Calls
{
    void printSTDOUT(k_thread *thread, SyscallData *data)
    {
        // 1 is for STDOUT
        FIL *stdout = thread->process->fileDescriptors->get(1);
        long prevPtr = f_tell(stdout);
        f_lseek(stdout, thread->process->stdoutReadPtr);
        size_t size = f_size(stdout) - thread->process->stdoutReadPtr;
        char buf[size + 1];
        unsigned int bw;
        f_read(stdout, buf, size, &bw);
        buf[size] = '\0';

        thread->process->stdoutReadPtr += bw;
        f_lseek(stdout, prevPtr);

        logInfo("%s", buf);
    }

    void readDir(k_thread *thread, ReadDir *data)
    {
        DIR *dir = thread->process->openDirectories->get(data->fd);
        FILINFO info;
        if (dir != NULL)
        {
            if (f_readdir(dir, &info) == FR_OK)
            {
                memcpy((void *)data->name, (void *)info.fname, strlen(info.fname));
                data->result = true;
                return;
            }

            data->result = false;
        }
        data->result = false;
    }

    void openDir(k_thread *thread, OpenDir *data)
    {
        DIR *dir = new DIR();
        FRESULT res = f_opendir(dir, data->path);
        if (res == FR_OK)
        {
            thread->process->openDirectories->add(dir);
            data->fd = thread->process->openDirectories->size() - 1;
            data->result = true;
        }
        else
        {
            data->result = false;
        }
    }

    void getcwd(k_thread *thread, GetCWDData *data)
    {
        memcpy((void *)data->cwd, (void *)thread->process->cwd, 256);
        data->result = true;
    }

    void chdir(k_thread *thread, ChdirData *data)
    {
        FILINFO filInfo;
        FRESULT res = f_stat(data->path, &filInfo);
        if (res == FR_OK)
        {
            memcpy((void *)thread->process->cwd, (void *)data->path, 256);
            data->result = true;
        }
        else
        {
            data->result = false;
        }
    }

    void debug(k_thread *thread, DebugData *data)
    {
        logDebugn("%s", data->message);
        data->result = true;
    }

    void open(k_thread *thread, OpenData *data)
    {
        k_process *proc = thread->process;

        FIL *fil = new FIL();
        FRESULT res = f_open(fil, data->name, data->flags);

        if (res != FR_OK)
        {
            data->result = false;
            return;
        }

        proc->fileDescriptors->add(fil);
        data->fd = proc->fileDescriptors->size() - 1;
        data->result = true;
        return;
    }

    void close(k_thread *thread, CloseData *data)
    {
        k_process *proc = thread->process;

        FIL *fil = proc->fileDescriptors->get(data->fd);
        if (fil == NULL)
        {
            data->result = false;
            return;
        }

        FRESULT res = f_close(fil);
        data->result = true;
        return;
    }

    void read(k_thread *thread, ReadData *data)
    {
        k_process *proc = thread->process;

        FIL *fil = proc->fileDescriptors->get(data->fd);
        if (fil == NULL)
        {
            data->result = false;
            return;
        }

        FRESULT res = f_read(fil, data->buf, data->count, &data->byteRead);
        data->result = true;
        return;
    }

    void write(k_thread *thread, WriteData *data)
    {
        k_process *proc = thread->process;

        FIL *fil = proc->fileDescriptors->get(data->fd);
        if (fil == NULL)
        {
            data->result = false;
            return;
        }

        FRESULT res = f_write(fil, data->buf, data->btw, (unsigned int *)data->bw);
        if (res == FR_OK)
            data->result = true;
        else
            data->result = false;
        return;
    }

    void exit(k_thread *thread, ExitData *data)
    {
        thread->status = DEAD;
        taskingSwitch();
    }

    void threadExit(k_thread *thread, SyscallData *data)
    {
        logInfon("'thread_exit' syscall not implemented");
        data->result = false;
        data->errno = ENOSYS;
    }

    void fork(k_thread *thread, ForkData *data)
    {
        logInfon("'fork' syscall not implemented");
        data->result = false;
        data->errno = ENOSYS;
    }

    void execve(k_thread *thread, ExecveData *data)
    {
        logInfon("'execve' syscall not implemented");
        data->result = false;
        data->errno = ENOSYS;
    }

    void waitpid(k_thread *thread, WaitPIDData *data)
    {
        logInfon("'waitpid' syscall not implemented");
        data->result = false;
        data->errno = ENOSYS;
    }

    void getpid(k_thread *thread, GetPIDData *data)
    {
        logInfon("'getpid' syscall not implemented");
        data->result = false;
        data->errno = ENOSYS;
    }

    void futexTID(k_thread *thread, SyscallData *data)
    {
        logInfon("'futex_tid' syscall not implemented");
        data->result = false;
        data->errno = ENOSYS;
    }

    void futexWait(k_thread *thread, FutexWaitData *data)
    {
        logInfon("'futex_wait' syscall not implemented");
        data->result = false;
        data->errno = ENOSYS;
    }

    void futexWake(k_thread *thread, FutexWakeData *data)
    {
        logInfon("'futex_wake' syscall not implemented");
        data->result = false;
        data->errno = ENOSYS;
    }

    void tcbSet(k_thread *thread, TCBSetData *data)
    {
        // fs_base MSR
        // processorSetMSR(0xC0000100, (virtual_address_t)data->pointer);
        // thread->context->fs_base = (register_t)data->pointer;
        thread->tls.userThread = (virtual_address_t)data->pointer;
        data->result = true;
    }

    void vmMap(k_thread *thread, VMMapData *data)
    {
        if (data->size == 0)
        {
            data->errno = EINVAL;
            data->result = false;
            return;
        }

        k_process *process = thread->process;
        if (data->hint != NULL)
        {
            if (process->processAllocator->allocateRange(
                    (virtual_address_t)data->hint, (virtual_address_t)data->size))
            {
                // Range allocated
                data->allocatedRange = data->hint;
                data->result = true;
                return;
            }
        }

        // Allocate just anywhere
        data->allocatedRange = (void *)process->processAllocator->allocateStack((virtual_address_t)data->size, false);
        if (data->allocatedRange == NULL)
        {
            data->result = false;
            return;
        }

        data->result = true;
    }

    void vmUnmap(k_thread *thread, VMUnmapData *data)
    {
        k_process *process = thread->process;
        process->processAllocator->freeStack((virtual_address_t)data->pointer);

        data->result = true;
    }

    void getppid(k_thread *thread, SyscallData *data)
    {
        logInfon("'getppid' syscall not implemented");
        data->result = false;
        data->errno = ENOSYS;
    }

    void clockGet(k_thread *thread, ClockGetData *data)
    {
        logInfon("'clock_get' syscall not implemented");
        data->result = false;
        data->errno = ENOSYS;
    }

    void sleep(k_thread *thread, SleepData *data)
    {
        logInfon("'sleep' syscall not implemented");
        data->result = false;
        data->errno = ENOSYS;
    }

    void getuid(k_thread *thread, SyscallData *data)
    {
        logInfon("'getuid' syscall not implemented");
        data->result = false;
        data->errno = ENOSYS;
    }

    void geteuid(k_thread *thread, SyscallData *data)
    {
        logInfon("'geteuid' syscall not implemented");
        data->result = false;
        data->errno = ENOSYS;
    }

    void getgid(k_thread *thread, SyscallData *data)
    {
        logInfon("'getgid' syscall not implemented");
        data->result = false;
        data->errno = ENOSYS;
    }

    void getegid(k_thread *thread, SyscallData *data)
    {
        logInfon("'getegid' syscall not implemented");
        data->result = false;
        data->errno = ENOSYS;
    }

    void setuid(k_thread *thread, SetUIDData *data)
    {
        logInfon("'setuid' syscall not implemented");
        data->result = false;
        data->errno = ENOSYS;
    }

    void seteuid(k_thread *thread, SetEUIDData *data)
    {
        logInfon("'seteuid' syscall not implemented");
        data->result = false;
        data->errno = ENOSYS;
    }

    void setgid(k_thread *thread, SetGIDData *data)
    {
        logInfon("'setgid' syscall not implemented");
        data->result = false;
        data->errno = ENOSYS;
    }

    void setegid(k_thread *thread, SetEGIDData *data)
    {
        logInfon("'setegid' syscall not implemented");
        data->result = false;
        data->errno = ENOSYS;
    }

    void yield(k_thread *thread, SyscallData *data)
    {
        logInfon("'yield' syscall not implemented");
        data->result = false;
        data->errno = ENOSYS;
    }

    void clone(k_thread *thread, CloneData *data)
    {
        logInfon("'clone' syscall not implemented");
        data->result = false;
        data->errno = ENOSYS;
    }
}