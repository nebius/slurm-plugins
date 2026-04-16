package arg

import (
	"fmt"
	"strconv"
	"strings"

	"github.com/nebius/nccl-inspector-preconf/internal/cfg"
	"github.com/nebius/nccl-inspector-preconf/internal/plugin"
)

// Arg describes one supported plugin argument.
type Arg struct {
	Name      string
	Usage     string
	HasValue  bool
	ValueInfo string
	ParseFn   ParseFn
}

type (
	// ParseFn stores one parsed argument value into the plugin configuration.
	ParseFn = func(config *cfg.Config, value string) error
)

// Parse validates and stores the argument value into config.
func (arg Arg) Parse(config *cfg.Config, value string) error {
	if !arg.HasValue {
		return nil
	}

	return arg.ParseFn(config, value)
}

const (
	// BoolValueInfo describes the accepted boolean value syntax.
	BoolValueInfo = "(1 | True) | (0 | False)"
	// PathValueInfo describes a filesystem path argument.
	PathValueInfo = "PATH"
)

// NewBoolArg constructs a boolean argument definition.
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

// NewStringArg constructs a string argument definition.
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

// ParseBoolValue parses the plugin boolean representation.
func ParseBoolValue(value string) (bool, error) {
	if value == "1" || strings.ToLower(value) == "true" {
		return true, nil
	}

	if value == "0" || strings.ToLower(value) == "false" {
		return false, nil
	}

	return false, fmt.Errorf("invalid boolean value %q", value)
}

// ParseStringValue validates a non-empty string argument.
func ParseStringValue(value string) (string, error) {
	if len(value) == 0 {
		return "", fmt.Errorf("invalid string value %q", value)
	}

	return value, nil
}

// ParseUintValue validates an unsigned integer argument and preserves its string form.
func ParseUintValue(value string) (string, error) {
	if len(value) == 0 {
		return "", fmt.Errorf("invalid uint value %q", value)
	}

	if _, err := strconv.ParseUint(value, 10, 64); err != nil {
		return "", fmt.Errorf("invalid uint value %q: %w", value, err)
	}

	return value, nil
}

// FormatBoolValue renders a boolean as the NCCL Inspector 1/0 syntax.
func FormatBoolValue(value bool) string {
	if value {
		return "1"
	}

	return "0"
}

// ParseByName parses one named argument into config.
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
	// SubstitutionJobId expands to the Slurm job ID.
	SubstitutionJobId = "%j"
	// SubstitutionStepId expands to the Slurm step ID.
	SubstitutionStepId = "%s"
	// SubstitutionJobStepId expands to "<job ID>.<step ID>".
	SubstitutionJobStepId = "%J"
)

// SubstituteJobId replaces all job-id placeholders in value.
func SubstituteJobId(value, jobId string) string {
	return strings.ReplaceAll(value, SubstitutionJobId, jobId)
}

// SubstituteStepId replaces all step-id placeholders in value.
func SubstituteStepId(value, stepId string) string {
	return strings.ReplaceAll(value, SubstitutionStepId, stepId)
}

// SubstituteJobStepId replaces job, step, and combined job-step placeholders.
func SubstituteJobStepId(value, jobId, stepId string) string {
	value = SubstituteJobId(value, jobId)
	value = SubstituteStepId(value, stepId)
	return strings.ReplaceAll(value, SubstitutionJobStepId, jobId+"."+stepId)
}
