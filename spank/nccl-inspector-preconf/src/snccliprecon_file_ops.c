#include "snccliprecon_file_ops.h"
#include "snccliprecon.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int snccliprecon_ensure_mode(const char *path) {
  struct stat st;
  if (stat(path, &st) != 0) {
    snccliprecon_log_debug2("ensure_mode: %s does not exist", path);
    return 0;
  }

  if ((st.st_mode & SNCCLIPRECON_DEFAULT_MODE) == SNCCLIPRECON_DEFAULT_MODE) {
    snccliprecon_log_debug2("ensure_mode: %s already has mode %d", path, SNCCLIPRECON_DEFAULT_MODE);
    return 0;
  }

  snccliprecon_log_debug2("ensure_mode: chmod %d %s", SNCCLIPRECON_DEFAULT_MODE, path);
  if (chmod(path, SNCCLIPRECON_DEFAULT_MODE) != 0) {
    if (errno == EPERM || errno == EACCES) {
      snccliprecon_log_debug2("ensure_mode: ignoring permission error for %s", path);
      return 0;
    }
    return -1;
  }

  return 0;
}

bool snccliprecon_dir_exists(const char *path) {
  struct stat st;
  return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

int snccliprecon_mkdir_all(const char *path) {
  char tmp[SNCCLIPRECON_PATH_MAX];
  int written = snprintf(tmp, sizeof(tmp), "%s", path);
  if (written < 0 || (size_t) written >= sizeof(tmp)) {
    errno = ENAMETOOLONG;
    return -1;
  }

  size_t len = strlen(tmp);
  while (len > 1 && tmp[len - 1] == '/') {
    tmp[--len] = '\0';
  }

  for (char *p = tmp + 1; *p != '\0'; ++p) {
    if (*p != '/') {
      continue;
    }

    *p = '\0';
    if (mkdir(tmp, SNCCLIPRECON_DEFAULT_MODE) != 0 && errno != EEXIST) {
      *p = '/';
      return -1;
    }
    *p = '/';
  }

  if (mkdir(tmp, SNCCLIPRECON_DEFAULT_MODE) != 0 && errno != EEXIST) {
    return -1;
  }

  return 0;
}

int snccliprecon_ensure_dir(const char *path) {
  struct stat st;
  if (stat(path, &st) != 0) {
    if (errno != ENOENT) {
      return -1;
    }

    snccliprecon_log_debug2("ensure_dir: creating %s", path);
    if (snccliprecon_mkdir_all(path) != 0) {
      return -1;
    }
  } else if (!S_ISDIR(st.st_mode)) {
    errno = ENOTDIR;
    return -1;
  } else {
    snccliprecon_log_debug2("ensure_dir: %s already exists", path);
  }

  return snccliprecon_ensure_mode(path);
}

int snccliprecon_with_lock(const char *path, snccliprecon_locked_fn_t fn, void *arg) {
  char lock_path[SNCCLIPRECON_PATH_MAX];
  int written = snprintf(lock_path, sizeof(lock_path), "%s.lock", path);
  if (written < 0 || (size_t) written >= sizeof(lock_path)) {
    errno = ENAMETOOLONG;
    return -1;
  }

  int fd = open(lock_path, O_CREAT | O_RDWR, SNCCLIPRECON_DEFAULT_MODE);
  if (fd < 0) {
    return -1;
  }

  snccliprecon_log_debug2("with_lock: acquiring %s", lock_path);
  if (snccliprecon_ensure_mode(lock_path) != 0) {
    close(fd);
    return -1;
  }

  if (flock(fd, LOCK_EX) != 0) {
    close(fd);
    return -1;
  }

  int rc = fn(arg);
  int saved_errno = errno;

  snccliprecon_log_debug2("with_lock: releasing %s", lock_path);
  (void) flock(fd, LOCK_UN);
  (void) unlink(lock_path);
  (void) close(fd);

  errno = saved_errno;
  return rc;
}
