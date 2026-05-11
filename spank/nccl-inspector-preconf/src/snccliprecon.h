#ifndef SNCCLIPRECON_H
#define SNCCLIPRECON_H

#include <slurm/slurm.h>
#include <slurm/spank.h>

#define SNCCLIPRECON_PATH_MAX 4096
#define SNCCLIPRECON_DEFAULT_MODE 0777
#define SNCCLIPRECON_TMP_DIR_BASE "/tmp/nccl_inspector_preconf"

#define SNCCLIPRECON_USER_INIT_OP "user-init"
#define SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP "task-init-privileged"
#define SNCCLIPRECON_TASK_EXIT_OP "task-exit"

#define SNCCLIPRECON_LOG_DIR_SET_BY_PLUGIN "SNCCLIPRECON_LOG_DIR_SET_BY_PLUGIN"
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

#endif
