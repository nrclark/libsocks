#define _POSIX_C_SOURCE 200809L

#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libsocks.h"

const char *progname;

int main(int argc, char **argv)
{
    ssize_t result;
    char buffer[1024];
    char *cmd;
    uint16_t cmd_len;

    progname = basename(argv[0]);

    if (argc != 3) {
        fprintf(stderr, "usage: %s FILENAME COMMAND\n", progname);
        exit(1);
    }

    cmd = argv[2];
    cmd_len = strnlen(cmd, 1024);

    result = socks_client_process(argv[1], cmd, cmd_len, buffer, 1024);

    if (result < 0) {
        perror(NULL);
        return result;
    }

    buffer[result] = '\x00';
    printf("response: [%s]\n", buffer);
    return 0;
}
