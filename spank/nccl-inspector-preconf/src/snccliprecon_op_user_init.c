#include "snccliprecon.h"
#include "snccliprecon_config.h"
#include "snccliprecon_env.h"

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

static const char *format_bool(bool value) { return value ? "1" : "0"; }

int snccliprecon_user_init(spank_t spank) {
    const snccliprecon_config_t *config = snccliprecon_config_get();
    if (!config->enabled) {
        return ESPANK_SUCCESS;
    }

    uint32_t job_id = 0;
    (void)spank_get_item(spank, S_JOB_ID, &job_id);

    char job_id_str[SNCCLIPRECON_JOB_ID_BUFFER_SIZE];
    (void)snprintf(job_id_str, sizeof(job_id_str), "%u", job_id);
    snccliprecon_log_debug2("%s: job=%u", SNCCLIPRECON_USER_INIT_OP, job_id);

    snccliprecon_env_set_if_missing(
        spank,
        "NCCL_PROFILER_PLUGIN",
        config->profiler_plugin,
        SNCCLIPRECON_USER_INIT_OP
    );

    char dump_dir[SNCCLIPRECON_PATH_MAX];
    if (snccliprecon_config_render_job_dump_dir(
            config, job_id_str, dump_dir, sizeof(dump_dir)
        ) != 0) {
        snccliprecon_log_error_errno(
            errno,
            "%s: cannot render NCCL Inspector dump dir",
            SNCCLIPRECON_USER_INIT_OP
        );
        return ESPANK_ERROR;
    }

    if (snccliprecon_env_set_if_missing(
            spank,
            "NCCL_INSPECTOR_DUMP_DIR",
            dump_dir,
            SNCCLIPRECON_USER_INIT_OP
        )) {
        (void)snccliprecon_env_set(
            spank,
            SNCCLIPRECON_LOG_DIR_SET_BY_PLUGIN,
            "1",
            SNCCLIPRECON_USER_INIT_OP
        );
    }

    snccliprecon_env_set_if_missing(
        spank,
        "NCCL_INSPECTOR_PROM_DUMP",
        format_bool(config->prom_dump),
        SNCCLIPRECON_USER_INIT_OP
    );
    snccliprecon_env_set_if_missing(
        spank,
        "NCCL_INSPECTOR_DUMP_VERBOSE",
        format_bool(config->dump_verbose),
        SNCCLIPRECON_USER_INIT_OP
    );
    snccliprecon_env_set_if_missing(
        spank,
        "NCCL_INSPECTOR_DUMP_THREAD_INTERVAL_MICROSECONDS",
        config->dump_thread_interval_microseconds,
        SNCCLIPRECON_USER_INIT_OP
    );

    return ESPANK_SUCCESS;
}
