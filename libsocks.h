#ifndef _LIBSOCKS_H_
#define _LIBSOCKS_H_

#include <stdint.h>
#include <sys/types.h>

/*----------------------------------------------------------------------------*/

enum {socks_packet_max = 4096};

extern const char *progname;

/*----------------------------------------------------------------------------*/

ssize_t socks_client_process(const char *filename, const char *input,
                             uint32_t nbyte, char *output, uint32_t bufsize);

/*----------------------------------------------------------------------------*/

int socks_server_open(const char *filename);
int socks_server_close(int socket_fd);

int socks_server_wait(int socket_fd);
int socks_server_poll(int socket_fd);

typedef int (*socks_callback_t)(int response_fd, const char *buf,
                                uint32_t nbyte);

/* Your server callback should use socks_server_respond() to send a response (if
 * necessary). The return code of your callback is presented as the
 * socks_server_process() return code (assuming the callback gets
 * triggered).*/
ssize_t socks_server_respond(int fd, const void *buf, uint32_t nbyte);

/* Socks_server_process() either returns an error (before running the callback),
 * or the retval of the callback(). */
int socks_server_process(int socket_fd, socks_callback_t callback);

/*----------------------------------------------------------------------------*/

#endif
