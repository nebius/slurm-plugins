#include "spank_shim.h"
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

spank_context_t snccliprecon_spank_context() {
  return spank_context();
}

spank_err_t snccliprecon_get_job_id(spank_t spank, uint32_t *job_id) {
  return spank_get_item(spank, S_JOB_ID, job_id);
}

void snccliprecon_log(const char *msg) {
  slurm_spank_log("%s", msg);
}

void snccliprecon_log_error(const char *msg) {
  slurm_error("%s", msg);
}

void snccliprecon_log_error_fmt(const char *fmt, ...) {
  char buffer[1024];
  va_list args;

  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  slurm_error("%s", buffer);
}

spank_err_t snccliprecon_getenv(spank_t spank, const char *key, char *buffer, int const length) {
  return spank_getenv(spank, key, buffer, length);
}

spank_err_t snccliprecon_setenv(spank_t spank, const char *key, const char *value) {
  return spank_setenv(spank, key, value, 1);
}

spank_err_t snccliprecon_parse_option(const char *name, const char *value) {
  return go_spank_parse_option((char *)name, (char *)value);
}
