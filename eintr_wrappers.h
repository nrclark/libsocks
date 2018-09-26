#ifndef _EINTR_WRAPPER_H_
#define _EINTR_WRAPPER_H_

#include <dirent.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

int accept_noeintr(int socket, struct sockaddr *restrict address,
                   socklen_t *restrict address_len);

int close_noeintr(int fildes);

int connect_noeintr(int socket, const struct sockaddr *address,
                    socklen_t address_len);

int fcntl_setown_noeintr(int fildes, pid_t owner);

pid_t fcntl_getown_noeintr(int fildes);

ssize_t read_noeintr(int fildes, void *buf, size_t nbyte);

int select_noeintr(int nfds, fd_set *restrict readfds,
                   fd_set *restrict writefds, fd_set *restrict errorfds,
                   struct timeval *restrict timeout);

ssize_t write_noeintr(int fildes, const void *buf, size_t nbyte);

int chown_noeintr(const char *path, uid_t owner, gid_t group);

int closedir_noeintr(DIR *dirp);

int fchdir_noeintr(int fildes);

#endif
