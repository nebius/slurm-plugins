//go:generate go run ../../cmd/spank-options-gen

package arg

import (
	"fmt"
	"strings"

	"github.com/nebius/nccl-inspector-preconf/internal/cfg"
	"github.com/nebius/nccl-inspector-preconf/internal/plugin"
)

var (
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
			"inspector-so",
			"path to the NCCL Inspector SO file.",
			PathValueInfo,
			func(config *cfg.Config, value string) error {
				parsed, err := ParseStringValue(value)
				if err != nil {
					return err
				}

				config.InspectorSO = parsed
				return nil
			},
		),
		NewStringArg(
			"out-dir",
			strings.Join(
				[]string{
					"path to the directory for storing NCCL Inspector outputs.",
					fmt.Sprintf(
						"Supports %s substitution for job ID.",
						SubstitutionJobId,
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

				config.LogDir = parsed
				return nil
			},
		),
	}
)
