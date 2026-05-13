#include "snccliprecon_env.h"
#include "snccliprecon.h"

bool snccliprecon_env_get(
    spank_t spank, const char *key, char *buffer, size_t buffer_size
) {
    if (buffer_size == 0) {
        return false;
    }

    buffer[0] = '\0';
    return spank_getenv(spank, key, buffer, (int)buffer_size) == ESPANK_SUCCESS;
}

bool snccliprecon_env_exists(spank_t spank, const char *key) {
    char buffer[SNCCLIPRECON_PATH_MAX];
    buffer[0] = '\0';
    return snccliprecon_env_get(spank, key, buffer, sizeof(buffer));
}

bool snccliprecon_env_set(
    spank_t spank, const char *key, const char *value, const char *op
) {
    if (spank_setenv(spank, key, value, 1) != ESPANK_SUCCESS) {
        snccliprecon_log_errorf("Cannot set %s", key);
        return false;
    }

    snccliprecon_log_debug2("%s: set %s=%s", op, key, value);
    return true;
}

bool snccliprecon_env_set_if_missing(
    spank_t spank, const char *key, const char *value, const char *op
) {
    if (snccliprecon_env_exists(spank, key)) {
        snccliprecon_log_debug2("%s: keeping existing %s", op, key);
        return false;
    }

    return snccliprecon_env_set(spank, key, value, op);
}
