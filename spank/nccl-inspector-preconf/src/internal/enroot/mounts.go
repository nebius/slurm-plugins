package enroot

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/nebius/nccl-inspector-preconf/internal/unix"
)

const (
	// MountsDir is Enroot's system mount directory.
	MountsDir = "/etc/enroot/mounts.d"
)

// mountFilePath renders the Enroot mount file path for one job step.
func mountFilePath(jobID, stepID string) string {
	return filepath.Join(
		MountsDir,
		fmt.Sprintf("40-nccl-inspector-%s-%s.fstab", jobID, stepID),
	)
}

// Mount describes one bind mount to be written into an Enroot fstab file.
type Mount struct {
	Path        string
	IsDir       bool
	IsReadWrite bool
}

// CreateMountFile creates or reuses the mount file for one job step.
func CreateMountFile(jobID, stepID string, mounts []Mount) error {
	path := mountFilePath(jobID, stepID)

	return unix.WithLock(path, func() error {
		if _, err := os.Stat(path); err == nil {
			return unix.EnsureMode(path)
		} else if !errors.Is(err, os.ErrNotExist) {
			return err
		}

		mountLines := make([]string, len(mounts))
		for _, mount := range mounts {
			create := "x-create=file"
			if mount.IsDir {
				create = "x-create=dir"
			}

			permissions := "ro"
			if mount.IsReadWrite {
				permissions = "rw"
			}

			mountLines = append(
				mountLines,
				fmt.Sprintf(
					"%s %s none %s,bind,%s 0 0\n",
					mount.Path,
					mount.Path,
					create,
					permissions,
				),
			)
		}

		if err := os.WriteFile(
			path,
			[]byte(strings.Join(mountLines, "")),
			unix.DefaultFileDirMode,
		); err != nil {
			return err
		}

		return unix.EnsureMode(path)
	})
}

// RemoveMountFile removes the mount file for one job step.
func RemoveMountFile(jobID, stepID string) error {
	path := mountFilePath(jobID, stepID)

	return unix.WithLock(path, func() error {
		if err := os.Remove(path); err == nil || errors.Is(err, os.ErrNotExist) {
			return nil
		} else {
			return err
		}
	})
}
