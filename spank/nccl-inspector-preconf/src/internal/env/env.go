package env

import "github.com/nebius/nccl-inspector-preconf/internal/bridge"

func Get(spank bridge.SpankContext, key string) (string, bool) {
	return spank.Get(key)
}

func Set(spank bridge.SpankContext, key, value string) (string, bool) {
	return value, spank.Set(key, value)
}

func SetIfMissing(spank bridge.SpankContext, key, value string) (string, bool) {
	if res, found := Get(spank, key); found {
		return res, false
	}
	return Set(spank, key, value)
}
