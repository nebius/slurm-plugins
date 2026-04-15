package main

/*
#cgo CFLAGS: -Wall -Wextra
#include "spank_shim.h"
#include <stdlib.h>
*/
import "C"
import (
	"errors"
	"unsafe"

	"github.com/nebius/nccl-inspector-preconf/internal/arg"
	argparse "github.com/nebius/nccl-inspector-preconf/internal/arg/parse"
	"github.com/nebius/nccl-inspector-preconf/internal/bridge"
	"github.com/nebius/nccl-inspector-preconf/internal/cfg"
	"github.com/nebius/nccl-inspector-preconf/internal/enroot"
	"github.com/nebius/nccl-inspector-preconf/internal/env"
	"github.com/nebius/nccl-inspector-preconf/internal/log"
	"github.com/nebius/nccl-inspector-preconf/internal/plugin"
	"github.com/nebius/nccl-inspector-preconf/internal/unix"
)

var (
	config = cfg.NewConfig()
)

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

//export snccliprecon_spank_user_init
//goland:noinspection GoSnakeCaseUsage
func snccliprecon_spank_user_init(spank C.spank_t, argc C.int, argv **C.char) C.int {
	_, _ = argc, argv

	if C.snccliprecon_spank_context() != C.S_CTX_REMOTE {
		return C.ESPANK_SUCCESS
	}

	if !config.Enabled {
		return C.ESPANK_SUCCESS
	}

	failFast, spankRCIfFailFast, jobId, _, _ := ensureOncePerWorker(spank, plugin.LockNameOpUserInit)
	if failFast {
		return spankRCIfFailFast
	}
	ctx := bridge.NewSpankContext(unsafe.Pointer(spank))

	env.SetIfMissing(ctx, "NCCL_PROFILER_PLUGIN", config.InspectorSO)
	{
		_, setByPlugin := env.SetIfMissing(ctx, "NCCL_INSPECTOR_DUMP_DIR", arg.SubstituteJobId(config.LogDir, jobId))
		if setByPlugin {
			env.Set(ctx, plugin.EnvLogDirSetByPlugin, "1")
		}
	}
	env.SetIfMissing(ctx, "NCCL_INSPECTOR_PROM_DUMP", "0")
	env.SetIfMissing(ctx, "NCCL_INSPECTOR_DUMP_THREAD_INTERVAL_MICROSECONDS", "1000000")
	env.SetIfMissing(ctx, "NCCL_INSPECTOR_DUMP_VERBOSE", "1")

	return C.ESPANK_SUCCESS
}

//export snccliprecon_spank_task_init_privileged
//goland:noinspection GoSnakeCaseUsage
func snccliprecon_spank_task_init_privileged(spank C.spank_t, argc C.int, argv **C.char) C.int {
	_, _ = argc, argv

	if C.snccliprecon_spank_context() != C.S_CTX_REMOTE {
		return C.ESPANK_SUCCESS
	}

	if !config.Enabled {
		return C.ESPANK_SUCCESS
	}

	failFast, spankRCIfFailFast, jobId, stepId, _ := ensureOncePerWorker(spank, plugin.LockNameOpTaskInitPrivileged)
	if failFast {
		return spankRCIfFailFast
	}
	ctx := bridge.NewSpankContext(unsafe.Pointer(spank))

	// region Ensure dump dir

	ensureDumpDir := func() error {
		if logDirSetByPlugin(ctx) {
			dumpDir := arg.SubstituteJobStepId(config.LogDir, jobId, stepId)
			env.Set(ctx, "NCCL_INSPECTOR_DUMP_DIR", dumpDir)
		}

		dumpDir, found := env.Get(ctx, "NCCL_INSPECTOR_DUMP_DIR")
		if !found || dumpDir == "" {
			return nil
		}

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

		inspectorSO, found := env.Get(ctx, "NCCL_PROFILER_PLUGIN")
		if !found || inspectorSO == "" {
			inspectorSO = config.InspectorSO
		}

		dumpDir, found := env.Get(ctx, "NCCL_INSPECTOR_DUMP_DIR")
		if !found || dumpDir == "" {
			return nil
		}

		return enroot.CreateMountFile(
			jobId,
			stepId,
			[]enroot.Mount{{
				Path:        inspectorSO,
				IsDir:       false,
				IsReadWrite: false,
			}, {
				Path:        dumpDir,
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

//export snccliprecon_spank_task_exit
//goland:noinspection GoSnakeCaseUsage
func snccliprecon_spank_task_exit(spank C.spank_t, argc C.int, argv **C.char) C.int {
	_, _ = argc, argv

	if C.snccliprecon_spank_context() != C.S_CTX_REMOTE {
		return C.ESPANK_SUCCESS
	}

	if !config.Enabled {
		return C.ESPANK_SUCCESS
	}

	failFast, spankRCIfFailFast, jobId, stepId, hostname := ensureOncePerWorker(spank, plugin.LockNameOpTaskExit)
	if failFast {
		return spankRCIfFailFast
	}

	if err := enroot.RemoveMountFile(jobId, stepId); err != nil {
		log.Error(err.Error())
		return C.ESPANK_ERROR
	}

	{
		_ = unix.RemoveLock(
			plugin.RenderWorkerOpLockName(jobId, stepId, hostname, plugin.LockNameOpUserInit),
		)
		_ = unix.RemoveLock(
			plugin.RenderWorkerOpLockName(jobId, stepId, hostname, plugin.LockNameOpTaskInitPrivileged),
		)
		_ = unix.RemoveLock(
			plugin.RenderWorkerOpLockName(jobId, stepId, hostname, plugin.LockNameOpTaskExit),
		)
	}

	return C.ESPANK_SUCCESS
}

func ensureOncePerWorker(spank C.spank_t, op string) (failFast bool, spankRCIfFailFast C.int, jobId, stepId, hostname string) {
	failFast = false
	spankRCIfFailFast = C.ESPANK_SUCCESS

	ctx := bridge.NewSpankContext(unsafe.Pointer(spank))
	jobId = ctx.GetJobId()

	stepId = ctx.GetStepId()
	if stepId == bridge.GetSbatchScriptID() {
		failFast = true
		return
	}

	hostname = unix.GetHostname()

	// Ensure hook ran once per worker.
	{
		if err := unix.CreateLock(
			plugin.RenderWorkerOpLockName(jobId, stepId, hostname, op),
		); err != nil {
			if errors.Is(err, unix.ErrLockExists) {
				failFast = true
				return
			}

			log.Message(err.Error())
			failFast = true
			spankRCIfFailFast = C.ESPANK_ERROR
			return
		}
	}

	return
}

func logDirSetByPlugin(ctx bridge.SpankContext) bool {
	value, found := env.Get(ctx, plugin.EnvLogDirSetByPlugin)
	return found && value == "1"
}

func main() {}
