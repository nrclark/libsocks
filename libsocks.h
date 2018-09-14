#ifndef _LIBSOCKS_H_
#define _LIBSOCKS_H_

#include <stdint.h>
#include <sys/types.h>

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
 * @param[in] socket_fd File descriptor of open libsocks server.
 * @return Exit status of function.
 * @retval 0 Server was stopped OK.
 * @retval (other) Server couldn't be stopped, and errno was set accordingly. */
int socks_server_close(int socket_fd);

/** @brief Waits for a client to connect to the libsocks server using select().
 * @param[in] socket_fd File descriptor of open libsocks server.
 * @return Exit status of function.
 * @retval 0 A client is now waiting, and socks_server_process() can be used.
 * @retval (other) The wait operation failed, and errno was set accordingly. */
int socks_server_wait(int socket_fd);

/** @brief Polls to see if a client is currently waiting for the libsocks
 * server.
 * @param[in] socket_fd File descriptor of open libsocks server.
 * @return Exit status of function.
 * @retval 0 No client is waiting.
 * @retval 1 A client is waiting, and socks_server_process() can be used.
 * @retval (other) The poll operation failed, and errno was set accordingly. */
int socks_server_poll(int socket_fd);

/** @brief Function for the user-provided implementation to use for responding
 * to a client. Use this in your callback function to send a response stored.
 * in buf. Should be called at most once per callback. Has the same signature
 * as write().
 * @param[in] response_fd File descriptor provided to your callback.
 * @param[in] buf Buffer holding your response.
 * @param[in] nbyte Length of your response (in bytes).
 * @return Number of bytes written. Will always be equal to nbyte except in
 * the event of an error.
 * @retval <0 Error writing data, and errno was set accordingly.
 * @retval >=0 Number of bytes written. */
ssize_t socks_server_respond(int response_fd, const void *buf, uint32_t nbyte);

/** @brief Function-type for the user-provided callback function. A function
 * of this type is given to socks_server_process(), which will call it
 * automatically when appropriate. The 'msg' pointer holds the incoming message
 * (of length 'len'). Your callback should use the socks_server_respond()
 * function to send the response (if any). */
typedef int (*socks_callback_t)(int response_fd, const char *msg, uint32_t len);

/** @brief Processing function for a libsocks server. Should be called only
 * when a client is connected and waiting, as determined by socks_server_wait()
 * or socks_server_poll(). Will automatically read data from the socket, provide
 * it to callback(), and manage the response. Sends an empty response if your
 * callback doesn't use socks_server_respond().
 * @param[in] socket_fd File descriptor of open libsocks() server.
 * @param[in] callback Callback function for the server to use.
 * @return Callback's exit code, except in the event of a communication failure.
 * If communications fail, the failed function's return code is provided
 * instead. */
int socks_server_process(int socket_fd, socks_callback_t callback);

/*----------------------------------------------------------------------------*/

#endif
