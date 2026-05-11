#include "spank_constants.h"
#include "spank_config.h"
#include "spank_enroot.h"
#include "spank_file_ops.h"
#include "spank_shim.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct mount_file_context {
  const char *path;
  const char *profiler_plugin;
  const char *dump_dir;
} mount_file_context_t;

static int create_mount_file_locked(void *arg) {
  mount_file_context_t *ctx = (mount_file_context_t *) arg;

  struct stat st;
  if (stat(ctx->path, &st) == 0) {
    snccliprecon_log_debug2("create_mount_file: %s already exists", ctx->path);
    return snccliprecon_ensure_mode(ctx->path);
  }

  if (errno != ENOENT) {
    return -1;
  }

  int fd = open(ctx->path, O_CREAT | O_TRUNC | O_WRONLY, SNCCLIPRECON_DEFAULT_MODE);
  if (fd < 0) {
    return -1;
  }

  snccliprecon_log_debug2("create_mount_file: writing %s", ctx->path);
  int rc = dprintf(
    fd,
    "%s %s none x-create=file,bind,ro 0 0\n"
    "%s %s none x-create=dir,bind,rw 0 0\n",
    ctx->profiler_plugin,
    ctx->profiler_plugin,
    ctx->dump_dir,
    ctx->dump_dir
  );
  if (rc < 0) {
    int saved_errno = errno;
    close(fd);
    errno = saved_errno;
    return -1;
  }

  if (close(fd) != 0) {
    return -1;
  }

  return snccliprecon_ensure_mode(ctx->path);
}

static int remove_mount_file_locked(void *arg) {
  const char *path = (const char *) arg;
  if (unlink(path) != 0 && errno != ENOENT) {
    return -1;
  }

  snccliprecon_log_debug2("removed Enroot mount file if present: %s", path);
  return 0;
}

int snccliprecon_enroot_create_mount_file(
  uint32_t job_id,
  uint32_t step_id,
  const char *profiler_plugin,
  const char *dump_dir
) {
  char path[SNCCLIPRECON_PATH_MAX];
  int written = snprintf(
    path,
    sizeof(path),
    "%s/40-nccl-inspector-%u-%u.fstab",
    SNCCLIPRECON_ENROOT_MOUNTS_DIR,
    job_id,
    step_id
  );
  if (written < 0 || (size_t) written >= sizeof(path)) {
    errno = ENAMETOOLONG;
    return -1;
  }

  mount_file_context_t ctx = {
    .path = path,
    .profiler_plugin = profiler_plugin,
    .dump_dir = dump_dir,
  };

  return snccliprecon_with_lock(path, create_mount_file_locked, &ctx);
}

int snccliprecon_enroot_remove_mount_file(uint32_t job_id, uint32_t step_id) {
  char path[SNCCLIPRECON_PATH_MAX];
  int written = snprintf(
    path,
    sizeof(path),
    "%s/40-nccl-inspector-%u-%u.fstab",
    SNCCLIPRECON_ENROOT_MOUNTS_DIR,
    job_id,
    step_id
  );
  if (written < 0 || (size_t) written >= sizeof(path)) {
    errno = ENAMETOOLONG;
    return -1;
  }

  return snccliprecon_with_lock(path, remove_mount_file_locked, path);
}
