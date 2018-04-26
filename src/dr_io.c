// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"
#include "dr_io_internal.h"

#if defined(_WIN32)

#include <windows.h>

struct dr_result_size dr_io_handle_read_ol(struct dr_io_handle *restrict const ih, void *restrict const buf, size_t count, dr_overlapped_t *restrict const ol) {
  if (dr_unlikely(count > 0xffffffff)) {
    count = 0xffffffff;
  }
  DWORD result;
  dr_assert(sizeof(dr_overlapped_t) == sizeof(OVERLAPPED));
  if (dr_unlikely(ReadFile((HANDLE)ih->fd, buf, count, &result, (OVERLAPPED *)ol) == 0)) {
    return DR_RESULT_GETLASTERROR(size);
  }
  return DR_RESULT_OK(size, result);
}

DR_WARN_UNUSED_RESULT static struct dr_result_size dr_io_handle_read(struct dr_io *restrict const io, void *restrict const buf, size_t count) {
  struct dr_io_handle *restrict const ih = container_of(io, struct dr_io_handle, io);
  return dr_io_handle_read_ol(ih, buf, count, NULL);
}

struct dr_result_size dr_io_handle_write_ol(struct dr_io_handle *restrict const ih, const void *restrict const buf, size_t count, dr_overlapped_t *restrict const ol) {
  if (dr_unlikely(count > 0xffffffff)) {
    count = 0xffffffff;
  }
  DWORD result;
  dr_assert(sizeof(dr_overlapped_t) == sizeof(OVERLAPPED));
  if (dr_unlikely(WriteFile((HANDLE)ih->fd, buf, count, &result, (OVERLAPPED *)ol) == 0)) {
    return DR_RESULT_GETLASTERROR(size);
  }
  return DR_RESULT_OK(size, result);
}

DR_WARN_UNUSED_RESULT static struct dr_result_size dr_io_handle_write(struct dr_io *restrict const io, const void *restrict const buf, size_t count) {
  struct dr_io_handle *restrict const ih = container_of(io, struct dr_io_handle, io);
  return dr_io_handle_write_ol(ih, buf, count, NULL);
}

void dr_close(dr_handle_t fd) {
  CloseHandle((HANDLE)fd);
}

void dr_io_handle_close(struct dr_io *restrict const io) {
  struct dr_io_handle *restrict const ih = container_of(io, struct dr_io_handle, io);
  CloseHandle((HANDLE)ih->fd);
}

void dr_ioserver_handle_close(struct dr_ioserver *restrict const ioserver) {
  struct dr_ioserver_handle *restrict const ihserver = container_of(ioserver, struct dr_ioserver_handle, ioserver);
  CloseHandle((HANDLE)ihserver->fd);
}

#else

#include <errno.h>
#include <unistd.h>

struct dr_result_size dr_io_handle_read(struct dr_io *restrict const io, void *restrict const buf, size_t count) {
  struct dr_io_handle *restrict const ih = container_of(io, struct dr_io_handle, io);
  const ssize_t result = read(ih->fd, buf, count);
  if (dr_unlikely(result < 0)) {
    return DR_RESULT_ERRNO(size);
  }
  return DR_RESULT_OK(size, result);
}

struct dr_result_size dr_io_handle_write(struct dr_io *restrict const io, const void *restrict const buf, size_t count) {
  struct dr_io_handle *restrict const ih = container_of(io, struct dr_io_handle, io);
  const ssize_t result = write(ih->fd, buf, count);
  if (dr_unlikely(result < 0)) {
    return DR_RESULT_ERRNO(size);
  }
  return DR_RESULT_OK(size, result);
}

void dr_close(dr_handle_t fd) {
  close(fd);
}

void dr_io_handle_close(struct dr_io *restrict const io) {
  struct dr_io_handle *restrict const ih = container_of(io, struct dr_io_handle, io);
  close(ih->fd);
}

void dr_ioserver_handle_close(struct dr_ioserver *restrict const ioserver) {
  struct dr_ioserver_handle *restrict const ihserver = container_of(ioserver, struct dr_ioserver_handle, ioserver);
  close(ihserver->fd);
}

#endif

static const struct dr_io_vtbl dr_io_handle_vtbl = {
  .read = dr_io_handle_read,
  .write = dr_io_handle_write,
  .close = dr_io_handle_close,
};

void dr_io_handle_init(struct dr_io_handle *restrict const ih, dr_handle_t fd) {
  *ih = (struct dr_io_handle) {
    .io.vtbl = &dr_io_handle_vtbl,
    .fd = fd,
  };
}
