package plugin

import "strings"

const (
	// Name is the SPANK plugin name.
	Name = "nccl_inspector_preconf"
	// Prefix is the option and helper symbol prefix used by the plugin.
	Prefix = "snccliprecon"
)

var (
	// EnvLogDirSetByPlugin marks that the plugin initialized the dump dir env var.
	EnvLogDirSetByPlugin = strings.ToUpper(Prefix) + "_LOG_DIR_SET_BY_PLUGIN"
)
