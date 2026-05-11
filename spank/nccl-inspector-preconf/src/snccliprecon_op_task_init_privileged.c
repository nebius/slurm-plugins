#include "snccliprecon_config.h"
#include "snccliprecon_enroot.h"
#include "snccliprecon_env.h"
#include "snccliprecon_file_ops.h"
#include "snccliprecon.h"
#include "snccliprecon_worker_lock.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int snccliprecon_task_init_privileged(spank_t spank) {
  const snccliprecon_config_t *config = snccliprecon_config_get();
  if (!config->enabled) {
    return ESPANK_SUCCESS;
  }

  uint32_t job_id = 0;
  uint32_t step_id = 0;
  (void) spank_get_item(spank, S_JOB_ID, &job_id);
  (void) spank_get_item(spank, S_JOB_STEPID, &step_id);
  snccliprecon_log_debug2("%s: job=%u step=%u", SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP, job_id, step_id);

  if (step_id == SLURM_BATCH_SCRIPT) {
    return ESPANK_SUCCESS;
  }

  char job_id_str[16];
  char step_id_str[16];
  (void) snprintf(job_id_str, sizeof(job_id_str), "%u", job_id);
  (void) snprintf(step_id_str, sizeof(step_id_str), "%u", step_id);

  char dump_dir[SNCCLIPRECON_PATH_MAX];
  char set_by_plugin[8];
  if (snccliprecon_env_get(spank, SNCCLIPRECON_LOG_DIR_SET_BY_PLUGIN, set_by_plugin, sizeof(set_by_plugin)) &&
      strcmp(set_by_plugin, "1") == 0) {
    if (snccliprecon_config_render_step_dump_dir(
          config,
          job_id_str,
          step_id_str,
          dump_dir,
          sizeof(dump_dir)
        ) != 0) {
      snccliprecon_log_errorf("Cannot render NCCL Inspector dump dir: %s", strerror(errno));
      return ESPANK_ERROR;
    }

    snccliprecon_log_debug2("%s: setting step dump dir %s", SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP, dump_dir);
    if (!snccliprecon_env_set(spank, "NCCL_INSPECTOR_DUMP_DIR", dump_dir, SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP)) {
      return ESPANK_ERROR;
    }
  }

  snccliprecon_log_debug2("%s: ensuring worker lock dir %s", SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP,
                          SNCCLIPRECON_TMP_DIR_BASE);
  if (snccliprecon_ensure_dir(SNCCLIPRECON_TMP_DIR_BASE) != 0) {
    snccliprecon_log_errorf("Cannot ensure %s: %s", SNCCLIPRECON_TMP_DIR_BASE, strerror(errno));
    return ESPANK_ERROR;
  }

  char hostname[256];
  if (snccliprecon_get_hostname(hostname, sizeof(hostname)) != 0) {
    snccliprecon_log_errorf("Cannot read hostname: %s", strerror(errno));
    return ESPANK_ERROR;
  }

  int once_rc = snccliprecon_create_once_per_worker_lock(
    job_id,
    step_id,
    hostname,
    SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP
  );
  if (once_rc > 0) {
    snccliprecon_log_debug2("%s: already ran for this worker step", SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP);
    return ESPANK_SUCCESS;
  }
  if (once_rc < 0) {
    snccliprecon_log_errorf("%s: Cannot create worker lock: %s", SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP, strerror(errno));
    return ESPANK_ERROR;
  }

  if (snccliprecon_env_get(spank, "NCCL_INSPECTOR_DUMP_DIR", dump_dir, sizeof(dump_dir)) && dump_dir[0] != '\0') {
    snccliprecon_log_debug("%s: ensuring dump dir %s", SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP, dump_dir);
    if (snccliprecon_ensure_dir(dump_dir) != 0) {
      snccliprecon_log_errorf("Cannot ensure NCCL Inspector dump dir %s: %s", dump_dir, strerror(errno));
      return ESPANK_ERROR;
    }
  } else {
    snccliprecon_log_debug("%s: NCCL_INSPECTOR_DUMP_DIR is absent - skipping ensuring its existence",
                           SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP);
  }

  if (!snccliprecon_dir_exists(SNCCLIPRECON_ENROOT_MOUNTS_DIR)) {
    snccliprecon_log_debug("%s: %s does not exist - skipping Enroot mount file creation",
                           SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP, SNCCLIPRECON_ENROOT_MOUNTS_DIR);
    return ESPANK_SUCCESS;
  }

  char profiler_plugin[SNCCLIPRECON_PATH_MAX];
  if (!snccliprecon_env_get(spank, "NCCL_PROFILER_PLUGIN", profiler_plugin, sizeof(profiler_plugin)) ||
      profiler_plugin[0] == '\0') {
    (void) snprintf(profiler_plugin, sizeof(profiler_plugin), "%s", config->profiler_plugin);
    snccliprecon_log_debug2("%s: using configured profiler plugin %s", SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP,
                            profiler_plugin);
  } else {
    snccliprecon_log_debug2("%s: using env profiler plugin %s", SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP, profiler_plugin);
  }

  if (!snccliprecon_env_get(spank, "NCCL_INSPECTOR_DUMP_DIR", dump_dir, sizeof(dump_dir)) || dump_dir[0] == '\0') {
    snccliprecon_log_debug("%s: NCCL_INSPECTOR_DUMP_DIR is absent - skipping Enroot mount file creation",
                           SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP);
    return ESPANK_SUCCESS;
  }

  snccliprecon_log_debug("%s: creating Enroot mount file", SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP);
  if (snccliprecon_enroot_create_mount_file(job_id, step_id, profiler_plugin, dump_dir) != 0) {
    snccliprecon_log_errorf("Cannot create Enroot mount file: %s", strerror(errno));
    return ESPANK_ERROR;
  }

  return ESPANK_SUCCESS;
}
