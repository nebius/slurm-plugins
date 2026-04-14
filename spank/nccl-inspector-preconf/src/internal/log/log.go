package log

import (
	"fmt"

	"github.com/nebius/nccl-inspector-preconf/internal/bridge"
	"github.com/nebius/nccl-inspector-preconf/internal/plugin"
)

func Message(msg string) {
	bridge.Log(prependLog(msg))
}

func Error(msg string) {
	bridge.LogError(prependLog(msg))
}

func prependLog(msg string) string {
	return fmt.Sprintf("[%s]: %s", plugin.Name, msg)
}
