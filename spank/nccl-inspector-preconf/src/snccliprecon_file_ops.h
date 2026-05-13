#ifndef SNCCLIPRECON_FILE_OPS_H
#define SNCCLIPRECON_FILE_OPS_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Callback executed while a lock file is held.
 *
 * @param arg Caller-provided callback argument.
 * @return Callback-specific status code.
 */
typedef int (*snccliprecon_locked_fn_t)(void *arg);

/**
 * @brief Ensure a path has the plugin's default permissive mode.
 *
 * Permission errors are ignored because shared cluster paths can be owned by a
 * different user while still being usable by the job.
 *
 * @param path File or directory path.
 * @return `0` on success or ignored permission errors, `-1` on fatal errors.
 */
int snccliprecon_ensure_mode(const char *path);

/**
 * @brief Check whether a path exists and is a directory.
 *
 * @param path Directory path.
 * @return `true` when the path is an existing directory, otherwise `false`.
 */
bool snccliprecon_dir_exists(const char *path);

/**
 * @brief Create a directory and all missing parent directories.
 *
 * @param path Directory path.
 * @return `0` on success, `-1` on failure with `errno` set.
 */
int snccliprecon_mkdir_all(const char *path);

/**
 * @brief Ensure a path exists as a directory with the default mode.
 *
 * @param path Directory path.
 * @return `0` on success, `-1` on failure with `errno` set.
 */
int snccliprecon_ensure_dir(const char *path);

/**
 * @brief Execute a callback while holding an exclusive file lock.
 *
 * Uses `<path>.lock` as a transient lock file and restores the callback's
 * `errno` value before returning.
 *
 * @param path Path whose sibling lock file should be used.
 * @param fn Callback to run while the lock is held.
 * @param arg Callback argument.
 * @return Callback return code, or `-1` if the lock could not be acquired.
 */
int snccliprecon_with_lock(
    const char *path, snccliprecon_locked_fn_t fn, void *arg
);

#endif
