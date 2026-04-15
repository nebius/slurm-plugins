#include "spank_shim.h"
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * Returns the current SPANK hook context.
 *
 * @return Current SPANK hook context.
 */
spank_context_t snccliprecon_spank_context() {
  return spank_context();
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
 * Emits an informational plugin log message.
 *
 * @param msg: Message text to log.
 */
void snccliprecon_log(const char *msg) {
  slurm_spank_log("%s", msg);
}

/**
 * Emits an error plugin log message.
 *
 * @param msg: Message text to log.
 */
void snccliprecon_log_error(const char *msg) {
  slurm_error("%s", msg);
}

/**
 * Emits a formatted error plugin log message.
 *
 * @param fmt: printf-style format string.
 *
 * @return Nothing.
 */
void snccliprecon_log_error_fmt(const char *fmt, ...) {
  char buffer[1024];
  va_list args;

  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  slurm_error("%s", buffer);
}

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
spank_err_t snccliprecon_getenv(spank_t spank, const char *key, char *buffer, int const length) {
  return spank_getenv(spank, key, buffer, length);
}

/**
 * Sets one SPANK environment variable.
 *
 * @param spank: SPANK context.
 * @param key: Environment variable name.
 * @param value: Environment variable value.
 *
 * @return SPANK status code.
 */
spank_err_t snccliprecon_setenv(spank_t spank, const char *key, const char *value) {
  return spank_setenv(spank, key, value, 1);
}

/**
 * Dispatches one generated SPANK option callback into Go parsing logic.
 *
 * @param name: Logical plugin argument name.
 * @param value: Argument value supplied by SPANK.
 *
 * @return SPANK status code.
 */
spank_err_t snccliprecon_parse_option(const char *name, const char *value) {
  return snccliprecon_spank_parse_option((char *)name, (char *)value);
}
