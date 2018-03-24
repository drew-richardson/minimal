// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

#include <string.h>

bool dr_str_eq(const struct dr_str *restrict const a, const struct dr_str *restrict const b) {
  return a->len == b->len && memcmp(a->buf, b->buf, a->len) == 0;
}
