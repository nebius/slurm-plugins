package plugin

import "strings"

const (
	Name   = "nccl_inspector_preconf"
	Prefix = "snccliprecon"
)

var (
	EnvLogDirSetByPlugin = strings.ToUpper(Prefix) + "_LOG_DIR_SET_BY_PLUGIN"
)
