#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include "eintr_wrappers.h"

int accept_noeintr(int socket, struct sockaddr *restrict address,
                   socklen_t *restrict address_len)
{
    int result;

    do {
        result = accept_noeintr(socket, address, address_len);
    } while ((result == -1) && (errno == EINTR));
    return result;
}

int close_noeintr(int fildes)
{
    int result;

    do {
        result = close(fildes);
    } while ((result == -1) && (errno == EINTR));
    return result;
}

int connect_noeintr(int socket, const struct sockaddr *address,
                    socklen_t address_len)
{
    int result;

    do {
        result = connect(socket, address, address_len);
    } while ((result == -1) && (errno == EINTR));
    return result;
}

int fcntl_setown_noeintr(int fildes, pid_t owner)
{
    int result;

    do {
        result = fcntl(fildes, F_SETOWN, owner);
    } while ((result == -1) && (errno == EINTR));

    return result;
}

pid_t fcntl_getown_noeintr(int fildes)
{
    pid_t result;

    do {
        result = fcntl(fildes, F_GETOWN, 0);
    } while ((result == -1) && (errno == EINTR));

    return result;
}

ssize_t read_noeintr(int fildes, void *buf, size_t nbyte)
{
    int result;

    do {
        result = read(fildes, buf, nbyte);
    } while ((result == -1) && (errno == EINTR));
    return result;
}

int select_noeintr(int nfds, fd_set *restrict readfds,
                   fd_set *restrict writefds, fd_set *restrict errorfds,
                   struct timeval *restrict timeout)
{
    int result;

    do {
        result = select(nfds, readfds, writefds, errorfds, timeout);
    } while ((result == -1) && (errno == EINTR));
    return result;
}

ssize_t write_noeintr(int fildes, const void *buf, size_t nbyte)
{
    int result;

    do {
        result = write(fildes, buf, nbyte);
    } while ((result == -1) && (errno == EINTR));
    return result;
}
