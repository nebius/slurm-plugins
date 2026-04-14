package arg

import (
	"fmt"
	"strings"

	"github.com/nebius/nccl-inspector-preconf/internal/cfg"
	"github.com/nebius/nccl-inspector-preconf/internal/plugin"
)

type Arg struct {
	Name      string
	Usage     string
	HasValue  bool
	ValueInfo string
	ParseFn   ParseFn
}

type (
	ParseFn = func(config *cfg.Config, value string) error
)

func (arg Arg) Parse(config *cfg.Config, value string) error {
	if !arg.HasValue {
		return nil
	}

	return arg.ParseFn(config, value)
}

const (
	BoolValueInfo = "(1 | True) | (0 | False)"
	PathValueInfo = "PATH"
)

func NewBoolArg(name, usage string, parseFn ParseFn) Arg {
	return Arg{
		Name: name,
		Usage: strings.Join(
			[]string{
				fmt.Sprintf("[%s]", plugin.Name),
				usage,
				"Possible values are case-insensitive.",
				fmt.Sprintf("%s env var is also supported.", GetArgNameForEnv(name)),
			},
			" ",
		),
		HasValue:  true,
		ValueInfo: BoolValueInfo,
		ParseFn:   parseFn,
	}
}

func NewStringArg(name, usage, valueInfo string, parseFn ParseFn) Arg {
	return Arg{
		Name: name,
		Usage: strings.Join(
			[]string{
				fmt.Sprintf("[%s]", plugin.Name),
				usage,
				fmt.Sprintf("%s env var is also supported.", GetArgNameForEnv(name)),
			},
			" ",
		),
		HasValue:  true,
		ValueInfo: valueInfo,
		ParseFn:   parseFn,
	}
}

func ParseBoolValue(value string) (bool, error) {
	if value == "1" || strings.ToLower(value) == "true" {
		return true, nil
	}

	if value == "0" || strings.ToLower(value) == "false" {
		return false, nil
	}

	return false, fmt.Errorf("invalid boolean value %q", value)
}

func ParseStringValue(value string) (string, error) {
	if len(value) == 0 {
		return "", fmt.Errorf("invalid string value %q", value)
	}

	return value, nil
}

func ParseByName(config *cfg.Config, name, value string) error {
	for _, definedArg := range Args {
		if definedArg.Name == name {
			if err := definedArg.Parse(config, value); err != nil {
				return fmt.Errorf("invalid value %q for %q: %w", value, name, err)
			}

			return nil
		}
	}

	return fmt.Errorf("unknown plugin arg %q", name)
}

const (
	SubstitutionJobId = "%j"
)

func SubstituteJobId(value, jobId string) string {
	return strings.ReplaceAll(value, SubstitutionJobId, jobId)
}
