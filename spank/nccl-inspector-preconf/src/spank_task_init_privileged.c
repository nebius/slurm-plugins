#include "spank_config.h"
#include "spank_shim.h"
#include "spank_task_init_privileged.h"

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SNCCLIPRECON_DEFAULT_MODE 0777
#define SNCCLIPRECON_ENROOT_MOUNTS_DIR "/etc/enroot/mounts.d"
#define SNCCLIPRECON_TMP_DIR_BASE "/tmp/nccl_inspector_preconf"
#define SNCCLIPRECON_LOG_DIR_SET_BY_PLUGIN "SNCCLIPRECON_LOG_DIR_SET_BY_PLUGIN"
#define SNCCLIPRECON_TASK_INIT_OP "task-init-privileged"

typedef struct mount_file_context {
  const char *path;
  const char *profiler_plugin;
  const char *dump_dir;
} mount_file_context_t;

static int ensure_mode(const char *path) {
  struct stat st;
  if (stat(path, &st) != 0) {
    snccliprecon_log_debug2("ensure_mode: %s does not exist", path);
    return 0;
  }

  if ((st.st_mode & 0777) == SNCCLIPRECON_DEFAULT_MODE) {
    snccliprecon_log_debug2("ensure_mode: %s already has mode 0777", path);
    return 0;
  }

  snccliprecon_log_debug2("ensure_mode: chmod 0777 %s", path);
  if (chmod(path, SNCCLIPRECON_DEFAULT_MODE) != 0) {
    if (errno == EPERM || errno == EACCES) {
      snccliprecon_log_debug2("ensure_mode: ignoring permission error for %s", path);
      return 0;
    }
    return -1;
  }

  return 0;
}

