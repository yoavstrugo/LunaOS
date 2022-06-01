
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>

#include <bits/ensure.h>
#include <mlibc/debug.hpp>
#include <mlibc/posix-sysdeps.hpp>

int accept(int fd, struct sockaddr *__restrict addr_ptr, socklen_t *__restrict addr_length) {
	int newfd;
	if(!mlibc::sys_accept) {
		MLIBC_MISSING_SYSDEP();
		errno = ENOSYS;
		return -1;
	}
	if(int e = mlibc::sys_accept(fd, &newfd, addr_ptr, addr_length); e) {
		errno = e;
		return -1;
	}
	return newfd;
}

int accept4(int fd, struct sockaddr *__restrict addr_ptr, socklen_t *__restrict addr_length, int flags) {
	if(flags & SOCK_NONBLOCK) {
		fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
	}

	int newfd;
	if(!mlibc::sys_accept) {
		MLIBC_MISSING_SYSDEP();
		errno = ENOSYS;
		return -1;
	}
	if(int e = mlibc::sys_accept(fd, &newfd, addr_ptr, addr_length); e) {
		errno = e;
		return -1;
	}

	if(flags & SOCK_CLOEXEC) {
		fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
	}

	return newfd;
}

int bind(int fd, const struct sockaddr *addr_ptr, socklen_t addr_len) {
	if(!mlibc::sys_bind) {
		MLIBC_MISSING_SYSDEP();
		errno = ENOSYS;
		return -1;
	}
	if(int e = mlibc::sys_bind(fd, addr_ptr, addr_len); e) {
		errno = e;
		return -1;
	}
	return 0;
}

int connect(int fd, const struct sockaddr *addr_ptr, socklen_t addr_len) {
	if(!mlibc::sys_connect) {
		MLIBC_MISSING_SYSDEP();
		errno = ENOSYS;
		return -1;
	}
	if(int e = mlibc::sys_connect(fd, addr_ptr, addr_len); e) {
		errno = e;
		return -1;
	}
	return 0;
}

int getpeername(int fd, struct sockaddr *addr_ptr, socklen_t *__restrict addr_length) {
	socklen_t actual_length;
	if(!mlibc::sys_peername) {
		MLIBC_MISSING_SYSDEP();
		errno = ENOSYS;
		return -1;
	}
	if(int e = mlibc::sys_peername(fd, addr_ptr, *addr_length, &actual_length); e) {
		errno = e;
		return -1;
	}
	*addr_length = actual_length;
	return 0;
}

int getsockname(int fd, struct sockaddr *__restrict addr_ptr, socklen_t *__restrict addr_length) {
	socklen_t actual_length;
	if(!mlibc::sys_sockname) {
		MLIBC_MISSING_SYSDEP();
		errno = ENOSYS;
		return -1;
	}
	if(int e = mlibc::sys_sockname(fd, addr_ptr, *addr_length, &actual_length); e) {
		errno = e;
		return -1;
	}
	*addr_length = actual_length;
	return 0;
}

int getsockopt(int fd, int layer, int number,
		void *__restrict buffer, socklen_t *__restrict size) {
	if(!mlibc::sys_getsockopt) {
		MLIBC_MISSING_SYSDEP();
		errno = ENOSYS;
		return -1;
	}
	return mlibc::sys_getsockopt(fd, layer, number, buffer, size);
}

int listen(int fd, int backlog) {
	if(!mlibc::sys_listen) {
		MLIBC_MISSING_SYSDEP();
		errno = ENOSYS;
		return -1;
	}
	if(int e = mlibc::sys_listen(fd, backlog); e) {
		errno = e;
		return -1;
	}
	return 0;
}

ssize_t recv(int sockfd, void *__restrict buf, size_t len, int flags) {
	return recvfrom(sockfd, buf, len, flags, NULL, NULL);
}

ssize_t recvfrom(int sockfd, void *__restrict buf, size_t len, int flags,
		struct sockaddr *__restrict src_addr, socklen_t *__restrict addrlen) {
	struct iovec iov = {};
	iov.iov_base = buf;
	iov.iov_len = len;

	struct msghdr hdr = {};
	hdr.msg_name = src_addr;
	if (addrlen) {
		hdr.msg_namelen = *addrlen;
	}
	hdr.msg_iov = &iov;
	hdr.msg_iovlen = 1;

	int ret = recvmsg(sockfd, &hdr, flags);
	if (ret < 0)
		return ret;

	if(addrlen)
		*addrlen = hdr.msg_namelen;
	return ret;
}

ssize_t recvmsg(int fd, struct msghdr *hdr, int flags) {
	ssize_t length;
	if(!mlibc::sys_msg_recv) {
		MLIBC_MISSING_SYSDEP();
		errno = ENOSYS;
		return -1;
	}
	if(int e = mlibc::sys_msg_recv(fd, hdr, flags, &length); e) {
		errno = e;
		return -1;
	}
	return length;
}

int recvmmsg(int, struct mmsghdr *, unsigned int, int, struct timespec *) {
	__ensure(!"Not implemented");
	__builtin_unreachable();
}

ssize_t send(int fd, const void *buffer, size_t size, int flags) {
	return sendto(fd, buffer, size, flags, nullptr, 0);
}

ssize_t sendto(int fd, const void *buffer, size_t size, int flags,
		const struct sockaddr *sock_addr, socklen_t addr_length) {
	struct iovec iov = {};
	iov.iov_base = const_cast<void *>(buffer);
	iov.iov_len = size;

	struct msghdr hdr = {};
	hdr.msg_name = const_cast<struct sockaddr *>(sock_addr);
	hdr.msg_namelen = addr_length;
	hdr.msg_iov = &iov;
	hdr.msg_iovlen = 1;

	return sendmsg(fd, &hdr, flags);
}

ssize_t sendmsg(int fd, const struct msghdr *hdr, int flags) {
	ssize_t length;
	if(!mlibc::sys_msg_send) {
		MLIBC_MISSING_SYSDEP();
		errno = ENOSYS;
		return -1;
	}
	if(int e = mlibc::sys_msg_send(fd, hdr, flags, &length); e) {
		errno = e;
		return -1;
	}
	return length;
}

int sendmmsg(int, struct mmsghdr *, unsigned int, int) {
	__ensure(!"Not implemented");
	__builtin_unreachable();
}

int setsockopt(int fd, int layer, int number,
		const void *buffer, socklen_t size) {
	if(!mlibc::sys_setsockopt) {
		MLIBC_MISSING_SYSDEP();
		errno = ENOSYS;
		return -1;
	}
	return mlibc::sys_setsockopt(fd, layer, number, buffer, size);
}

int shutdown(int, int) {
	mlibc::infoLogger() << "mlibc: shutdown() is a no-op!" << frg::endlog;
	return 0;
}

int sockatmark(int) {
	__ensure(!"Not implemented");
	__builtin_unreachable();
}

int socket(int family, int type, int protocol) {
	int fd;
	if(!mlibc::sys_socket) {
		MLIBC_MISSING_SYSDEP();
		errno = ENOSYS;
		return -1;
	}
	if(int e = mlibc::sys_socket(family, type, protocol, &fd); e) {
		errno = e;
		return -1;
	}
	return fd;
}

int socketpair(int domain, int type, int protocol, int sv[2]) {
	if(!mlibc::sys_socketpair) {
		MLIBC_MISSING_SYSDEP();
		errno = ENOSYS;
		return -1;
	}
	if(int e = mlibc::sys_socketpair(domain, type, protocol, sv); e) {
		errno = e;
		return -1;
	}
	return 0;
}

// connectpair() is provided by the platform

