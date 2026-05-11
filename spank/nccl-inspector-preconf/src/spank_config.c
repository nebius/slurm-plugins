#include "spank_config.h"
#include "spank_env.h"
#include "spank_shim.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#if defined(__x86_64__) || defined(__amd64__)
#define SNCCLIPRECON_DEFAULT_PROFILER_PLUGIN "/usr/lib/x86_64-linux-gnu/libnccl-profiler-inspector.so"
#elif defined(__aarch64__) || defined(__arm64__)
#define SNCCLIPRECON_DEFAULT_PROFILER_PLUGIN "/usr/lib/aarch64-linux-gnu/libnccl-profiler-inspector.so"
#else
#define SNCCLIPRECON_DEFAULT_PROFILER_PLUGIN "/usr/lib/libnccl-profiler-inspector.so"
#endif

static const char *DEFAULT_DUMP_DIR = "/opt/soperator-outputs/nccl_profiles";
static const char *DEFAULT_DUMP_THREAD_INTERVAL_MICROSECONDS = "1000000";

static snccliprecon_config_t config;

static int set_string(char *dst, size_t dst_size, const char *value) {
  if (value == NULL || value[0] == '\0') {
    return -1;
  }

  int written = snprintf(dst, dst_size, "%s", value);
  if (written < 0 || (size_t) written >= dst_size) {
    return -1;
  }

  return 0;
}

static int set_uint_string(char *dst, size_t dst_size, const char *value) {
  if (value == NULL || value[0] == '\0') {
    return -1;
  }

  unsigned long long parsed = 0;
  for (const char *p = value; *p != '\0'; ++p) {
    if (*p < '0' || *p > '9') {
      return -1;
    }

    unsigned int digit = (unsigned int) (*p - '0');
    if (parsed > (ULLONG_MAX - digit) / 10) {
      return -1;
    }
    parsed = parsed * 10 + digit;
  }

  return set_string(dst, dst_size, value);
}

static int parse_bool(const char *value, bool *result) {
  if (value == NULL) {
    return -1;
  }

  if (strcmp(value, "1") == 0 || strcasecmp(value, "true") == 0) {
    *result = true;
    return 0;
  }

  if (strcmp(value, "0") == 0 || strcasecmp(value, "false") == 0) {
    *result = false;
    return 0;
  }

  return -1;
}

static void parse_env_option(spank_t spank, const char *env_name, const char *option_name) {
  char value[SNCCLIPRECON_PATH_MAX];

  if (!snccliprecon_env_get(spank, env_name, value, sizeof(value))) {
    snccliprecon_log_debug2("config: env %s is not set", env_name);
    return;
  }

  snccliprecon_log_debug2("config: parsing env %s for option %s", env_name, option_name);
  spank_err_t rc = snccliprecon_config_parse_option(option_name, value);
  if (rc != ESPANK_SUCCESS) {
    snccliprecon_log_errorf("Invalid plugin env %s value: %s", env_name, value);
  }
}

const snccliprecon_config_t *snccliprecon_config_get(void) {
  return &config;
}

void snccliprecon_config_reset(void) {
  config.enabled = true;
  (void) set_string(config.profiler_plugin, sizeof(config.profiler_plugin), SNCCLIPRECON_DEFAULT_PROFILER_PLUGIN);
  config.prom_dump = false;
  (void) set_string(config.dump_dir, sizeof(config.dump_dir), DEFAULT_DUMP_DIR);
  config.dump_verbose = true;
  (void) set_string(
    config.dump_thread_interval_microseconds,
    sizeof(config.dump_thread_interval_microseconds),
    DEFAULT_DUMP_THREAD_INTERVAL_MICROSECONDS
  );
  snccliprecon_log_debug2("config: reset defaults");
}

