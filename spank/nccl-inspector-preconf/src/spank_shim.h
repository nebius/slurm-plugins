#ifndef SNCCLIPRECON_H
#define SNCCLIPRECON_H

#include <slurm/slurm.h>
#include <slurm/spank.h>
#include <stdint.h>

// region GoExports

extern int snccliprecon_spank_init(spank_t spank, int argc, char **argv);

extern int snccliprecon_spank_user_init(spank_t spank, int argc, char **argv);

extern int snccliprecon_spank_task_init_privileged(spank_t spank, int argc, char **argv);

extern int snccliprecon_spank_task_exit(spank_t spank, int argc, char **argv);

extern int snccliprecon_spank_parse_option(char *name, char *value);

// endregion GoExports

// region HelpersExports

spank_context_t snccliprecon_spank_context(void);

spank_err_t snccliprecon_get_job_id(spank_t spank, uint32_t *job_id);

spank_err_t snccliprecon_get_step_id(spank_t spank, uint32_t *step_id);

void snccliprecon_log(const char *msg);

void snccliprecon_log_error(const char *msg);

void snccliprecon_log_error_fmt(const char *fmt, ...);

spank_err_t snccliprecon_getenv(spank_t spank, const char *key, char *buffer, int length);

spank_err_t snccliprecon_setenv(spank_t spank, const char *key, const char *value);

spank_err_t snccliprecon_parse_option(const char *name, const char *value);

spank_err_t snccliprecon_args_register(spank_t spank);

extern uint32_t snccliprecon_slurm_batch_script_id;

// endregion HelpersExports

#endif
