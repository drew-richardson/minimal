// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

#include <string.h>

static const uint32_t header_size = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint16_t);
static const uint32_t qid_size = sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint64_t);

#define dr_encode_uintn(buf, val) \
  do { \
    for (size_t i = 0; i < sizeof(val); ++i) { \
      (buf)[i] = ((val) >> 8 * i) & 0xff; \
    } \
  } while (false)

void dr_encode_uint8(uint8_t *restrict const buf, const uint8_t val) {
  dr_encode_uintn(buf, val);
}

void dr_encode_uint16(uint8_t *restrict const buf, const uint16_t val) {
  dr_encode_uintn(buf, val);
}

void dr_encode_uint32(uint8_t *restrict const buf, const uint32_t val) {
  dr_encode_uintn(buf, val);
}

void dr_encode_uint64(uint8_t *restrict const buf, const uint64_t val) {
  dr_encode_uintn(buf, val);
}

static void dr_9p_encode_qid(uint8_t *restrict const buf, const struct dr_file *restrict const f) {
  dr_encode_uint8(buf, f->mode >> 24);
  dr_encode_uint32(buf + sizeof(uint8_t), f->vers);
  dr_encode_uint64(buf + sizeof(uint8_t) + sizeof(uint32_t), (uintptr_t)f); // DR xor file
}

WARN_UNUSED_RESULT static uint32_t dr_9p_encode_stat(uint8_t *restrict const buf, const uint32_t size, const struct dr_file *restrict const f) {
  const uint32_t ssize = sizeof(uint16_t) + sizeof(uint32_t) + qid_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint16_t) + f->name.len + sizeof(uint16_t) + f->uid->name.len + sizeof(uint16_t) + f->gid->name.len + sizeof(uint16_t) + f->muid->name.len;
  uint32_t spos = 0;
  if (dr_unlikely(size < spos + sizeof(uint16_t) + ssize)) {
    return FAIL_UINT32;
  }
  dr_encode_uint16(buf + spos, ssize);
  spos += sizeof(uint16_t);
  dr_encode_uint16(buf + spos, 0);
  spos += sizeof(uint16_t);
  dr_encode_uint32(buf + spos, 0);
  spos += sizeof(uint32_t);
  dr_9p_encode_qid(buf + spos, f);
  spos += qid_size;
  dr_encode_uint32(buf + spos, f->mode);
  spos += sizeof(uint32_t);
  dr_encode_uint32(buf + spos, f->atime/DR_NS_PER_S);
  spos += sizeof(uint32_t);
  dr_encode_uint32(buf + spos, f->mtime/DR_NS_PER_S);
  spos += sizeof(uint32_t);
  dr_encode_uint64(buf + spos, f->length);
  spos += sizeof(uint64_t);
  dr_encode_uint16(buf + spos, f->name.len);
  spos += sizeof(uint16_t);
  memcpy(buf + spos, f->name.buf, f->name.len);
  spos += f->name.len;
  dr_encode_uint16(buf + spos, f->uid->name.len);
  spos += sizeof(uint16_t);
  memcpy(buf + spos, f->uid->name.buf, f->uid->name.len);
  spos += f->uid->name.len;
  dr_encode_uint16(buf + spos, f->gid->name.len);
  spos += sizeof(uint16_t);
  memcpy(buf + spos, f->gid->name.buf, f->gid->name.len);
  spos += f->gid->name.len;
  dr_encode_uint16(buf + spos, f->muid->name.len);
  spos += sizeof(uint16_t);
  memcpy(buf + spos, f->muid->name.buf, f->muid->name.len);
  spos += f->muid->name.len;
  return spos;
}

static void dr_9p_encode_header(uint8_t *restrict const buf, const uint8_t type, const uint16_t tag) {
  dr_encode_uint8(buf + sizeof(uint32_t), type);
  dr_encode_uint16(buf + sizeof(uint32_t) + sizeof(uint8_t), tag);
}

static bool dr_9p_encode_finish(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  dr_encode_uint32(buf, size);
  *pos = size;
  return true;
}

// size[4] Tversion tag[2] msize[4] version[s]

WARN_UNUSED_RESULT static bool dr_9p_encode_uint32_str(const uint8_t type, uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t arg0, const struct dr_str *restrict const arg1) {
  const uint16_t arg1_len = arg1->len;
  if (dr_unlikely(size < header_size + sizeof(uint32_t) + sizeof(uint16_t) + arg1_len)) {
    return false;
  }
  dr_9p_encode_header(buf, type, tag);
  dr_encode_uint32(buf + header_size, arg0);
  dr_encode_uint16(buf + header_size + sizeof(uint32_t), arg1_len);
  memcpy(buf + header_size + sizeof(uint32_t) + sizeof(uint16_t), arg1->buf, arg1_len);
  return dr_9p_encode_finish(buf, header_size + sizeof(uint32_t) + sizeof(uint16_t) + arg1_len, pos);
}

