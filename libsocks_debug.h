#ifndef _DEBUG_H_
#define _DEBUG_H_

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

__attribute__ ((unused)) static char cwd_buffer[4096];
__attribute__ ((unused)) static char * cwd(void)
{
    return getcwd(cwd_buffer, sizeof(cwd_buffer));
}

__attribute__ ((unused)) static int show_chdir(const char *path)
{
    int result = chdir(path);

    if (result != 0) {
        fprintf(stderr, "Couldn't chdir to [%s] (%s)\n", path,
                strerror(errno));
        return result;
    }

    fprintf(stdout, "Changed dir to [%s]\n", cwd());
    return 0;
}

__attribute__ ((unused)) static int show_fchdir(int fd)
{
    int result = fchdir(fd);

    if (result != 0) {
        fprintf(stderr, "Couldn't fchdir to [%d] (%s)\n", fd, strerror(errno));
        return result;
    }

    fprintf(stdout, "Changed dir to [%s]\n", cwd());
    return 0;
}

__attribute__ ((unused)) static int show_mkdir(const char *path, mode_t mode)
{
    int result = mkdir(path, mode);

    if (result != 0) {
        fprintf(stderr, "Couldn't mkdir [%s] (%s)\n", path, strerror(errno));
        return result;
    }

    fprintf(stdout, "Created directory [%s]\n", path);
    return 0;
}

#define mkdir(path, mode) show_mkdir(path, mode)
#define fchdir(fd) show_fchdir(fd)
#define chdir(path) show_chdir(path)

#endif
