#define _POSIX_C_SOURCE 200809L

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "eintr_wrappers.h"

#define wrap_call(dtype, x) \
    do { \
        dtype result; \
        do { \
            result = x; \
        } while ((result == -1) && (errno == EINTR)); \
        return result; \
    } while (0)

int accept_noeintr(int socket, struct sockaddr *restrict address,
                   socklen_t *restrict address_len)
{
    wrap_call(int, accept(socket, address, address_len));
}

int close_noeintr(int fildes)
{
    wrap_call(int, close(fildes));
}

int connect_noeintr(int socket, const struct sockaddr *address,
                    socklen_t address_len)
{
    wrap_call(int, connect(socket, address, address_len));
}

int fcntl_setown_noeintr(int fildes, pid_t owner)
{
    wrap_call(int, fcntl(fildes, F_SETOWN, owner));
}

pid_t fcntl_getown_noeintr(int fildes)
{
    wrap_call(pid_t, fcntl(fildes, F_GETOWN, 0));
}

ssize_t read_noeintr(int fildes, void *buf, size_t nbyte)
{
    wrap_call(ssize_t, read(fildes, buf, nbyte));
}

int select_noeintr(int nfds, fd_set *restrict readfds,
                   fd_set *restrict writefds, fd_set *restrict errorfds,
                   struct timeval *restrict timeout)
{
    wrap_call(int, select(nfds, readfds, writefds, errorfds, timeout));
}

ssize_t write_noeintr(int fildes, const void *buf, size_t nbyte)
{
    wrap_call(ssize_t, write(fildes, buf, nbyte));
}

int chmod_noeintr(const char *path, mode_t mode)
{
    wrap_call(int, chmod(path, mode));
}

int chown_noeintr(const char *path, uid_t owner, gid_t group)
{
    wrap_call(int, chown(path, owner, group));
}

int closedir_noeintr(DIR *dirp)
{
    wrap_call(int, closedir(dirp));
}

int fchdir_noeintr(int fildes)
{
    wrap_call(int, fchdir(fildes));
}
