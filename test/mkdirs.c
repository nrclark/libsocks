#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <inttypes.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libsocks_dirs.h"

static uid_t target_uid = (uid_t) -1;
static gid_t target_gid = (gid_t) -1;
static mode_t target_mode = 0755;
static char progname[PATH_MAX];
char **remaining = NULL;

static const char help[] = \
"Usage: %s [-u UID] [-g GID] [-m MODE] PATH [PATH] [...]\n"
"\n"
"Creates a hierarchical directory path.\n"
"\n";

static void set_progname(char *argv0)
{
    size_t length = strnlen(basename(argv0), sizeof(progname) - 1);
    strncpy(progname, basename(argv0), length);
    progname[length] = '\x00';
}

static intmax_t scan_intmax(const char *input, int base)
{
    char *endptr;
    intmax_t result;
    result = strtoimax(input, &endptr, base);

    if ((errno != 0) || (endptr == input)) {
        fprintf(stderr, "Couldn't scan numeric literal [%s]\n", input);
        exit(-1);
    }
    return result;
}

static uid_t scan_uid(const char *input)
{
    return (uid_t)(scan_intmax(input, 0));
}

static gid_t scan_gid(const char *input)
{
    return (gid_t)(scan_intmax(input, 0));
}

static mode_t scan_mode(const char *input)
{
    return (mode_t)(scan_intmax(input, 8));
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

static void scan_opts(int argc, char **argv)
{
    const char optstring[] = ":u:g:m:";

    check_help(argc, argv);
    int opt = getopt(argc, argv, optstring);

    while (opt != -1) {
        switch (opt) {
            case 'u':
                target_uid = scan_uid(optarg);
                break;

            case 'g':
                target_gid = scan_gid(optarg);
                break;

            case 'm':
                target_mode = scan_mode(optarg);
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

    if (*remaining == NULL) {
        fprintf(stderr, "No path(s) specified.\n");
        exit(-1);
    }
}

int main(int argc, char **argv)
{
    int result;
    scan_opts(argc, argv);

    while (*remaining != NULL) {
        size_t length = strnlen(*remaining, PATH_MAX);
        (*remaining)[length] = '\x00';

        result = socks_mkdirs(*remaining, (unsigned int) length, target_mode,
                              target_uid, target_gid);

        if (result != 0) {
            exit(result);
        }
        remaining++;
    }

    return result;
}
