#ifndef SNCCLIPRECON_SUBSTITUTE_H
#define SNCCLIPRECON_SUBSTITUTE_H

#include <stddef.h>

int snccliprecon_substitute_job_id(
  const char *input,
  const char *job_id,
  char *output,
  size_t output_size
);

int snccliprecon_substitute_job_step_id(
  const char *input,
  const char *job_id,
  const char *step_id,
  char *output,
  size_t output_size
);

#endif
