#ifndef LIBSOCKS_DIRS_H
#define LIBSOCKS_DIRS_H

#define _POSIX_C_SOURCE 200809L

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/** @brief Creates a directory (if needed) along with any required parent
 * directories. Jumps into the target directory on completion.
 *
 * @param[in] path Path to create (if needed) and enter.
 * @param[in] length Length of the input path string.
 * @param[in] mode Access mode to set for newly-created directories. 0755
 * is a reasonable default.
 * @param[in] uid Owner's UID to set for newly-created directories. Use -1 if
 * the uid should stay unchanged. Chown capability is required.
 * @param[in] gid Owner's GID to set for newly-created directories. Use -1 if
 * the gid should stay unchanged. Chown capability is required.
 * @return Exit code of function.
 * @retval 0 Operation completed successfully.
 * @retval other An error occurred, and errno was set accordingly. */
int socks_mkdirs_chdir(const char *path, unsigned int length, mode_t mode,
                       uid_t uid, gid_t gid);

/** @brief Creates a directory (if needed) along with any required parent
 * directories, similar to mkdirs_chdir. Returns to the original directory on
 * completion.
 *
 * @param[in] path Path to create (if needed) and enter.
 * @param[in] length Length of the input path string.
 * @param[in] mode Access mode to set for newly-created directories. 0755
 * is a reasonable default.
 * @param[in] uid Owner's UID to set for newly-created directories. Use -1 if
 * the uid should stay unchanged. Chown capability is required.
 * @param[in] gid Owner's GID to set for newly-created directories. Use -1 if
 * the gid should stay unchanged. Chown capability is required.
 * @return Exit code of function.
 * @retval 0 Operation completed successfully.
 * @retval other An error occurred, and errno was set accordingly. */
int socks_mkdirs(const char *path, unsigned int length, mode_t mode, uid_t uid,
                 gid_t gid);

/** @brief Stores the current working directory in a one-deep FIFO, for later
 * retrieval with socks_restore_cwd(). Intended to be used before a call to
 * socks_mkdirs_chdir().
 * @return Exit code of function.
 * @retval 0 Operation completed successfully.
 * @retval -1 An error occurred. */
int socks_store_cwd(void);

/** @brief Switches back to the working directory stored by socks_store_cwd().
 * Intended to be used after a call to socks_mkdirs_chdir().
 * @return Exit code of function.
 * @retval 0 Operation completed successfully.
 * @retval -1 An error occurred. */
int socks_restore_cwd(void);

#endif
