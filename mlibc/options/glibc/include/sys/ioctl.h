#ifndef _SYS_IOCTL_H
#define _SYS_IOCTL_H

#include <mlibc-config.h>

// On Linux, sys/ioctl.h includes the termios ioctls.
#ifdef __MLIBC_LINUX_OPTION
#	include <asm/ioctls.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

int ioctl(int fd, unsigned long request, ...);

#define FIONREAD 0x541B
#define FIONBIO 0x5421
#define FIONCLEX 0x5450
#define FIOCLEX 0x5451

#define SIOCGIFCONF 0x8912

#ifdef __cplusplus
}
#endif

#endif // _SYS_IOCTL_H
