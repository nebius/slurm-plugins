#include "snccliprecon.h"
#include "snccliprecon_config.h"
#include "snccliprecon_enroot.h"
#include "snccliprecon_env.h"
#include "snccliprecon_file_ops.h"
#include "snccliprecon_worker_lock.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    SNCCLIPRECON_WORKER_STEP_ALREADY_DONE = 1,
};

static int set_step_dump_dir_if_owned_by_plugin(
    spank_t spank, const snccliprecon_config_t *config, const char *job_id,
    const char *step_id, char *dump_dir, size_t dump_dir_size
) {
    char set_by_plugin[SNCCLIPRECON_ENV_FLAG_BUFFER_SIZE];
    if (!snccliprecon_env_get(
            spank,
            SNCCLIPRECON_LOG_DIR_SET_BY_PLUGIN,
            set_by_plugin,
            sizeof(set_by_plugin)
        ) ||
        strcmp(set_by_plugin, "1") != 0) {
        return ESPANK_SUCCESS;
    }

    if (snccliprecon_config_render_step_dump_dir(
            config, job_id, step_id, dump_dir, dump_dir_size
        ) != 0) {
        snccliprecon_log_error_errno(
            errno, "Cannot render NCCL Inspector dump dir"
        );
        return ESPANK_ERROR;
    }

    snccliprecon_log_debug2(
        "%s: setting step dump dir %s",
        SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP,
        dump_dir
    );
    if (!snccliprecon_env_set(
            spank,
            "NCCL_INSPECTOR_DUMP_DIR",
            dump_dir,
            SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP
        )) {
        return ESPANK_ERROR;
    }

    return ESPANK_SUCCESS;
}

static int create_worker_lock_once(
    uint32_t job_id, uint32_t step_id, char *hostname, size_t hostname_size
) {
    snccliprecon_log_debug2(
        "%s: ensuring worker lock dir %s",
        SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP,
        SNCCLIPRECON_TMP_DIR_BASE
    );
    if (snccliprecon_ensure_dir(SNCCLIPRECON_TMP_DIR_BASE) != 0) {
        snccliprecon_log_error_errno(
            errno, "Cannot ensure %s", SNCCLIPRECON_TMP_DIR_BASE
        );
        return ESPANK_ERROR;
    }

    if (snccliprecon_get_hostname(hostname, hostname_size) != 0) {
        snccliprecon_log_error_errno(errno, "Cannot read hostname");
        return ESPANK_ERROR;
    }

    int once_rc = snccliprecon_create_once_per_worker_lock(
        job_id, step_id, hostname, SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP
    );
    if (once_rc > 0) {
        snccliprecon_log_debug2(
            "%s: already ran for this worker step",
            SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP
        );
        return SNCCLIPRECON_WORKER_STEP_ALREADY_DONE;
    }

    if (once_rc < 0) {
        snccliprecon_log_error_errno(
            errno,
            "%s: Cannot create worker lock",
            SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP
        );
        return ESPANK_ERROR;
    }

    return ESPANK_SUCCESS;
}

static int
ensure_dump_dir_if_set(spank_t spank, char *dump_dir, size_t dump_dir_size) {
    if (!snccliprecon_env_get(
            spank, "NCCL_INSPECTOR_DUMP_DIR", dump_dir, dump_dir_size
        ) ||
        dump_dir[0] == '\0') {
        snccliprecon_log_debug(
            "%s: NCCL_INSPECTOR_DUMP_DIR is absent - skipping ensuring its "
            "existence",
            SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP
        );
        return ESPANK_SUCCESS;
    }

    snccliprecon_log_debug(
        "%s: ensuring dump dir %s",
        SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP,
        dump_dir
    );
    if (snccliprecon_ensure_dir(dump_dir) != 0) {
        snccliprecon_log_error_errno(
            errno, "Cannot ensure NCCL Inspector dump dir %s", dump_dir
        );
        return ESPANK_ERROR;
    }

    return ESPANK_SUCCESS;
}

