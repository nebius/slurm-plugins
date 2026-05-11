#include "spank_config.h"
#include "spank_shim.h"

int snccliprecon_spank_init(spank_t spank, int argc, char **argv) {
  switch (spank_context()) {
    case S_CTX_LOCAL:
    case S_CTX_REMOTE:
      break;
    default:
      return ESPANK_SUCCESS;
  }

  if (snccliprecon_args_register(spank) != ESPANK_SUCCESS) {
    return ESPANK_ERROR;
  }
  snccliprecon_config_parse_args(spank, argc, argv);

  return ESPANK_SUCCESS;
}
