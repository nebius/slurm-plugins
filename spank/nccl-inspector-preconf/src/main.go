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
	"github.com/nebius/nccl-inspector-preconf/internal/enroot"
	"github.com/nebius/nccl-inspector-preconf/internal/env"
	"github.com/nebius/nccl-inspector-preconf/internal/log"
	"github.com/nebius/nccl-inspector-preconf/internal/unix"
)

var (
	config = cfg.NewConfig()
)

//export go_spank_parse_option
//goland:noinspection GoSnakeCaseUsage
func go_spank_parse_option(name *C.char, value *C.char) C.int {
	if name == nil || value == nil {
		return C.ESPANK_BAD_ARG
	}

	if err := arg.ParseByName(config, C.GoString(name), C.GoString(value)); err != nil {
		log.Message(err.Error())
		return C.ESPANK_BAD_ARG
	}

	return C.ESPANK_SUCCESS
}

//export go_spank_init
//goland:noinspection GoSnakeCaseUsage
func go_spank_init(spank C.spank_t, argc C.int, argv **C.char) C.int {
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

//export go_spank_user_init
//goland:noinspection GoSnakeCaseUsage
func go_spank_user_init(spank C.spank_t, argc C.int, argv **C.char) C.int {
	_, _ = argc, argv

	if C.snccliprecon_spank_context() != C.S_CTX_REMOTE {
		return C.ESPANK_SUCCESS
	}

	if !config.Enabled {
		return C.ESPANK_SUCCESS
	}

	ctx := bridge.NewSpankContext(unsafe.Pointer(spank))
	jobId := ctx.GetJobId()

	env.SetIfMissing(ctx, "NCCL_PROFILER_PLUGIN", config.InspectorSO)
	if setByPlugin := env.SetIfMissing(ctx, "NCCL_INSPECTOR_DUMP_DIR", arg.SubstituteJobId(config.LogDir, jobId)); setByPlugin {
		config.LogDirSetByPlugin = true
	}
	env.SetIfMissing(ctx, "NCCL_INSPECTOR_PROM_DUMP", "0")
	env.SetIfMissing(ctx, "NCCL_INSPECTOR_DUMP_THREAD_INTERVAL_MICROSECONDS", "1000000")
	env.SetIfMissing(ctx, "NCCL_INSPECTOR_DUMP_VERBOSE", "1")

	return C.ESPANK_SUCCESS
}

//export go_spank_task_init_privileged
//goland:noinspection GoSnakeCaseUsage
func go_spank_task_init_privileged(spank C.spank_t, argc C.int, argv **C.char) C.int {
	_, _ = argc, argv

	if C.snccliprecon_spank_context() != C.S_CTX_REMOTE {
		return C.ESPANK_SUCCESS
	}

	if !config.Enabled {
		return C.ESPANK_SUCCESS
	}

	ctx := bridge.NewSpankContext(unsafe.Pointer(spank))
	jobId := ctx.GetJobId()
	stepId := ctx.GetStepId()

	// region Ensure dump dir

	ensureDumpDir := func() error {
		if !config.LogDirSetByPlugin {
			return nil
		}

		dumpDir := arg.SubstituteJobStepId(config.LogDir, jobId, stepId)
		env.Set(ctx, "NCCL_INSPECTOR_DUMP_DIR", dumpDir)

		return unix.EnsureDir(dumpDir)
	}
	if err := ensureDumpDir(); err != nil {
		log.Error(err.Error())
		return C.ESPANK_ERROR
	}

	// endregion Ensure dump dir

	// region Ensure Enroot mount

	ensureEnrootMount := func() error {
		if !unix.DirExists(enroot.MountsDir) {
			return nil
		}

		return enroot.CreateMountFile(
			jobId,
			stepId,
			[]enroot.Mount{{
				Path:        config.InspectorSO,
				IsDir:       false,
				IsReadWrite: false,
			}, {
				Path:        config.LogDir,
				IsDir:       true,
				IsReadWrite: true,
			}},
		)
	}
	if err := ensureEnrootMount(); err != nil {
		log.Error(err.Error())
		return C.ESPANK_ERROR
	}

	// endregion Ensure Enroot mount

	return C.ESPANK_SUCCESS
}

//export go_spank_task_exit
//goland:noinspection GoSnakeCaseUsage
func go_spank_task_exit(spank C.spank_t, argc C.int, argv **C.char) C.int {
	_, _ = argc, argv

	if C.snccliprecon_spank_context() != C.S_CTX_REMOTE {
		return C.ESPANK_SUCCESS
	}

	if !config.Enabled {
		return C.ESPANK_SUCCESS
	}

	ctx := bridge.NewSpankContext(unsafe.Pointer(spank))

	if err := enroot.RemoveMountFile(ctx.GetJobId(), ctx.GetStepId()); err != nil {
		log.Error(err.Error())
		return C.ESPANK_ERROR
	}

	return C.ESPANK_SUCCESS
}

//export go_spank_exit
//goland:noinspection GoSnakeCaseUsage
func go_spank_exit(spank C.spank_t, argc C.int, argv **C.char) C.int {
	_, _, _ = spank, argc, argv

	if C.snccliprecon_spank_context() != C.S_CTX_REMOTE {
		return C.ESPANK_SUCCESS
	}

	if !config.Enabled {
		return C.ESPANK_SUCCESS
	}

	return C.ESPANK_SUCCESS
}

func main() {}
