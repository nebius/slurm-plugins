#ifndef SNCCLIPRECON_FILE_OPS_H
#define SNCCLIPRECON_FILE_OPS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef int (*snccliprecon_locked_fn_t)(void *arg);

int snccliprecon_ensure_mode(const char *path);

bool snccliprecon_dir_exists(const char *path);

int snccliprecon_mkdir_all(const char *path);

int snccliprecon_ensure_dir(const char *path);

int snccliprecon_with_lock(const char *path, snccliprecon_locked_fn_t fn, void *arg);

int snccliprecon_get_hostname(char *hostname, size_t hostname_size);

int snccliprecon_render_worker_lock_path(
  uint32_t job_id,
  uint32_t step_id,
  const char *hostname,
  const char *op,
  char *path,
  size_t path_size
);

int snccliprecon_create_once_per_worker_lock(
  uint32_t job_id,
  uint32_t step_id,
  const char *hostname,
  const char *op
);

void snccliprecon_remove_worker_lock(
  uint32_t job_id,
  uint32_t step_id,
  const char *hostname,
  const char *op
);

#endif
