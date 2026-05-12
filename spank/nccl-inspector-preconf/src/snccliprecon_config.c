#include "snccliprecon_config.h"
#include "snccliprecon.h"
#include "snccliprecon_env.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#if defined(__x86_64__) || defined(__amd64__)
#define SNCCLIPRECON_DEFAULT_PROFILER_PLUGIN                                   \
    "/usr/lib/x86_64-linux-gnu/libnccl-profiler-inspector.so"
#elif defined(__aarch64__) || defined(__arm64__)
#define SNCCLIPRECON_DEFAULT_PROFILER_PLUGIN                                   \
    "/usr/lib/aarch64-linux-gnu/libnccl-profiler-inspector.so"
#else
#define SNCCLIPRECON_DEFAULT_PROFILER_PLUGIN                                   \
    "/usr/lib/libnccl-profiler-inspector.so"
#endif

static const char *DEFAULT_DUMP_DIR = "/opt/soperator-outputs/nccl_profiles";
static const char *DEFAULT_DUMP_THREAD_INTERVAL_MICROSECONDS = "1000000";

enum {
    SNCCLIPRECON_DECIMAL_BASE                      = 10,
    SNCCLIPRECON_PLUGSTACK_OPTION_NAME_BUFFER_SIZE = 128,
};

static snccliprecon_config_t config;

typedef int (*config_value_parser_t)(const char *value);

typedef struct dump_dir_render {
    const char *template;
    const char *job_id;
    const char *step_id;
    bool        render_step;
} dump_dir_render_t;

typedef struct config_option_parser {
    const char           *name;
    config_value_parser_t parse;
} config_option_parser_t;

static int spank_option_enabled(int val, const char *optarg, int remote);

static int
spank_option_profiler_plugin(int val, const char *optarg, int remote);

static int spank_option_dump_dir(int val, const char *optarg, int remote);

static int spank_option_prom_dump(int val, const char *optarg, int remote);

static int spank_option_dump_verbose(int val, const char *optarg, int remote);

static int spank_option_dump_thread_interval_microseconds(
    int val, const char *optarg, int remote
);

static int parse_config_enabled(const char *value);

static int parse_config_profiler_plugin(const char *value);

static int parse_config_dump_dir(const char *value);

static int parse_config_prom_dump(const char *value);

static int parse_config_dump_verbose(const char *value);

static int parse_config_dump_thread_interval_microseconds(const char *value);

static const config_option_parser_t config_option_parsers[] = {
    {.name = "enabled", .parse = parse_config_enabled},
    {.name = "profiler-plugin", .parse = parse_config_profiler_plugin},
    {.name = "dump-dir", .parse = parse_config_dump_dir},
    {.name = "prom-dump", .parse = parse_config_prom_dump},
    {.name = "dump-verbose", .parse = parse_config_dump_verbose},
    {.name  = "dump-thread-interval-microseconds",
     .parse = parse_config_dump_thread_interval_microseconds},
    {.name = NULL, .parse = NULL},
};

