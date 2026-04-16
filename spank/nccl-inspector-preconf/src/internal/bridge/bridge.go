package bridge

/*
#cgo CFLAGS: -I${SRCDIR}/../..
#include "spank_shim.h"
#include <stdlib.h>
*/
import "C"

import (
	"fmt"
	"unsafe"
)

// region Context

// SpankContext wraps a raw spank_t handle for Go code.
type SpankContext struct {
	raw unsafe.Pointer
}

// NewSpankContext wraps a raw SPANK pointer for bridge helpers.
func NewSpankContext(raw unsafe.Pointer) SpankContext {
	return SpankContext{raw: raw}
}

// endregion Context

// region Env

// Get reads one SPANK environment variable.
func (context SpankContext) Get(key string) (string, bool) {
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	buffer := make([]byte, 4096)
	if rc := C.snccliprecon_getenv(
		(C.spank_t)(context.raw),
		cKey,
		(*C.char)(unsafe.Pointer(&buffer[0])),
		C.int(len(buffer)),
	); rc != C.ESPANK_SUCCESS {
		return "", false
	}

	length := 0
	for length < len(buffer) && buffer[length] != 0 {
		length++
	}

	return string(buffer[:length]), true
}

// Set writes one SPANK environment variable.
func (context SpankContext) Set(key, value string) bool {
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	cValue := C.CString(value)
	defer C.free(unsafe.Pointer(cValue))

	return C.snccliprecon_setenv(
		(C.spank_t)(context.raw),
		cKey,
		cValue,
	) == C.ESPANK_SUCCESS
}

// endregion Env

// region Job

// GetJobId returns the current Slurm job ID as a string.
func (context SpankContext) GetJobId() string {
	retrieve := func() uint32 {
		var jobID C.uint32_t

		if rc := C.snccliprecon_get_job_id(
			(C.spank_t)(context.raw),
			&jobID,
		); rc != C.ESPANK_SUCCESS {
			return 0
		}

		return uint32(jobID)
	}

	return fmt.Sprintf("%d", retrieve())
}

// GetStepId returns the current Slurm step ID as a string.
func (context SpankContext) GetStepId() string {
	retrieve := func() uint32 {
		var stepID C.uint32_t

		if rc := C.snccliprecon_get_step_id(
			(C.spank_t)(context.raw),
			&stepID,
		); rc != C.ESPANK_SUCCESS {
			return 0
		}

		return uint32(stepID)
	}

	return fmt.Sprintf("%d", retrieve())
}

// GetSbatchScriptID returns Slurm's batch-script sentinel step ID.
func GetSbatchScriptID() string {
	var stepID C.uint32_t = C.snccliprecon_slurm_batch_script_id
	return fmt.Sprintf("%d", uint32(stepID))
}

// endregion Job

// region Log

// Log emits an informational SPANK log message.
func Log(msg string) {
	cString := C.CString(msg)
	defer C.free(unsafe.Pointer(cString))

	C.snccliprecon_log(cString)
}

// LogError emits an error SPANK log message.
func LogError(msg string) {
	cString := C.CString(msg)
	defer C.free(unsafe.Pointer(cString))

	C.snccliprecon_log_error(cString)
}

// endregion Log

// region Utils

// CStringArrayToStrings converts a C argv-style array into Go strings.
func CStringArrayToStrings(argc int, argv unsafe.Pointer) []string {
	if argc == 0 || argv == nil {
		return nil
	}

	raw := unsafe.Slice((**C.char)(argv), argc)
	args := make([]string, 0, len(raw))
	for _, value := range raw {
		args = append(args, C.GoString(value))
	}

	return args
}

// endregion Utils
