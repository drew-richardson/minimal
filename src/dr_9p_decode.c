// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

static const uint32_t header_size = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint16_t);
static const uint32_t qid_size = sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint64_t);

#define dr_decode_uintn(type, result, buf) \
  do { \
    for (size_t i = 0; i < sizeof(result); ++i) { \
      (result) |= ((type)(buf)[i]) << 8 * i; \
    } \
  } while (false)

uint8_t dr_decode_uint8(const uint8_t *restrict const buf) {
  uint8_t result = 0;
  dr_decode_uintn(uint8_t, result, buf);
  return result;
}

uint16_t dr_decode_uint16(const uint8_t *restrict const buf) {
  uint16_t result = 0;
  dr_decode_uintn(uint16_t, result, buf);
  return result;
}

uint32_t dr_decode_uint32(const uint8_t *restrict const buf) {
  uint32_t result = 0;
  dr_decode_uintn(uint32_t, result, buf);
  return result;
}

uint64_t dr_decode_uint64(const uint8_t *restrict const buf) {
  uint64_t result = 0;
  dr_decode_uintn(uint64_t, result, buf);
  return result;
}

// Move into a function so that the warning only happens once :(
static void dr_9p_set_str(struct dr_str *restrict const str, const uint16_t len, const uint8_t *restrict const buf) {
  str->len = len;
  // DR Is there better way to handle const?
  str->buf = (char *)buf;
}

static void dr_9p_decode_qid(struct dr_9p_qid *restrict const qid, const uint8_t *restrict const buf) {
  qid->type = dr_decode_uint8(buf);
  qid->vers = dr_decode_uint32(buf + sizeof(uint8_t));
  qid->path = dr_decode_uint64(buf + sizeof(uint8_t) + sizeof(uint32_t));
}

uint32_t dr_9p_decode_stat(struct dr_9p_stat *restrict const stat, const uint8_t *restrict const buf, const uint32_t size) {
  if (dr_unlikely(size < sizeof(uint16_t))) {
    return FAIL_UINT32;
  }
  const uint16_t stat_size = dr_decode_uint16(buf);
  if (dr_unlikely((size < sizeof(uint16_t) + stat_size) ||
		  (size < sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint32_t) + qid_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint16_t)))) {
    return FAIL_UINT32;
  }
  const uint16_t name_len = dr_decode_uint16(buf + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint32_t) + qid_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint64_t));
  if (dr_unlikely(size < sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint32_t) + qid_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint16_t) + name_len + sizeof(uint16_t))) {
    return FAIL_UINT32;
  }
  const uint16_t uid_len = dr_decode_uint16(buf + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint32_t) + qid_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint16_t) + name_len);
  if (dr_unlikely(size < sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint32_t) + qid_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint16_t) + name_len + sizeof(uint16_t) + uid_len + sizeof(uint16_t))) {
    return FAIL_UINT32;
  }
  const uint16_t gid_len = dr_decode_uint16(buf + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint32_t) + qid_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint16_t) + name_len + sizeof(uint16_t) + uid_len);
  if (dr_unlikely(size < sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint32_t) + qid_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint16_t) + name_len + sizeof(uint16_t) + uid_len + sizeof(uint16_t) + gid_len + sizeof(uint16_t))) {
    return FAIL_UINT32;
  }
  const uint16_t muid_len = dr_decode_uint16(buf + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint32_t) + qid_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint16_t) + name_len + sizeof(uint16_t) + uid_len + sizeof(uint16_t) + gid_len);
  if (dr_unlikely(size < sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint32_t) + qid_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint16_t) + name_len + sizeof(uint16_t) + uid_len + sizeof(uint16_t) + gid_len + sizeof(uint16_t) + muid_len)) {
    return FAIL_UINT32;
  }
  uint32_t spos = sizeof(uint16_t);
  stat->type = dr_decode_uint16(buf + spos);
  spos += sizeof(uint16_t);
  stat->dev = dr_decode_uint32(buf + spos);
  spos += sizeof(uint32_t);
  dr_9p_decode_qid(&stat->qid, buf + spos);
  spos += qid_size;
  stat->mode = dr_decode_uint32(buf + spos);
  spos += sizeof(uint32_t);
  stat->atime = dr_decode_uint32(buf + spos);
  spos += sizeof(uint32_t);
  stat->mtime = dr_decode_uint32(buf + spos);
  spos += sizeof(uint32_t);
  stat->length = dr_decode_uint64(buf + spos);
  spos += sizeof(uint64_t) + sizeof(uint16_t);
  dr_9p_set_str(&stat->name, name_len, buf + spos);
  spos += name_len + sizeof(uint16_t);
  dr_9p_set_str(&stat->uid, uid_len, buf + spos);
  spos += uid_len + sizeof(uint16_t);
  dr_9p_set_str(&stat->gid, gid_len, buf + spos);
  spos += gid_len + sizeof(uint16_t);
  dr_9p_set_str(&stat->muid, muid_len, buf + spos);
  spos += muid_len;
  return spos;
}

