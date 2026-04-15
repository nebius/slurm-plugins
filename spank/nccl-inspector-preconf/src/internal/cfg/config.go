package cfg

import (
	"fmt"

	"github.com/nebius/nccl-inspector-preconf/internal/unix"
)

type Config struct {
	Enabled                        bool
	ProfilerPlugin                 string
	PromDump                       bool
	DumpDir                        string
	DumpVerbose                    bool
	DumpThreadIntervalMicroseconds string
}

func NewConfig() *Config {
	return &Config{
		Enabled:                        true,
		ProfilerPlugin:                 fmt.Sprintf("/usr/lib/%s/libnccl-profiler-inspector.so", unix.LibArch()),
		PromDump:                       false,
		DumpDir:                        "/opt/soperator-outputs/nccl_profiles",
		DumpVerbose:                    true,
		DumpThreadIntervalMicroseconds: "1000000",
	}
}
