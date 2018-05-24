// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"
#include "dr_io_internal.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

DR_WARN_UNUSED_RESULT static struct dr_result_size dr_io_ro_fixed_read(struct dr_io *restrict const io, void *restrict const buf, size_t count) {
  struct dr_io_ro_fixed *restrict const ro_fixed = container_of(io, struct dr_io_ro_fixed, io);
  if (ro_fixed->pos >= ro_fixed->count) {
    return DR_RESULT_OK(size, 0);
  }
  const size_t bytes = dr_min_size(count, ro_fixed->count - ro_fixed->pos);
  memcpy(buf, ro_fixed->buf + ro_fixed->pos, bytes);
  ro_fixed->pos += bytes;
  return DR_RESULT_OK(size, bytes);
}

static const struct dr_io_vtbl dr_io_ro_fixed_vtbl = {
  .read = dr_io_ro_fixed_read,
  .write = dr_io_enosys_write,
  .close = dr_io_noop_close,
};

void dr_io_ro_fixed_init(struct dr_io_ro_fixed *restrict const ro_fixed, const void *restrict const buf, size_t count) {
  *ro_fixed = (struct dr_io_ro_fixed) {
    .io.vtbl = &dr_io_ro_fixed_vtbl,
    .count = count,
    .buf = (const uint8_t *)buf,
  };
}

DR_WARN_UNUSED_RESULT static struct dr_result_size dr_io_wo_fixed_write(struct dr_io *restrict const io, const void *restrict const buf, size_t count) {
  struct dr_io_wo *restrict const wo_fixed = container_of(io, struct dr_io_wo, io);
  if (wo_fixed->pos >= wo_fixed->count) {
    return DR_RESULT_OK(size, 0);
  }
  const size_t bytes = dr_min_size(count, wo_fixed->count - wo_fixed->pos);
  memcpy(wo_fixed->buf + wo_fixed->pos, buf, bytes);
  wo_fixed->pos += bytes;
  return DR_RESULT_OK(size, bytes);
}

static const struct dr_io_vtbl dr_io_wo_fixed_vtbl = {
  .read = dr_io_enosys_read,
  .write = dr_io_wo_fixed_write,
  .close = dr_io_noop_close,
};

void dr_io_wo_fixed_init(struct dr_io_wo *restrict const wo_fixed, void *restrict const buf, size_t count) {
  *wo_fixed = (struct dr_io_wo) {
    .io.vtbl = &dr_io_wo_fixed_vtbl,
    .count = count,
    .buf = (uint8_t *)buf,
  };
}

DR_WARN_UNUSED_RESULT static struct dr_result_size dr_io_wo_resize_write(struct dr_io *restrict const io, const void *restrict const buf, size_t count) {
  struct dr_io_wo *restrict const wo_resize = container_of(io, struct dr_io_wo, io);
  const size_t new_pos = wo_resize->pos + count;
  if (dr_unlikely(new_pos > wo_resize->count)) {
    const size_t c = 2*new_pos;
    uint8_t *restrict const b = (uint8_t *)realloc(wo_resize->buf, c);
    if (dr_unlikely(b == NULL)) {
      return DR_RESULT_ERRNO(size);
    }
    wo_resize->count = c;
    wo_resize->buf = b;
  }
  memcpy(wo_resize->buf + wo_resize->pos, buf, count);
  wo_resize->pos = new_pos;
  return DR_RESULT_OK(size, count);
}

static void dr_io_wo_resize_close(struct dr_io *restrict const io) {
  struct dr_io_wo *restrict const wo_resize = container_of(io, struct dr_io_wo, io);
  free(wo_resize->buf);
}

static const struct dr_io_vtbl dr_io_wo_resize_vtbl = {
  .read = dr_io_enosys_read,
  .write = dr_io_wo_resize_write,
  .close = dr_io_wo_resize_close,
};

struct dr_result_void dr_io_wo_resize_init(struct dr_io_wo *restrict const wo_resize, size_t count) {
  uint8_t *restrict const buf = (uint8_t *)malloc(count);
  if (dr_unlikely(buf == NULL)) {
    return DR_RESULT_ERRNO_VOID();
  }
  *wo_resize = (struct dr_io_wo) {
    .io.vtbl = &dr_io_wo_resize_vtbl,
    .count = count,
    .buf = buf,
  };
  return DR_RESULT_OK_VOID();
}

static void dr_io_rw_validate(struct dr_io_rw *restrict const rw) {
  dr_assert(rw->read_pos <= rw->write_pos);
  dr_assert(rw->read_pos < rw->count);
  dr_assert(rw->write_pos - rw->read_pos <= rw->count);
}

DR_WARN_UNUSED_RESULT static struct dr_result_size dr_io_rw_read(struct dr_io *restrict const io, void *restrict const buf, size_t count) {
  struct dr_io_rw *restrict const rw = container_of(io, struct dr_io_rw, io);
  dr_io_rw_validate(rw);
  if (dr_unlikely(rw->read_pos >= rw->write_pos)) {
    return DR_RESULT_ERRNUM(size, DR_ERR_ISO_C, EAGAIN);
  }
  count = dr_min_size(count, rw->write_pos - rw->read_pos);
  const size_t new_read_pos = rw->read_pos + count;
  if (new_read_pos <= rw->count) {
    memcpy(buf, rw->buf + rw->read_pos, count);
  } else {
    memcpy(buf, rw->buf + rw->read_pos, rw->count - rw->read_pos);
    memcpy(((uint8_t *)buf) + rw->count - rw->read_pos, rw->buf, new_read_pos - rw->count);
  }
  rw->read_pos = new_read_pos;
  if (rw->read_pos >= rw->count) {
    rw->read_pos -= rw->count;
    rw->write_pos -= rw->count;
  }
  dr_io_rw_validate(rw);
  return DR_RESULT_OK(size, count);
}

