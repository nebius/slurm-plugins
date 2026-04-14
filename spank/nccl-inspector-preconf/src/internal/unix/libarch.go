package unix

import (
	"fmt"
	"runtime"
)

func LibArch() string {
	var res string

	switch runtime.GOARCH {
	case "amd64":
		res = "x86_64"
	case "arm64":
		res = "aarch64"
	default:
		res = runtime.GOARCH
	}

	return fmt.Sprintf("%s-linux-gnu", res)
}
