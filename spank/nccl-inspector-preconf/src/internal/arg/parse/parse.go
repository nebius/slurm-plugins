package argparse

import (
	"fmt"
	"strings"

	"github.com/nebius/nccl-inspector-preconf/internal/arg"
	"github.com/nebius/nccl-inspector-preconf/internal/cfg"
	"github.com/nebius/nccl-inspector-preconf/internal/env"
	"github.com/nebius/nccl-inspector-preconf/internal/log"
)

func ParseArgs(config *cfg.Config, spank env.Context, passedArgs []string) {
	ParseArgsFromPlugstack(config, passedArgs)
	ParseArgsFromEnv(config, spank)
}

func ParseArgsFromPlugstack(config *cfg.Config, passedArgs []string) {
	for _, passedArg := range passedArgs {
		recognized := false

		for _, definedArg := range arg.Args {
			if argValue, ok := strings.CutPrefix(passedArg, definedArg.Name+"="); ok {
				recognized = true
				if err := definedArg.Parse(config, argValue); err != nil {
					log.Message(fmt.Sprintf("Invalid plugin arg %s: %v", passedArg, err))
				}
				break
			}
		}

		if !recognized {
			log.Message(fmt.Sprintf("Unknown plugin arg: %s", passedArg))
		}
	}
}

func ParseArgsFromEnv(config *cfg.Config, spank env.Context) {
	_ = config
	_ = spank
	// Placeholder for env-backed parsing once env var handling is implemented.
}
