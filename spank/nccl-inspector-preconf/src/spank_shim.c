#include "spank_shim.h"
#include <slurm/spank.h>
#include <string.h>
#include <stdbool.h>

SPANK_PLUGIN(nccl_inspector_preconf, 1);

// region Fast-exit

bool nccl_inspector_enabled(spank_t spank) {
  char inspector_enabled[8] = {0};
  if (spank_getenv(spank, "NCCL_INSPECTOR_ENABLE", inspector_enabled, sizeof(inspector_enabled)) != ESPANK_SUCCESS) {
    return false;
  }

  return strcmp(inspector_enabled, "1") == 0;
}

// endregion Fast-exit

// region Logging

static const char *PLUGIN_LOG_PREFIX = "[nccl_inspector_preconf]: ";
static const char *PLUGIN_LOG_FAST_EXIT = " - fast-exiting...";
void log_fast_exit(const char *reason) {
  slurm_spank_log("%s%s%s", PLUGIN_LOG_PREFIX, reason, PLUGIN_LOG_FAST_EXIT);
}

// endregion Logging

/**
 * Bridges Slurm's init hook to generated option registration and Go logic.
 *
 * @param spank: SPANK context.
 * @param argc: Number of plugin arguments.
 * @param argv: Plugin argument vector.
 *
 * @return SPANK status code.
 */
int slurm_spank_init(spank_t spank, int argc, char **argv) {
  if (snccliprecon_args_register(spank) != ESPANK_SUCCESS) {
    return ESPANK_ERROR;
  }

  return snccliprecon_spank_init(spank, argc, argv);
}

/**
 * Bridges Slurm's user-init hook to Go logic.
 *
 * @param spank: SPANK context.
 * @param argc: Number of plugin arguments.
 * @param argv: Plugin argument vector.
 *
 * @return SPANK status code.
 */
int slurm_spank_user_init(spank_t spank, int argc, char **argv) {
  if (spank_remote(spank) != 1) {
    log_fast_exit("not running in remote context");
    return ESPANK_SUCCESS;
  }

  if (!nccl_inspector_enabled(spank)) {
    log_fast_exit("NCCL Inspector is not enabled explicitly");
    return ESPANK_SUCCESS;
  }

  return snccliprecon_spank_user_init(spank, argc, argv);
}

/**
 * Bridges Slurm's privileged task-init hook to Go logic.
 *
 * @param spank: SPANK context.
 * @param argc: Number of plugin arguments.
 * @param argv: Plugin argument vector.
 *
 * @return SPANK status code.
 */
int slurm_spank_task_init_privileged(spank_t spank, int argc, char **argv) {
  if (spank_remote(spank) != 1) {
    log_fast_exit("not running in remote context");
    return ESPANK_SUCCESS;
  }

  if (!nccl_inspector_enabled(spank)) {
    log_fast_exit("NCCL Inspector is not enabled explicitly");
    return ESPANK_SUCCESS;
  }

  return snccliprecon_spank_task_init_privileged(spank, argc, argv);
}

/**
 * Bridges Slurm's task-exit hook to Go logic.
 *
 * @param spank: SPANK context.
 * @param argc: Number of plugin arguments.
 * @param argv: Plugin argument vector.
 *
 * @return SPANK status code.
 */
int slurm_spank_task_exit(spank_t spank, int argc, char **argv) {
  if (spank_remote(spank) != 1) {
    log_fast_exit("not running in remote context");
    return ESPANK_SUCCESS;
  }

  if (!nccl_inspector_enabled(spank)) {
    log_fast_exit("NCCL Inspector is not enabled explicitly");
    return ESPANK_SUCCESS;
  }

  return snccliprecon_spank_task_exit(spank, argc, argv);
}

/**
 * Slurm sentinel step ID used to detect batch-script contexts.
 */
uint32_t snccliprecon_slurm_batch_script_id = (uint32_t) (0xfffffffb);
