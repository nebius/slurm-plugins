#include "spank_substitute.h"

#include <errno.h>
#include <stdbool.h>
#include <string.h>

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

static int substitute(
  const char *input,
  const char *job_id,
  const char *step_id,
  bool substitute_step,
  char *output,
  size_t output_size
) {
  if (output_size == 0) {
    errno = EINVAL;
    return -1;
  }

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

    if (substitute_step && input[i] == '%' && input[i + 1] == 's') {
      if (append_string(output, output_size, &offset, step_id) != 0) {
        return -1;
      }
      ++i;
      continue;
    }

    if (substitute_step && input[i] == '%' && input[i + 1] == 'J') {
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

int snccliprecon_substitute_job_id(
  const char *input,
  const char *job_id,
  char *output,
  size_t output_size
) {
  return substitute(input, job_id, "", false, output, output_size);
}

int snccliprecon_substitute_job_step_id(
  const char *input,
  const char *job_id,
  const char *step_id,
  char *output,
  size_t output_size
) {
  return substitute(input, job_id, step_id, true, output, output_size);
}
