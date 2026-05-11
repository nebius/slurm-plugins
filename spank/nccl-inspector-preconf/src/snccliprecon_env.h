#ifndef SNCCLIPRECON_ENV_H
#define SNCCLIPRECON_ENV_H

#include <stdbool.h>
#include <stddef.h>
#include <slurm/spank.h>

bool snccliprecon_env_get(spank_t spank, const char *key, char *buffer, size_t buffer_size);

bool snccliprecon_env_exists(spank_t spank, const char *key);

bool snccliprecon_env_set(spank_t spank, const char *key, const char *value, const char *op);

bool snccliprecon_env_set_if_missing(spank_t spank, const char *key, const char *value, const char *op);

#endif
