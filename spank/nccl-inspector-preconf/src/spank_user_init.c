#include "spank_config.h"
#include "spank_constants.h"
#include "spank_env.h"
#include "spank_shim.h"
#include "spank_substitute.h"

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static const char *format_bool(bool value) {
  return value ? "1" : "0";
}

int snccliprecon_user_init(spank_t spank) {
  const snccliprecon_config_t *config = snccliprecon_config_get();
  if (!config->enabled) {
    return ESPANK_SUCCESS;
  }

  uint32_t job_id = 0;
  (void) snccliprecon_get_job_id(spank, &job_id);

  char job_id_str[16];
  (void) snprintf(job_id_str, sizeof(job_id_str), "%u", job_id);
  snccliprecon_log_debug2("%s: job=%u", SNCCLIPRECON_USER_INIT_OP, job_id);

  snccliprecon_env_set_if_missing(spank, "NCCL_PROFILER_PLUGIN", config->profiler_plugin, SNCCLIPRECON_USER_INIT_OP);

  char dump_dir[SNCCLIPRECON_PATH_MAX];
  if (snccliprecon_substitute_job_id(config->dump_dir, job_id_str, dump_dir, sizeof(dump_dir)) != 0) {
    snccliprecon_log_errorf("%s: cannot render NCCL Inspector dump dir: %s", SNCCLIPRECON_USER_INIT_OP,
                            strerror(errno));
    return ESPANK_ERROR;
  }

  if (snccliprecon_env_set_if_missing(spank, "NCCL_INSPECTOR_DUMP_DIR", dump_dir, SNCCLIPRECON_USER_INIT_OP)) {
    (void) snccliprecon_env_set(spank, SNCCLIPRECON_LOG_DIR_SET_BY_PLUGIN, "1", SNCCLIPRECON_USER_INIT_OP);
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
