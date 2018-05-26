// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

#include <errno.h>
#include <stdlib.h>

struct dr_io_handle dr_stdin;
struct dr_io_handle_wo_buf dr_stdout;
static uint8_t dr_stdout_buf[1<<12];
struct dr_io_handle_wo_buf dr_stderr;
static uint8_t dr_stderr_buf[1<<12];

static void dr_flush_console(void) {
  {
    const struct dr_io_handle_wo_buf_vtbl *restrict const vtbl = container_of_const(dr_stdout.ih.io.vtbl, const struct dr_io_handle_wo_buf_vtbl, io);
    const struct dr_result_void r = vtbl->flush(&dr_stdout);
    (void)r;
  }
  {
    const struct dr_io_handle_wo_buf_vtbl *restrict const vtbl = container_of_const(dr_stderr.ih.io.vtbl, const struct dr_io_handle_wo_buf_vtbl, io);
    const struct dr_result_void r = vtbl->flush(&dr_stderr);
    (void)r;
  }
}

#if defined(_WIN32)

#include <windows.h>

struct dr_result_void dr_console_startup(void) {
  HANDLE handle;

  handle = GetStdHandle(STD_INPUT_HANDLE);
  if (dr_unlikely(handle == INVALID_HANDLE_VALUE)) {
    return DR_RESULT_GETLASTERROR_VOID();
  }
  dr_io_handle_init(&dr_stdin, (dr_handle_t)handle);

  handle = GetStdHandle(STD_OUTPUT_HANDLE);
  if (dr_unlikely(handle == INVALID_HANDLE_VALUE)) {
    return DR_RESULT_GETLASTERROR_VOID();
  }
  dr_io_handle_wo_fixed_init(&dr_stdout, (dr_handle_t)handle, dr_stdout_buf, sizeof(dr_stdout_buf));

  handle = GetStdHandle(STD_ERROR_HANDLE);
  if (dr_unlikely(handle == INVALID_HANDLE_VALUE)) {
    return DR_RESULT_GETLASTERROR_VOID();
  }
  dr_io_handle_wo_fixed_init(&dr_stderr, (dr_handle_t)handle, dr_stderr_buf, sizeof(dr_stderr_buf));

  if (atexit(dr_flush_console) != 0) {
    return DR_RESULT_ERRNO_VOID();
  }

  return DR_RESULT_OK_VOID();
}

#else

#include <unistd.h>

struct dr_result_void dr_console_startup(void) {
  dr_io_handle_init(&dr_stdin, STDIN_FILENO);
  dr_io_handle_wo_fixed_init(&dr_stdout, STDOUT_FILENO, dr_stdout_buf, sizeof(dr_stdout_buf));
  dr_io_handle_wo_fixed_init(&dr_stderr, STDERR_FILENO, dr_stderr_buf, sizeof(dr_stderr_buf));
  if (atexit(dr_flush_console) != 0) {
    return DR_RESULT_ERRNO_VOID();
  }
  return DR_RESULT_OK_VOID();
}

#endif
