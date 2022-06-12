#pragma once

#include <syscalls/syscalls_data.hpp>
#include <tasking/tasking.hpp>

#define SYS_FUTEX_TID 0
#define SYS_FUTEX_WAIT 1
#define SYS_FUTEX_WAKE 2
#define SYS_TCB_SET 3
#define SYS_VM_MAP 4
#define SYS_VM_UNMAP 5
#define SYS_ANON_ALLOCATE 6
#define SYS_ANON_FREE 7
#define SYS_LIBC_PANIC 8
#define SYS_LIBC_LOG 9
#define SYS_EXIT 10
#define SYS_GETPID 11
#define SYS_GETPPID 12
#define SYS_CLOCK_GET 13
#define SYS_GETCWD 14
#define SYS_CHDIR 15
#define SYS_SLEEP 16
#define SYS_GETUID 17
#define SYS_GETEUID 18
#define SYS_SETUID 19
#define SYS_SETEUID 20
#define SYS_GETGID 21
#define SYS_GETEGID 22
#define SYS_SETGID 23
#define SYS_SETEGID 24
#define SYS_YIELD 25
#define SYS_CLONE 26
#define SYS_THREAD_EXIT 27
#define SYS_WAITPID 28
#define SYS_FORK 29
#define SYS_EXECVE 30
#define SYS_GETENTROPY 31
#define SYS_OPEN 32
#define SYS_CLOSE 33
#define SYS_READ 34
#define SYS_WRITE 35

#define SYS_DEBUG 255

namespace Syscall
{
    namespace Calls
    {
        void 
        debug(k_thread *thread, DebugData *data); 

        void
        open(k_thread *thread, OpenData *data);

        void
        close(k_thread *thread, CloseData *data);

        void
        read(k_thread *thread, ReadData *data);

        void 
        write(k_thread *thread, WriteData *data);

        void
        exit(k_thread *thread, ExitData *data);

        void
        threadExit(k_thread *thread, SyscallData *data);

        void
        fork(k_thread *thread, ForkData *data);

        void
        execve(k_thread *thread, ExecveData *data);

        void
        yield(k_thread *thread, SyscallData *data);

        void
        clone(k_thread *thread, CloneData *data);

        void
        waitpid(k_thread *thread, WaitPIDData *data);

        void
        getpid(k_thread *thread, GetPIDData *data);

        void
        getppid(k_thread *thread, SyscallData *data);

        void
        getuid(k_thread *thread, SyscallData *data);

        void
        geteuid(k_thread *thread, SyscallData *data);

        void
        getgid(k_thread *thread, SyscallData *data);

        void
        getegid(k_thread *thread, SyscallData *data);

        void
        setuid(k_thread *thread, SetUIDData *data);

        void
        seteuid(k_thread *thread, SetEUIDData *data);

        void
        setgid(k_thread *thread, SetGIDData *data);

        void
        setegid(k_thread *thread, SetEGIDData *data);

        void
        futexTID(k_thread *thread, SyscallData *data);

        void
        futexWait(k_thread *thread, FutexWaitData *data);

        void
        futexWake(k_thread *thread, FutexWakeData *data);

        void
        tcbSet(k_thread *thread, TCBSetData *data);

        void
        vmMap(k_thread *thread, VMMapData *data);

        void
        vmUnmap(k_thread *thread, VMUnmapData *data);

        void
        clockGet(k_thread *thread, ClockGetData *data);

        void
        getcwd(k_thread *thread, SyscallData *data);

        void
        chdir(k_thread *thread, ChdirData *data);

        void
        sleep(k_thread *thread, SleepData *data);
    }
}