static bool dir_exists(const char *path) {
  struct stat st;
  return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static int mkdir_all(const char *path) {
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

static int ensure_dir(const char *path) {
  if (!dir_exists(path)) {
    snccliprecon_log_debug2("ensure_dir: creating %s", path);
    if (mkdir_all(path) != 0) {
      return -1;
    }
  } else {
    snccliprecon_log_debug2("ensure_dir: %s already exists", path);
  }

  return ensure_mode(path);
}

static bool get_env(spank_t spank, const char *key, char *buffer, size_t buffer_size) {
  if (buffer_size == 0) {
    return false;
  }

  buffer[0] = '\0';
  return spank_getenv(spank, key, buffer, (int) buffer_size) == ESPANK_SUCCESS;
}

static int append_string(char *dst, size_t dst_size, size_t *offset, const char *value) {
  size_t value_len = strlen(value);
  if (*offset + value_len >= dst_size) {
    errno = ENAMETOOLONG;
    return -1;
  }

  memcpy(dst + *offset, value, value_len);
  *offset += value_len;
  dst[*offset] = '\0';
  return 0;
}

static int substitute_job_step_id(
    const char *input,
    const char *job_id,
    const char *step_id,
    char *output,
    size_t output_size
) {
  size_t offset = 0;
  output[0] = '\0';

  for (size_t i = 0; input[i] != '\0'; ++i) {
    if (input[i] == '%' && input[i + 1] == 'j') {
      if (append_string(output, output_size, &offset, job_id) != 0) {
        return -1;
      }
      ++i;
      continue;
    }

    if (input[i] == '%' && input[i + 1] == 's') {
      if (append_string(output, output_size, &offset, step_id) != 0) {
        return -1;
      }
      ++i;
      continue;
    }

    if (input[i] == '%' && input[i + 1] == 'J') {
      if (append_string(output, output_size, &offset, job_id) != 0 ||
          append_string(output, output_size, &offset, ".") != 0 ||
          append_string(output, output_size, &offset, step_id) != 0) {
        return -1;
      }
      ++i;
      continue;
    }

    if (offset + 1 >= output_size) {
      errno = ENAMETOOLONG;
      return -1;
    }

    output[offset++] = input[i];
    output[offset] = '\0';
  }

  return 0;
}

static int with_lock(const char *path, int (*fn)(void *), void *arg) {
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
  if (ensure_mode(lock_path) != 0) {
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

static int create_mount_file_locked(void *arg) {
  mount_file_context_t *ctx = (mount_file_context_t *) arg;

  struct stat st;
  if (stat(ctx->path, &st) == 0) {
    snccliprecon_log_debug2("create_mount_file: %s already exists", ctx->path);
    return ensure_mode(ctx->path);
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

  return ensure_mode(ctx->path);
}

static int create_mount_file(uint32_t job_id, uint32_t step_id, const char *profiler_plugin, const char *dump_dir) {
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

  return with_lock(path, create_mount_file_locked, &ctx);
}

static int create_once_per_worker_lock(uint32_t job_id, uint32_t step_id) {
  char hostname[256];
  if (gethostname(hostname, sizeof(hostname)) != 0 || hostname[0] == '\0') {
    (void) snprintf(hostname, sizeof(hostname), "%s", "unknown");
  }
  hostname[sizeof(hostname) - 1] = '\0';

  char lock_path[SNCCLIPRECON_PATH_MAX];
  int written = snprintf(
      lock_path,
      sizeof(lock_path),
      "%s/%s.%u.%u.%s.lock",
      SNCCLIPRECON_TMP_DIR_BASE,
      hostname,
      job_id,
      step_id,
      SNCCLIPRECON_TASK_INIT_OP
  );
  if (written < 0 || (size_t) written >= sizeof(lock_path)) {
    errno = ENAMETOOLONG;
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
  if (ensure_mode(lock_path) != 0) {
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

int snccliprecon_task_init_privileged(spank_t spank) {
  const snccliprecon_config_t *config = snccliprecon_config_get();
  if (!config->enabled) {
    return ESPANK_SUCCESS;
  }

  uint32_t job_id = 0;
  uint32_t step_id = 0;
  (void) snccliprecon_get_job_id(spank, &job_id);
  (void) snccliprecon_get_step_id(spank, &step_id);
  snccliprecon_log_debug2("job=%u step=%u", job_id, step_id);

  if (step_id == SLURM_BATCH_SCRIPT) {
    snccliprecon_log_debug2("skipping batch script step");
    return ESPANK_SUCCESS;
  }

  char job_id_str[16];
  char step_id_str[16];
  (void) snprintf(job_id_str, sizeof(job_id_str), "%u", job_id);
  (void) snprintf(step_id_str, sizeof(step_id_str), "%u", step_id);

  char dump_dir[SNCCLIPRECON_PATH_MAX];
  char set_by_plugin[8];
  if (get_env(spank, SNCCLIPRECON_LOG_DIR_SET_BY_PLUGIN, set_by_plugin, sizeof(set_by_plugin)) &&
      strcmp(set_by_plugin, "1") == 0) {
    if (substitute_job_step_id(config->dump_dir, job_id_str, step_id_str, dump_dir, sizeof(dump_dir)) != 0) {
      snccliprecon_log_errorf("Cannot render NCCL Inspector dump dir: %s", strerror(errno));
      return ESPANK_ERROR;
    }

    snccliprecon_log_debug2("setting step dump dir %s", dump_dir);
    if (spank_setenv(spank, "NCCL_INSPECTOR_DUMP_DIR", dump_dir, 1) != ESPANK_SUCCESS) {
      snccliprecon_log_error("Cannot set NCCL_INSPECTOR_DUMP_DIR");
      return ESPANK_ERROR;
    }
  }

  snccliprecon_log_debug2("ensuring worker lock dir %s", SNCCLIPRECON_TMP_DIR_BASE);
  if (ensure_dir(SNCCLIPRECON_TMP_DIR_BASE) != 0) {
    snccliprecon_log_errorf("Cannot ensure %s: %s", SNCCLIPRECON_TMP_DIR_BASE, strerror(errno));
    return ESPANK_ERROR;
  }

  int once_rc = create_once_per_worker_lock(job_id, step_id);
  if (once_rc > 0) {
    snccliprecon_log_debug2("already ran for this worker step");
    return ESPANK_SUCCESS;
  }
  if (once_rc < 0) {
    snccliprecon_log_errorf("Cannot create worker lock: %s", strerror(errno));
    return ESPANK_ERROR;
  }

  if (get_env(spank, "NCCL_INSPECTOR_DUMP_DIR", dump_dir, sizeof(dump_dir)) && dump_dir[0] != '\0') {
    snccliprecon_log_debug("ensuring dump dir %s", dump_dir);
    if (ensure_dir(dump_dir) != 0) {
      snccliprecon_log_errorf("Cannot ensure NCCL Inspector dump dir %s: %s", dump_dir, strerror(errno));
      return ESPANK_ERROR;
    }
  } else {
    snccliprecon_log_debug("NCCL_INSPECTOR_DUMP_DIR is absent - skipping ensuring its existence");
  }

  if (!dir_exists(SNCCLIPRECON_ENROOT_MOUNTS_DIR)) {
    snccliprecon_log_debug("%s does not exist - skipping Enroot mount file creation", SNCCLIPRECON_ENROOT_MOUNTS_DIR);
    return ESPANK_SUCCESS;
  }

  char profiler_plugin[SNCCLIPRECON_PATH_MAX];
  if (!get_env(spank, "NCCL_PROFILER_PLUGIN", profiler_plugin, sizeof(profiler_plugin)) ||
      profiler_plugin[0] == '\0') {
    (void) snprintf(profiler_plugin, sizeof(profiler_plugin), "%s", config->profiler_plugin);
    snccliprecon_log_debug2("using configured profiler plugin %s", profiler_plugin);
  } else {
    snccliprecon_log_debug2("using env profiler plugin %s", profiler_plugin);
  }

  if (!get_env(spank, "NCCL_INSPECTOR_DUMP_DIR", dump_dir, sizeof(dump_dir)) || dump_dir[0] == '\0') {
    snccliprecon_log_debug("NCCL_INSPECTOR_DUMP_DIR is absent - skipping Enroot mount file creation");
    return ESPANK_SUCCESS;
  }

  snccliprecon_log_debug("creating Enroot mount file");
  if (create_mount_file(job_id, step_id, profiler_plugin, dump_dir) != 0) {
    snccliprecon_log_errorf("Cannot create Enroot mount file: %s", strerror(errno));
    return ESPANK_ERROR;
  }

  return ESPANK_SUCCESS;
}
