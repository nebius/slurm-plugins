package plugin

import (
	"path"
	"strings"
)

const (
	TmpDirBase = "/tmp/" + Name
)

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
	LockNameOpUserInit           = "user-init"
	LockNameOpTaskInitPrivileged = "task-init-privileged"
	LockNameOpTaskExit           = "task-exit"
)
