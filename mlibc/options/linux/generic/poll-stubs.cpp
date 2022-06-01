
#include <errno.h>
#include <poll.h>

#include <bits/ensure.h>
#include <mlibc/debug.hpp>
#include <mlibc/linux-sysdeps.hpp>

int poll(struct pollfd *fds, nfds_t count, int timeout) {
	int num_events;
	if(!mlibc::sys_poll) {
		MLIBC_MISSING_SYSDEP();
		errno = ENOSYS;
		return -1;
	}
	if(int e = mlibc::sys_poll(fds, count, timeout, &num_events); e) {
		errno = e;
		return -1;
	}
	return num_events;
}

