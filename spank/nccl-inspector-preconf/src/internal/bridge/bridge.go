package bridge

/*
#cgo CFLAGS: -I${SRCDIR}/../..
#include "spank_shim.h"
#include <stdlib.h>
*/
import "C"

import (
	"unsafe"
)

// region Context

type SpankContext struct {
	raw unsafe.Pointer
}

func NewSpankContext(raw unsafe.Pointer) SpankContext {
	return SpankContext{raw: raw}
}

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

// endregion Context

// region Env

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

// region Log

func Log(msg string) {
	cString := C.CString(msg)
	defer C.free(unsafe.Pointer(cString))

	C.snccliprecon_log(cString)
}

// endregion Log