bool dr_9p_encode_Tversion(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t msize, const struct dr_str *restrict const version) {
  return dr_9p_encode_uint32_str(DR_TVERSION, buf, size, pos, tag, msize, version);
}

// size[4] Rversion tag[2] msize[4] version[s]

bool dr_9p_encode_Rversion(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t msize, const struct dr_str *restrict const version) {
  return dr_9p_encode_uint32_str(DR_RVERSION, buf, size, pos, tag, msize, version);
}

// size[4] Tauth tag[2] afid[4] uname[s] aname[s]
// size[4] Rauth tag[2] aqid[13]

// size[4] Rerror tag[2] ename[s]

void dr_9p_encode_Rerror(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const struct dr_str *restrict const ename) {
  const uint32_t ename_pos = header_size + sizeof(uint16_t);
  dr_assert(size >= ename_pos);
  dr_9p_encode_header(buf, DR_RERROR, tag);
  const uint16_t available = size - ename_pos;
  const uint16_t writable = ename->len;
  const uint16_t written = writable > available ? available : writable;
  if (dr_likely(written > 0)) {
    memcpy(buf + ename_pos, ename->buf, written);
  }
  dr_encode_uint16(buf + header_size, written);
  dr_9p_encode_finish(buf, ename_pos + written, pos);
}

void dr_9p_encode_Rerror_err(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const struct dr_error *restrict const error) {
  const uint32_t ename_pos = header_size + sizeof(uint16_t);
  dr_assert(size >= ename_pos);
  dr_9p_encode_header(buf, DR_RERROR, tag);
  const uint16_t available = size - ename_pos;
  const uint16_t writable = dr_log_format((char *)(buf + ename_pos), available, error);
  const uint16_t written = writable > available ? available == 0 ? 0 : available - 1 /* '\0' */ : writable;
  dr_encode_uint16(buf + header_size, written);
  dr_9p_encode_finish(buf, ename_pos + written, pos);
}

// size[4] Tflush tag[2] oldtag[2]
// size[4] Rflush tag[2]

// size[4] Tattach tag[2] fid[4] afid[4] uname[s] aname[s]

bool dr_9p_encode_Tattach(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t fid, const uint32_t afid, const struct dr_str *restrict const uname, const struct dr_str *restrict const aname) {
  const uint16_t uname_len = uname->len;
  const uint16_t aname_len = aname->len;
  if (dr_unlikely(size < header_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t) + uname_len + sizeof(uint16_t) + aname_len)) {
    return false;
  }
  dr_9p_encode_header(buf, DR_TATTACH, tag);
  dr_encode_uint32(buf + header_size, fid);
  dr_encode_uint32(buf + header_size + sizeof(uint32_t), afid);
  dr_encode_uint16(buf + header_size + sizeof(uint32_t) + sizeof(uint32_t), uname_len);
  memcpy(buf + header_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t), uname->buf, uname_len);
  dr_encode_uint16(buf + header_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t) + uname_len, aname_len);
  memcpy(buf + header_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t) + uname_len + sizeof(uint16_t), aname->buf, aname_len);
  return dr_9p_encode_finish(buf, header_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t) + uname_len + sizeof(uint16_t) + aname_len, pos);
}

// size[4] Rattach tag[2] qid[13]

bool dr_9p_encode_Rattach(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const struct dr_file *restrict const f) {
  if (dr_unlikely(size < header_size + qid_size)) {
    return false;
  }
  dr_9p_encode_header(buf, DR_RATTACH, tag);
  dr_9p_encode_qid(buf + header_size, f);
  return dr_9p_encode_finish(buf, header_size + qid_size, pos);
}

// size[4] Twalk tag[2] fid[4] newfid[4] nwname[2] nwname*(wname[s])

bool dr_9p_encode_Twalk_iterator(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t fid, const uint32_t newfid, uint16_t *restrict const nwname) {
  if (dr_unlikely(size < header_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t))) {
    return false;
  }
  dr_9p_encode_header(buf, DR_TWALK, tag);
  dr_encode_uint32(buf + header_size, fid);
  dr_encode_uint32(buf + header_size + sizeof(uint32_t), newfid);
  *nwname = 0;
  *pos = header_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t);
  return true;
}