static int set_string(char *dst, size_t dst_size, const char *value) {
    if (value == NULL || value[0] == '\0') {
        return -1;
    }

    int written = snprintf(dst, dst_size, "%s", value);
    if (written < 0 || (size_t)written >= dst_size) {
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

        unsigned int digit = (unsigned int)(*p - '0');
        if (parsed > (ULLONG_MAX - digit) / SNCCLIPRECON_DECIMAL_BASE) {
            return -1;
        }
        parsed = parsed * SNCCLIPRECON_DECIMAL_BASE + digit;
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

static void
parse_env_option(spank_t spank, const char *env_name, const char *option_name) {
    char value[SNCCLIPRECON_PATH_MAX];

    if (!snccliprecon_env_get(spank, env_name, value, sizeof(value))) {
        snccliprecon_log_debug2("config: env %s is not set", env_name);
        return;
    }

    snccliprecon_log_debug2(
        "config: parsing env %s for option %s", env_name, option_name
    );
    int rc = snccliprecon_config_parse_option(option_name, value);
    if (rc != ESPANK_SUCCESS) {
        snccliprecon_log_errorf(
            "Invalid plugin env %s value: %s", env_name, value
        );
    }
}

static int
append_string(char *dst, size_t dst_size, size_t *offset, const char *value) {
    size_t value_len = strlen(value);
    if (*offset + value_len >= dst_size) {
        errno = ENAMETOOLONG;
        return -1;
    }

    memcpy(dst + *offset, value, value_len);
    *offset      += value_len;
    dst[*offset]  = '\0';
    return 0;
}

static int render_dump_dir(
    const dump_dir_render_t *render, char *output, size_t output_size
) {
    if (output_size == 0) {
        errno = EINVAL;
        return -1;
    }

    size_t offset = 0;
    output[0]     = '\0';

    for (size_t i = 0; render->template[i] != '\0'; ++i) {
        if (render->template[i] == '%' && render->template[i + 1] == 'j') {
            if (append_string(output, output_size, &offset, render->job_id) !=
                0) {
                return -1;
            }
            ++i;
            continue;
        }

        if (render->render_step && render->template[i] == '%' &&
            render->template[i + 1] == 's') {
            if (append_string(output, output_size, &offset, render->step_id) !=
                0) {
                return -1;
            }
            ++i;
            continue;
        }

        if (render->render_step && render->template[i] == '%' &&
            render->template[i + 1] == 'J') {
            if (append_string(output, output_size, &offset, render->job_id) !=
                    0 ||
                append_string(output, output_size, &offset, ".") != 0 ||
                append_string(output, output_size, &offset, render->step_id) !=
                    0) {
                return -1;
            }
            ++i;
            continue;
        }

        if (offset + 1 >= output_size) {
            errno = ENAMETOOLONG;
            return -1;
        }

        output[offset++] = render->template[i];
        output[offset]   = '\0';
    }

    return 0;
}

const snccliprecon_config_t *snccliprecon_config_get(void) { return &config; }

int snccliprecon_config_render_job_dump_dir(
    const snccliprecon_config_t *config, const char *job_id, char *output,
    size_t output_size
) {
    dump_dir_render_t render = {
        .template    = config->dump_dir,
        .job_id      = job_id,
        .step_id     = "",
        .render_step = false,
    };
    return render_dump_dir(&render, output, output_size);
}

int snccliprecon_config_render_step_dump_dir(
    const snccliprecon_config_t *config, const char *job_id,
    const char *step_id, char *output, size_t output_size
) {
    dump_dir_render_t render = {
        .template    = config->dump_dir,
        .job_id      = job_id,
        .step_id     = step_id,
        .render_step = true,
    };
    return render_dump_dir(&render, output, output_size);
}

void snccliprecon_config_reset(void) {
    config.enabled = true;
    (void)set_string(
        config.profiler_plugin,
        sizeof(config.profiler_plugin),
        SNCCLIPRECON_DEFAULT_PROFILER_PLUGIN
    );
    config.prom_dump = false;
    (void)set_string(
        config.dump_dir, sizeof(config.dump_dir), DEFAULT_DUMP_DIR
    );
    config.dump_verbose = true;
    (void)set_string(
        config.dump_thread_interval_microseconds,
        sizeof(config.dump_thread_interval_microseconds),
        DEFAULT_DUMP_THREAD_INTERVAL_MICROSECONDS
    );
    snccliprecon_log_debug2("config: reset defaults");
}

static int parse_config_enabled(const char *value) {
    bool parsed = false;
    if (parse_bool(value, &parsed) != 0) {
        return ESPANK_BAD_ARG;
    }

    config.enabled = parsed;
    snccliprecon_log_debug2(
        "config: enabled=%s", config.enabled ? "true" : "false"
    );
    return ESPANK_SUCCESS;
}

static int parse_config_profiler_plugin(const char *value) {
    if (set_string(
            config.profiler_plugin, sizeof(config.profiler_plugin), value
        ) != 0) {
        return ESPANK_BAD_ARG;
    }

    snccliprecon_log_debug2(
        "config: profiler-plugin=%s", config.profiler_plugin
    );
    return ESPANK_SUCCESS;
}

static int parse_config_dump_dir(const char *value) {
    if (set_string(config.dump_dir, sizeof(config.dump_dir), value) != 0) {
        return ESPANK_BAD_ARG;
    }

    snccliprecon_log_debug2("config: dump-dir=%s", config.dump_dir);
    return ESPANK_SUCCESS;
}

static int parse_config_prom_dump(const char *value) {
    bool parsed = false;
    if (parse_bool(value, &parsed) != 0) {
        return ESPANK_BAD_ARG;
    }

    config.prom_dump = parsed;
    snccliprecon_log_debug2(
        "config: prom-dump=%s", config.prom_dump ? "true" : "false"
    );
    return ESPANK_SUCCESS;
}

static int parse_config_dump_verbose(const char *value) {
    bool parsed = false;
    if (parse_bool(value, &parsed) != 0) {
        return ESPANK_BAD_ARG;
    }

    config.dump_verbose = parsed;
    snccliprecon_log_debug2(
        "config: dump-verbose=%s", config.dump_verbose ? "true" : "false"
    );
    return ESPANK_SUCCESS;
}

static int parse_config_dump_thread_interval_microseconds(const char *value) {
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

int snccliprecon_config_parse_option(const char *name, const char *value) {
    if (name == NULL || value == NULL) {
        return ESPANK_BAD_ARG;
    }

    for (const config_option_parser_t *parser = config_option_parsers;
         parser->name != NULL;
         ++parser) {
        if (strcmp(name, parser->name) == 0) {
            return parser->parse(value);
        }
    }

    return ESPANK_BAD_ARG;
}

void snccliprecon_config_parse_args(spank_t spank, int argc, char **argv) {
    snccliprecon_log_debug2("config: parsing argc=%d", argc);
    snccliprecon_config_reset();

    for (int i = 0; i < argc; ++i) {
        char *equals = argv[i] == NULL ? NULL : strchr(argv[i], '=');
        if (equals == NULL) {
            snccliprecon_log_errorf(
                "Unknown plugin arg: %s", argv[i] == NULL ? "(null)" : argv[i]
            );
            continue;
        }

        size_t name_len = (size_t)(equals - argv[i]);
        char   name[SNCCLIPRECON_PLUGSTACK_OPTION_NAME_BUFFER_SIZE];
        if (name_len == 0 || name_len >= sizeof(name)) {
            snccliprecon_log_errorf("Unknown plugin arg: %s", argv[i]);
            continue;
        }

        memcpy(name, argv[i], name_len);
        name[name_len] = '\0';

        snccliprecon_log_debug2("config: parsing plugstack option %s", name);
        int rc = snccliprecon_config_parse_option(name, equals + 1);
        if (rc != ESPANK_SUCCESS) {
            snccliprecon_log_errorf("Invalid plugin arg %s", argv[i]);
        }
    }

    if (spank_remote(spank) != 1) {
        snccliprecon_log_debug2(
            "config: skipping env config outside remote context"
        );
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
        "config: final enabled=%s profiler-plugin=%s prom-dump=%s dump-dir=%s "
        "dump-verbose=%s "
        "dump-thread-interval-microseconds=%s",
        config.enabled ? "true" : "false",
        config.profiler_plugin,
        config.prom_dump ? "true" : "false",
        config.dump_dir,
        config.dump_verbose ? "true" : "false",
        config.dump_thread_interval_microseconds
    );
}

static struct spank_option spank_opts[] = {
    {
        .name    = "snccliprecon-enabled",
        .arginfo = "(1 | True) | (0 | False)",
        .usage =
            "[nccl_inspector_preconf] whether to enable nccl_inspector_preconf "
            "plugin. Possible values are case-insensitive. "
            "SNCCLIPRECON_ENABLED env var is also supported.",
        .has_arg = true,
        .val     = 0,
        .cb      = spank_option_enabled,
    },
    {
        .name    = "snccliprecon-profiler-plugin",
        .arginfo = "PATH",
        .usage = "[nccl_inspector_preconf] path to the NCCL Inspector profiler "
                 "plugin SO file. SNCCLIPRECON_PROFILER_PLUGIN env var is also "
                 "supported.",
        .has_arg = true,
        .val     = 0,
        .cb      = spank_option_profiler_plugin,
    },
    {
        .name    = "snccliprecon-dump-dir",
        .arginfo = "PATH",
        .usage =
            "[nccl_inspector_preconf] path to the directory for storing NCCL "
            "Inspector outputs. Supports %j, %s, and %J substitutions for <job "
            "ID>, <step ID> and <job ID>.<step ID> accordingly. "
            "SNCCLIPRECON_DUMP_DIR env var is also supported.",
        .has_arg = true,
        .val     = 0,
        .cb      = spank_option_dump_dir,
    },
    {
        .name    = "snccliprecon-prom-dump",
        .arginfo = "(1 | True) | (0 | False)",
        .usage =
            "[nccl_inspector_preconf] whether to enable NCCL Inspector "
            "Prometheus dump output. Possible values are case-insensitive. "
            "SNCCLIPRECON_PROM_DUMP env var is also supported.",
        .has_arg = true,
        .val     = 0,
        .cb      = spank_option_prom_dump,
    },
    {
        .name    = "snccliprecon-dump-verbose",
        .arginfo = "(1 | True) | (0 | False)",
        .usage   = "[nccl_inspector_preconf] whether to enable verbose NCCL "
                   "Inspector dumps. Possible values are case-insensitive. "
                   "SNCCLIPRECON_DUMP_VERBOSE env var is also supported.",
        .has_arg = true,
        .val     = 0,
        .cb      = spank_option_dump_verbose,
    },
    {
        .name    = "snccliprecon-dump-thread-interval-microseconds",
        .arginfo = "UINT",
        .usage   = "[nccl_inspector_preconf] interval between NCCL Inspector "
                   "dump thread runs in microseconds. "
                   "SNCCLIPRECON_DUMP_THREAD_INTERVAL_MICROSECONDS env var is "
                   "also supported.",
        .has_arg = true,
        .val     = 0,
        .cb      = spank_option_dump_thread_interval_microseconds,
    },
    SPANK_OPTIONS_TABLE_END
};

int snccliprecon_args_register(spank_t spank) {
    for (int i = 0; spank_opts[i].name != NULL; ++i) {
        spank_err_t res = spank_option_register(spank, &spank_opts[i]);
        if (res != ESPANK_SUCCESS) {
            snccliprecon_log_errorf(
                "Cannot register option %s: %s",
                spank_opts[i].name,
                spank_strerror(res)
            );
            return ESPANK_ERROR;
        }
    }

    return ESPANK_SUCCESS;
}

static int spank_option_enabled(int val, const char *optarg, int remote) {
    (void)val;
    (void)remote;

    if (optarg == NULL || *optarg == '\0') {
        snccliprecon_log_errorf(
            "--%s: argument required", "snccliprecon-enabled"
        );
        return ESPANK_BAD_ARG;
    }

    return snccliprecon_config_parse_option("enabled", optarg);
}

static int
spank_option_profiler_plugin(int val, const char *optarg, int remote) {
    (void)val;
    (void)remote;

    if (optarg == NULL || *optarg == '\0') {
        snccliprecon_log_errorf(
            "--%s: argument required", "snccliprecon-profiler-plugin"
        );
        return ESPANK_BAD_ARG;
    }

    return snccliprecon_config_parse_option("profiler-plugin", optarg);
}

static int spank_option_dump_dir(int val, const char *optarg, int remote) {
    (void)val;
    (void)remote;

    if (optarg == NULL || *optarg == '\0') {
        snccliprecon_log_errorf(
            "--%s: argument required", "snccliprecon-dump-dir"
        );
        return ESPANK_BAD_ARG;
    }

    return snccliprecon_config_parse_option("dump-dir", optarg);
}

static int spank_option_prom_dump(int val, const char *optarg, int remote) {
    (void)val;
    (void)remote;

    if (optarg == NULL || *optarg == '\0') {
        snccliprecon_log_errorf(
            "--%s: argument required", "snccliprecon-prom-dump"
        );
        return ESPANK_BAD_ARG;
    }

    return snccliprecon_config_parse_option("prom-dump", optarg);
}

static int spank_option_dump_verbose(int val, const char *optarg, int remote) {
    (void)val;
    (void)remote;

    if (optarg == NULL || *optarg == '\0') {
        snccliprecon_log_errorf(
            "--%s: argument required", "snccliprecon-dump-verbose"
        );
        return ESPANK_BAD_ARG;
    }

    return snccliprecon_config_parse_option("dump-verbose", optarg);
}

static int spank_option_dump_thread_interval_microseconds(
    int val, const char *optarg, int remote
) {
    (void)val;
    (void)remote;

    if (optarg == NULL || *optarg == '\0') {
        snccliprecon_log_errorf(
            "--%s: argument required",
            "snccliprecon-dump-thread-interval-microseconds"
        );
        return ESPANK_BAD_ARG;
    }

    return snccliprecon_config_parse_option(
        "dump-thread-interval-microseconds", optarg
    );
}
