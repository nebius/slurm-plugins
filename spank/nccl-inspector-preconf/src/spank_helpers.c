#include "spank_shim.h"
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#define SNCCLIPRECON_LOG_BUFFER_SIZE 2048
#define SNCCLIPRECON_HOSTNAME_BUFFER_SIZE 256

typedef void (*snccliprecon_log_fn_t)(const char *format, ...);

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
