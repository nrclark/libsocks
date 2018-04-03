#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libsocks.h"

const char *progname;
static volatile char shutdown = 0;

int callback(int response_fd, const char *input, uint32_t nbyte)
{
    printf("responding to command: [%s]\n", input);

    if (strcmp(input,"ping") == 0) {
        return socks_respond(response_fd, "pong", sizeof("pong"));
    }

    if (strcmp(input,"pong") == 0) {
        return socks_respond(response_fd, "pango", sizeof("pango"));
    }

    if (strcmp(input,"empty") == 0) {
        return 0;
    }

    if (strcmp(input,"fail") == 0) {
        return -1;
    }

    if (strcmp(input,"shutdown") == 0) {
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

    if (socks_fd != 0) {
        perror(NULL);
    }


    while (1) {
        if (shutdown) {
            break;
        }

        result = socks_server_wait(socks_fd);

        if (result != 0) {
            perror(NULL);
            return result;
        }

        result = socks_server_process(socks_fd, callback);

        if (result != 0) {
            perror(NULL);
            return result;
        }
    }

    result = socks_server_close(result);

    if (result != 0) {
        perror(NULL);
    }

    return result;
}    
