#include "snccliprecon.h"
#include "snccliprecon_config.h"
#include "snccliprecon_enroot.h"
#include "snccliprecon_file_ops.h"
#include "snccliprecon_worker_lock.h"

#include <errno.h>
#include <stdint.h>

int snccliprecon_task_exit(spank_t spank) {
    const snccliprecon_config_t *config = snccliprecon_config_get();
    if (!config->enabled) {
        return ESPANK_SUCCESS;
    }

    uint32_t job_id  = 0;
    uint32_t step_id = 0;
    (void)spank_get_item(spank, S_JOB_ID, &job_id);
    (void)spank_get_item(spank, S_JOB_STEPID, &step_id);
    snccliprecon_log_debug2(
        "%s: job=%u step=%u", SNCCLIPRECON_TASK_EXIT_OP, job_id, step_id
    );

    if (step_id == SLURM_BATCH_SCRIPT) {
        return ESPANK_SUCCESS;
    }

    char hostname[SNCCLIPRECON_HOSTNAME_BUFFER_SIZE];
    if (snccliprecon_get_hostname(hostname, sizeof(hostname)) != 0) {
        snccliprecon_log_error_errno(errno, "Cannot read hostname");
        return ESPANK_ERROR;
    }

    if (snccliprecon_ensure_dir(SNCCLIPRECON_TMP_DIR_BASE) != 0) {
        snccliprecon_log_error_errno(
            errno, "Cannot ensure %s", SNCCLIPRECON_TMP_DIR_BASE
        );
        return ESPANK_ERROR;
    }

    int once_rc = snccliprecon_create_once_per_worker_lock(
        job_id, step_id, hostname, SNCCLIPRECON_TASK_EXIT_OP
    );
    if (once_rc > 0) {
        snccliprecon_log_debug2(
            "%s: already ran for this worker step", SNCCLIPRECON_TASK_EXIT_OP
        );
        return ESPANK_SUCCESS;
    }
    if (once_rc < 0) {
        snccliprecon_log_error_errno(
            errno, "%s: Cannot create worker lock", SNCCLIPRECON_TASK_EXIT_OP
        );
        return ESPANK_ERROR;
    }

    if (snccliprecon_dir_exists(SNCCLIPRECON_ENROOT_MOUNTS_DIR)) {
        if (snccliprecon_enroot_remove_mount_file(job_id, step_id) != 0) {
            snccliprecon_log_error_errno(
                errno, "Cannot remove Enroot mount file"
            );
            return ESPANK_ERROR;
        }
    } else {
        snccliprecon_log_debug(
            "%s: %s does not exist - skipping Enroot mount file removal",
            SNCCLIPRECON_TASK_EXIT_OP,
            SNCCLIPRECON_ENROOT_MOUNTS_DIR
        );
    }

    snccliprecon_remove_worker_lock(
        job_id, step_id, hostname, SNCCLIPRECON_USER_INIT_OP
    );
    snccliprecon_remove_worker_lock(
        job_id, step_id, hostname, SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP
    );
    snccliprecon_remove_worker_lock(
        job_id, step_id, hostname, SNCCLIPRECON_TASK_EXIT_OP
    );

    return ESPANK_SUCCESS;
}
