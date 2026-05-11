#include "spank_shim.h"
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#define SNCCLIPRECON_LOG_BUFFER_SIZE 2048
#define SNCCLIPRECON_HOSTNAME_BUFFER_SIZE 256

static const char *snccliprecon_hostname(char *hostname, size_t hostname_size) {
  if (hostname_size == 0) {
    return "unknown";
  }

  if (gethostname(hostname, hostname_size) != 0 || hostname[0] == '\0') {
    (void) snprintf(hostname, hostname_size, "%s", "unknown");
  }
  hostname[hostname_size - 1] = '\0';

  return hostname;
}

static void snccliprecon_vlog_at(snccliprecon_log_fn_t log_fn, const char *format, va_list args) {
  char buffer[SNCCLIPRECON_LOG_BUFFER_SIZE];
  char hostname[SNCCLIPRECON_HOSTNAME_BUFFER_SIZE];

  vsnprintf(buffer, sizeof(buffer), format, args);
  log_fn(SNCCLIPRECON_LOG_PREFIX_FORMAT "%s", snccliprecon_hostname(hostname, sizeof(hostname)), buffer);
}

/**
 * Reads the current Slurm job ID from SPANK.
 *
 * @param spank: SPANK context.
 * @param job_id: Output location for the Slurm job ID.
 *
 * @return SPANK status code.
 */
spank_err_t snccliprecon_get_job_id(spank_t spank, uint32_t *job_id) {
  return spank_get_item(spank, S_JOB_ID, job_id);
}

/**
 * Reads the current Slurm step ID from SPANK.
 *
 * @param spank: SPANK context.
 * @param step_id: Output location for the Slurm step ID.
 *
 * @return SPANK status code.
 */
spank_err_t snccliprecon_get_step_id(spank_t spank, uint32_t *step_id) {
  return spank_get_item(spank, S_JOB_STEPID, step_id);
}

/**
 * Emits a prefixed plugin log message through the provided Slurm log function.
 *
 * @param log_fn Slurm log function.
 * @param format printf-style format string.
 */
void snccliprecon_log_at(snccliprecon_log_fn_t log_fn, const char *format, ...) {
  va_list args;

  va_start(args, format);
  snccliprecon_vlog_at(log_fn, format, args);
  va_end(args);
}

/**
 * Emits a prefixed user-visible plugin log message.
 *
 * @param msg: Message text to log.
 */
void snccliprecon_log(const char *msg) {
  snccliprecon_log_at(slurm_spank_log, "%s", msg);
}

/**
 * Emits a prefixed error plugin log message.
 *
 * @param msg: Message text to log.
 */
void snccliprecon_log_error(const char *msg) {
  snccliprecon_log_at(slurm_error, "%s", msg);
}

/**
 * Emits a prefixed formatted error plugin log message.
 *
 * @param format: printf-style format string.
 *
 * @return Nothing.
 */
void snccliprecon_log_errorf(const char *format, ...) {
  va_list args;

  va_start(args, format);
  snccliprecon_vlog_at(slurm_error, format, args);
  va_end(args);
}

/**
 * Emits a prefixed debug plugin log message.
 *
 * @param format: printf-style format string.
 */
void snccliprecon_log_debug(const char *format, ...) {
  va_list args;

  va_start(args, format);
  snccliprecon_vlog_at(slurm_debug, format, args);
  va_end(args);
}

/**
 * Emits a prefixed debug2 plugin log message.
 *
 * @param format: printf-style format string.
 */
void snccliprecon_log_debug2(const char *format, ...) {
  va_list args;

  va_start(args, format);
  snccliprecon_vlog_at(slurm_debug2, format, args);
  va_end(args);
}
