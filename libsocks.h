#ifndef _LIBSOCKS_H_
#define _LIBSOCKS_H_

#include <stdint.h>
#include <sys/types.h>

/*----------------------------------------------------------------------------*/

enum {socks_packet_max = 4096};

extern const char *progname;

/*----------------------------------------------------------------------------*/

/** @brief Sends a packet of data to a libsocks server and receives the
 * server's response. Returns the number of bytes received into output.
 * @param[in] filename Filename of target socketfile.
 * @param[in] input Input packet to send to server.
 * @param[in] nbyte Length of input packet (in bytes).
 * @param[out] output Pointer to output data buffer
 * @param[in] maxlen Maximum length of output packet to receive.
 * @return Number of bytes returned from server, or a negative number in the
 * event of an error.
 * @retval <0 A communications error occured, and errno was set accordingly.
 * @retval >=0 Length of response from server. */
ssize_t socks_client_process(const char *filename, const char *input,
                             uint32_t nbyte, char *output, uint32_t maxlen);

/*----------------------------------------------------------------------------*/

/** @brief Creates a unix-domain socket and opens it as a libsocks server.
 * Returns a file descriptor for the open socket.
 * @param[in] filename Filename of target socketfile.
 * @return File descriptor for the open socket, or a negative number in the
 * event of an error.
 * @retval <0 Socketfile couldn't be opened, and errno was set accordingly.
 * @retval >=0 File descriptor for the open socket. */
int socks_server_open(const char *filename);

/** @brief Shuts down an active libsocks server.
 * @return Exit status of function.
 * @retval 0 Server was stopped OK.
 * @retval (other) Server couldn't be stopped, and errno was set accordingly. */
int socks_server_close(int socket_fd);

/** Waits for a client to connect to the libsocks server using select().
 * @return Exit status of function.
 * @retval 0 A client is now waiting, and socks_server_process() can be used.
 * @retval (other) The wait operation failed, and errno was set accordingly. */
int socks_server_wait(int socket_fd);

/** Polls to see if a client is currently waiting for the libsocks server.
 * @return Exit status of function.
 * @retval 0 No client is waiting.
 * @retval 1 A client is waiting, and socks_server_process() can be used.
 * @retval (other) The poll operation failed, and errno was set accordingly. */
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
