#include "spank_shim.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

spank_context_t snccliprecon_spank_context() {
  return spank_context();
}

void snccliprecon_log(const char *msg) {
  slurm_spank_log(msg);
}

spank_err_t snccliprecon_getenv(spank_t spank, const char *key, char *buffer, int const length) {
  return spank_getenv(spank, key, buffer, length);
}

spank_err_t snccliprecon_setenv(spank_t spank, const char *key, const char *value) {
  return spank_setenv(spank, key, value, 1);
}
