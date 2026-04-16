package unix

import (
	"os"
)

// DirExists reports whether path exists and is a directory.
func DirExists(path string) bool {
	info, err := os.Stat(path)
	if err != nil {
		return false
	}

	return info.IsDir()
}

// EnsureDir creates path if needed and normalizes its permissions.
func EnsureDir(path string) error {
	if !DirExists(path) {
		if err := os.MkdirAll(path, DefaultFileDirMode); err != nil {
			return err
		}
	}

	return EnsureMode(path)
}