bool dr_9p_decode_header(uint8_t *restrict const type, uint16_t *restrict const tag, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  if (dr_unlikely(size < sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint16_t))) {
    return false;
  }
  *type = dr_decode_uint8(buf + sizeof(uint32_t));
  *tag = dr_decode_uint16(buf + sizeof(uint32_t) + sizeof(uint8_t));
  *pos = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint16_t);
  return true;
}

// size[4] Tversion tag[2] msize[4] version[s]

bool dr_9p_decode_Tversion(uint32_t *restrict const msize, struct dr_str *restrict const version, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  if (dr_unlikely(size < header_size + sizeof(uint32_t) + sizeof(uint16_t))) {
    return false;
  }
  const uint16_t version_len = dr_decode_uint16(buf + header_size + sizeof(uint32_t));
  if (dr_unlikely(size != header_size + sizeof(uint32_t) + sizeof(uint16_t) + version_len)) {
    return false;
  }
  *msize = dr_decode_uint32(buf + header_size);
  dr_9p_set_str(version, version_len, buf + header_size + sizeof(uint32_t) + sizeof(uint16_t));
  *pos = header_size + sizeof(uint32_t) + sizeof(uint16_t) + version_len;
  return true;
}

// size[4] Rversion tag[2] msize[4] version[s]

bool dr_9p_decode_Rversion(uint32_t *restrict const msize, struct dr_str *restrict const version, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  return dr_9p_decode_Tversion(msize, version, buf, size, pos);
}

// size[4] Tauth tag[2] afid[4] uname[s] aname[s]
// size[4] Rauth tag[2] aqid[13]

bool dr_9p_decode_Tauth(uint32_t *restrict const afid, struct dr_str *restrict const uname, struct dr_str *restrict const aname, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  if (dr_unlikely(size < header_size + sizeof(uint32_t) + sizeof(uint16_t))) {
    return false;
  }
  const uint16_t uname_len = dr_decode_uint16(buf + header_size + sizeof(uint32_t));
  if (dr_unlikely(size < header_size + sizeof(uint32_t) + sizeof(uint16_t) + uname_len + sizeof(uint16_t))) {
    return false;
  }
  const uint16_t aname_len = dr_decode_uint16(buf + header_size + sizeof(uint32_t) + sizeof(uint16_t) + uname_len);
  if (dr_unlikely(size != header_size + sizeof(uint32_t) + sizeof(uint16_t) + uname_len + sizeof(uint16_t) + aname_len)) {
    return false;
  }
  *afid = dr_decode_uint32(buf + header_size);
  dr_9p_set_str(uname, uname_len, buf + header_size + sizeof(uint32_t) + sizeof(uint16_t));
  dr_9p_set_str(aname, aname_len, buf + header_size + sizeof(uint32_t) + sizeof(uint16_t) + uname_len + sizeof(uint16_t));
  *pos = header_size + sizeof(uint32_t) + sizeof(uint16_t) + uname_len + sizeof(uint16_t) + aname_len;
  return true;
}

// size[4] Rerror tag[2] ename[s]

bool dr_9p_decode_Rerror(struct dr_str *restrict const ename, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  if (dr_unlikely(size < header_size + sizeof(uint16_t))) {
    return false;
  }
  const uint16_t ename_len = dr_decode_uint16(buf + header_size);
  if (dr_unlikely(size != header_size + sizeof(uint16_t) + ename_len)) {
    return false;
  }
  dr_9p_set_str(ename, ename_len, buf + header_size + sizeof(uint16_t));
  *pos = header_size + sizeof(uint16_t) + ename_len;
  return true;
}

