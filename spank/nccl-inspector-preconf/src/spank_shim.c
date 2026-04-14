#include "spank_shim.h"
#include <slurm/spank.h>

SPANK_PLUGIN(nccl_inspector_preconf, 1);

int slurm_spank_init(spank_t spank, int argc, char **argv) {
  return go_spank_init(spank, ac, argv);
}

int slurm_spank_user_init(spank_t spank, int argc, char **argv) {
  return go_spank_user_init(spank, ac, argv);
}

int slurm_spank_task_init_privileged(spank_t spank, int argc, char **argv) {
  return go_spank_task_init_privileged(spank, ac, argv);
}

int slurm_spank_exit(spank_t spank, int argc, char **argv) {
  return go_spank_exit(spank, argc, argv);
}
