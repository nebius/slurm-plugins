#include "spank_shim.h"
#include <slurm/spank.h>

SPANK_PLUGIN(nccl_inspector_preconf, 1);

int slurm_spank_init(spank_t spank, int argc, char **argv) {
  if (snccliprecon_args_register(spank) != ESPANK_SUCCESS) {
    return ESPANK_ERROR;
  }

  return snccliprecon_spank_init(spank, argc, argv);
}

int slurm_spank_user_init(spank_t spank, int argc, char **argv) {
  return snccliprecon_spank_user_init(spank, argc, argv);
}

int slurm_spank_task_init_privileged(spank_t spank, int argc, char **argv) {
  return snccliprecon_spank_task_init_privileged(spank, argc, argv);
}

int slurm_spank_task_exit(spank_t spank, int argc, char **argv) {
  return snccliprecon_spank_task_exit(spank, argc, argv);
}

uint32_t snccliprecon_slurm_batch_script_id = (uint32_t)(0xfffffffb);
