#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "eintr_wrappers.h"
#include "libsocks_dirs.h"

#ifdef VERBOSE_DEBUG
/* Replace chdir/fchdir/mkdir calls with verbose equivalents. Intended for
 * debug purposes. */
#include "libsocks_debug.h"
#endif

/*----------------------------------------------------------------------------*/

static DIR *dirp = NULL;
static int cwd_fd = -1;

/*----------------------------------------------------------------------------*/

static ino_t inode_at(int fd, const char *restrict path)
{
    struct stat buffer;
    int result;

    result = fstatat(fd, path, &buffer, 0);
    if (result != 0) {
        return (ino_t)(-1);
    }

    return buffer.st_ino;
}

static int mkdir_if_needed(const char *path, mode_t mode, uid_t uid, gid_t gid)
{
    int result = -1;
    ino_t parent_inode = (ino_t)(-1);
    ino_t expected_inode = (ino_t)(-1);

    if (strcmp(path, ".") == 0) {
        return 0;
    }

    if (strcmp(path, "..") == 0) {
        return chdir("..");
    }

    expected_inode = inode_at(AT_FDCWD, ".");

    if (expected_inode == (ino_t)(-1)) {
        return -1;
    }

    result = mkdir(path, mode);

    if ((result != 0) && (errno != EEXIST)) {
        return result;
    }

    if (result == 0) {
        result = chown_noeintr(path, uid, gid);
        if (result != 0) {
            int prev_errno = errno;
            rmdir(path);
            errno = prev_errno;
            return result;
        }
    }

    result = chdir(path);
    parent_inode = inode_at(AT_FDCWD, "..");

    if (expected_inode != parent_inode) {
        fprintf(stderr, "inodes don't match\n");
        return -1;
    }

    return result;
}

static unsigned int load_block(const char *restrict path, unsigned int offset,
                               unsigned int maxlen, char *restrict target)
{
    unsigned int count = 0;
    path += offset;
    maxlen -= offset;
    char path_valid = 0;

    while (*path == '/') {
        count++;
        path++;
        maxlen--;
    }

    while ((*path != '/') && (*path != '\x00') && (maxlen != 0)) {
        path_valid = 1;
        *(target++) = *(path++);
        count++;
        maxlen--;
    }

    *(target) = '\x00';

    if (path_valid == 0) {
        return 0;
    }

    return count;
}

/** @brief Returns 1 if the input path is a valid directory, and 0 otherwise.
 * Follows symlinks. */
static int is_directory(const char *path)
{
    struct stat stat_buffer;
    int result;
    result = stat(path, &stat_buffer);

    if (result != 0) {
        return 0;
    }

    return (S_ISDIR(stat_buffer.st_mode) != 0);
}

/** @brief Finds the length of the longest existing directory-only path in
 * 'path'. Respects symlinks. Returns the length of the path segment. */
static unsigned int find_existing(char *path, size_t maxlen)
{
    unsigned int position = 0;
    unsigned int x = 0;
    size_t length = strnlen(path, maxlen);
    int result;

    while (1) {
        if (path[x] == '\x00') {
            if (x != 0) {
                if (is_directory(path)) {
                    position = x;
                }
            }
            break;
        }

        if (x == length) {
            break;
        }

        if ((path[x] == '/') && (x != 0)) {
            path[x] = '\x00';
            result = is_directory(path);
            path[x] = '/';

            if (result == 0) {
                break;
            }

            position = x;
        }
        x++;
    }

    return position;
}

static void safe_strncpy(char *dest, const char *src, size_t maxlen)
{
    size_t length = strnlen(src, maxlen);

    memcpy(dest, src, length);
    dest[length] = '\x00';
}

static void split_existing(char *path, unsigned int maxlen, char **exist,
                           char **remainder)
{
    size_t length = strnlen(path, maxlen);
    unsigned int index;

    if (length == 0) {
        **exist = '\x00';
        **remainder = '\x00';
    }

    index = find_existing(path, length);

    if (index == 0) {
        if (path[0] == '/') {
            safe_strncpy(*exist, "/", 2);
            safe_strncpy(*remainder, path + 1, length - 1);
        } else {
            safe_strncpy(*exist, ".", 2);
            safe_strncpy(*remainder, path, length);
        }
        return;
    }

    safe_strncpy(*exist, path, index);
    if (index != length) {
        safe_strncpy(*remainder, path + index + 1, length - index - 1);
    } else {
        **remainder = '\x00';
    }
}

/*----------------------------------------------------------------------------*/

int socks_store_cwd(void)
{
    if ((dirp != NULL) || (cwd_fd != -1)) {
        fprintf(stderr, "cwd buffer is already full\n");
        return -1;
    }

    dirp = opendir(".");

    if (dirp == NULL) {
        fprintf(stderr, "%s\n", strerror(errno));
        return -1;
    }

    cwd_fd = dirfd(dirp);

    if (cwd_fd < 0) {
        fprintf(stderr, "%s\n", strerror(errno));
        closedir_noeintr(dirp);
        return -1;
    }

    return 0;
}

int socks_restore_cwd(void)
{
    int result;

    if ((dirp == NULL) || (cwd_fd == -1)) {
        fprintf(stderr, "cwd buffer is empty\n");
        return -1;
    }

    result = fchdir_noeintr(cwd_fd);

    if (result < 0) {
        fprintf(stderr, "%s\n", strerror(errno));
        return result;
    }

    cwd_fd = -1;

    result = closedir_noeintr(dirp);
    dirp = NULL;

    if (result < 0) {
        fprintf(stderr, "%s\n", strerror(errno));
        return result;
    }

    return 0;
}

/*----------------------------------------------------------------------------*/

int socks_mkdirs_chdir(const char *path, unsigned int length, mode_t mode,
                       uid_t uid, gid_t gid)
{
    int result;
    char buffer[3 * length + 3];

    char *path_copy = &buffer[0];
    char *existing = &buffer[length + 1];
    char *remainder = &buffer[2 * length + 2];
    char *block = path_copy;

    // First, the path is split into existing and non-existing components.

    safe_strncpy(path_copy, path, length);
    split_existing(path_copy, length, &existing, &remainder);

    // Next, we chdir into the existing portion of the path.

    result = chdir(existing);

    if (result != 0) {
        fprintf(stderr, "%s\n", strerror(errno));
        return -1;
    }

    // Next, we create all segments of the remainder. This is done using
    // mkdir_if_needed, which validates the parent inode of each directory
    // as it gets created. This prevents symlink-based attacks.

    if (remainder[0] != '\x00') {
        unsigned int offset = 0;

        while (1) {
            unsigned int used = load_block(remainder, offset, length, block);
            if (used == 0) {
                break;
            }

            offset += used;
            result = mkdir_if_needed(block, mode, uid, gid);
            if (result != 0) {
                fprintf(stderr, "%s\n", strerror(errno));
                return -1;
            }
        }
    }

    return 0;
}

int socks_mkdirs(const char *path, unsigned int length, mode_t mode, uid_t uid,
                 gid_t gid)
{
    int result;
    int mkdirs_errno;
    int mkdirs_result;

    result = socks_store_cwd();

    if (result != 0) {
        fprintf(stderr, "%s\n", strerror(errno));
        return result;
    }

    mkdirs_result = socks_mkdirs_chdir(path, length, mode, uid, gid);
    mkdirs_errno = errno;

    result = socks_restore_cwd();

    if (result != 0) {
        fprintf(stderr, "%s\n", strerror(errno));
    }

    errno = mkdirs_errno;
    return mkdirs_result;
}
