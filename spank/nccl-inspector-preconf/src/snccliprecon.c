#include "snccliprecon.h"
#include "snccliprecon_config.h"
#include "snccliprecon_env.h"
#include <stdarg.h>
#include <stdio.h>
#include <slurm/spank.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

SPANK_PLUGIN(nccl_inspector_preconf, 1);

typedef void (*snccliprecon_log_fn_t)(const char *format, ...);

#define SNCCLIPRECON_LOG_BUFFER_SIZE 2048
#define SNCCLIPRECON_HOSTNAME_BUFFER_SIZE 256

static const char *snccliprecon_log_hostname(char *hostname, size_t hostname_size) {
  if (hostname_size == 0) {
    return "unknown";
  }

  if (gethostname(hostname, hostname_size) != 0 || hostname[0] == '\0') {
    (void) snprintf(hostname, hostname_size, "%s", "unknown");
  }
  hostname[hostname_size - 1] = '\0';

  return hostname;
}

static void snccliprecon_vlog_at(snccliprecon_log_fn_t log_fn, const char *format, va_list args) {
  char buffer[SNCCLIPRECON_LOG_BUFFER_SIZE];
  char hostname[SNCCLIPRECON_HOSTNAME_BUFFER_SIZE];

  vsnprintf(buffer, sizeof(buffer), format, args);
  log_fn(SNCCLIPRECON_LOG_PREFIX_FORMAT "%s", snccliprecon_log_hostname(hostname, sizeof(hostname)), buffer);
}

void snccliprecon_log_errorf(const char *format, ...) {
  va_list args;

  va_start(args, format);
  snccliprecon_vlog_at(slurm_error, format, args);
  va_end(args);
}

void snccliprecon_log_debug(const char *format, ...) {
  va_list args;

  va_start(args, format);
  snccliprecon_vlog_at(slurm_debug, format, args);
  va_end(args);
}

void snccliprecon_log_debug2(const char *format, ...) {
  va_list args;

  va_start(args, format);
  snccliprecon_vlog_at(slurm_debug2, format, args);
  va_end(args);
}

// region Fast-exit

static bool nccl_inspector_enabled(spank_t spank) {
  char inspector_enabled[8] = {0};
  if (!snccliprecon_env_get(spank, "NCCL_INSPECTOR_ENABLE", inspector_enabled, sizeof(inspector_enabled))) {
    snccliprecon_log_debug("NCCL_INSPECTOR_ENABLE is not set");
    return false;
  }

  snccliprecon_log_debug2("NCCL_INSPECTOR_ENABLE=%s", inspector_enabled);
  return strcmp(inspector_enabled, "1") == 0;
}

// endregion Fast-exit

int snccliprecon_spank_init(spank_t spank, int argc, char **argv) {
  switch (spank_context()) {
    case S_CTX_LOCAL:
    case S_CTX_REMOTE:
      break;
    default:
      return ESPANK_SUCCESS;
  }

  if (snccliprecon_args_register(spank) != ESPANK_SUCCESS) {
    return ESPANK_ERROR;
  }
  snccliprecon_config_parse_args(spank, argc, argv);

  return ESPANK_SUCCESS;
}

/**
 * Runs plugin initialization.
 *
 * @param spank: SPANK context.
 * @param argc: Number of plugin arguments.
 * @param argv: Plugin argument vector.
 *
 * @return SPANK status code.
 */
int slurm_spank_init(spank_t spank, int argc, char **argv) {
  return snccliprecon_spank_init(spank, argc, argv);
}

/**
 * Exports NCCL Inspector environment defaults before user code starts.
 *
 * @param spank: SPANK context.
 * @param argc: Number of plugin arguments.
 * @param argv: Plugin argument vector.
 *
 * @return SPANK status code.
 */
int slurm_spank_user_init(spank_t spank, int argc, char **argv) {
  (void) argc;
  (void) argv;

  if (spank_remote(spank) != 1) {
    return ESPANK_SUCCESS;
  }

  if (!nccl_inspector_enabled(spank)) {
    return ESPANK_SUCCESS;
  }

  return snccliprecon_user_init(spank);
}

/**
 * Prepares worker-local NCCL Inspector files before launching a task.
 *
 * @param spank: SPANK context.
 * @param argc: Number of plugin arguments.
 * @param argv: Plugin argument vector.
 *
 * @return SPANK status code.
 */
int slurm_spank_task_init_privileged(spank_t spank, int argc, char **argv) {
  (void) argc;
  (void) argv;

  if (spank_remote(spank) != 1) {
    return ESPANK_SUCCESS;
  }

  if (!nccl_inspector_enabled(spank)) {
    return ESPANK_SUCCESS;
  }

  return snccliprecon_task_init_privileged(spank);
}

/**
 * Cleans up worker-local NCCL Inspector files after task exit.
 *
 * @param spank: SPANK context.
 * @param argc: Number of plugin arguments.
 * @param argv: Plugin argument vector.
 *
 * @return SPANK status code.
 */
int slurm_spank_task_exit(spank_t spank, int argc, char **argv) {
  (void) argc;
  (void) argv;

  if (spank_remote(spank) != 1) {
    return ESPANK_SUCCESS;
  }

  if (!nccl_inspector_enabled(spank)) {
    return ESPANK_SUCCESS;
  }

  return snccliprecon_task_exit(spank);
}
