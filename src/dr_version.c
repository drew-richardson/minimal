// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

#include <stdio.h>

int dr_get_version_short(char *restrict const str, const size_t size) {
  if (DR_VERSION_PATCH == 0) {
    return snprintf(str, size, "%u.%u%s", DR_VERSION_MAJOR, DR_VERSION_MINOR, DR_VERSION_EXTRA);
  } else {
    return snprintf(str, size, "%u.%u.%u%s", DR_VERSION_MAJOR, DR_VERSION_MINOR, DR_VERSION_PATCH, DR_VERSION_EXTRA);
  }
}

int dr_get_version_long(char *restrict const str, const size_t size) {
  int written = dr_get_version_short(str, size);
  if (dr_likely(written >= 0 && (size_t)written < size)) {
    written += snprintf(str + written, size - written, " %s-%s %s", dr_source_revision(), dr_source_status(), dr_source_date());
  }
  return written;
}
