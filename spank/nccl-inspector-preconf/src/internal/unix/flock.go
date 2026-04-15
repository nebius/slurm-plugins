package unix

import (
	"errors"
	"os"
	"syscall"
)

var ErrLockExists = errors.New("lock already exists")

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

func CreateLock(path string) error {
	lockPath := path + ".lock"

	lockFile, err := os.OpenFile(
		lockPath,
		os.O_CREATE|os.O_EXCL|os.O_RDWR,
		DefaultFileDirMode,
	)
	if err != nil {
		if os.IsExist(err) {
			return ErrLockExists
		}
		return err
	}

	if err = EnsureMode(lockPath); err != nil {
		lockFile.Close()
		return err
	}

	return lockFile.Close()
}

func RemoveLock(path string) error {
	lockPath := path + ".lock"

	if err := os.Remove(lockPath); err != nil && !errors.Is(err, os.ErrNotExist) {
		return err
	}

	return nil
}