bool dr_9p_encode_Twalk_add(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, uint16_t *restrict const nwname, const struct dr_str *restrict const name) {
  const uint32_t p = *pos;
  const uint16_t name_len = name->len;
  if (dr_unlikely(size < p + sizeof(uint16_t) + name_len)) {
    return false;
  }
  dr_encode_uint16(buf + p, name_len);
  memcpy(buf + p + sizeof(uint16_t), name->buf, name_len);
  ++*nwname;
  *pos = p + sizeof(uint16_t) + name_len;
  return true;
}

bool dr_9p_encode_Twalk_finish(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t nwname) {
  const uint32_t p = *pos;
  if (dr_unlikely(size < p)) {
    return false;
  }
  dr_encode_uint16(buf + header_size + sizeof(uint32_t) + sizeof(uint32_t), nwname);
  return dr_9p_encode_finish(buf, p, pos);
}

// size[4] Rwalk tag[2] nwqid[2] nwqid*(wqid[13])

bool dr_9p_encode_Rwalk_iterator(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, uint16_t *restrict const nwqid) {
  if (dr_unlikely(size < header_size + sizeof(uint16_t))) {
    return false;
  }
  dr_9p_encode_header(buf, DR_RWALK, tag);
  *nwqid = 0;
  *pos = header_size + sizeof(uint16_t);
  return true;
}

bool dr_9p_encode_Rwalk_add(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, uint16_t *restrict const nwqid, const struct dr_file *restrict const f) {
  const uint32_t p = *pos;
  if (dr_unlikely(size < p + qid_size)) {
    return false;
  }
  dr_9p_encode_qid(buf + p, f);
  ++*nwqid;
  *pos += qid_size;
  return true;
}

bool dr_9p_encode_Rwalk_finish(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t nwqid) {
  const uint32_t p = *pos;
  if (dr_unlikely(size < p)) {
    return false;
  }
  dr_encode_uint16(buf + header_size, nwqid);
  return dr_9p_encode_finish(buf, p, pos);
}

// size[4] Topen tag[2] fid[4] mode[1]

bool dr_9p_encode_Topen(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t fid, const uint8_t mode) {
  if (dr_unlikely(size < header_size + sizeof(uint32_t) + sizeof(uint8_t))) {
    return false;
  }
  dr_9p_encode_header(buf, DR_TOPEN, tag);
  dr_encode_uint32(buf + header_size, fid);
  dr_encode_uint8(buf + header_size + sizeof(uint32_t), mode);
  return dr_9p_encode_finish(buf, header_size + sizeof(uint32_t) + sizeof(uint8_t), pos);
}

// The iounit field returned by open(9P), if non-zero, reports the maximum size that is guaranteed to be transferred atomically.

WARN_UNUSED_RESULT static bool dr_9p_encode_qid_uint32(const uint8_t type, uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const struct dr_file *restrict const arg0, const uint32_t arg1) {
  if (dr_unlikely(size < header_size + qid_size + sizeof(uint32_t))) {
    return false;
  }
  dr_9p_encode_header(buf, type, tag);
  dr_9p_encode_qid(buf + header_size, arg0);
  dr_encode_uint32(buf + header_size + qid_size, arg1);
  return dr_9p_encode_finish(buf, header_size + qid_size + sizeof(uint32_t), pos);
}

// size[4] Ropen tag[2] qid[13] iounit[4]

bool dr_9p_encode_Ropen(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const struct dr_file *restrict const f, const uint32_t iounit) {
  return dr_9p_encode_qid_uint32(DR_ROPEN, buf, size, pos, tag, f, iounit);
}

// size[4] Topenfd tag[2] fid[4] mode[1]
// size[4] Ropenfd tag[2] qid[13] iounit[4] unixfd[4]

// size[4] Tcreate tag[2] fid[4] name[s] perm[4] mode[1]

bool dr_9p_encode_Tcreate(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t fid, const struct dr_str *restrict const name, const uint32_t perm, const uint8_t mode) {
  const uint16_t name_len = name->len;
  if (dr_unlikely(size < header_size + sizeof(uint32_t) + sizeof(uint16_t) + name_len + sizeof(uint32_t) + sizeof(uint8_t))) {
    return false;
  }
  dr_9p_encode_header(buf, DR_TCREATE, tag);
  dr_encode_uint32(buf + header_size, fid);
  dr_encode_uint16(buf + header_size + sizeof(uint32_t), name_len);
  memcpy(buf + header_size + sizeof(uint32_t) + sizeof(uint16_t), name->buf, name_len);
  dr_encode_uint32(buf + header_size + sizeof(uint32_t) + sizeof(uint16_t) + name_len, perm);
  dr_encode_uint8(buf + header_size + sizeof(uint32_t) + sizeof(uint16_t) + name_len + sizeof(uint32_t), mode);
  return dr_9p_encode_finish(buf, header_size + sizeof(uint32_t) + sizeof(uint16_t) + name_len + sizeof(uint32_t) + sizeof(uint8_t), pos);
}

