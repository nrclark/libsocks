#include <errno.h>
#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "libsocks.h"

const char *progname;
static volatile char shutdown = 0;

static int sleep_ms(unsigned int count)
{
    unsigned int seconds = (count / 1000);
    unsigned int msec = count - (seconds * 1000);
    long int nsec = msec * 1000000L;

    struct timespec duration = {
        .tv_sec = seconds,
        .tv_nsec = nsec
    };

    return nanosleep(&duration, NULL);
}

static int callback(int response_fd, const char *input, uint32_t nbyte)
{
    printf("responding to command: [%s]\n", input);

    if (strcmp(input, "ping") == 0) {
        return socks_respond(response_fd, "pong", sizeof("pong"));
    }

    if (strcmp(input, "pong") == 0) {
        return socks_respond(response_fd, "pango", sizeof("pango"));
    }

    if (strcmp(input, "empty") == 0) {
        return 0;
    }

    if (strcmp(input, "fail") == 0) {
        return -1;
    }

    if (strcmp(input, "sleep") == 0) {
        sleep_ms(5000);
        return 0;
    }

    if (strcmp(input, "shutdown") == 0) {
        shutdown = 1;
        return 0;
    }

    return socks_respond(response_fd, input, nbyte);
}

int main(int argc, char **argv)
{
    int result;
    progname = basename(argv[0]);

    if (argc != 2) {
        fprintf(stderr, "usage: %s FILENAME\n", progname);
        exit(1);
    }

    int socks_fd = socks_server_open(argv[1]);

    if (socks_fd < 0) {
        perror(NULL);
        fprintf(stderr, "%s: couldn't open socketfile [%s] (%s)\n",
                argv[0], argv[1], strerror(errno));
        exit(socks_fd);
    }

    while (1) {
        if (shutdown) {
            break;
        }

        result = socks_server_wait(socks_fd);

        if (result != 0) {
            fprintf(stderr, "socks_erver_wait: failed (%s)\n", strerror(errno));
            return result;
        }

        result = socks_server_process(socks_fd, callback);

        if (result != 0) {
            if (errno != 0) {
                fprintf(stderr, "socks_server_process: failed (%s)\n", strerror(errno));
                return result;
            }
            fprintf(stderr, "warn: command failed with code %d\n", result);
        }
    }

    result = socks_server_close(socks_fd);

    if (result != 0) {
        perror(NULL);
    }

    return result;
}
