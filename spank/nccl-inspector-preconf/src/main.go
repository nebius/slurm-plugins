package main

/*
#cgo CFLAGS: -Wall -Wextra
#include "spank_shim.h"
#include <stdlib.h>
*/
import "C"
import (
	"unsafe"

	"github.com/nebius/nccl-inspector-preconf/internal/arg"
	argparse "github.com/nebius/nccl-inspector-preconf/internal/arg/parse"
	"github.com/nebius/nccl-inspector-preconf/internal/bridge"
	"github.com/nebius/nccl-inspector-preconf/internal/cfg"
	"github.com/nebius/nccl-inspector-preconf/internal/log"
)

var (
	// config stores plugin settings parsed from plugstack args and env vars.
	config = cfg.NewConfig()
)

// snccliprecon_spank_parse_option parses one generated SPANK option callback value.
//
//export snccliprecon_spank_parse_option
//goland:noinspection GoSnakeCaseUsage
func snccliprecon_spank_parse_option(name *C.char, value *C.char) C.int {
	if name == nil || value == nil {
		return C.ESPANK_BAD_ARG
	}

	if err := arg.ParseByName(config, C.GoString(name), C.GoString(value)); err != nil {
		log.Message(err.Error())
		return C.ESPANK_BAD_ARG
	}

	return C.ESPANK_SUCCESS
}

// snccliprecon_spank_init parses plugin arguments during SPANK init.
//
//export snccliprecon_spank_init
//goland:noinspection GoSnakeCaseUsage
func snccliprecon_spank_init(spank C.spank_t, argc C.int, argv **C.char) C.int {
	switch C.snccliprecon_spank_context() {
	case C.S_CTX_LOCAL, C.S_CTX_REMOTE:
		{
			argparse.ParseArgs(
				config,
				bridge.NewSpankContext(unsafe.Pointer(spank)),
				bridge.CStringArrayToStrings(int(argc), unsafe.Pointer(argv)),
			)
		}
	}

	return C.ESPANK_SUCCESS
}

// main is required for the Go main package but is unused in the built plugin.
func main() {}
