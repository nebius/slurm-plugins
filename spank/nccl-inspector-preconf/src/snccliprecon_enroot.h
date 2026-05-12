#ifndef SNCCLIPRECON_ENROOT_H
#define SNCCLIPRECON_ENROOT_H

#include <stdint.h>

#define SNCCLIPRECON_ENROOT_MOUNTS_DIR "/etc/enroot/mounts.d"

/**
 * @brief Create the Enroot mount file for a job step.
 *
 * The mount file binds the NCCL profiler plugin shared object read-only and
 * the NCCL Inspector dump directory read-write into Enroot containers.
 *
 * @param job_id Slurm job id.
 * @param step_id Slurm step id.
 * @param profiler_plugin Path to the NCCL profiler plugin shared object.
 * @param dump_dir Path to the NCCL Inspector dump directory.
 * @return `0` on success, `-1` on failure with `errno` set.
 */
int snccliprecon_enroot_create_mount_file(
    uint32_t job_id, uint32_t step_id, const char *profiler_plugin,
    const char *dump_dir
);

/**
 * @brief Remove the Enroot mount file for a job step.
 *
 * Missing mount files are treated as a successful cleanup.
 *
 * @param job_id Slurm job id.
 * @param step_id Slurm step id.
 * @return `0` on success, `-1` on failure with `errno` set.
 */
int snccliprecon_enroot_remove_mount_file(uint32_t job_id, uint32_t step_id);

#endif
