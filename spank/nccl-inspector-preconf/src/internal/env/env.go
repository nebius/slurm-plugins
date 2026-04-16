package env

import "github.com/nebius/nccl-inspector-preconf/internal/bridge"

// Get reads one SPANK environment variable.
func Get(spank bridge.SpankContext, key string) (string, bool) {
	return spank.Get(key)
}

// Set writes one SPANK environment variable and returns the written value.
func Set(spank bridge.SpankContext, key, value string) (string, bool) {
	return value, spank.Set(key, value)
}

// SetIfMissing writes one SPANK environment variable only if it is absent.
func SetIfMissing(spank bridge.SpankContext, key, value string) (string, bool) {
	if res, found := Get(spank, key); found {
		return res, false
	}
	return Set(spank, key, value)
}
