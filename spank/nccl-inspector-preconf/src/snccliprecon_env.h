#ifndef SNCCLIPRECON_ENV_H
#define SNCCLIPRECON_ENV_H

#include <slurm/spank.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Read a value from the SPANK job environment.
 *
 * The output buffer is always initialized to an empty string before calling
 * Slurm.
 *
 * @param spank SPANK context.
 * @param key Environment variable name.
 * @param buffer Destination buffer.
 * @param buffer_size Destination buffer size.
 * @return `true` when Slurm returned the variable, otherwise `false`.
 */
bool snccliprecon_env_get(
    spank_t spank, const char *key, char *buffer, size_t buffer_size
);

/**
 * @brief Check whether a variable exists in the SPANK job environment.
 *
 * @param spank SPANK context.
 * @param key Environment variable name.
 * @return `true` when the variable exists, otherwise `false`.
 */
bool snccliprecon_env_exists(spank_t spank, const char *key);

/**
 * @brief Set or overwrite a variable in the SPANK job environment.
 *
 * @param spank SPANK context.
 * @param key Environment variable name.
 * @param value Environment variable value.
 * @param op Hook or operation name used in debug logs.
 * @return `true` on success, otherwise `false`.
 */
bool snccliprecon_env_set(
    spank_t spank, const char *key, const char *value, const char *op
);

/**
 * @brief Set a variable only when it is absent from the job environment.
 *
 * @param spank SPANK context.
 * @param key Environment variable name.
 * @param value Environment variable value.
 * @param op Hook or operation name used in debug logs.
 * @return `true` when the variable was set, otherwise `false`.
 */
bool snccliprecon_env_set_if_missing(
    spank_t spank, const char *key, const char *value, const char *op
);

#endif
