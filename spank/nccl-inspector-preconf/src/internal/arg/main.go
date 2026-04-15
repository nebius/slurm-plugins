//go:generate go run ../../cmd/spank-options-gen

package arg

import (
	"fmt"
	"strings"

	"github.com/nebius/nccl-inspector-preconf/internal/cfg"
	"github.com/nebius/nccl-inspector-preconf/internal/plugin"
)

var (
	// Args is the canonical list of supported plugin arguments.
	Args = []Arg{
		NewBoolArg(
			"enabled",
			fmt.Sprintf("whether to enable %s plugin.", plugin.Name),
			func(config *cfg.Config, value string) error {
				parsed, err := ParseBoolValue(value)
				if err != nil {
					return err
				}

				config.Enabled = parsed
				return nil
			},
		),
		NewStringArg(
			"profiler-plugin",
			"path to the NCCL Inspector profiler plugin SO file.",
			PathValueInfo,
			func(config *cfg.Config, value string) error {
				parsed, err := ParseStringValue(value)
				if err != nil {
					return err
				}

				config.ProfilerPlugin = parsed
				return nil
			},
		),
		NewStringArg(
			"dump-dir",
			strings.Join(
				[]string{
					"path to the directory for storing NCCL Inspector outputs.",
					fmt.Sprintf(
						"Supports %s, %s, and %s substitutions for <job ID>, <step ID> and <job ID>.<step ID> accordingly.",
						SubstitutionJobId,
						SubstitutionStepId,
						SubstitutionJobStepId,
					),
				},
				" ",
			),
			PathValueInfo,
			func(config *cfg.Config, value string) error {
				parsed, err := ParseStringValue(value)
				if err != nil {
					return err
				}

				config.DumpDir = parsed
				return nil
			},
		),
		NewBoolArg(
			"prom-dump",
			"whether to enable NCCL Inspector Prometheus dump output.",
			func(config *cfg.Config, value string) error {
				parsed, err := ParseBoolValue(value)
				if err != nil {
					return err
				}

				config.PromDump = parsed
				return nil
			},
		),
		NewBoolArg(
			"dump-verbose",
			"whether to enable verbose NCCL Inspector dumps.",
			func(config *cfg.Config, value string) error {
				parsed, err := ParseBoolValue(value)
				if err != nil {
					return err
				}

				config.DumpVerbose = parsed
				return nil
			},
		),
		NewStringArg(
			"dump-thread-interval-microseconds",
			"interval between NCCL Inspector dump thread runs in microseconds.",
			"UINT",
			func(config *cfg.Config, value string) error {
				parsed, err := ParseStringValue(value)
				if err != nil {
					return err
				}

				config.DumpThreadIntervalMicroseconds = parsed
				return nil
			},
		),
	}
)
