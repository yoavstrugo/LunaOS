#pragma once

namespace Syscall
{
    // Data structures
    struct SyscallData
    {
        bool result;
        int errno;
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

    struct ReadData : SyscallData
    {
        // Params
        unsigned int fd;
        void *buf;
        unsigned int count;

        // To set
        unsigned int byteRead;
    };
}
