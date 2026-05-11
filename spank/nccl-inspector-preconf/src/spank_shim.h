#ifndef SNCCLIPRECON_H
#define SNCCLIPRECON_H

#include <slurm/slurm.h>
#include <slurm/spank.h>
#include <stdint.h>

#define SNCCLIPRECON_LOG_PREFIX_FORMAT "[nccl_inspector_preconf] @ %s: "

// region GoExports

/**
 * Go implementation of the `slurm_spank_init` hook.
 *
 * @param spank: SPANK context.
 * @param argc: Number of plugin arguments.
 * @param argv: Plugin argument vector.
 *
 * @return SPANK status code.
 */
extern int snccliprecon_spank_init(spank_t spank, int argc, char **argv);

/**
 * Go implementation of the `slurm_spank_user_init` hook.
 *
 * @param spank: SPANK context.
 * @param argc: Number of plugin arguments.
 * @param argv: Plugin argument vector.
 *
 * @return SPANK status code.
 */
extern int snccliprecon_spank_user_init(spank_t spank, int argc, char **argv);

/**
 * Go parser entrypoint used by generated SPANK option callbacks.
 *
 * @param name: Logical plugin argument name.
 * @param value: Argument value supplied by SPANK.
 *
 * @return SPANK status code.
 */
extern int snccliprecon_spank_parse_option(char *name, char *value);

// endregion GoExports

// region CHookExports

/**
 * C implementation of the `slurm_spank_task_init_privileged` hook.
 *
 * @param spank: SPANK context.
 *
 * @return SPANK status code.
 */
int snccliprecon_task_init_privileged(spank_t spank);

/**
 * C implementation of the `slurm_spank_task_exit` hook.
 *
 * @param spank: SPANK context.
 *
 * @return SPANK status code.
 */
int snccliprecon_task_exit(spank_t spank);

// endregion CHookExports

// region HelpersExports

/**
 * Returns the current SPANK hook context.
 *
 * @return Current SPANK hook context.
 */
spank_context_t snccliprecon_spank_context(void);

/**
 * Reads the current Slurm job ID from SPANK.
 *
 * @param spank: SPANK context.
 * @param job_id: Output location for the Slurm job ID.
 *
 * @return SPANK status code.
 */
spank_err_t snccliprecon_get_job_id(spank_t spank, uint32_t *job_id);

/**
 * Reads the current Slurm step ID from SPANK.
 *
 * @param spank: SPANK context.
 * @param step_id: Output location for the Slurm step ID.
 *
 * @return SPANK status code.
 */
spank_err_t snccliprecon_get_step_id(spank_t spank, uint32_t *step_id);

/**
 * Slurm logging function pointer.
 */
typedef void (*snccliprecon_log_fn_t)(const char *format, ...);

/**
 * Emits a prefixed plugin log message through the provided Slurm log function.
 *
 * @param log_fn: Slurm log function.
 * @param format: printf-style format string.
 */
void snccliprecon_log_at(snccliprecon_log_fn_t log_fn, const char *format, ...)
__attribute__((format(printf, 2, 3)));

/**
 * Emits a prefixed user-visible plugin log message.
 *
 * @param msg: Message text to log.
 */
void snccliprecon_log(const char *msg);

/**
 * Emits a prefixed error plugin log message.
 *
 * @param msg: Message text to log.
 */
void snccliprecon_log_error(const char *msg);

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
 * Reads one SPANK environment variable into the provided buffer.
 *
 * @param spank: SPANK context.
 * @param key: Environment variable name.
 * @param buffer: Output buffer for the variable value.
 * @param length: Size of buffer in bytes.
 *
 * @return SPANK status code.
 */
spank_err_t snccliprecon_getenv(spank_t spank, const char *key, char *buffer, int length);

/**
 * Sets one SPANK environment variable.
 *
 * @param spank: SPANK context.
 * @param key: Environment variable name.
 * @param value: Environment variable value.
 *
 * @return SPANK status code.
 */
spank_err_t snccliprecon_setenv(spank_t spank, const char *key, const char *value);

/**
 * Dispatches one generated SPANK option callback into Go parsing logic.
 *
 * @param name: Logical plugin argument name.
 * @param value: Argument value supplied by SPANK.
 *
 * @return SPANK status code.
 */
spank_err_t snccliprecon_parse_option(const char *name, const char *value);

/**
 * Registers the generated SPANK option table.
 *
 * @param spank: SPANK context.
 *
 * @return SPANK status code.
 */
spank_err_t snccliprecon_args_register(spank_t spank);

/**
 * Slurm's sentinel step ID used for batch script contexts.
 */
extern uint32_t snccliprecon_slurm_batch_script_id;

// endregion HelpersExports

#endif
