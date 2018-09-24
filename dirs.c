#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>

static char cwd_buffer[4096];
static char * cwd()
{
    return getcwd(cwd_buffer, sizeof(cwd_buffer));
}

static int show_chdir(const char *path)
{
    int result = chdir(path);

    if (result != 0) {
        fprintf(stderr, "Couldn't show_chdir to [%s] (%s)\n", path,
                strerror(errno));
        return result;
    }

    fprintf(stdout, "Changed dir to [%s]\n", cwd());
    return 0;
}

static int show_fchdir(int fd)
{
    int result = fchdir(fd);

    if (result != 0) {
        fprintf(stderr, "Couldn't fchdir to [%d] (%s)\n", fd, strerror(errno));
        return result;
    }

    fprintf(stdout, "Changed dir to [%s]\n", cwd());
    return 0;
}

ino_t inode_at(int fd, const char *restrict path)
{
    struct stat buffer;
    int result;

    result = fstatat(fd, path, &buffer, 0);
    if (result != 0) {
        return (ino_t)(-1);
    }

    return buffer.st_ino;
}

static int mkdir_if_needed(const char *path, mode_t mode)
{
    int result = -1;
    ino_t parent_inode = (ino_t)(-1);
    ino_t expected_inode = (ino_t)(-1);

    if (strcmp(path, ".") == 0) {
        return 0;
    }

    if (strcmp(path, "..") == 0) {
        return show_chdir("..");
    }

    expected_inode = inode_at(AT_FDCWD, ".");

    if (expected_inode == (ino_t)(-1)) {
        return -1;
    }

    result = mkdir(path, mode);

    if ((result != 0) && (errno != EEXIST)) {
        return result;
    }

    result = show_chdir(path);
    parent_inode = inode_at(AT_FDCWD, "..");

    if (expected_inode != parent_inode) {
        fprintf(stderr, "inodes don't match\n");
        return -1;
    }

    return 0;
}

static int load_block(const char *restrict path, unsigned int offset,
                      unsigned int maxlen, char *restrict target)
{
    unsigned int count = 0;
    path += offset;
    maxlen -= offset;

    while (*path == '/') {
        count++;
        path++;
        maxlen--;
    }

    while ((*path != '/') && (*path != '\x00') && (maxlen != 0)) {
        *(target++) = *(path++);
        count++;
        maxlen--;
    }

    if (count != 0) {
        *(target) = '\x00';
    }

    return count;
}

int mkdirs(const char *path, unsigned int length, mode_t mode)
{
    int cwd_fd;
    int result;
    DIR *dirp = NULL;
    char buffer[length + 1];

    unsigned int used = 0;
    unsigned int offset = 0;

    for (unsigned int x = 0; x < (length + 1); x++) {
        buffer[x] = 0;
    }

    buffer[length] = '\x00';
    dirp = opendir(".");

    if (dirp == NULL) {
        fprintf(stderr, "%s\n", strerror(errno));
        return -1;
    }

    cwd_fd = dirfd(dirp);

    if (cwd_fd < 0) {
        fprintf(stderr, "%s\n", strerror(errno));
        closedir(dirp);
        return -1;
    }

    if (path[0] == '/') {
        result = show_chdir("/");

        if (result != 0) {
            fprintf(stderr, "%s\n", strerror(errno));
            closedir(dirp);
            return -1;
        }
    }

    while (1) {
        used = load_block(path, offset, length, buffer);
        if (used == 0) {
            break;
        }

        offset += used;
        result = mkdir_if_needed(buffer, mode);
        if (result != 0) {
            fprintf(stderr, "%s\n", strerror(errno));
            closedir(dirp);
            return -1;
        }
    }

    result = show_fchdir(cwd_fd);

    if (result < 0) {
        fprintf(stderr, "%s\n", strerror(errno));
    }

    closedir(dirp);
    return result;
}

int main(int argc, char **argv)
{
    int result = -1;
    printf("---------------------\n");

    for (int x = 1; x < argc; x++) {
        unsigned int length = strnlen(argv[x], PATH_MAX);
        argv[x][length] = '\x00';

        result = mkdirs(argv[x], length, 0700);
        printf("@ result = %d\n", result);
    }

    return result;
}
