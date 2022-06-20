#pragma once

#include <stddef.h>

namespace Syscall
{
    typedef long off_t;
    typedef int time_t;

    typedef int pid_t;
    typedef int uid_t;
    typedef int pgid_t;
    typedef int sid_t;
    typedef int gid_t;
    typedef int euid_t;
    typedef int egid_t;

    // Data structures
    struct SyscallData
    {
        bool result;
        int errno;
    };

    struct OpenDir : SyscallData
    {
        unsigned int fd;
        const char *path;
    };

    struct DebugData : SyscallData
    {
        const char *message;
    };

    struct ExitData : SyscallData
    {
        int status;
    };

    struct ForkData : SyscallData
    {
        pid_t pid;
    };

    struct ExecveData : SyscallData
    {
        const char *pathname;
        char *const *argv;
        char *const *envp;
    };

    struct WaitPIDData : SyscallData
    {
        pid_t pid;
        int *wstatus;
        int options;
    };

    struct GetPIDData : SyscallData
    {
        pid_t pid;
    };

    struct OpenData : SyscallData
    {
        // Params
        char *name;
        int flags;

        // To set
        unsigned int fd;
    };

    struct CloseData : SyscallData
    {
        // Params
        unsigned int fd;
    };

    struct WriteData : SyscallData 
    {
        int fd;
        void const *buf;
        unsigned long btw;
        long *bw;
    };

    struct ReadData : SyscallData
    {
        // Params
        unsigned int fd;
        void *buf;
        unsigned int count;

        // To set
        unsigned int byteRead;
    };

    struct GetCWDData : SyscallData
    {
        char *cwd;
        size_t size;
    };

    struct ReadDir : SyscallData
    {
        unsigned int fd;
        unsigned int inode;
        const char *name;
        const char *type;
    };

    struct CloneData : SyscallData
    {
        void *entry;
        void *userArg;
        void *tcb;
        pid_t *pidOut;
    };

    struct SetUIDData : SyscallData
    {
        uid_t uid;
    };

    struct SetEUIDData : SyscallData
    {
        uid_t euid;
    };

    struct SetGIDData : SyscallData
    {
        gid_t gid;
    };

    struct SetEGIDData : SyscallData
    {
        gid_t egid;
    };

    struct FutexWaitData : SyscallData
    {
        int *pointer;
        int expected;
        const struct timespec *time;
    };

    struct FutexWakeData : SyscallData
    {
        int *pointer;
    };

    struct TCBSetData : SyscallData
    {
        void *pointer;
    };

    struct VMMapData : SyscallData
    {
        void *hint;
        size_t size;
        int prot;
        int flags;
        int fd;
        off_t offset;
        void **window;

        void *allocatedRange;
    };

    struct VMUnmapData : SyscallData
    {
        void *pointer;
        size_t size;
    };

    struct ClockGetData : SyscallData
    {
        int clock;
        time_t *secs;
        long *nanos;
    };

    struct ChdirData : SyscallData
    {
        const char *path;
    };

    struct SleepData : SyscallData
    {
        time_t *secs;
        long *nanos;
    };
}