static void get_profiler_plugin(
    spank_t spank, const snccliprecon_config_t *config, char *profiler_plugin
) {
    if (!snccliprecon_env_get(
            spank,
            "NCCL_PROFILER_PLUGIN",
            profiler_plugin,
            SNCCLIPRECON_PATH_MAX
        ) ||
        profiler_plugin[0] == '\0') {
        (void)snprintf(
            profiler_plugin,
            SNCCLIPRECON_PATH_MAX,
            "%s",
            config->profiler_plugin
        );
        snccliprecon_log_debug2(
            "%s: using configured profiler plugin %s",
            SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP,
            profiler_plugin
        );
        return;
    }

    snccliprecon_log_debug2(
        "%s: using env profiler plugin %s",
        SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP,
        profiler_plugin
    );
}

static int create_enroot_mount_if_ready(
    spank_t spank, const snccliprecon_config_t *config, uint32_t job_id,
    uint32_t step_id, const char *dump_dir
) {
    if (!snccliprecon_dir_exists(SNCCLIPRECON_ENROOT_MOUNTS_DIR)) {
        snccliprecon_log_debug(
            "%s: %s does not exist - skipping Enroot mount file creation",
            SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP,
            SNCCLIPRECON_ENROOT_MOUNTS_DIR
        );
        return ESPANK_SUCCESS;
    }

    if (dump_dir[0] == '\0') {
        snccliprecon_log_debug(
            "%s: NCCL_INSPECTOR_DUMP_DIR is absent - skipping Enroot mount "
            "file creation",
            SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP
        );
        return ESPANK_SUCCESS;
    }

    char profiler_plugin[SNCCLIPRECON_PATH_MAX];
    get_profiler_plugin(spank, config, profiler_plugin);

    snccliprecon_log_debug(
        "%s: creating Enroot mount file", SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP
    );
    if (snccliprecon_enroot_create_mount_file(
            job_id, step_id, profiler_plugin, dump_dir
        ) != 0) {
        snccliprecon_log_error_errno(errno, "Cannot create Enroot mount file");
        return ESPANK_ERROR;
    }

    return ESPANK_SUCCESS;
}

int snccliprecon_task_init_privileged(spank_t spank) {
    const snccliprecon_config_t *config = snccliprecon_config_get();
    if (!config->enabled) {
        return ESPANK_SUCCESS;
    }

    uint32_t job_id  = 0;
    uint32_t step_id = 0;
    (void)spank_get_item(spank, S_JOB_ID, &job_id);
    (void)spank_get_item(spank, S_JOB_STEPID, &step_id);
    snccliprecon_log_debug2(
        "%s: job=%u step=%u",
        SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP,
        job_id,
        step_id
    );

    if (step_id == SLURM_BATCH_SCRIPT) {
        return ESPANK_SUCCESS;
    }

    char job_id_str[SNCCLIPRECON_JOB_ID_BUFFER_SIZE];
    char step_id_str[SNCCLIPRECON_STEP_ID_BUFFER_SIZE];
    (void)snprintf(job_id_str, sizeof(job_id_str), "%u", job_id);
    (void)snprintf(step_id_str, sizeof(step_id_str), "%u", step_id);

    char dump_dir[SNCCLIPRECON_PATH_MAX];
    if (set_step_dump_dir_if_owned_by_plugin(
            spank, config, job_id_str, step_id_str, dump_dir, sizeof(dump_dir)
        ) != ESPANK_SUCCESS) {
        return ESPANK_ERROR;
    }

    char hostname[SNCCLIPRECON_HOSTNAME_BUFFER_SIZE];
    int  lock_rc =
        create_worker_lock_once(job_id, step_id, hostname, sizeof(hostname));
    if (lock_rc == SNCCLIPRECON_WORKER_STEP_ALREADY_DONE) {
        return ESPANK_SUCCESS;
    }

    if (lock_rc != ESPANK_SUCCESS) {
        return ESPANK_ERROR;
    }

    if (ensure_dump_dir_if_set(spank, dump_dir, sizeof(dump_dir)) !=
        ESPANK_SUCCESS) {
        return ESPANK_ERROR;
    }

    return create_enroot_mount_if_ready(
        spank, config, job_id, step_id, dump_dir
    );
}