spank_err_t snccliprecon_config_parse_option(const char *name, const char *value) {
  if (name == NULL || value == NULL) {
    return ESPANK_BAD_ARG;
  }

  if (strcmp(name, "enabled") == 0) {
    bool parsed = false;
    if (parse_bool(value, &parsed) != 0) {
      return ESPANK_BAD_ARG;
    }
    config.enabled = parsed;
    snccliprecon_log_debug2("config: enabled=%s", config.enabled ? "true" : "false");
    return ESPANK_SUCCESS;
  }

  if (strcmp(name, "profiler-plugin") == 0) {
    if (set_string(config.profiler_plugin, sizeof(config.profiler_plugin), value) != 0) {
      return ESPANK_BAD_ARG;
    }
    snccliprecon_log_debug2("config: profiler-plugin=%s", config.profiler_plugin);
    return ESPANK_SUCCESS;
  }

  if (strcmp(name, "dump-dir") == 0) {
    if (set_string(config.dump_dir, sizeof(config.dump_dir), value) != 0) {
      return ESPANK_BAD_ARG;
    }
    snccliprecon_log_debug2("config: dump-dir=%s", config.dump_dir);
    return ESPANK_SUCCESS;
  }

  if (strcmp(name, "prom-dump") == 0) {
    bool parsed = false;
    if (parse_bool(value, &parsed) != 0) {
      return ESPANK_BAD_ARG;
    }
    config.prom_dump = parsed;
    snccliprecon_log_debug2("config: prom-dump=%s", config.prom_dump ? "true" : "false");
    return ESPANK_SUCCESS;
  }

  if (strcmp(name, "dump-verbose") == 0) {
    bool parsed = false;
    if (parse_bool(value, &parsed) != 0) {
      return ESPANK_BAD_ARG;
    }
    config.dump_verbose = parsed;
    snccliprecon_log_debug2("config: dump-verbose=%s", config.dump_verbose ? "true" : "false");
    return ESPANK_SUCCESS;
  }

  if (strcmp(name, "dump-thread-interval-microseconds") == 0) {
    if (set_uint_string(
          config.dump_thread_interval_microseconds,
          sizeof(config.dump_thread_interval_microseconds),
          value
        ) != 0) {
      return ESPANK_BAD_ARG;
    }
    snccliprecon_log_debug2(
      "config: dump-thread-interval-microseconds=%s",
      config.dump_thread_interval_microseconds
    );
    return ESPANK_SUCCESS;
  }

  return ESPANK_BAD_ARG;
}

void snccliprecon_config_parse_args(spank_t spank, int argc, char **argv) {
  snccliprecon_log_debug2("config: parsing argc=%d", argc);
  snccliprecon_config_reset();

  for (int i = 0; i < argc; ++i) {
    char *equals = argv[i] == NULL ? NULL : strchr(argv[i], '=');
    if (equals == NULL) {
      snccliprecon_log_errorf("Unknown plugin arg: %s", argv[i] == NULL ? "(null)" : argv[i]);
      continue;
    }

    size_t name_len = (size_t) (equals - argv[i]);
    char name[128];
    if (name_len == 0 || name_len >= sizeof(name)) {
      snccliprecon_log_errorf("Unknown plugin arg: %s", argv[i]);
      continue;
    }

    memcpy(name, argv[i], name_len);
    name[name_len] = '\0';

    snccliprecon_log_debug2("config: parsing plugstack option %s", name);
    spank_err_t rc = snccliprecon_config_parse_option(name, equals + 1);
    if (rc != ESPANK_SUCCESS) {
      snccliprecon_log_errorf("Invalid plugin arg %s", argv[i]);
    }
  }

  if (spank_remote(spank) != 1) {
    snccliprecon_log_debug2("config: skipping env config outside remote context");
    return;
  }

  parse_env_option(spank, "SNCCLIPRECON_ENABLED", "enabled");
  parse_env_option(spank, "SNCCLIPRECON_PROFILER_PLUGIN", "profiler-plugin");
  parse_env_option(spank, "SNCCLIPRECON_DUMP_DIR", "dump-dir");
  parse_env_option(spank, "SNCCLIPRECON_PROM_DUMP", "prom-dump");
  parse_env_option(spank, "SNCCLIPRECON_DUMP_VERBOSE", "dump-verbose");
  parse_env_option(
    spank,
    "SNCCLIPRECON_DUMP_THREAD_INTERVAL_MICROSECONDS",
    "dump-thread-interval-microseconds"
  );
  snccliprecon_log_debug2(
    "config: final enabled=%s profiler-plugin=%s prom-dump=%s dump-dir=%s dump-verbose=%s "
    "dump-thread-interval-microseconds=%s",
    config.enabled ? "true" : "false",
    config.profiler_plugin,
    config.prom_dump ? "true" : "false",
    config.dump_dir,
    config.dump_verbose ? "true" : "false",
    config.dump_thread_interval_microseconds
  );
}
