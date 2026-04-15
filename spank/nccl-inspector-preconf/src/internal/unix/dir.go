package unix

import (
	"os"
)

func DirExists(path string) bool {
	info, err := os.Stat(path)
	if err != nil {
		return false
	}

	return info.IsDir()
}

func EnsureDir(path string) error {
	if !DirExists(path) {
		if err := os.MkdirAll(path, DefaultFileDirMode); err != nil {
			return err
		}
	}

	return EnsureMode(path)
}
