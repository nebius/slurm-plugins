#ifndef SNCCLIPRECON_ENROOT_H
#define SNCCLIPRECON_ENROOT_H

#include <stdint.h>

#define SNCCLIPRECON_ENROOT_MOUNTS_DIR "/etc/enroot/mounts.d"

int snccliprecon_enroot_create_mount_file(
  uint32_t job_id,
  uint32_t step_id,
  const char *profiler_plugin,
  const char *dump_dir
);

int snccliprecon_enroot_remove_mount_file(uint32_t job_id, uint32_t step_id);

#endif
