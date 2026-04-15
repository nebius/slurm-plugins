package arg

import (
	"fmt"
	"strings"

	"github.com/nebius/nccl-inspector-preconf/internal/plugin"
)

// GetArgNameForEnv renders the environment variable name for one argument.
func GetArgNameForEnv(name string) string {
	return strings.ReplaceAll(
		fmt.Sprintf(
			"%s_%s",
			strings.ToUpper(plugin.Prefix),
			strings.ToUpper(name),
		),
		"-", "_",
	)
}
