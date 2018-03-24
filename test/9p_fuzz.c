// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

#include <string.h>

#define DR_9P_BUF_SIZE (1<<13)

static void check_str(struct dr_str *restrict const str) {
  memset(str->buf, 0, str->len);
}

static void check_stat(struct dr_9p_stat *restrict const stat) {
  check_str(&stat->name);
  check_str(&stat->uid);
  check_str(&stat->gid);
  check_str(&stat->muid);
}

int main(void) {
  {
    const struct dr_result_void r = dr_console_startup();
    DR_IF_RESULT_ERR(r, err) {
      dr_log_error("dr_console_startup failed", err);
      return -1;
    } DR_FI_RESULT;
  }
  uint8_t buf[DR_9P_BUF_SIZE];
  size_t bytes;
  {
    const struct dr_result_size r = dr_read(dr_stdin, buf, sizeof(buf));
    DR_IF_RESULT_ERR(r, err) {
      dr_log_error("dr_read failed", err);
      return -1;
    } DR_ELIF_RESULT_OK(size_t, r, value) {
      bytes = value;
    } DR_FI_RESULT;
  }
  if (dr_unlikely(bytes == 0)) {
    dr_log("nothing read");
    return -1;
  }
  if (dr_unlikely(bytes < sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint16_t))) {
    dr_log("Short read");
    return -1;
  }
  const uint32_t size = dr_decode_uint32(buf);
  if (dr_unlikely(bytes != size)) {
    dr_log("Short read");
    return -1;
  }
  uint32_t pos;
  uint8_t type;
  uint16_t tag;
  if (dr_unlikely(!dr_9p_decode_header(&type, &tag, buf, size, &pos))) {
    dr_log("dr_9p_decode_header failed");
    return -1;
  }
  switch (type) {
  case DR_TVERSION: {
    uint32_t msize;
    struct dr_str version;
    if (dr_unlikely(!dr_9p_decode_Tversion(&msize, &version, buf, size, &pos))) {
      dr_log("dr_9p_decode_Tversion failed");
      return -1;
    }
    check_str(&version);
    return 0;
  }
  case DR_RVERSION: {
    uint32_t msize;
    struct dr_str version;
    if (dr_unlikely(!dr_9p_decode_Rversion(&msize, &version, buf, size, &pos))) {
      dr_log("dr_9p_decode_Rversion failed");
      return -1;
    }
    check_str(&version);
    return 0;
  }
  case DR_TAUTH: {
    uint32_t afid;
    struct dr_str uname;
    struct dr_str aname;
    if (dr_unlikely(!dr_9p_decode_Tauth(&afid, &uname, &aname, buf, size, &pos))) {
      dr_log("dr_9p_decode_Tauth failed");
      return -1;
    }
    check_str(&uname);
    check_str(&aname);
    return 0;
  }
  case DR_RERROR: {
    struct dr_str ename;
    if (dr_unlikely(!dr_9p_decode_Rerror(&ename, buf, size, &pos))) {
      dr_log("dr_9p_decode_Rerror failed");
      return -1;
    }
    check_str(&ename);
    return 0;
  }
  case DR_TATTACH: {
    uint32_t fid;
    uint32_t afid;
    struct dr_str uname;
    struct dr_str aname;
    if (dr_unlikely(!dr_9p_decode_Tattach(&fid, &afid, &uname, &aname, buf, size, &pos))) {
      dr_log("dr_9p_decode_Tattach failed");
      return -1;
    }
    check_str(&uname);
    check_str(&aname);
    return 0;
  }
  case DR_RATTACH: {
    struct dr_9p_qid qid;
    if (dr_unlikely(!dr_9p_decode_Rattach(&qid, buf, size, &pos))) {
      dr_log("dr_9p_decode_Rattach failed");
      return -1;
    }
    return 0;
  }
  case DR_TWALK: {
    uint32_t fid;
    uint32_t newfid;
    uint16_t nwname;
    if (dr_unlikely(!dr_9p_decode_Twalk_iterator(&fid, &newfid, &nwname, buf, size, &pos))) {
      dr_log("dr_9p_decode_Twalk_iterator failed");
      return -1;
    }
    for (uint_fast16_t i = 0; i < nwname; ++i) {
      struct dr_str wname;
      if (dr_unlikely(!dr_9p_decode_Twalk_advance(&wname, buf, size, &pos))) {
	dr_log("dr_9p_decode_Twalk_advance failed");
	return -1;
      }
      check_str(&wname);
    }
    if (dr_unlikely(!dr_9p_decode_Twalk_finish(size, pos))) {
      dr_log("dr_9p_decode_Twalk_finish failed");
      return -1;
    }
    return 0;
  }
  case DR_RWALK: {
    uint16_t nwqid;
    if (dr_unlikely(!dr_9p_decode_Rwalk_iterator(&nwqid, buf, size, &pos))) {
      dr_log("dr_9p_decode_Rwalk_iterator failed");
      return -1;
    }
    for (uint_fast16_t i = 0; i < nwqid; ++i) {
      struct dr_9p_qid qid;
      if (dr_unlikely(!dr_9p_decode_Rwalk_advance(&qid, buf, size, &pos))) {
	dr_log("dr_9p_decode_Rwalk_advance failed");
	return -1;
      }
    }
    if (dr_unlikely(!dr_9p_decode_Rwalk_finish(size, pos))) {
      dr_log("dr_9p_decode_Rwalk_finish failed");
      return -1;
    }
    return 0;
  }
  case DR_TOPEN: {
    uint32_t fid;
    uint8_t mode;
    if (dr_unlikely(!dr_9p_decode_Topen(&fid, &mode, buf, size, &pos))) {
      dr_log("dr_9p_decode_Topen failed");
      return -1;
    }
    return 0;
  }
  case DR_ROPEN: {
    struct dr_9p_qid qid;
    uint32_t iounit;
    if (dr_unlikely(!dr_9p_decode_Ropen(&qid, &iounit, buf, size, &pos))) {
      dr_log("dr_9p_decode_Ropen failed");
      return -1;
    }
    return 0;
  }
  case DR_TCREATE: {
    uint32_t fid;
    struct dr_str name;
    uint32_t perm;
    uint8_t mode;
    if (dr_unlikely(!dr_9p_decode_Tcreate(&fid, &name, &perm, &mode, buf, size, &pos))) {
      dr_log("dr_9p_decode_Tcreate failed");
      return -1;
    }
    check_str(&name);
    return 0;
  }
  case DR_RCREATE: {
    struct dr_9p_qid qid;
    uint32_t iounit;
    if (dr_unlikely(!dr_9p_decode_Rcreate(&qid, &iounit, buf, size, &pos))) {
      dr_log("dr_9p_decode_Rcreate failed");
      return -1;
    }
    return 0;
  }
  case DR_TREAD: {
    uint32_t fid;
    uint64_t offset;
    uint32_t count;
    if (dr_unlikely(!dr_9p_decode_Tread(&fid, &offset, &count, buf, size, &pos))) {
      dr_log("dr_9p_decode_Tread failed");
      return -1;
    }
    return 0;
  }
  case DR_RREAD: {
    uint32_t count;
    const void *restrict data;
    if (dr_unlikely(!dr_9p_decode_Rread(&count, &data, buf, size, &pos))) {
      dr_log("dr_9p_decode_Rread failed");
      return -1;
    }
    memset(buf + ((intptr_t)data - (intptr_t)buf), 0, count);
    return 0;
  }
  case DR_TWRITE: {
    uint32_t fid;
    uint64_t offset;
    uint32_t count;
    const void *restrict data;
    if (dr_unlikely(!dr_9p_decode_Twrite(&fid, &offset, &count, &data, buf, size, &pos))) {
      dr_log("dr_9p_decode_Twrite failed");
      return -1;
    }
    memset(buf + ((intptr_t)data - (intptr_t)buf), 0, count);
    return 0;
  }
  case DR_RWRITE: {
    uint32_t count;
    if (dr_unlikely(!dr_9p_decode_Rwrite(&count, buf, size, &pos))) {
      dr_log("dr_9p_decode_Rwrite failed");
      return -1;
    }
    return 0;
  }
  case DR_TCLUNK: {
    uint32_t fid;
    if (dr_unlikely(!dr_9p_decode_Tclunk(&fid, buf, size, &pos))) {
      dr_log("dr_9p_decode_Tclunk failed");
      return -1;
    }
    return 0;
  }
  case DR_RCLUNK: {
    if (dr_unlikely(!dr_9p_decode_Rclunk(size, &pos))) {
      dr_log("dr_9p_decode_Rclunk failed");
      return -1;
    }
    return 0;
  }
  case DR_TREMOVE: {
    uint32_t fid;
    if (dr_unlikely(!dr_9p_decode_Tremove(&fid, buf, size, &pos))) {
      dr_log("dr_9p_decode_Tremove failed");
      return -1;
    }
    return 0;
  }
  case DR_RREMOVE: {
    if (dr_unlikely(!dr_9p_decode_Rremove(size, &pos))) {
      dr_log("dr_9p_decode_Rremove failed");
      return -1;
    }
    return 0;
  }
  case DR_TSTAT: {
    uint32_t fid;
    if (dr_unlikely(!dr_9p_decode_Tstat(&fid, buf, size, &pos))) {
      dr_log("dr_9p_decode_Tstat failed");
      return -1;
    }
    return 0;
  }
  case DR_RSTAT: {
    struct dr_9p_stat stat;
    if (dr_unlikely(!dr_9p_decode_Rstat(&stat, buf, size, &pos))) {
      dr_log("dr_9p_decode_Rstat failed");
      return -1;
    }
    check_stat(&stat);
    return 0;
  }
  case DR_TWSTAT: {
    uint32_t fid;
    struct dr_9p_stat stat;
    if (dr_unlikely(!dr_9p_decode_Twstat(&fid, &stat, buf, size, &pos))) {
      dr_log("dr_9p_decode_Twstat failed");
      return -1;
    }
    return 0;
  }
  case DR_RWSTAT: {
    if (dr_unlikely(!dr_9p_decode_Rwstat(size, &pos))) {
      dr_log("dr_9p_decode_Rwstat failed");
      return -1;
    }
    return 0;
  }
  default:
    dr_log("Unrecognized message");
    return -1;
  }
}
