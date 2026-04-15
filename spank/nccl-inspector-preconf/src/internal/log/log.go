package log

import (
	"fmt"

	"github.com/nebius/nccl-inspector-preconf/internal/bridge"
	"github.com/nebius/nccl-inspector-preconf/internal/plugin"
)

// Message emits an informational plugin log message.
func Message(msg string) {
	bridge.Log(prependLog(msg))
}

// Error emits an error plugin log message.
func Error(msg string) {
	bridge.LogError(prependLog(msg))
}

// prependLog prefixes a log line with the plugin name.
func prependLog(msg string) string {
	return fmt.Sprintf("[%s]: %s", plugin.Name, msg)
}