// size[4] Tflush tag[2] oldtag[2]
// size[4] Rflush tag[2]

// size[4] Tattach tag[2] fid[4] afid[4] uname[s] aname[s]

bool dr_9p_decode_Tattach(uint32_t *restrict const fid, uint32_t *restrict const afid, struct dr_str *restrict const uname, struct dr_str *restrict const aname, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  if (dr_unlikely(size < header_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t))) {
    return false;
  }
  const uint16_t uname_len = dr_decode_uint16(buf + header_size + sizeof(uint32_t) + sizeof(uint32_t));
  if (dr_unlikely(size < header_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t) + uname_len + sizeof(uint16_t))) {
    return false;
  }
  const uint16_t aname_len = dr_decode_uint16(buf + header_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t) + uname_len);
  if (dr_unlikely(size != header_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t) + uname_len + sizeof(uint16_t) + aname_len)) {
    return false;
  }
  *fid = dr_decode_uint32(buf + header_size);
  *afid = dr_decode_uint32(buf + header_size + sizeof(uint32_t));
  dr_9p_set_str(uname, uname_len, buf + header_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t));
  dr_9p_set_str(aname, aname_len, buf + header_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t) + uname_len + sizeof(uint16_t));
  *pos = header_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t) + uname_len + sizeof(uint16_t) + aname_len;
  return true;
}

// size[4] Rattach tag[2] qid[13]

bool dr_9p_decode_Rattach(struct dr_9p_qid *restrict const qid, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  if (dr_unlikely(size != header_size + qid_size)) {
    return false;
  }
  dr_9p_decode_qid(qid, buf + header_size);
  *pos = header_size + qid_size;
  return true;
}

// size[4] Twalk tag[2] fid[4] newfid[4] nwname[2] nwname*(wname[s])

bool dr_9p_decode_Twalk_iterator(uint32_t *restrict const fid, uint32_t *restrict const newfid, uint16_t *restrict const nwname, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  if (dr_unlikely(size < header_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t))) {
    return false;
  }
  *fid = dr_decode_uint32(buf + header_size);
  *newfid = dr_decode_uint32(buf + header_size + sizeof(uint32_t));
  *nwname = dr_decode_uint16(buf + header_size + sizeof(uint32_t) + sizeof(uint32_t));
  *pos = header_size + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t);
  return true;
}

bool dr_9p_decode_Twalk_advance(struct dr_str *restrict const wname, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  const uint32_t p = *pos;
  if (dr_unlikely(size < p + sizeof(uint16_t))) {
    return false;
  }
  const uint16_t wname_len = dr_decode_uint16(buf + p);
  if (dr_unlikely(size < p + sizeof(uint16_t) + wname_len)) {
    return false;
  }
  dr_9p_set_str(wname, wname_len, buf + p + sizeof(uint16_t));
  *pos += sizeof(uint16_t) + wname_len;
  return true;
}

bool dr_9p_decode_Twalk_finish(const uint32_t size, const uint32_t pos) {
  return pos == size;
}

// size[4] Rwalk tag[2] nwqid[2] nwqid*(wqid[13])

bool dr_9p_decode_Rwalk_iterator(uint16_t *restrict const nwqid, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  if (dr_unlikely(size < header_size + sizeof(uint16_t))) {
    return false;
  }
  const uint16_t nw = dr_decode_uint16(buf + header_size);
  if (dr_unlikely(size != header_size + sizeof(uint16_t) + nw*qid_size)) {
    return false;
  }
  *nwqid = nw;
  *pos = header_size + sizeof(uint16_t);
  return true;
}

bool dr_9p_decode_Rwalk_advance(struct dr_9p_qid *restrict const qid, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  const uint32_t p = *pos;
  if (dr_unlikely(size < p + qid_size)) {
    return false;
  }
  dr_9p_decode_qid(qid, buf + p);
  *pos += qid_size;
  return true;
}

bool dr_9p_decode_Rwalk_finish(const uint32_t size, const uint32_t pos) {
  return pos == size;
}

// size[4] Topen tag[2] fid[4] mode[1]

