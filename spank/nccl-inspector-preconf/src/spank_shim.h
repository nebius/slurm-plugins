#ifndef SNCCLIPRECON_H
#define SNCCLIPRECON_H

#include <slurm/spank.h>
#include <stdint.h>

// region GoExports

extern int go_spank_init(spank_t spank, int argc, char **argv);

extern int go_spank_user_init(spank_t spank, int argc, char **argv);

extern int go_spank_task_init_privileged(spank_t spank, int argc, char **argv);

extern int go_spank_exit(spank_t spank, int argc, char **argv);

// endregion GoExports

// region HelpersExports

spank_context_t snccliprecon_spank_context(void);

void snccliprecon_log(const char *msg);

spank_err_t snccliprecon_getenv(spank_t spank, const char *key, char *buffer, int length);

spank_err_t snccliprecon_setenv(spank_t spank, const char *key, const char *value);

// endregion HelpersExports

#endif