// size[4] Rcreate tag[2] qid[13] iounit[4]

bool dr_9p_encode_Rcreate(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const struct dr_file *restrict const f, const uint32_t iounit) {
  return dr_9p_encode_qid_uint32(DR_RCREATE, buf, size, pos, tag, f, iounit);
}

// size[4] Tread tag[2] fid[4] offset[8] count[4]

bool dr_9p_encode_Tread(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t fid, const uint64_t offset, const uint32_t count) {
  if (dr_unlikely(size < header_size + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint32_t))) {
    return false;
  }
  dr_9p_encode_header(buf, DR_TREAD, tag);
  dr_encode_uint32(buf + header_size, fid);
  dr_encode_uint64(buf + header_size + sizeof(uint32_t), offset);
  dr_encode_uint32(buf + header_size + sizeof(uint32_t) + sizeof(uint64_t), count);
  return dr_9p_encode_finish(buf, header_size + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint32_t), pos);
}

// size[4] Rread tag[2] count[4] data[count]

bool dr_9p_encode_Rread_iterator(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag) {
  if (dr_unlikely(size < header_size + sizeof(uint32_t))) {
    return false;
  }
  dr_9p_encode_header(buf, DR_RREAD, tag);
  *pos = header_size + sizeof(uint32_t);
  return true;
}

bool dr_9p_encode_Rread_finish(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint32_t count) {
  if (dr_unlikely(size < header_size + sizeof(uint32_t) + count)) {
    return false;
  }
  dr_encode_uint32(buf + header_size, count);
  return dr_9p_encode_finish(buf, header_size + sizeof(uint32_t) + count, pos);
}

// size[4] Twrite tag[2] fid[4] offset[8] count[4] data[count]

bool dr_9p_encode_Twrite_iterator(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t fid, const uint64_t offset) {
  if (dr_unlikely(size < header_size + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint32_t))) {
    return false;
  }
  dr_9p_encode_header(buf, DR_TWRITE, tag);
  dr_encode_uint32(buf + header_size, fid);
  dr_encode_uint64(buf + header_size + sizeof(uint32_t), offset);
  *pos = header_size + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint32_t);
  return true;
}

bool dr_9p_encode_Twrite_finish(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint32_t count) {
  if (dr_unlikely(size < header_size + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint32_t) + count)) {
    return false;
  }
  dr_encode_uint32(buf + header_size + sizeof(uint32_t) + sizeof(uint64_t), count);
  return dr_9p_encode_finish(buf, header_size + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint32_t) + count, pos);
}

// size[4] Rwrite tag[2] count[4]

WARN_UNUSED_RESULT static bool dr_9p_encode_uint32(const uint8_t type, uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t arg0) {
  if (dr_unlikely(size < header_size + sizeof(uint32_t))) {
    return false;
  }
  dr_9p_encode_header(buf, type, tag);
  dr_encode_uint32(buf + header_size, arg0);
  return dr_9p_encode_finish(buf, header_size + sizeof(uint32_t), pos);
}

bool dr_9p_encode_Rwrite(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t count) {
  return dr_9p_encode_uint32(DR_RWRITE, buf, size, pos, tag, count);
}

// size[4] Tclunk tag[2] fid[4]

bool dr_9p_encode_Tclunk(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t fid) {
  return dr_9p_encode_uint32(DR_TCLUNK, buf, size, pos, tag, fid);
}

// size[4] Rclunk tag[2]

WARN_UNUSED_RESULT static bool dr_9p_encode_null(const uint8_t type, uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag) {
  if (dr_unlikely(size < header_size)) {
    return false;
  }
  dr_9p_encode_header(buf, type, tag);
  return dr_9p_encode_finish(buf, header_size, pos);
}

bool dr_9p_encode_Rclunk(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag) {
  return dr_9p_encode_null(DR_RCLUNK, buf, size, pos, tag);
}

// size[4] Tremove tag[2] fid[4]

bool dr_9p_encode_Tremove(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t fid) {
  return dr_9p_encode_uint32(DR_TREMOVE, buf, size, pos, tag, fid);
}

// size[4] Rremove tag[2]

bool dr_9p_encode_Rremove(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag) {
  return dr_9p_encode_null(DR_RREMOVE, buf, size, pos, tag);
}

// size[4] Tstat tag[2] fid[4]

