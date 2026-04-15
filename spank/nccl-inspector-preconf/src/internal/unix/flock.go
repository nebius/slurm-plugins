package unix

import (
	"os"
	"syscall"
)

func WithLock(path string, fn func() error) error {
	lockPath := path + ".lock"

	lockFile, err := os.OpenFile(
		lockPath,
		os.O_CREATE|os.O_RDWR,
		DefaultFileDirMode,
	)
	if err != nil {
		return err
	}
	defer lockFile.Close()

	if err = EnsureMode(lockPath); err != nil {
		return err
	}

	if err = syscall.Flock(
		int(lockFile.Fd()),
		syscall.LOCK_EX,
	); err != nil {
		return err
	}
	defer syscall.Flock(
		int(lockFile.Fd()),
		syscall.LOCK_UN,
	)

	return fn()
}