bool dr_9p_decode_Topen(uint32_t *restrict const fid, uint8_t *restrict const mode, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  if (dr_unlikely(size != header_size + sizeof(uint32_t) + sizeof(uint8_t))) {
    return false;
  }
  *fid = dr_decode_uint32(buf + header_size);
  *mode = dr_decode_uint8(buf + header_size + sizeof(uint32_t));
  *pos = header_size + sizeof(uint32_t) + sizeof(uint8_t);
  return true;
}

// The iounit field returned by open(9P), if non-zero, reports the maximum size that is guaranteed to be transferred atomically.

// size[4] Ropen tag[2] qid[13] iounit[4]

bool dr_9p_decode_Ropen(struct dr_9p_qid *restrict const qid, uint32_t *restrict const iounit, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  if (dr_unlikely(size != header_size + qid_size + sizeof(uint32_t))) {
    return false;
  }
  dr_9p_decode_qid(qid, buf + header_size);
  *iounit = dr_decode_uint32(buf + header_size + qid_size);
  *pos = header_size + qid_size + sizeof(uint32_t);
  return true;
}

// size[4] Topenfd tag[2] fid[4] mode[1]
// size[4] Ropenfd tag[2] qid[13] iounit[4] unixfd[4]

// size[4] Tcreate tag[2] fid[4] name[s] perm[4] mode[1]

bool dr_9p_decode_Tcreate(uint32_t *restrict const fid, struct dr_str *restrict const name, uint32_t *restrict const perm, uint8_t *restrict const mode, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  if (dr_unlikely(size < header_size + sizeof(uint32_t) + sizeof(uint16_t))) {
    return false;
  }
  const uint16_t name_len = dr_decode_uint16(buf + header_size + sizeof(uint32_t));
  if (dr_unlikely(size != header_size + sizeof(uint32_t) + sizeof(uint16_t) + name_len + sizeof(uint32_t) + sizeof(uint8_t))) {
    return false;
  }
  *fid = dr_decode_uint32(buf + header_size);
  dr_9p_set_str(name, name_len, buf + header_size + sizeof(uint32_t) + sizeof(uint16_t));
  *perm = dr_decode_uint32(buf + header_size + sizeof(uint32_t) + sizeof(uint16_t) + name_len);
  *mode = dr_decode_uint8(buf + header_size + sizeof(uint32_t) + sizeof(uint16_t) + name_len + sizeof(uint32_t));
  *pos = header_size + sizeof(uint32_t) + sizeof(uint16_t) + name_len + sizeof(uint32_t) + sizeof(uint8_t);
  return true;
}

// size[4] Rcreate tag[2] qid[13] iounit[4]

bool dr_9p_decode_Rcreate(struct dr_9p_qid *restrict const qid, uint32_t *restrict const iounit, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  return dr_9p_decode_Ropen(qid, iounit, buf, size, pos);
}

// size[4] Tread tag[2] fid[4] offset[8] count[4]

bool dr_9p_decode_Tread(uint32_t *restrict const fid, uint64_t *restrict const offset, uint32_t *restrict const count, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  if (dr_unlikely(size != header_size + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint32_t))) {
    return false;
  }
  *fid = dr_decode_uint32(buf + header_size);
  *offset = dr_decode_uint64(buf + header_size + sizeof(uint32_t));
  *count = dr_decode_uint32(buf + header_size + sizeof(uint32_t) + sizeof(uint64_t));
  *pos = header_size + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint32_t);
  return true;
}

// size[4] Rread tag[2] count[4] data[count]

bool dr_9p_decode_Rread(uint32_t *restrict const count, const void *restrict *restrict const data, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  if (dr_unlikely(size < header_size + sizeof(uint32_t))) {
    return false;
  }
  const uint32_t data_count = dr_decode_uint32(buf + header_size);
  if (dr_unlikely(size != header_size + sizeof(uint32_t) + data_count)) {
    return false;
  }
  *count = data_count;
  *data = buf + header_size + sizeof(uint32_t);
  *pos = header_size + sizeof(uint32_t) + data_count;
  return true;
}

// size[4] Twrite tag[2] fid[4] offset[8] count[4] data[count]

