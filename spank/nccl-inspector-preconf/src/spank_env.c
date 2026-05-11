#include "spank_config.h"
#include "spank_env.h"
#include "spank_shim.h"

spank_err_t snccliprecon_getenv(spank_t spank, const char *key, char *buffer, int const length) {
  return spank_getenv(spank, key, buffer, length);
}

spank_err_t snccliprecon_setenv(spank_t spank, const char *key, const char *value) {
  return spank_setenv(spank, key, value, 1);
}

bool snccliprecon_env_get(spank_t spank, const char *key, char *buffer, size_t buffer_size) {
  if (buffer_size == 0) {
    return false;
  }

  buffer[0] = '\0';
  return snccliprecon_getenv(spank, key, buffer, (int) buffer_size) == ESPANK_SUCCESS;
}

bool snccliprecon_env_exists(spank_t spank, const char *key) {
  char buffer[SNCCLIPRECON_PATH_MAX];
  buffer[0] = '\0';
  return snccliprecon_env_get(spank, key, buffer, sizeof(buffer));
}

bool snccliprecon_env_set(spank_t spank, const char *key, const char *value, const char *op) {
  if (snccliprecon_setenv(spank, key, value) != ESPANK_SUCCESS) {
    snccliprecon_log_errorf("Cannot set %s", key);
    return false;
  }

  snccliprecon_log_debug2("%s: set %s=%s", op, key, value);
  return true;
}

bool snccliprecon_env_set_if_missing(spank_t spank, const char *key, const char *value, const char *op) {
  if (snccliprecon_env_exists(spank, key)) {
    snccliprecon_log_debug2("%s: keeping existing %s", op, key);
    return false;
  }

  return snccliprecon_env_set(spank, key, value, op);
}
