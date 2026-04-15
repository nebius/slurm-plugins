package unix

import (
	"os"
)

// GetHostname returns the current worker hostname or "unknown".
func GetHostname() string {
	hostname, err := os.Hostname()
	if err != nil || hostname == "" {
		return "unknown"
	}
	return hostname
}