DR_WARN_UNUSED_RESULT static struct dr_result_size dr_io_rw_fixed_write(struct dr_io *restrict const io, const void *restrict const buf, size_t count) {
  struct dr_io_rw *restrict const rw_fixed = container_of(io, struct dr_io_rw, io);
  dr_io_rw_validate(rw_fixed);
  if (dr_unlikely(rw_fixed->write_pos - rw_fixed->read_pos >= rw_fixed->count)) {
    return DR_RESULT_ERRNUM(size, DR_ERR_ISO_C, EAGAIN);
  }
  count = dr_min_size(count, rw_fixed->count + rw_fixed->read_pos - rw_fixed->write_pos);
  const size_t new_write_pos = rw_fixed->write_pos + count;
  size_t begin = rw_fixed->write_pos;
  size_t end = new_write_pos;
  if (begin >= rw_fixed->count) {
    begin -= rw_fixed->count;
    end -= rw_fixed->count;
  }
  if (end <= rw_fixed->count) {
    memcpy(rw_fixed->buf + begin, buf, count);
  } else {
    memcpy(rw_fixed->buf + begin, buf, rw_fixed->count - begin);
    memcpy(rw_fixed->buf, ((const uint8_t *)buf) + rw_fixed->count - begin, end - rw_fixed->count);
  }
  rw_fixed->write_pos = new_write_pos;
  dr_io_rw_validate(rw_fixed);
  return DR_RESULT_OK(size, count);
}

static const struct dr_io_vtbl dr_io_rw_fixed_vtbl = {
  .read = dr_io_rw_read,
  .write = dr_io_rw_fixed_write,
  .close = dr_io_noop_close,
};

void dr_io_rw_fixed_init(struct dr_io_rw *restrict const rw_fixed, void *restrict const buf, size_t count) {
  *rw_fixed = (struct dr_io_rw) {
    .io.vtbl = &dr_io_rw_fixed_vtbl,
    .count = count,
    .buf = (uint8_t *)buf,
  };
}

static void dr_io_rw_resize_resize(struct dr_io_rw *restrict const rw_resize, const size_t c, uint8_t *restrict const buf) {
  const size_t count = rw_resize->write_pos - rw_resize->read_pos;
  const size_t new_read_pos = rw_resize->read_pos + count;
  if (new_read_pos <= rw_resize->count) {
    memcpy(buf, rw_resize->buf + rw_resize->read_pos, count);
  } else {
    memcpy(buf, rw_resize->buf + rw_resize->read_pos, rw_resize->count - rw_resize->read_pos);
    memcpy(((uint8_t *)buf) + rw_resize->count - rw_resize->read_pos, rw_resize->buf, new_read_pos - rw_resize->count);
  }
  free(rw_resize->buf);
  rw_resize->count = c;
  rw_resize->read_pos = 0;
  rw_resize->write_pos = count;
  rw_resize->buf = buf;
}

DR_WARN_UNUSED_RESULT static struct dr_result_size dr_io_rw_resize_write(struct dr_io *restrict const io, const void *restrict const buf, size_t count) {
  struct dr_io_rw *restrict const rw_resize = container_of(io, struct dr_io_rw, io);
  dr_io_rw_validate(rw_resize);
  if (dr_unlikely(rw_resize->write_pos - rw_resize->read_pos + count > rw_resize->count)) {
    const size_t c = 2*(rw_resize->write_pos - rw_resize->read_pos + count);
    uint8_t *restrict const b = (uint8_t *)malloc(c);
    if (dr_unlikely(b == NULL)) {
      return DR_RESULT_ERRNO(size);
    }
    dr_io_rw_resize_resize(rw_resize, c, b);
    dr_io_rw_validate(rw_resize);
  }
  const size_t new_write_pos = rw_resize->write_pos + count;
  size_t begin = rw_resize->write_pos;
  size_t end = new_write_pos;
  if (begin >= rw_resize->count) {
    begin -= rw_resize->count;
    end -= rw_resize->count;
  }
  if (end <= rw_resize->count) {
    memcpy(rw_resize->buf + begin, buf, count);
  } else {
    memcpy(rw_resize->buf + begin, buf, rw_resize->count - begin);
    memcpy(rw_resize->buf, ((const uint8_t *)buf) + rw_resize->count - begin, end - rw_resize->count);
  }
  rw_resize->write_pos = new_write_pos;
  dr_io_rw_validate(rw_resize);
  return DR_RESULT_OK(size, count);
}

static void dr_io_rw_resize_close(struct dr_io *restrict const io) {
  struct dr_io_rw *restrict const rw_resize = container_of(io, struct dr_io_rw, io);
  free(rw_resize->buf);
}

static struct dr_io_vtbl dr_io_rw_resize_vtbl = {
  .read = dr_io_rw_read,
  .write = dr_io_rw_resize_write,
  .close = dr_io_rw_resize_close,
};

struct dr_result_void dr_io_rw_resize_init(struct dr_io_rw *restrict const rw_resize, size_t count) {
  uint8_t *restrict const buf = (uint8_t *)malloc(count);
  if (dr_unlikely(buf == NULL)) {
    return DR_RESULT_ERRNO_VOID();
  }
  *rw_resize = (struct dr_io_rw) {
    .io.vtbl = &dr_io_rw_resize_vtbl,
    .count = count,
    .buf = buf,
  };
  return DR_RESULT_OK_VOID();
}
