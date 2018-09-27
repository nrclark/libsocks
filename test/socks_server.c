#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <inttypes.h>
#include <libgen.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "libsocks.h"

static char progname[PATH_MAX];
static volatile char shutdown = 0;
static volatile char blocking = 1;
static mode_t socket_mode = 0755;
char **remaining = NULL;

static const char help[] = \
"Usage: %s [-u UID] SOCKET_PATH\n"
"\n"
"Launches a libsocks demo server connected to SOCKET_PATH. Socket can \n"
"optionally be launched with user-specified permissions.\n"
"\n";

/*----------------------------------------------------------------------------*/

static void set_progname(char *argv0)
{
    size_t length = strnlen(basename(argv0), sizeof(progname) - 1);
    strncpy(progname, basename(argv0), length);
    progname[length] = '\x00';
}

static void check_help(int argc, char **argv)
{
    set_progname(argv[0]);

    for (int x = 1; x < argc; x++) {
        if ((strcmp(argv[x], "--help") == 0) || (strcmp(argv[x], "-h") == 0)) {
            printf(help, progname);
            exit(0);
        }
    }
}

static mode_t scan_mode(const char *input)
{
    char *endptr;
    intmax_t result;
    result = strtoimax(input, &endptr, 8);

    if ((errno != 0) || (endptr == input)) {
        fprintf(stderr, "Couldn't scan octal literal [%s]\n", input);
        exit(-1);
    }

    return (mode_t) result;
}

static void scan_opts(int argc, char **argv)
{
    const char optstring[] = ":m:";

    check_help(argc, argv);
    int opt = getopt(argc, argv, optstring);

    while (opt != -1) {
        switch (opt) {
            case 'm':
                socket_mode = scan_mode(optarg);
                break;

            case ':':
                fprintf(stderr, "Option -%c requires an operand.\n", optopt);
                exit(-1);
                break;

            case '?':
                fprintf(stderr, "Unrecognized option: '-%c'\n", optopt);
                exit(-1);
                break;

            default:
                break;
        }
        opt = getopt(argc, argv, optstring);
    }

    remaining = argv + optind;

    switch (argc - optind) {
        case 0:
            fprintf(stderr, "No socket path specified.\n");
            exit(-1);
            break;

        case 1:
            break;

        default:
            fprintf(stderr, "Multiple socket paths specified.\n");
            exit(-1);
    }
}

/*----------------------------------------------------------------------------*/

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

static int callback(int response_fd, const char *input, uint16_t nbyte)
{
    ssize_t result;
    printf("responding to command: [%s]\n", input);

    if (strcmp(input, "ping") == 0) {
        result = socks_server_respond(response_fd, "pong", sizeof("pong"));
        return (int)((result < 0) ? result : 0);
    }

    if (strcmp(input, "pong") == 0) {
        result = socks_server_respond(response_fd, "pango", sizeof("pango"));
        return (int)((result < 0) ? result : 0);
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

    if (strcmp(input, "set_nonblocking") == 0) {
        blocking = 0;
        result = socks_server_respond(response_fd, "ok", sizeof("ok"));
        return (int)((result < 0) ? result : 0);
    }

    if (strcmp(input, "set_blocking") == 0) {
        blocking = 1;
        result = socks_server_respond(response_fd, "ok", sizeof("ok"));
        return (int)((result < 0) ? result : 0);
    }

    if (strcmp(input, "shutdown") == 0) {
        shutdown = 1;
        return 0;
    }

    return (int) socks_server_respond(response_fd, input, nbyte);
}

/*----------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
    int result;
    int socks_fd;

    scan_opts(argc, argv);
    socks_fd = socks_server_open(*remaining, socket_mode);

    if (socks_fd < 0) {
        perror(NULL);
        fprintf(stderr, "%s: couldn't open socketfile [%s] (%s)\n",
                argv[0], *remaining, strerror(errno));
        exit(socks_fd);
    }

    while (1) {
        if (shutdown) {
            break;
        }

        if (blocking) {
            result = socks_server_wait(socks_fd);

            if (result != 0) {
                fprintf(stderr, "socks_server_wait: failed (%s)\n",
                        strerror(errno));
                return result;
            }
        } else {
            while (1) {
                result = socks_server_poll(socks_fd);
                if (result < 0) {
                    fprintf(stderr, "socks_server_wait: failed (%s)\n",
                            strerror(errno));
                    return result;
                }

                if (result) {
                    break;
                }
                sleep_ms(2);
            }
        }

        result = socks_server_process(socks_fd, callback);

        if (result != 0) {
            if (errno != 0) {
                fprintf(stderr, "socks_server_process: failed (%s)\n",
                        strerror(errno));
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
