#pragma once

#include <abi-bits/pid_t.h>
#include <abi-bits/uid_t.h>
#include <abi-bits/gid_t.h>
#include <abi-bits/time_t.h>
#include <stddef.h>

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

typedef enum
{
    FR_OK = 0,              /* (0) Succeeded */
    FR_DISK_ERR,            /* (1) A hard error occurred in the low level disk I/O layer */
    FR_INT_ERR,             /* (2) Assertion failed */
    FR_NOT_READY,           /* (3) The physical drive cannot work */
    FR_NO_FILE,             /* (4) Could not find the file */
    FR_NO_PATH,             /* (5) Could not find the path */
    FR_INVALID_NAME,        /* (6) The path name format is invalid */
    FR_DENIED,              /* (7) Access denied due to prohibited access or directory full */
    FR_EXIST,               /* (8) Access denied due to prohibited access */
    FR_INVALID_OBJECT,      /* (9) The file/directory object is invalid */
    FR_WRITE_PROTECTED,     /* (10) The physical drive is write protected */
    FR_INVALID_DRIVE,       /* (11) The logical drive number is invalid */
    FR_NOT_ENABLED,         /* (12) The volume has no work area */
    FR_NO_FILESYSTEM,       /* (13) There is no valid FAT volume */
    FR_MKFS_ABORTED,        /* (14) The f_mkfs() aborted due to any problem */
    FR_TIMEOUT,             /* (15) Could not get a grant to access the volume within defined period */
    FR_LOCKED,              /* (16) The operation is rejected according to the file sharing policy */
    FR_NOT_ENOUGH_CORE,     /* (17) LFN working buffer could not be allocated */
    FR_TOO_MANY_OPEN_FILES, /* (18) Number of open files > FF_FS_LOCK */
    FR_INVALID_PARAMETER    /* (19) Given parameter is invalid */
} FRESULT;

#define FA_READ 0x01
#define FA_WRITE 0x02
#define FA_OPEN_EXISTING 0x00
#define FA_CREATE_NEW 0x04
#define FA_CREATE_ALWAYS 0x08
#define FA_OPEN_ALWAYS 0x10
#define FA_OPEN_APPEND 0x30

typedef long off_t;

namespace Luna
{
    // Data structures
    struct SyscallData
    {
        bool result;
        int err;
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

    struct ReadData : SyscallData
    {
        // Params
        unsigned int fd;
        void *buf;
        unsigned int count;

        // To set
        unsigned int byteRead;
    };

    struct WriteData : SyscallData 
    {
        int fd;
        void const *buf;
        unsigned long btw;
        long *bw;
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

    static inline void syscall(unsigned long long call, SyscallData *data)
    {
        // Syscall number is passed in 'rax'
        // Syscall data is passed in 'rbx'
        asm volatile(
            "int $0x80" ::
                "a"(call),
            "b"(data));
    }
}
