#ifndef SNCCLIPRECON_WORKER_LOCK_H
#define SNCCLIPRECON_WORKER_LOCK_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Read the current worker hostname for logs and lock file names.
 *
 * @param hostname Destination buffer.
 * @param hostname_size Destination buffer size.
 * @return `0` on success, `-1` when the destination buffer is invalid.
 */
int snccliprecon_get_hostname(char *hostname, size_t hostname_size);

/**
 * @brief Render a worker-step lock path.
 *
 * The path includes hostname, job id, step id, and operation name so each
 * worker can independently guard hook work for a job step.
 *
 * @param job_id Slurm job id.
 * @param step_id Slurm step id.
 * @param hostname Worker hostname.
 * @param op Hook or operation name.
 * @param path Destination buffer.
 * @param path_size Destination buffer size.
 * @return `0` on success, `-1` on failure with `errno` set.
 */
int snccliprecon_render_worker_lock_path(
    uint32_t job_id, uint32_t step_id, const char *hostname, const char *op,
    char *path, size_t path_size
);

/**
 * @brief Create a once-per-worker lock for a job step operation.
 *
 * @param job_id Slurm job id.
 * @param step_id Slurm step id.
 * @param hostname Worker hostname.
 * @param op Hook or operation name.
 * @return `0` when the lock was created, positive when it already exists,
 *         `-1` on failure with `errno` set.
 */
int snccliprecon_create_once_per_worker_lock(
    uint32_t job_id, uint32_t step_id, const char *hostname, const char *op
);

/**
 * @brief Remove a worker-step lock if it exists.
 *
 * Cleanup failures are logged at debug2 level and do not abort the caller.
 *
 * @param job_id Slurm job id.
 * @param step_id Slurm step id.
 * @param hostname Worker hostname.
 * @param op Hook or operation name.
 */
void snccliprecon_remove_worker_lock(
    uint32_t job_id, uint32_t step_id, const char *hostname, const char *op
);

#endif
