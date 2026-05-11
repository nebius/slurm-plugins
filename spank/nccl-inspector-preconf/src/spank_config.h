#ifndef SNCCLIPRECON_CONFIG_H
#define SNCCLIPRECON_CONFIG_H

#include <stdbool.h>
#include <slurm/spank.h>
#include <stddef.h>

#define SNCCLIPRECON_PATH_MAX 4096

typedef struct snccliprecon_config {
  bool enabled;
  char profiler_plugin[SNCCLIPRECON_PATH_MAX];
  bool prom_dump;
  char dump_dir[SNCCLIPRECON_PATH_MAX];
  bool dump_verbose;
  char dump_thread_interval_microseconds[64];
} snccliprecon_config_t;

const snccliprecon_config_t *snccliprecon_config_get(void);

void snccliprecon_config_reset(void);

void snccliprecon_config_parse_args(spank_t spank, int argc, char **argv);

spank_err_t snccliprecon_config_parse_option(const char *name, const char *value);

#endif
