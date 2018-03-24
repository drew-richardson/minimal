// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

#if defined(_WIN32)

#include <windows.h>

struct dr_result_size dr_read_ol(dr_handle_t fd, void *restrict const buf, size_t count, struct _OVERLAPPED *restrict const ol) {
  if (dr_unlikely(count > 0xffffffff)) {
    count = 0xffffffff;
  }
  DWORD result;
  if (dr_unlikely(ReadFile((HANDLE)fd, buf, count, &result, ol) == 0)) {
    return DR_RESULT_GETLASTERROR(size);
  }
  return DR_RESULT_OK(size, result);
}

struct dr_result_size dr_read(dr_handle_t fd, void *restrict const buf, size_t count) {
  return dr_read_ol(fd, buf, count, NULL);
}

struct dr_result_size dr_write_ol(dr_handle_t fd, const void *restrict const buf, size_t count, struct _OVERLAPPED *restrict const ol) {
  if (dr_unlikely(count > 0xffffffff)) {
    count = 0xffffffff;
  }
  DWORD result;
  if (dr_unlikely(WriteFile((HANDLE)fd, buf, count, &result, ol) == 0)) {
    return DR_RESULT_GETLASTERROR(size);
  }
  return DR_RESULT_OK(size, result);
}

struct dr_result_size dr_write(dr_handle_t fd, const void *restrict const buf, size_t count) {
  return dr_write_ol(fd, buf, count, NULL);
}

void dr_close(dr_handle_t fd) {
  CloseHandle((HANDLE)fd);
}

#else

#include <errno.h>
#include <unistd.h>

struct dr_result_size dr_read(dr_handle_t fd, void *restrict const buf, size_t count) {
  const ssize_t result = read(fd, buf, count);
  if (dr_unlikely(result < 0)) {
    return DR_RESULT_ERRNO(size);
  }
  return DR_RESULT_OK(size, result);
}

struct dr_result_size dr_write(dr_handle_t fd, const void *restrict const buf, size_t count) {
  const ssize_t result = write(fd, buf, count);
  if (dr_unlikely(result < 0)) {
    return DR_RESULT_ERRNO(size);
  }
  return DR_RESULT_OK(size, result);
}

void dr_close(dr_handle_t fd) {
  close(fd);
}

#endif
