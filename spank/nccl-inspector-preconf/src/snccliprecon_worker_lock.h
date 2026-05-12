#ifndef SNCCLIPRECON_WORKER_LOCK_H
#define SNCCLIPRECON_WORKER_LOCK_H

#include <stddef.h>
#include <stdint.h>

int snccliprecon_get_hostname(char *hostname, size_t hostname_size);

int snccliprecon_render_worker_lock_path(
    uint32_t job_id, uint32_t step_id, const char *hostname, const char *op,
    char *path, size_t path_size
);

int snccliprecon_create_once_per_worker_lock(
    uint32_t job_id, uint32_t step_id, const char *hostname, const char *op
);

void snccliprecon_remove_worker_lock(
    uint32_t job_id, uint32_t step_id, const char *hostname, const char *op
);

#endif
