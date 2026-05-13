#ifndef SNCCLIPRECON_H
#define SNCCLIPRECON_H

#include <slurm/slurm.h>
#include <slurm/spank.h>
#include <stddef.h>

enum {
    SNCCLIPRECON_PATH_MAX                         = 4096,
    SNCCLIPRECON_DEFAULT_MODE                     = 0777,
    SNCCLIPRECON_LOG_BUFFER_SIZE                  = 2048,
    SNCCLIPRECON_HOSTNAME_BUFFER_SIZE             = 256,
    SNCCLIPRECON_ERROR_BUFFER_SIZE                = 256,
    SNCCLIPRECON_JOB_ID_BUFFER_SIZE               = 16,
    SNCCLIPRECON_STEP_ID_BUFFER_SIZE              = 16,
    SNCCLIPRECON_ENV_FLAG_BUFFER_SIZE             = 8,
    SNCCLIPRECON_DUMP_THREAD_INTERVAL_BUFFER_SIZE = 64,
};

#define SNCCLIPRECON_TMP_DIR_BASE "/tmp/nccl_inspector_preconf"

#define SNCCLIPRECON_USER_INIT_OP            "user-init"
#define SNCCLIPRECON_TASK_INIT_PRIVILEGED_OP "task-init-privileged"
#define SNCCLIPRECON_TASK_EXIT_OP            "task-exit"

#define SNCCLIPRECON_LOG_DIR_SET_BY_PLUGIN "SNCCLIPRECON_LOG_DIR_SET_BY_PLUGIN"
#define SNCCLIPRECON_LOG_PREFIX_FORMAT     "[nccl_inspector_preconf] @ %s: "

/**
 * @brief Initialize the plugin in supported SPANK contexts.
 *
 * Registers plugin options and parses plugstack arguments. Runtime setup is
 * only meaningful in remote contexts, but options must also be registered in
 * the local context so `srun` can accept them.
 *
 * @param spank SPANK context.
 * @param argc Number of plugin arguments.
 * @param argv Plugin argument vector.
 * @return SPANK status code.
 */
int snccliprecon_spank_init(spank_t spank, int argc, char **argv);

/**
 * @brief Export NCCL Inspector defaults before user code starts.
 *
 * Sets NCCL Inspector environment variables when the plugin is enabled and the
 * job has not provided explicit values.
 *
 * @param spank SPANK context.
 * @return SPANK status code.
 */
int snccliprecon_user_init(spank_t spank);

/**
 * @brief Prepare worker-local state before launching a task.
 *
 * Renders the step-level dump directory, creates it if needed, creates the
 * Enroot mount file, and guards the work so it runs once per worker and step.
 *
 * @param spank SPANK context.
 * @return SPANK status code.
 */
int snccliprecon_task_init_privileged(spank_t spank);

/**
 * @brief Remove worker-local state after a task exits.
 *
 * Removes the Enroot mount file and worker lock files once per worker and step.
 *
 * @param spank SPANK context.
 * @return SPANK status code.
 */
int snccliprecon_task_exit(spank_t spank);

/**
 * @brief Emit a prefixed error log message.
 *
 * @param format printf-style format string.
 */
void snccliprecon_log_errorf(const char *format, ...)
    __attribute__((format(printf, 1, 2)));

/**
 * @brief Emit a prefixed error log message with an errno description.
 *
 * @param errnum errno value to render.
 * @param format printf-style format string.
 */
void snccliprecon_log_error_errno(int errnum, const char *format, ...)
    __attribute__((format(printf, 2, 3)));

/**
 * @brief Emit a prefixed debug log message.
 *
 * @param format printf-style format string.
 */
void snccliprecon_log_debug(const char *format, ...)
    __attribute__((format(printf, 1, 2)));

/**
 * @brief Emit a prefixed debug2 log message.
 *
 * @param format printf-style format string.
 */
void snccliprecon_log_debug2(const char *format, ...)
    __attribute__((format(printf, 1, 2)));

/**
 * @brief Emit a prefixed debug2 log message with an errno description.
 *
 * @param errnum errno value to render.
 * @param format printf-style format string.
 */
void snccliprecon_log_debug2_errno(int errnum, const char *format, ...)
    __attribute__((format(printf, 2, 3)));

#endif
