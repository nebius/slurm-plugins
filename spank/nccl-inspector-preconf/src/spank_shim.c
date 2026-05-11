#include "spank_shim.h"
#include "spank_config.h"
#include <slurm/spank.h>
#include <string.h>
#include <stdbool.h>

SPANK_PLUGIN(nccl_inspector_preconf, 1);

// region Fast-exit

bool nccl_inspector_enabled(spank_t spank) {
  char inspector_enabled[8] = {0};
  if (spank_getenv(spank, "NCCL_INSPECTOR_ENABLE", inspector_enabled, sizeof(inspector_enabled)) != ESPANK_SUCCESS) {
    snccliprecon_log_debug("NCCL_INSPECTOR_ENABLE is not set");
    return false;
  }

  snccliprecon_log_debug2("NCCL_INSPECTOR_ENABLE=%s", inspector_enabled);
  return strcmp(inspector_enabled, "1") == 0;
}

// endregion Fast-exit

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

  snccliprecon_config_parse_args(spank, argc, argv);

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
    return ESPANK_SUCCESS;
  }

  if (!nccl_inspector_enabled(spank)) {
    return ESPANK_SUCCESS;
  }

  return snccliprecon_spank_user_init(spank, argc, argv);
}

/**
 * Bridges Slurm's privileged task-init hook to C logic.
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
 * Bridges Slurm's task-exit hook to C logic.
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

/**
 * Slurm sentinel step ID used to detect batch-script contexts.
 */
uint32_t snccliprecon_slurm_batch_script_id = (uint32_t) (SLURM_BATCH_SCRIPT);
