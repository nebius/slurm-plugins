package arg

import (
	"fmt"
	"strings"

	"github.com/nebius/nccl-inspector-preconf/internal/plugin"
)

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
