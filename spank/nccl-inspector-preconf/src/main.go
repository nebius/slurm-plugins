package main

/*
#cgo CFLAGS: -Wall -Wextra
#include "spank_shim.h"
#include <stdlib.h>
*/
import "C"
import (
	"fmt"

	"github.com/nebius/nccl-inspector-preconf/internal/cfg"
)

var (
	config = cfg.NewConfig()
)

//export go_spank_init
//goland:noinspection GoSnakeCaseUsage
func go_spank_init(spank C.spank_t, argc C.int, argv **C.char) C.int {
	fmt.Println("Hi from go_spank_init")
	config.Print()
	config.A = "2"

	ctx := C.snccliprecon_spank_context()
	fmt.Printf("Context: %v+\n", ctx)
	switch ctx {
	case C.S_CTX_LOCAL, C.S_CTX_REMOTE:
		{
		}
	}

	return C.ESPANK_SUCCESS
}

//export go_spank_user_init
//goland:noinspection GoSnakeCaseUsage
func go_spank_user_init(spank C.spank_t, argc C.int, argv **C.char) C.int {
	fmt.Println("Hi from go_spank_user_init")
	config.Print()
	config.A = "3"

	return 0
}

//export go_spank_task_init_privileged
//goland:noinspection GoSnakeCaseUsage
func go_spank_task_init_privileged(spank C.spank_t, argc C.int, argv **C.char) C.int {
	fmt.Println("Hi from go_spank_task_init_privileged")
	config.Print()
	config.A = "4"

	return 0
}

//export go_spank_exit
//goland:noinspection GoSnakeCaseUsage
func go_spank_exit(spank C.spank_t, argc C.int, argv **C.char) C.int {
	fmt.Println("Hi from go_spank_exit")
	config.Print()

	return 0
}

func main() {}
