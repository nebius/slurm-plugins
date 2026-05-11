#ifndef SNCCLIPRECON_CONFIG_H
#define SNCCLIPRECON_CONFIG_H

#include "snccliprecon.h"

#include <stdbool.h>
#include <stddef.h>

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

spank_err_t snccliprecon_args_register(spank_t spank);

int snccliprecon_config_render_job_dump_dir(
  const snccliprecon_config_t *config,
  const char *job_id,
  char *output,
  size_t output_size
);

int snccliprecon_config_render_step_dump_dir(
  const snccliprecon_config_t *config,
  const char *job_id,
  const char *step_id,
  char *output,
  size_t output_size
);

#endif
