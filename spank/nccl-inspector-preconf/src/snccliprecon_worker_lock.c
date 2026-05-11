#include "snccliprecon_file_ops.h"
#include "snccliprecon.h"
#include "snccliprecon_worker_lock.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int snccliprecon_get_hostname(char *hostname, size_t hostname_size) {
  if (hostname_size == 0) {
    errno = EINVAL;
    return -1;
  }

  if (gethostname(hostname, hostname_size) != 0 || hostname[0] == '\0') {
    (void) snprintf(hostname, hostname_size, "%s", "unknown");
  }
  hostname[hostname_size - 1] = '\0';

  return 0;
}

int snccliprecon_render_worker_lock_path(
  uint32_t job_id,
  uint32_t step_id,
  const char *hostname,
  const char *op,
  char *path,
  size_t path_size
) {
  int written = snprintf(
    path,
    path_size,
    "%s/%s.%u.%u.%s.lock",
    SNCCLIPRECON_TMP_DIR_BASE,
    hostname,
    job_id,
    step_id,
    op
  );
  if (written < 0 || (size_t) written >= path_size) {
    errno = ENAMETOOLONG;
    return -1;
  }

  return 0;
}

int snccliprecon_create_once_per_worker_lock(
  uint32_t job_id,
  uint32_t step_id,
  const char *hostname,
  const char *op
) {
  char lock_path[SNCCLIPRECON_PATH_MAX];
  if (snccliprecon_render_worker_lock_path(job_id, step_id, hostname, op, lock_path, sizeof(lock_path)) != 0) {
    return -1;
  }

  int fd = open(lock_path, O_CREAT | O_EXCL | O_RDWR, SNCCLIPRECON_DEFAULT_MODE);
  if (fd < 0) {
    if (errno == EEXIST) {
      snccliprecon_log_debug2("worker lock already exists: %s", lock_path);
      return 1;
    }
    return -1;
  }

  snccliprecon_log_debug2("created worker lock: %s", lock_path);
  if (snccliprecon_ensure_mode(lock_path) != 0) {
    int saved_errno = errno;
    close(fd);
    errno = saved_errno;
    return -1;
  }

  if (close(fd) != 0) {
    return -1;
  }

  return 0;
}

void snccliprecon_remove_worker_lock(uint32_t job_id, uint32_t step_id, const char *hostname, const char *op) {
  char lock_path[SNCCLIPRECON_PATH_MAX];
  if (snccliprecon_render_worker_lock_path(job_id, step_id, hostname, op, lock_path, sizeof(lock_path)) != 0) {
    snccliprecon_log_debug2("%s: cannot render worker lock: %s", op, strerror(errno));
    return;
  }

  if (unlink(lock_path) != 0 && errno != ENOENT) {
    snccliprecon_log_debug2("%s: cannot remove worker lock %s: %s", op, lock_path, strerror(errno));
    return;
  }

  snccliprecon_log_debug2("removed worker lock if present: %s", lock_path);
}