bool dr_9p_decode_Twrite(uint32_t *restrict const fid, uint64_t *restrict const offset, uint32_t *restrict const count, const void *restrict *restrict const data, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  if (dr_unlikely(size < header_size + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint32_t))) {
    return false;
  }
  const uint32_t data_count = dr_decode_uint32(buf + header_size + sizeof(uint32_t) + sizeof(uint64_t));
  if (dr_unlikely(size != header_size + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint32_t) + data_count)) {
    return false;
  }
  *fid = dr_decode_uint32(buf + header_size);
  *offset = dr_decode_uint64(buf + header_size + sizeof(uint32_t));
  *count = data_count;
  *data = buf + header_size + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint32_t);
  *pos = header_size + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint32_t) + data_count;
  return true;
}

// size[4] Rwrite tag[2] count[4]

bool dr_9p_decode_Rwrite(uint32_t *restrict const count, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  if (dr_unlikely(size != header_size + sizeof(uint32_t))) {
    return false;
  }
  *count = dr_decode_uint32(buf + header_size);
  *pos = header_size + sizeof(uint32_t);
  return true;
}

// size[4] Tclunk tag[2] fid[4]

bool dr_9p_decode_Tclunk(uint32_t *restrict const fid, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  return dr_9p_decode_Rwrite(fid, buf, size, pos);
}

// size[4] Rclunk tag[2]

bool dr_9p_decode_Rclunk(const uint32_t size, uint32_t *restrict const pos) {
  if (dr_unlikely(size != header_size)) {
    return false;
  }
  *pos = header_size;
  return true;
}

// size[4] Tremove tag[2] fid[4]

bool dr_9p_decode_Tremove(uint32_t *restrict const fid, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  return dr_9p_decode_Rwrite(fid, buf, size, pos);
}

// size[4] Rremove tag[2]

bool dr_9p_decode_Rremove(const uint32_t size, uint32_t *restrict const pos) {
  return dr_9p_decode_Rclunk(size, pos);
}

// size[4] Tstat tag[2] fid[4]

bool dr_9p_decode_Tstat(uint32_t *restrict const fid, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  return dr_9p_decode_Rwrite(fid, buf, size, pos);
}

// size[4] Rstat tag[2] stat[n]

bool dr_9p_decode_Rstat(struct dr_9p_stat *restrict const stat, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  if (dr_unlikely(size < header_size + sizeof(uint16_t))) {
    return false;
  }
  const uint16_t payload_size = dr_decode_uint16(buf + header_size);
  if (dr_unlikely((size != header_size + sizeof(uint16_t) + payload_size))) {
    return false;
  }
  const uint32_t read = dr_9p_decode_stat(stat, buf + header_size + sizeof(uint16_t), size - header_size - sizeof(uint16_t));
  if (dr_unlikely(read == FAIL_UINT32)) {
    return false;
  }
  if (dr_unlikely(header_size + sizeof(uint16_t) + read != size)) {
    return false;
  }
  *pos = header_size + sizeof(uint16_t) + read;
  return true;
}

// size[4] Twstat tag[2] fid[4] stat[n]

bool dr_9p_decode_Twstat(uint32_t *restrict const fid, struct dr_9p_stat *restrict const stat, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos) {
  if (dr_unlikely(size < header_size + sizeof(uint32_t) + sizeof(uint16_t))) {
    return false;
  }
  const uint16_t payload_size = dr_decode_uint16(buf + header_size + sizeof(uint32_t));
  if (dr_unlikely(size != header_size + sizeof(uint32_t) + sizeof(uint16_t) + payload_size)) {
    return false;
  }
  *fid = dr_decode_uint32(buf + header_size);
  const uint32_t read = dr_9p_decode_stat(stat, buf + header_size + sizeof(uint32_t) + sizeof(uint16_t), size - header_size - sizeof(uint32_t) - sizeof(uint16_t));
  if (dr_unlikely(read == FAIL_UINT32)) {
    return false;
  }
  if (dr_unlikely(header_size + sizeof(uint32_t) + sizeof(uint16_t) + read != size)) {
    return false;
  }
  *pos = header_size + sizeof(uint32_t) + sizeof(uint16_t) + payload_size;
  return true;
}

// size[4] Rwstat tag[2]

bool dr_9p_decode_Rwstat(const uint32_t size, uint32_t *restrict const pos) {
  return dr_9p_decode_Rclunk(size, pos);
}
