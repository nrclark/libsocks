#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "mkpath.h"

static const char mkpath_errstring[] = "%s: cannot create directory '%s': ";
static const char path_toolong[] = "%s: error: pathname too long\n";

/*----------------------------------------------------------------------------*/

static int mkpath_lowlevel(char *path, mode_t mode)
{
    unsigned int x;
    int result = -1;

    for (x = 1; path[x] != '\x00'; x++) {
        if (path[x] != '/') {
            continue;
        }

        if ((path[x + 1] != '\x00') && (path[x + 1] != '/')) {
            path[x] = '\x00';

            result = mkdir(path, mode);
            if ((result != 0) && (errno != EEXIST)) {
                fprintf(stderr, mkpath_errstring, progname, path);
                perror(NULL);
                return result;
            }

            path[x] = '/';
        }
    }

    result = mkdir(path, mode);

    if ((result != 0) && (errno != EEXIST)) {
        fprintf(stderr, mkpath_errstring, progname, path);
        perror(NULL);
        return result;
    }

    return 0;
}

int mkpath(const char *path, mode_t mode)
{
    char buffer[PATH_MAX + 2];
    size_t length = strnlen(path, PATH_MAX + 1);

    if (length > PATH_MAX) {
        fprintf(stderr, path_toolong, progname);
        return -1;
    }

    memcpy(buffer, path, length + 1);
    return mkpath_lowlevel(buffer, mode);
}
