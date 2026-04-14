package cfg

import (
	"fmt"

	"github.com/nebius/nccl-inspector-preconf/internal/log"
	"github.com/nebius/nccl-inspector-preconf/internal/unix"
)

type Config struct {
	Enabled     bool
	InspectorSO string
	LogDir      string
	A           string
}

func NewConfig() *Config {
	return &Config{
		Enabled:     true,
		InspectorSO: fmt.Sprintf("/usr/lib/%s/libnccl-profiler-inspector.so", unix.LibArch()),
		LogDir:      "/opt/soperator-outputs/nccl_profiles",
		A:           "1",
	}
}

func (c *Config) Print() {
	log.Message("Config:")
	log.Message(fmt.Sprintf("	Enabled: %v\n", c.Enabled))
	log.Message(fmt.Sprintf("	InspectorSO: %s\n", c.InspectorSO))
	log.Message(fmt.Sprintf("	LogDir: %s\n", c.LogDir))
	log.Message(fmt.Sprintf("	A: %s\n", c.A))
}
