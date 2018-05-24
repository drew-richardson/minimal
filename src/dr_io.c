// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"
#include "dr_io_internal.h"

#include <errno.h>
#include <string.h>

struct dr_result_size dr_io_enosys_read(struct dr_io *restrict const io, void *restrict const buf, size_t count) {
  (void)io;
  (void)buf;
  (void)count;
  return DR_RESULT_ERRNUM(size, DR_ERR_ISO_C, ENOSYS);
}

struct dr_result_size dr_io_enosys_write(struct dr_io *restrict const io, const void *restrict const buf, size_t count) {
  (void)io;
  (void)buf;
  (void)count;
  return DR_RESULT_ERRNUM(size, DR_ERR_ISO_C, ENOSYS);
}

void dr_io_noop_close(struct dr_io *restrict const io) {
  // Do nothing
  (void)io;
}

struct dr_result_size dr_write_all_fn(struct dr_io *restrict const io, dr_io_write_fn_t write, const void *restrict const buf, size_t count) {
  const uint8_t *restrict const s = (const uint8_t *)buf;
  size_t pos = 0;
  while (pos < count) {
    const struct dr_result_size r = write(io, s, count);
    DR_IF_RESULT_ERR(r, err) {
      return DR_RESULT_ERROR(size, err);
    } DR_ELIF_RESULT_OK(size_t, r, value) {
      if (value == 0) {
	break;
      }
      pos += value;
    } DR_FI_RESULT;
  }
  return DR_RESULT_OK(size, pos);
}

struct dr_result_size dr_write_all(struct dr_io *restrict const io, const void *restrict const buf, size_t count) {
  return dr_write_all_fn(io, io->vtbl->write, buf, count);
}

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

DR_WARN_UNUSED_RESULT static struct dr_result_size dr_io_handle_wo_fixed_write(struct dr_io *restrict const io, const void *restrict const buf, size_t count) {
  struct dr_io_handle_wo_buf *restrict const ih_wo_fixed = container_of(io, struct dr_io_handle_wo_buf, ih.io);
  if (count == 0) {
    // Do nothing
    return DR_RESULT_OK(size, count);
  }
  if (ih_wo_fixed->pos + count < ih_wo_fixed->count && ((const char *)buf)[count - 1] != '\n') {
    memcpy(ih_wo_fixed->buf + ih_wo_fixed->pos, buf, count);
    ih_wo_fixed->pos += count;
    return DR_RESULT_OK(size, count);
  }
  {
    struct dr_result_size r = dr_write_all_fn(io, dr_io_handle_write, ih_wo_fixed->buf, ih_wo_fixed->pos);
    DR_IF_RESULT_ERR(r, err) {
      return DR_RESULT_ERROR(size, err);
    } DR_ELIF_RESULT_OK(size_t, r, value) {
      dr_assert(value == ih_wo_fixed->pos); // DR Handle this better
      ih_wo_fixed->pos = 0;
    } DR_FI_RESULT;
  }
  {
    struct dr_result_size r = dr_write_all_fn(io, dr_io_handle_write, buf, count);
    DR_IF_RESULT_ERR(r, err) {
      return DR_RESULT_ERROR(size, err);
    } DR_ELIF_RESULT_OK(size_t, r, value) {
      dr_assert(value == count); // DR Handle this better
    } DR_FI_RESULT;
  }
  return DR_RESULT_OK(size, count);
}

DR_WARN_UNUSED_RESULT static struct dr_result_void dr_io_handle_wo_fixed_flush(struct dr_io_handle_wo_buf *restrict const ih_wo_fixed) {
  {
    struct dr_result_size r = dr_write_all_fn(&ih_wo_fixed->ih.io, dr_io_handle_write, ih_wo_fixed->buf, ih_wo_fixed->pos);
    DR_IF_RESULT_ERR(r, err) {
      return DR_RESULT_ERROR_VOID(err);
    } DR_ELIF_RESULT_OK(size_t, r, value) {
      dr_assert(value == ih_wo_fixed->pos); // DR Handle this better
      ih_wo_fixed->pos = 0;
    } DR_FI_RESULT;
  }
  return DR_RESULT_OK_VOID();
}

static const struct dr_io_handle_wo_buf_vtbl dr_io_handle_wo_fixed_vtbl = {
  .io.read = dr_io_enosys_read,
  .io.write = dr_io_handle_wo_fixed_write,
  .io.close = dr_io_handle_close,
  .flush = dr_io_handle_wo_fixed_flush,
};

void dr_io_handle_wo_fixed_init(struct dr_io_handle_wo_buf *restrict const ih_wo, dr_handle_t fd, void *restrict const buf, size_t count) {
  *ih_wo = (struct dr_io_handle_wo_buf) {
    .ih.io.vtbl = &dr_io_handle_wo_fixed_vtbl.io,
    .ih.fd = fd,
    .count = count,
    .buf = (uint8_t *)buf,
  };
}
