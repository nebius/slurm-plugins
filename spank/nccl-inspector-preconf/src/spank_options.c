/* Materialized SPANK option table. */

#include "spank_config.h"
#include "spank_shim.h"
#include <stdbool.h>
#include <stddef.h>

static int spank_option_enabled(int val, const char *optarg, int remote);

static int spank_option_profiler_plugin(int val, const char *optarg, int remote);

static int spank_option_dump_dir(int val, const char *optarg, int remote);

static int spank_option_prom_dump(int val, const char *optarg, int remote);

static int spank_option_dump_verbose(int val, const char *optarg, int remote);

static int spank_option_dump_thread_interval_microseconds(int val, const char *optarg, int remote);


/// SPANK plugin option table.
struct spank_option spank_opts[] = {
  {
    .name = "snccliprecon-enabled",
    .arginfo = "(1 | True) | (0 | False)",
    .usage =
    "[nccl_inspector_preconf] whether to enable nccl_inspector_preconf plugin. Possible values are case-insensitive. SNCCLIPRECON_ENABLED env var is also supported.",
    .has_arg = true,
    .val = 0,
    .cb = spank_option_enabled,
  },
  {
    .name = "snccliprecon-profiler-plugin",
    .arginfo = "PATH",
    .usage =
    "[nccl_inspector_preconf] path to the NCCL Inspector profiler plugin SO file. SNCCLIPRECON_PROFILER_PLUGIN env var is also supported.",
    .has_arg = true,
    .val = 0,
    .cb = spank_option_profiler_plugin,
  },
  {
    .name = "snccliprecon-dump-dir",
    .arginfo = "PATH",
    .usage =
    "[nccl_inspector_preconf] path to the directory for storing NCCL Inspector outputs. Supports %j, %s, and %J substitutions for <job ID>, <step ID> and <job ID>.<step ID> accordingly. SNCCLIPRECON_DUMP_DIR env var is also supported.",
    .has_arg = true,
    .val = 0,
    .cb = spank_option_dump_dir,
  },
  {
    .name = "snccliprecon-prom-dump",
    .arginfo = "(1 | True) | (0 | False)",
    .usage =
    "[nccl_inspector_preconf] whether to enable NCCL Inspector Prometheus dump output. Possible values are case-insensitive. SNCCLIPRECON_PROM_DUMP env var is also supported.",
    .has_arg = true,
    .val = 0,
    .cb = spank_option_prom_dump,
  },
  {
    .name = "snccliprecon-dump-verbose",
    .arginfo = "(1 | True) | (0 | False)",
    .usage =
    "[nccl_inspector_preconf] whether to enable verbose NCCL Inspector dumps. Possible values are case-insensitive. SNCCLIPRECON_DUMP_VERBOSE env var is also supported.",
    .has_arg = true,
    .val = 0,
    .cb = spank_option_dump_verbose,
  },
  {
    .name = "snccliprecon-dump-thread-interval-microseconds",
    .arginfo = "UINT",
    .usage =
    "[nccl_inspector_preconf] interval between NCCL Inspector dump thread runs in microseconds. SNCCLIPRECON_DUMP_THREAD_INTERVAL_MICROSECONDS env var is also supported.",
    .has_arg = true,
    .val = 0,
    .cb = spank_option_dump_thread_interval_microseconds,
  },
  SPANK_OPTIONS_TABLE_END
};

/**
 * Register plugin arguments as SPANK options.
 *
 * @param spank SPANK context.
 *
 * @retval ESPANK_SUCCESS Successfully registered options.
 * @retval ESPANK_ERROR Something went wrong.
 */
spank_err_t snccliprecon_args_register(spank_t spank) {
  spank_err_t res = ESPANK_SUCCESS;

  for (int i = 0; spank_opts[i].name != NULL; ++i) {
    res = spank_option_register(spank, &spank_opts[i]);
    if (res != ESPANK_SUCCESS) {
      snccliprecon_log_errorf(
        "Cannot register option %s: %s",
        spank_opts[i].name,
        spank_strerror(res)
      );
      return ESPANK_ERROR;
    }
  }

  return res;
}

spank_err_t snccliprecon_parse_option(const char *name, const char *value) {
  return snccliprecon_config_parse_option(name, value);
}


/**
 * Implementation of enabled option registration callback.
 *
 * @related spank_opt_cb_f
 */
static int spank_option_enabled(int val, const char *optarg, int remote) {
  (void) val;
  (void) remote;

  if (optarg == NULL || *optarg == '\0') {
    snccliprecon_log_errorf(
      "--%s: argument required",
      "snccliprecon-enabled"
    );
    return ESPANK_BAD_ARG;
  }

  return snccliprecon_parse_option(
    "enabled",
    optarg
  );
}

/**
 * Implementation of profiler-plugin option registration callback.
 *
 * @related spank_opt_cb_f
 */
static int spank_option_profiler_plugin(int val, const char *optarg, int remote) {
  (void) val;
  (void) remote;

  if (optarg == NULL || *optarg == '\0') {
    snccliprecon_log_errorf(
      "--%s: argument required",
      "snccliprecon-profiler-plugin"
    );
    return ESPANK_BAD_ARG;
  }

  return snccliprecon_parse_option(
    "profiler-plugin",
    optarg
  );
}

/**
 * Implementation of dump-dir option registration callback.
 *
 * @related spank_opt_cb_f
 */
static int spank_option_dump_dir(int val, const char *optarg, int remote) {
  (void) val;
  (void) remote;

  if (optarg == NULL || *optarg == '\0') {
    snccliprecon_log_errorf(
      "--%s: argument required",
      "snccliprecon-dump-dir"
    );
    return ESPANK_BAD_ARG;
  }

  return snccliprecon_parse_option(
    "dump-dir",
    optarg
  );
}

/**
 * Implementation of prom-dump option registration callback.
 *
 * @related spank_opt_cb_f
 */
static int spank_option_prom_dump(int val, const char *optarg, int remote) {
  (void) val;
  (void) remote;

  if (optarg == NULL || *optarg == '\0') {
    snccliprecon_log_errorf(
      "--%s: argument required",
      "snccliprecon-prom-dump"
    );
    return ESPANK_BAD_ARG;
  }

  return snccliprecon_parse_option(
    "prom-dump",
    optarg
  );
}

/**
 * Implementation of dump-verbose option registration callback.
 *
 * @related spank_opt_cb_f
 */
static int spank_option_dump_verbose(int val, const char *optarg, int remote) {
  (void) val;
  (void) remote;

  if (optarg == NULL || *optarg == '\0') {
    snccliprecon_log_errorf(
      "--%s: argument required",
      "snccliprecon-dump-verbose"
    );
    return ESPANK_BAD_ARG;
  }

  return snccliprecon_parse_option(
    "dump-verbose",
    optarg
  );
}

/**
 * Implementation of dump-thread-interval-microseconds option registration callback.
 *
 * @related spank_opt_cb_f
 */
static int spank_option_dump_thread_interval_microseconds(int val, const char *optarg, int remote) {
  (void) val;
  (void) remote;

  if (optarg == NULL || *optarg == '\0') {
    snccliprecon_log_errorf(
      "--%s: argument required",
      "snccliprecon-dump-thread-interval-microseconds"
    );
    return ESPANK_BAD_ARG;
  }

  return snccliprecon_parse_option(
    "dump-thread-interval-microseconds",
    optarg
  );
}
