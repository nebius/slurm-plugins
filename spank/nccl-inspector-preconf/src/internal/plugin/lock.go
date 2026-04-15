package plugin

import (
	"path"
	"strings"
)

const (
	// TmpDirBase is the worker-local directory that stores durable worker locks.
	TmpDirBase = "/tmp/" + Name
)

// RenderWorkerOpLockName renders the durable once-per-worker lock name.
func RenderWorkerOpLockName(jobId, stepId, hostname, op string) string {
	return path.Join(
		TmpDirBase,
		strings.Join(
			[]string{hostname, jobId, stepId, op},
			".",
		),
	)
}

const (
	// LockNameOpUserInit marks the user-init hook execution.
	LockNameOpUserInit = "user-init"
	// LockNameOpTaskInitPrivileged marks the task-init-privileged hook execution.
	LockNameOpTaskInitPrivileged = "task-init-privileged"
	// LockNameOpTaskExit marks the task-exit hook execution.
	LockNameOpTaskExit = "task-exit"
)
