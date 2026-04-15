package argparse

import (
	"fmt"
	"strings"

	"github.com/nebius/nccl-inspector-preconf/internal/arg"
	"github.com/nebius/nccl-inspector-preconf/internal/bridge"
	"github.com/nebius/nccl-inspector-preconf/internal/cfg"
	"github.com/nebius/nccl-inspector-preconf/internal/log"
)

// ParseArgs parses plugin arguments from both plugstack args and SPANK env.
func ParseArgs(config *cfg.Config, spank bridge.SpankContext, passedArgs []string) {
	ParseArgsFromPlugstack(config, passedArgs)
	ParseArgsFromEnv(config, spank)
}

// ParseArgsFromPlugstack parses args passed directly in plugstack configuration.
func ParseArgsFromPlugstack(config *cfg.Config, passedArgs []string) {
	for _, passedArg := range passedArgs {
		recognized := false

		for _, definedArg := range arg.Args {
			if argValue, ok := strings.CutPrefix(passedArg, definedArg.Name+"="); ok {
				recognized = true
				if err := definedArg.Parse(config, argValue); err != nil {
					log.Error(fmt.Sprintf("Invalid plugin arg %s: %v", passedArg, err))
				}
				break
			}
		}

		if !recognized {
			log.Error(fmt.Sprintf("Unknown plugin arg: %s", passedArg))
		}
	}
}

// ParseArgsFromEnv parses args exported through SPANK environment variables.
func ParseArgsFromEnv(config *cfg.Config, spank bridge.SpankContext) {
	for _, definedArg := range arg.Args {
		var (
			value string
			found bool
		)
		if value, found = spank.Get(arg.GetArgNameForEnv(definedArg.Name)); !found {
			continue
		}

		if err := definedArg.Parse(config, value); err != nil {
			log.Error(fmt.Sprintf("Invalid plugin arg %s value: %v", definedArg.Name, err))
		}
	}
}
