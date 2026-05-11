#ifndef SNCCLIPRECON_FILE_OPS_H
#define SNCCLIPRECON_FILE_OPS_H

#include <stdbool.h>
#include <stddef.h>

typedef int (*snccliprecon_locked_fn_t)(void *arg);

int snccliprecon_ensure_mode(const char *path);

bool snccliprecon_dir_exists(const char *path);

int snccliprecon_mkdir_all(const char *path);

int snccliprecon_ensure_dir(const char *path);

int snccliprecon_with_lock(const char *path, snccliprecon_locked_fn_t fn, void *arg);

#endif
