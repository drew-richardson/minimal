// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

#include <stdio.h>
#include <string.h>

#if DR_VERSION_PATCH == 0
#define DR_VERSION_STR DR_VERSION_MAJOR_STR "." DR_VERSION_MINOR_STR DR_VERSION_EXTRA
#else
#define DR_VERSION_STR DR_VERSION_MAJOR_STR "." DR_VERSION_MINOR_STR "." DR_VERSION_PATCH_STR DR_VERSION_EXTRA
#endif

int dr_get_version_short(char *restrict const str, const size_t size) {
  memcpy(str, DR_VERSION_STR, dr_min_size(sizeof(DR_VERSION_STR) - 1, size));
  return sizeof(DR_VERSION_STR) - 1;
}

int dr_get_version_long(char *restrict const str, const size_t size) {
  struct dr_print p;
  return dr_print_finalize(
    dr_print_s(
      dr_print_c(
        dr_print_s(
          dr_print_c(
            dr_print_s(
              dr_print_c(
		dr_print_init_pos(&p, str, size, dr_get_version_short(str, size)),
              ' '),
            dr_source_revision()),
          '-'),
        dr_source_status()),
      ' '),
    dr_source_date())
  );
}
