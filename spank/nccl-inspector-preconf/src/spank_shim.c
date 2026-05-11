#include "spank_shim.h"
#include "spank_config.h"
#include "spank_env.h"
#include <slurm/spank.h>
#include <string.h>
#include <stdbool.h>

SPANK_PLUGIN(nccl_inspector_preconf, 1);

// region Fast-exit

bool nccl_inspector_enabled(spank_t spank) {
  char inspector_enabled[8] = {0};
  if (!snccliprecon_env_get(spank, "NCCL_INSPECTOR_ENABLE", inspector_enabled, sizeof(inspector_enabled))) {
    snccliprecon_log_debug("NCCL_INSPECTOR_ENABLE is not set");
    return false;
  }

  snccliprecon_log_debug2("NCCL_INSPECTOR_ENABLE=%s", inspector_enabled);
  return strcmp(inspector_enabled, "1") == 0;
}

// endregion Fast-exit

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
