#include <luna/syscall.h>
#include <stddef.h>
#include <bits/ensure.h>
#include <abi-bits/pid_t.h>
#include <mlibc/debug.hpp>
#include <mlibc/all-sysdeps.hpp>
#include <mlibc/thread-entry.hpp>
#include <errno.h>
#include <sys/resource.h>
#include <dirent.h>

int ff_get_errno(FRESULT res)
{
    switch (res)
    {
    case FR_DISK_ERR:
    case FR_INT_ERR:
    case FR_NOT_READY:
    case FR_NOT_ENABLED:
    case FR_NO_FILESYSTEM:
        return EIO;
    case FR_NO_FILE:
    case FR_NO_PATH:
    case FR_INVALID_NAME:
        return ENOENT;
    case FR_DENIED:
        return EACCES;
    case FR_EXIST:
        return EEXIST;
    case FR_TOO_MANY_OPEN_FILES:
        return ENFILE;
    default:
        return -1;
    }
}

namespace mlibc
{

    void sys_libc_log(char const *msg)
    {
        Luna::DebugData data;
        data.message = msg;
        // Luna::syscall(SYS_DEBUG, &data);
    }

    void sys_libc_panic()
    {
        sys_libc_log("libc panic!");
        __builtin_trap();
        for (;;)
            ;
    }

    int sys_open(const char *pathname, int flags, int *fd)
    {
        int kFlags = 0;

        if (flags & O_RDONLY)
            kFlags |= FA_READ;
        else if (flags & O_WRONLY)
            kFlags |= FA_WRITE;
        else if (flags & O_RDWR)
            kFlags |= FA_READ | FA_WRITE;

        if (flags & O_APPEND)
            kFlags |= FA_OPEN_APPEND;
        if ((flags & O_CREAT) && !(flags & O_EXCL))
            kFlags |= FA_OPEN_ALWAYS;
        else if ((flags & O_CREAT) && (flags & O_EXCL))
            kFlags |= FA_CREATE_NEW;
        else
            kFlags |= FA_OPEN_EXISTING;

        if (flags & O_TRUNC)
            kFlags |= FA_CREATE_ALWAYS;

        Luna::OpenData data;
        data.flags = kFlags;
        data.name = (char *)pathname;

        Luna::syscall(SYS_OPEN, &data);

        if (data.result)
        {
            *fd = data.fd;
            return 0;
        }

        // errno = data.err;
        return -1;
    }

    int sys_open_dir(const char *path, int *handle)
    {
        Luna::OpenDir data;
        data.path = path;
        Luna::syscall(SYS_OPENDIR, &data);

        if (data.result)
        {
            *handle = data.fd;
            return 0;
        }
        return -1;
    }

    int sys_read_entries(int handle, void *buffer, size_t max_size, size_t *bytes_read)
    {
        Luna::ReadDir data;
        data.fd = handle;
        Luna::syscall(SYS_READDIR, &data);

        if (data.result)
        {
            dirent *dir = (dirent *)buffer;
            if (data.name[0] == '\0')
            {
                *bytes_read = 0;
                return 0;
            }

            strcpy(dir->d_name, data.name);
            dir->d_ino = 1;
            dir->d_off = 0;
            dir->d_reclen = sizeof(dirent);
            dir->d_type = 0;

            *bytes_read = sizeof(dirent);
            return 0;
        }

        return -1;
    }

    // int sys_fork(pid_t *child)
    // {
    //     Luna::ForkData data;
    //     Luna::syscall(SYS_FORK, &data);
    // }

    // int mlibc::sys_execve(const char *path, char *const *argv, char *const *envp)
    // {

    // }

    int sys_close(int fd)
    {
        Luna::CloseData data;
        data.fd = fd;
        Luna::syscall(SYS_CLOSE, &data);

        if (data.result)
        {
            return 0;
        }

        // errno = data.err;
        return -1;
    }

    int sys_read(int fd, void *buf, size_t count, ssize_t *bytes_read)
    {
        Luna::ReadData data;
        data.fd = fd;
        data.buf = buf;
        data.count = count;
        Luna::syscall(SYS_READ, &data);

        if (data.result)
        {
            *bytes_read = data.byteRead;
            return 0;
        }

        // errno = data.err;
        return -1;
    }

    int sys_write(int fd, void const *buf, unsigned long btw, long *bw)
    {
        Luna::WriteData data;
        data.fd = fd;
        data.buf = buf;
        data.btw = btw;
        data.bw = bw;
        Luna::syscall(SYS_WRITE, &data);

        if (data.result)
            return 0;

        // errno = data.err;
        return -1;
    }

    int sys_seek(int fd, off_t offset, int whence, off_t *new_offset)
    {
        // errno = ENOSYS;
        return -1;
    }

    int sys_stat(fsfd_target fsfdt, int fd, const char *path, int flags,
                 struct stat *statbuf)
    {
        // errno = ENOSYS;
        return -1;
    }

    int sys_anon_allocate(unsigned long size, void **ptr)
    {
        Luna::VMMapData data;
        data.hint = NULL;
        data.size = size;

        Luna::syscall(SYS_VM_MAP, &data);

        if (!data.result)
            return -1;

        *ptr = data.allocatedRange;
        return 0;
    }

    int sys_anon_free(void *pointer, size_t size)
    {
        Luna::VMUnmapData data;
        data.size = size;
        data.pointer = pointer;

        Luna::syscall(SYS_VM_MAP, &data);

        if (!data.result)
            return -1;
        return 0;
    }

    int sys_vm_map(void *hint, size_t size, int prot, int flags, int fd, off_t offset, void **window)
    {
        // errno = ENOSYS;
        return -1;
    }

    int sys_vm_unmap(void *pointer, size_t size)
    {
        // errno = ENOSYS;
        return -1;
    }

    int sys_tcb_set(void *pointer)
    {
        Luna::TCBSetData data;
        data.pointer = pointer;
        Luna::syscall(SYS_TCB_SET, &data);
        return 0;
    }

    int sys_futex_tid()
    {
        return 1;
    }

    int sys_futex_wait(int *pointer, int expected, const struct timespec *time)
    {
        return -1;
    }

    int sys_futex_wake(int *pointer)
    {
        return -1;
    }

    int sys_clock_get(int, long *, long *)
    {
        return -1;
    }

    int sys_getcwd(char *buffer, size_t size)
    {
        Luna::GetCWDData data;
        data.cwd = buffer;
        data.size = size;
        Luna::syscall(SYS_GETCWD, &data);
        if (data.result)
            return 0;
        return -1;
    }

    int sys_chdir(const char *path)
    {
        Luna::ChdirData data;
        data.path = path;
        Luna::syscall(SYS_CHDIR, &data);
        if (data.result)
            return 0;
        return -1;
    }

    void sys_exit(int signal)
    {
        Luna::ExitData data;
        data.status = signal;
        syscall(SYS_EXIT, &data);
        __builtin_unreachable();
        return;
    }

}