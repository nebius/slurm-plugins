#ifndef SNCCLIPRECON_CONFIG_H
#define SNCCLIPRECON_CONFIG_H

#include <stdbool.h>
#include <slurm/spank.h>
#include <stddef.h>

#ifndef SNCCLIPRECON_PATH_MAX
#define SNCCLIPRECON_PATH_MAX 4096
#endif

typedef struct snccliprecon_config {
  bool enabled;
  char profiler_plugin[SNCCLIPRECON_PATH_MAX];
  char dump_dir[SNCCLIPRECON_PATH_MAX];
} snccliprecon_config_t;

const snccliprecon_config_t *snccliprecon_config_get(void);
void snccliprecon_config_reset(void);
void snccliprecon_config_parse_args(spank_t spank, int argc, char **argv);
spank_err_t snccliprecon_config_parse_option(const char *name, const char *value);

#endif
