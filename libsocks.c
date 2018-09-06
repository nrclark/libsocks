#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>

#include "libsocks.h"

static const char path_toolong[] = "%s: error: pathname too long\n";

/*----------------------------------------------------------------------------*/

/** @brief Serializes a uint32_t into a little-endian 4-char array. Used to
 * ensure predictable serialization across platforms.
 * @param[in] input Value to serialize
 * @param[out] output Destination for serialized data */
static void serialize_uint32(uint32_t input, char output[4])
{
    output[0] = (input >> 0) & 0xFF;
    output[1] = (input >> 8) & 0xFF;
    output[2] = (input >> 16) & 0xFF;
    output[3] = (input >> 24) & 0xFF;
}

/** @brief Deserializes a little-endian 4-char array and returns the result.
 * Used to ensure predictable deserialization across platforms.
 * @param[in] input Serialized data
 * @return Deserialized uint32 */
static uint32_t deserialize_uint32(const char input[4])
{
    uint32_t result = 0;

    result += (uint32_t)(input[0]) << 0;
    result += (uint32_t)(input[1]) << 8;
    result += (uint32_t)(input[2]) << 16;
    result += (uint32_t)(input[3]) << 24;

    return result;
}

/** @brief Reads a specified number of bytes from a file-descriptor. Retries
 * until enough bytes are received (or until the read command fails).
 * @param[in] filedes File descriptor used to read data
 * @param[out] buf Pointer to target data buffer
 * @param[in] nbyte Number of bytes to read
 * @return Number of bytes retrieved, or -1 in the event of an error.
 * @retval <0 A read error occured, and errno was set accordingly.
 * @retval >=0 Number of bytes retrieved. */
static ssize_t read_count(int filedes, char *buf, size_t nbyte)
{
    size_t total = 0;

    while (total < nbyte) {
        ssize_t result = read(filedes, buf, (nbyte - total));

        if (result < 0) {
            return result;
        }

        total += (size_t) result;
    }

    return (ssize_t) nbyte;
}

/** @brief Writes a specified number of bytes to a file-descriptor. Retries
 * until enough bytes are written (or until the write command fails).
 * @param[in] filedes File descriptor used to write data
 * @param[in] buf Pointer to input data buffer
 * @param[in] nbyte Number of bytes to write
 * @return Number of bytes written, or -1 in the event of an error.
 * @retval <0 A write error occured, and errno was set accordingly.
 * @retval >=0 Number of bytes written. */
static ssize_t write_count(int filedes, const char *buf, size_t nbyte)
{
    size_t remaining = nbyte;

    while (remaining != 0) {
        ssize_t result = write(filedes, buf, remaining);

        if (result < 0) {
            return result;
        }

        remaining -= (size_t) result;
        buf += result;
    }

    return (ssize_t) nbyte;
}

static int fd_socket_setflag(int fd)
{
    return fcntl(fd, F_SETOWN, getpid());
}

static int fd_socket_clearflag(int fd)
{
    return fcntl(fd, F_SETOWN, 0);
}

static int fd_socket_checkflag(int fd)
{
    pid_t result = fcntl(fd, F_GETOWN, 0);

    if (errno == 0) {
        return (result != 0);
    }

    return -1;
}

/*----------------------------------------------------------------------------*/

#define get_size(type, field) sizeof(((type *)0)->field)

enum {
    sun_path_size = get_size(struct sockaddr_un, sun_path) - 1,
    address_maxlen = (sun_path_size > PATH_MAX) ? sun_path_size : PATH_MAX
};

static int socks_address_make(const char *filename, struct sockaddr_un *result)
{
    size_t length = strnlen(filename, PATH_MAX + 1);

    if (length > address_maxlen) {
        fprintf(stderr, path_toolong, progname);
        return -1;
    }

    memcpy(result->sun_path, filename, length);
    result->sun_path[length] = '\x00';
    result->sun_family = AF_UNIX;
    return 0;
}

static ssize_t socks_recv(int fd, void *buf, size_t bufsize)
{
    char header[4];
    uint32_t msgsize;
    ssize_t result;

    result = read_count(fd, header, 4);

    if (result < 0) {
        return result;
    }

    msgsize = deserialize_uint32(header);

    if (msgsize > bufsize) {
        errno = EMSGSIZE;
        return -1;
    }

    return read_count(fd, buf, msgsize);
}