bool dr_9p_encode_Tstat(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t fid) {
  return dr_9p_encode_uint32(DR_TSTAT, buf, size, pos, tag, fid);
}

// size[4] Rstat tag[2] stat[n]

bool dr_9p_encode_Rstat(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const struct dr_file *restrict const f) {
  if (dr_unlikely(size < header_size + sizeof(uint16_t))) {
    return false;
  }
  dr_9p_encode_header(buf, DR_RSTAT, tag);
  const uint32_t written = dr_9p_encode_stat(buf + header_size + sizeof(uint16_t), size - header_size - sizeof(uint16_t), f);
  if (dr_unlikely(written == FAIL_UINT32)) {
    return false;
  }
  dr_encode_uint16(buf + header_size, written);
  return dr_9p_encode_finish(buf, header_size + sizeof(uint16_t) + written, pos);
}

// size[4] Twstat tag[2] fid[4] stat[n]

bool dr_9p_encode_Twstat(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t fid, const struct dr_9p_stat *restrict const stat) {
  const uint32_t ssize = sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint16_t) + stat->name.len + sizeof(uint16_t) + stat->uid.len + sizeof(uint16_t) + stat->gid.len + sizeof(uint16_t) + stat->muid.len;
  if (dr_unlikely(size < header_size + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t) + ssize)) {
    return false;
  }
  dr_9p_encode_header(buf, DR_TWSTAT, tag);
  uint32_t spos = header_size;
  dr_encode_uint32(buf + spos, fid);
  spos += sizeof(uint32_t);
  dr_encode_uint16(buf + spos, ssize + sizeof(uint16_t));
  spos += sizeof(uint16_t);
  dr_encode_uint16(buf + spos, ssize);
  spos += sizeof(uint16_t);
  dr_encode_uint16(buf + spos, stat->type);
  spos += sizeof(uint16_t);
  dr_encode_uint32(buf + spos, stat->dev);
  spos += sizeof(uint32_t);
  dr_encode_uint8(buf + spos, stat->qid.type);
  spos += sizeof(uint8_t);
  dr_encode_uint32(buf + spos, stat->qid.vers);
  spos += sizeof(uint32_t);
  dr_encode_uint64(buf + spos, stat->qid.path);
  spos += sizeof(uint64_t);
  dr_encode_uint32(buf + spos, stat->mode);
  spos += sizeof(uint32_t);
  dr_encode_uint32(buf + spos, stat->atime);
  spos += sizeof(uint32_t);
  dr_encode_uint32(buf + spos, stat->mtime);
  spos += sizeof(uint32_t);
  dr_encode_uint64(buf + spos, stat->length);
  spos += sizeof(uint64_t);
  dr_encode_uint16(buf + spos, stat->name.len);
  spos += sizeof(uint16_t);
  memcpy(buf + spos, stat->name.buf, stat->name.len);
  spos += stat->name.len;
  dr_encode_uint16(buf + spos, stat->uid.len);
  spos += sizeof(uint16_t);
  memcpy(buf + spos, stat->uid.buf, stat->uid.len);
  spos += stat->uid.len;
  dr_encode_uint16(buf + spos, stat->gid.len);
  spos += sizeof(uint16_t);
  memcpy(buf + spos, stat->gid.buf, stat->gid.len);
  spos += stat->gid.len;
  dr_encode_uint16(buf + spos, stat->muid.len);
  spos += sizeof(uint16_t);
  memcpy(buf + spos, stat->muid.buf, stat->muid.len);
  spos += stat->muid.len;
  return dr_9p_encode_finish(buf, spos, pos);
}

// size[4] Rwstat tag[2]

bool dr_9p_encode_Rwstat(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag) {
  return dr_9p_encode_null(DR_RWSTAT, buf, size, pos, tag);
}


struct dr_result_uint32 dr_dir_read(const struct dr_fd *restrict const fd, const uint64_t offset, const uint32_t count, void *restrict const b) {
  uint8_t *restrict const buf = (uint8_t *)b;
  if (offset != 0) {
    // DR Fix later
    return DR_RESULT_OK(uint32, 0);
  }
  const struct dr_dir *restrict const dir = container_of_const(fd->file, const struct dr_dir, file);
  uint32_t pos = 0;
  // DR Specific to this dr_dir impl
  for (uint_fast32_t i = 0; i < dir->entry_count; ++i) {
    const struct dr_file *restrict const f = dir->entries[i];
    const uint32_t written = dr_9p_encode_stat(buf + pos, count - pos, f);
    if (dr_unlikely(written == FAIL_UINT32)) {
      // Buffer is too small
      break;
    }
    pos += written;
  }
  return DR_RESULT_OK(uint32, pos);
}
