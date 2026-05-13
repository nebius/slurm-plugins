#ifndef SNCCLIPRECON_CONFIG_H
#define SNCCLIPRECON_CONFIG_H

#include "snccliprecon.h"

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Process-local plugin configuration.
 *
 * The structure stores the effective NCCL Inspector defaults after built-in
 * defaults, plugstack arguments, environment overrides, and SPANK options are
 * parsed for the current process context.
 */
typedef struct snccliprecon_config {
    bool enabled;
    char profiler_plugin[SNCCLIPRECON_PATH_MAX];
    bool prom_dump;
    char dump_dir[SNCCLIPRECON_PATH_MAX];
    bool dump_verbose;
    char dump_thread_interval_microseconds
        [SNCCLIPRECON_DUMP_THREAD_INTERVAL_BUFFER_SIZE];
} snccliprecon_config_t;

/**
 * @brief Return the active process-local plugin configuration.
 *
 * @return Pointer to the current configuration.
 */
const snccliprecon_config_t *snccliprecon_config_get(void);

/**
 * @brief Reset the active configuration to built-in defaults.
 */
void snccliprecon_config_reset(void);

/**
 * @brief Parse plugstack arguments and remote environment overrides.
 *
 * Built-in defaults are applied first. Arguments from `plugstack.conf` are
 * parsed next. In a remote SPANK context, matching `SNCCLIPRECON_*`
 * environment variables are parsed after plugstack arguments.
 *
 * @param spank SPANK context.
 * @param argc Number of plugin arguments.
 * @param argv Plugin argument vector.
 */
void snccliprecon_config_parse_args(spank_t spank, int argc, char **argv);

/**
 * @brief Parse one plugin option into the active configuration.
 *
 * @param name Canonical option name without the `snccliprecon-` prefix.
 * @param value Option value.
 * @return SPANK status code.
 */
int snccliprecon_config_parse_option(const char *name, const char *value);

/**
 * @brief Register all plugin-provided `srun` options with Slurm.
 *
 * @param spank SPANK context.
 * @return SPANK status code.
 */
int snccliprecon_args_register(spank_t spank);

/**
 * @brief Render a job-level NCCL Inspector dump directory.
 *
 * Expands `%j` placeholders and leaves step-only placeholders unresolved until
 * the step id is available on the worker.
 *
 * @param config Configuration containing the dump directory template.
 * @param job_id Job id string.
 * @param output Destination buffer.
 * @param output_size Destination buffer size.
 * @return `0` on success, `-1` on failure with `errno` set.
 */
int snccliprecon_config_render_job_dump_dir(
    const snccliprecon_config_t *config, const char *job_id, char *output,
    size_t output_size
);

/**
 * @brief Render a step-level NCCL Inspector dump directory.
 *
 * Expands `%j`, `%s`, and `%J` placeholders using the current job and step ids.
 *
 * @param config Configuration containing the dump directory template.
 * @param job_id Job id string.
 * @param step_id Step id string.
 * @param output Destination buffer.
 * @param output_size Destination buffer size.
 * @return `0` on success, `-1` on failure with `errno` set.
 */
int snccliprecon_config_render_step_dump_dir(
    const snccliprecon_config_t *config, const char *job_id,
    const char *step_id, char *output, size_t output_size
);

#endif
