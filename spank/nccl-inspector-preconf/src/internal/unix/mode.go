package unix

import (
	"errors"
	"os"
	"syscall"
)

const (
	DefaultFileDirMode = os.FileMode(0o777)
)

func EnsureMode(path string) error {
	info, err := os.Stat(path)
	if err != nil {
		return nil
	}

	if info.Mode().Perm() == DefaultFileDirMode.Perm() {
		return nil
	}

	if err = os.Chmod(path, DefaultFileDirMode); err != nil {
		if errors.Is(err, os.ErrPermission) || errors.Is(err, syscall.EPERM) || errors.Is(err, syscall.EACCES) {
			return nil
		}

		return err
	}

	return nil
}
