package cfg

import (
	"fmt"

	"github.com/nebius/nccl-inspector-preconf/internal/unix"
)

type Config struct {
	Enabled     bool
	InspectorSO string
	LogDir      string
}

func NewConfig() *Config {
	return &Config{
		Enabled:     true,
		InspectorSO: fmt.Sprintf("/usr/lib/%s/libnccl-profiler-inspector.so", unix.LibArch()),
		LogDir:      "/opt/soperator-outputs/nccl_profiles",
	}
}
