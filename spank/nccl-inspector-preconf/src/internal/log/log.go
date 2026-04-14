package log

import "github.com/nebius/nccl-inspector-preconf/internal/bridge"

func Message(msg string) {
	bridge.Log(msg)
}