static ssize_t socks_send(int fd, const void *buf, uint32_t nbyte)
{
    char header[4];
    ssize_t result;

    serialize_uint32(nbyte, header);
    result = write_count(fd, header, 4);

    if (result < 0) {
        return result;
    }

    return write_count(fd, buf, nbyte);
}

static int socks_process_request(int connection_fd, socks_callback_t callback,
                                 uint32_t input_size)
{
    int result;
    char buffer[input_size + 1];
    int callback_result;

    buffer[input_size] = '\x00';
    result = read_count(connection_fd, buffer, input_size);

    if (result < 0) {
        return result;
    }

    result = fd_socket_clearflag(connection_fd);

    if (result < 0) {
        fprintf(stderr, "couldn't clear flag\n");
        return result;
    }

    callback_result = callback(connection_fd, buffer, input_size);

    switch (fd_socket_checkflag(connection_fd)) {
        case 0:
            result = socks_server_respond(connection_fd, "", 0);
            break;

        case 1:
            break;

        default:
            fprintf(stderr, "couldn't check flag\n");
            break;
    }

    if (result == 0) {
        return callback_result;
    }

    return result;
}

static int socks_server_select(int socket_fd, struct timeval *restrict timeout)
{
    fd_set read_fds;
    fd_set error_fds;

    FD_ZERO(&read_fds);
    FD_ZERO(&error_fds);
    FD_SET(socket_fd, &read_fds);
    FD_SET(socket_fd, &error_fds);

    int result = select(socket_fd + 1, &read_fds, NULL, &error_fds, timeout);

    /* We might normally use FD_ISSET here, but this isn't necessary
     * because we're only listening for one item (the socket). */

    if (result > 0) {
        return 1;
    }

    return result;
}

/*----------------------------------------------------------------------------*/

ssize_t socks_server_respond(int fd, const void *buf, uint32_t nbyte)
{
    if (fd_socket_setflag(fd) != 0) {
        fprintf(stderr, "setflag failed!\n");
    }

    return socks_send(fd, buf, nbyte);
}

int socks_server_open(const char *filename)
{
    int result;
    int socket_fd;
    struct sockaddr_un address;

    result = socks_address_make(filename, &address);

    if (result < 0) {
        return result;
    }

    socket_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);

    if (socket_fd < 0) {
        return socket_fd;
    }

    result = bind(socket_fd, (struct sockaddr *) &address, sizeof(address));

    if (result != 0) {
        return -1;
    }

    result = listen(socket_fd, 4);

    if (result != 0) {
        return -1;
    }

    return socket_fd;
}

int socks_server_process(int socket_fd, socks_callback_t callback)
{
    int connection_fd;
    ssize_t result;
    char header[4];
    uint32_t msgsize;

    connection_fd = accept(socket_fd, 0, 0);

    if (connection_fd < 0) {
        return connection_fd;
    }

    result = read_count(connection_fd, header, 4);

    if (result < 0) {
        return (int) result;
    }

    msgsize = deserialize_uint32(header);
    result = socks_process_request(connection_fd, callback, msgsize);
    close(connection_fd);

    return result;
}

int socks_server_poll(int socket_fd)
{
    struct timeval nodelay = {.tv_sec = 0, .tv_usec = 0};
    int result = socks_server_select(socket_fd, &nodelay);

    if (result > 0) {
        return 1;
    }

    return result;
}

int socks_server_wait(int socket_fd)
{
    int result = socks_server_select(socket_fd, NULL);

    if (result > 0) {
        return 0;
    }

    if (result == 0) {
        return -1;
    }

    return result;
}

int socks_server_close(int socket_fd)
{
    return close(socket_fd);
}

/*----------------------------------------------------------------------------*/

ssize_t socks_client_process(const char *filename, const char *input,
                             uint32_t nbyte, char *output, uint32_t maxlen)
{
    ssize_t result;
    int socket_fd;
    struct sockaddr_un address;

    result = socks_address_make(filename, &address);

    if (result < 0) {
        return result;
    }

    socket_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);

    if (socket_fd < 0) {
        fprintf(stderr, "Couldn't open socket [%s]\n", filename);
        return socket_fd;
    }

    result = connect(socket_fd, (struct sockaddr *) &address, sizeof(address));

    if (result != 0) {
        fprintf(stderr, "Couldn't connect to socket [%s]\n", filename);
        close(socket_fd);
        return result;
    }

    result = socks_send(socket_fd, input, nbyte);

    if (result < 0) {
        close(socket_fd);
        return result;
    }

    result = socks_recv(socket_fd, output, maxlen);
    close(socket_fd);
    return result;
}
