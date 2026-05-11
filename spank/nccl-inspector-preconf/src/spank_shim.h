#ifndef SNCCLIPRECON_H
#define SNCCLIPRECON_H

#include <slurm/slurm.h>
#include <slurm/spank.h>

#define SNCCLIPRECON_LOG_PREFIX_FORMAT "[nccl_inspector_preconf] @ %s: "

/**
 * Handles `slurm_spank_init`.
 *
 * @param spank: SPANK context.
 * @param argc: Number of plugin arguments.
 * @param argv: Plugin argument vector.
 *
 * @return SPANK status code.
 */
int snccliprecon_spank_init(spank_t spank, int argc, char **argv);

/**
 * Handles `slurm_spank_user_init`.
 *
 * @param spank: SPANK context.
 *
 * @return SPANK status code.
 */
int snccliprecon_user_init(spank_t spank);

/**
 * Handles `slurm_spank_task_init_privileged`.
 *
 * @param spank: SPANK context.
 *
 * @return SPANK status code.
 */
int snccliprecon_task_init_privileged(spank_t spank);

/**
 * Handles `slurm_spank_task_exit`.
 *
 * @param spank: SPANK context.
 *
 * @return SPANK status code.
 */
int snccliprecon_task_exit(spank_t spank);

/**
 * Emits a prefixed formatted error plugin log message.
 *
 * @param format: printf-style format string.
 */
void snccliprecon_log_errorf(const char *format, ...)
__attribute__((format(printf, 1, 2)));

/**
 * Emits a prefixed debug plugin log message.
 *
 * @param format: printf-style format string.
 */
void snccliprecon_log_debug(const char *format, ...)
__attribute__((format(printf, 1, 2)));

/**
 * Emits a prefixed debug2 plugin log message.
 *
 * @param format: printf-style format string.
 */
void snccliprecon_log_debug2(const char *format, ...)
__attribute__((format(printf, 1, 2)));

/**
 * Registers the SPANK option table.
 *
 * @param spank: SPANK context.
 *
 * @return SPANK status code.
 */
spank_err_t snccliprecon_args_register(spank_t spank);

#endif
