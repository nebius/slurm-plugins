#include "spank_config.h"
#include "spank_constants.h"
#include "spank_enroot.h"
#include "spank_file_ops.h"
#include "spank_shim.h"

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SNCCLIPRECON_LOG_DIR_SET_BY_PLUGIN "SNCCLIPRECON_LOG_DIR_SET_BY_PLUGIN"

static bool get_env(spank_t spank, const char *key, char *buffer, size_t buffer_size) {
  if (buffer_size == 0) {
    return false;
  }

  buffer[0] = '\0';
  return spank_getenv(spank, key, buffer, (int) buffer_size) == ESPANK_SUCCESS;
}

static int append_string(char *dst, size_t dst_size, size_t *offset, const char *value) {
  size_t value_len = strlen(value);
  if (*offset + value_len >= dst_size) {
    errno = ENAMETOOLONG;
    return -1;
  }

  memcpy(dst + *offset, value, value_len);
  *offset += value_len;
  dst[*offset] = '\0';
  return 0;
}

static int substitute_job_step_id(
  const char *input,
  const char *job_id,
  const char *step_id,
  char *output,
  size_t output_size
) {
  size_t offset = 0;
  output[0] = '\0';

  for (size_t i = 0; input[i] != '\0'; ++i) {
    if (input[i] == '%' && input[i + 1] == 'j') {
      if (append_string(output, output_size, &offset, job_id) != 0) {
        return -1;
      }
      ++i;
      continue;
    }

    if (input[i] == '%' && input[i + 1] == 's') {
      if (append_string(output, output_size, &offset, step_id) != 0) {
        return -1;
      }
      ++i;
      continue;
    }

    if (input[i] == '%' && input[i + 1] == 'J') {
      if (append_string(output, output_size, &offset, job_id) != 0 ||
          append_string(output, output_size, &offset, ".") != 0 ||
          append_string(output, output_size, &offset, step_id) != 0) {
        return -1;
      }
      ++i;
      continue;
    }

    if (offset + 1 >= output_size) {
      errno = ENAMETOOLONG;
      return -1;
    }

    output[offset++] = input[i];
    output[offset] = '\0';
  }

  return 0;
}

int snccliprecon_task_init_privileged(spank_t spank) {
  const snccliprecon_config_t *config = snccliprecon_config_get();
  if (!config->enabled) {
    return ESPANK_SUCCESS;
  }

  uint32_t job_id = 0;
  uint32_t step_id = 0;
  (void) snccliprecon_get_job_id(spank, &job_id);
  (void) snccliprecon_get_step_id(spank, &step_id);
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
  if (get_env(spank, SNCCLIPRECON_LOG_DIR_SET_BY_PLUGIN, set_by_plugin, sizeof(set_by_plugin)) &&
      strcmp(set_by_plugin, "1") == 0) {
    if (substitute_job_step_id(config->dump_dir, job_id_str, step_id_str, dump_dir, sizeof(dump_dir)) != 0) {
      snccliprecon_log_errorf("Cannot render NCCL Inspector dump dir: %s", strerror(errno));
      return ESPANK_ERROR;
    }

    snccliprecon_log_debug2("%s: setting step dump dir %s", SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP, dump_dir);
    if (spank_setenv(spank, "NCCL_INSPECTOR_DUMP_DIR", dump_dir, 1) != ESPANK_SUCCESS) {
      snccliprecon_log_error("Cannot set NCCL_INSPECTOR_DUMP_DIR");
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

  if (get_env(spank, "NCCL_INSPECTOR_DUMP_DIR", dump_dir, sizeof(dump_dir)) && dump_dir[0] != '\0') {
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
  if (!get_env(spank, "NCCL_PROFILER_PLUGIN", profiler_plugin, sizeof(profiler_plugin)) ||
      profiler_plugin[0] == '\0') {
    (void) snprintf(profiler_plugin, sizeof(profiler_plugin), "%s", config->profiler_plugin);
    snccliprecon_log_debug2("%s: using configured profiler plugin %s", SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP,
                            profiler_plugin);
  } else {
    snccliprecon_log_debug2("%s: using env profiler plugin %s", SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP, profiler_plugin);
  }

  if (!get_env(spank, "NCCL_INSPECTOR_DUMP_DIR", dump_dir, sizeof(dump_dir)) || dump_dir[0] == '\0') {
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
