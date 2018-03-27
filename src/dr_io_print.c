// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

#include <string.h>

struct dr_result_size dr_fputs(const char *restrict const s, struct dr_io *restrict const io) {
  return dr_write_all(io, s, strlen(s));
}

struct dr_result_size dr_puts(const char *restrict const s) {
  return dr_fputs(s, &dr_stdout.io);
}

struct dr_result_size dr_fprintf(struct dr_io *restrict const io, const char *restrict const fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  const struct dr_result_size r = dr_vfprintf(io, fmt, ap);
  va_end(ap);
  return r;
}

struct dr_result_size dr_printf(const char *restrict const fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  const struct dr_result_size r = dr_vfprintf(&dr_stdout.io, fmt, ap);
  va_end(ap);
  return r;
}

struct dr_result_size dr_vsnprintf(char *restrict const s, size_t n, const char *restrict const fmt, va_list ap) {
  struct dr_io_wo io;
  dr_io_wo_fixed_init(&io, s, n);
  const struct dr_result_size r = dr_vfprintf(&io.io, fmt, ap);
  DR_IF_RESULT_OK(size_t, r, value) {
    s[dr_min_size(io.count - 1, value)] = '\0';
  } DR_FI_RESULT;
  io.io.vtbl->close(&io.io);
  return r;
}

struct dr_result_size dr_snprintf(char *restrict const s, size_t n, const char *restrict const fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  const struct dr_result_size r = dr_vsnprintf(s, n, fmt, ap);
  va_end(ap);
  return r;
}
