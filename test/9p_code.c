// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE (1<<13)
#define HEADER_OFFSET 7

#define QID_PATH(f) \
    (uint64_t)(uintptr_t)(f) >> 0*8, (uint64_t)(uintptr_t)(f) >> 1*8, \
    (uint64_t)(uintptr_t)(f) >> 2*8, (uint64_t)(uintptr_t)(f) >> 3*8, \
    (uint64_t)(uintptr_t)(f) >> 4*8, (uint64_t)(uintptr_t)(f) >> 5*8, \
    (uint64_t)(uintptr_t)(f) >> 6*8, (uint64_t)(uintptr_t)(f) >> 7*8

static char u0name_buf[] = { 'o', 'w', 'n' };
static char u1name_buf[] = { 'i', 'd' };

static struct dr_user u0 = {
  .name.len = sizeof(u0name_buf),
  .name.buf = u0name_buf,
};

static struct dr_user u1 = {
  .name.len = sizeof(u1name_buf),
  .name.buf = u1name_buf,
};

int main(void) {
  dr_assert(0xdeadbeef == dr_decode_uint32((const uint8_t[]) { 0xef, 0xbe, 0xad, 0xde }));
  // DR dr_9p_decode_stat
  {
    const uint8_t buf[] = { 0xff, 0xff, 0xff, 0xff, 0x12, 0x34, 0x56 };
    uint8_t type;
    uint16_t tag;
    uint32_t pos;
    dr_assert(dr_9p_decode_header(&type, &tag, buf, sizeof(buf), &pos) &&
	      type == 0x12 &&
	      tag == 0x5634 &&
	      pos == HEADER_OFFSET);
    for (size_t i = 0; i < sizeof(buf); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (i > 0) {
	memcpy(b, buf, i);
      }
      dr_assert(!dr_9p_decode_header(&type, &tag, b, i, &pos));
      free(b);
    }
  }
  {
    const uint8_t buf[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x12, 0x34, 0x56, 0x78, 0x06, 0x00, '9', 'P', '2', '0', '0', '0' };
    uint32_t msize;
    struct dr_str version;
    uint32_t pos = HEADER_OFFSET;
    dr_assert(dr_9p_decode_Tversion(&msize, &version, buf, sizeof(buf), &pos) &&
	      msize == 0x78563412 &&
	      version.len == 6 &&
	      memcmp(version.buf, "9P2000", 6) == 0 &&
	      pos == sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (i > 0) {
	memcpy(b, buf, i);
      }
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Tversion(&msize, &version, b, i, &pos));
      free(b);
    }
    dr_assert(!dr_9p_decode_Tversion(&msize, &version, buf, sizeof(buf) + 1, &pos));
  }
  {
    const uint8_t buf[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x12, 0x34, 0x56, 0x78, 0x06, 0x00, '9', 'P', '2', '0', '0', '0' };
    uint32_t msize;
    struct dr_str version;
    uint32_t pos = HEADER_OFFSET;
    dr_assert(dr_9p_decode_Rversion(&msize, &version, buf, sizeof(buf), &pos) &&
	      msize == 0x78563412 &&
	      version.len == 6 &&
	      memcmp(version.buf, "9P2000", 6) == 0 &&
	      pos == sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (i > 0) {
	memcpy(b, buf, i);
      }
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Rversion(&msize, &version, b, i, &pos));
      free(b);
    }
    dr_assert(!dr_9p_decode_Rversion(&msize, &version, buf, sizeof(buf) + 1, &pos));
  }
  {
    const uint8_t buf[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x12, 0x34, 0x56, 0x78, 0x0e, 0x00, 'd', 'r', 'e', 'w', 'r', 'i', 'c', 'h', 'a', 'r', 'd', 's', 'o', 'n', 0x01, 0x00, '/' };
    uint32_t afid;
    struct dr_str uname;
    struct dr_str aname;
    uint32_t pos = HEADER_OFFSET;
    dr_assert(dr_9p_decode_Tauth(&afid, &uname, &aname, buf, sizeof(buf), &pos) &&
	      afid == 0x78563412 &&
	      uname.len == 14 &&
	      memcmp(uname.buf, "drewrichardson", 14) == 0 &&
	      aname.len == 1 &&
	      memcmp(aname.buf, "/", 1) == 0 &&
	      pos == sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (i > 0) {
	memcpy(b, buf, i);
      }
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Tauth(&afid, &uname, &aname, b, i, &pos));
      free(b);
    }
    dr_assert(!dr_9p_decode_Tauth(&afid, &uname, &aname, buf, sizeof(buf) + 1, &pos));
  }
  {
    const uint8_t buf[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x09, 0x00, 's', 'o', 'm', 'e', 't', 'h', 'i', 'n', 'g' };
    struct dr_str ename;
    uint32_t pos = HEADER_OFFSET;
    dr_assert(dr_9p_decode_Rerror(&ename, buf, sizeof(buf), &pos) &&
	      ename.len == 9 &&
	      memcmp(ename.buf, "something", 9) == 0 &&
	      pos == sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (i > 0) {
	memcpy(b, buf, i);
      }
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Rerror(&ename, b, i, &pos));
      free(b);
    }
    dr_assert(!dr_9p_decode_Rerror(&ename, buf, sizeof(buf) + 1, &pos));
  }
  {
    const uint8_t buf[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x0e, 0x00, 'd', 'r', 'e', 'w', 'r', 'i', 'c', 'h', 'a', 'r', 'd', 's', 'o', 'n', 0x01, 0x00, '/' };
    uint32_t fid;
    uint32_t afid;
    struct dr_str uname;
    struct dr_str aname;
    uint32_t pos = HEADER_OFFSET;
    dr_assert(dr_9p_decode_Tattach(&fid, &afid, &uname, &aname, buf, sizeof(buf), &pos) &&
	      fid == 0x78563412 &&
	      afid == 0xf0debc9a &&
	      uname.len == 14 &&
	      memcmp(uname.buf, "drewrichardson", 14) == 0 &&
	      aname.len == 1 &&
	      memcmp(aname.buf, "/", 1) == 0 &&
	      pos == sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (i > 0) {
	memcpy(b, buf, i);
      }
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Tattach(&fid, &afid, &uname, &aname, b, i, &pos));
      free(b);
    }
    dr_assert(!dr_9p_decode_Tattach(&fid, &afid, &uname, &aname, buf, sizeof(buf) + 1, &pos));
  }
  {
    const uint8_t buf[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xca, 0xde, 0xad, 0xbe, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };
    struct dr_9p_qid qid;
    uint32_t pos = HEADER_OFFSET;
    dr_assert(dr_9p_decode_Rattach(&qid, buf, sizeof(buf), &pos) &&
	      qid.type == 0xca &&
	      qid.vers == 0xefbeadde &&
	      qid.path == 0xefcdab8967452301 &&
	      pos == sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (i > 0) {
	memcpy(b, buf, i);
      }
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Rattach(&qid, b, i, &pos));
      free(b);
    }
    dr_assert(!dr_9p_decode_Rattach(&qid, buf, sizeof(buf) + 1, &pos));
  }
  {
    const uint8_t buf[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x02, 0x00, 0x03, 0x00, 'd', 'i', 'r', 0x04, 0x00, 'f', 'i', 'l', 'e' };
    uint32_t fid;
    uint32_t newfid;
    uint16_t nwname;
    uint32_t pos = HEADER_OFFSET;
    struct dr_str wname;
    dr_assert(dr_9p_decode_Twalk_iterator(&fid, &newfid, &nwname, buf, sizeof(buf), &pos) &&
	      fid == 0x78563412 &&
	      newfid == 0xf0debc9a &&
	      nwname == 2 &&
	      dr_9p_decode_Twalk_advance(&wname, buf, sizeof(buf), &pos) &&
	      wname.len == 3 &&
	      memcmp(wname.buf, "dir", 3) == 0 &&
	      dr_9p_decode_Twalk_advance(&wname, buf, sizeof(buf), &pos) &&
	      wname.len == 4 &&
	      memcmp(wname.buf, "file", 4) == 0 &&
	      dr_9p_decode_Twalk_finish(sizeof(buf), pos) &&
	      pos == sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (i > 0) {
	memcpy(b, buf, i);
      }
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Twalk_iterator(&fid, &newfid, &nwname, b, i, &pos) ||
		!dr_9p_decode_Twalk_advance(&wname, b, i, &pos) ||
		!dr_9p_decode_Twalk_advance(&wname, b, i, &pos) ||
		!dr_9p_decode_Twalk_finish(i, pos));
      free(b);
    }
    dr_assert(!dr_9p_decode_Twalk_iterator(&fid, &newfid, &nwname, buf, sizeof(buf) + 1, &pos) ||
	      !dr_9p_decode_Twalk_advance(&wname, buf, sizeof(buf) + 1, &pos) ||
	      !dr_9p_decode_Twalk_advance(&wname, buf, sizeof(buf) + 1, &pos) ||
	      !dr_9p_decode_Twalk_finish(sizeof(buf) + 1, pos));
  }
  {
    const uint8_t buf[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x02, 0x00, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23 };
    uint16_t nwqid;
    uint32_t pos = HEADER_OFFSET;
    struct dr_9p_qid qid;
    dr_assert(dr_9p_decode_Rwalk_iterator(&nwqid, buf, sizeof(buf), &pos) &&
	      nwqid == 2 &&
	      dr_9p_decode_Rwalk_advance(&qid, buf, sizeof(buf), &pos) &&
	      qid.type == 0x01 &&
	      qid.vers == 0x89674523 &&
	      qid.path == 0x8967452301efcdab &&
	      dr_9p_decode_Rwalk_advance(&qid, buf, sizeof(buf), &pos) &&
	      qid.type == 0xab &&
	      qid.vers == 0x2301efcd &&
	      qid.path == 0x2301efcdab896745 &&
	      dr_9p_decode_Rwalk_finish(sizeof(buf), pos) &&
	      pos == sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (i > 0) {
	memcpy(b, buf, i);
      }
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Rwalk_iterator(&nwqid, b, i, &pos) ||
		!dr_9p_decode_Rwalk_advance(&qid, b, i, &pos) ||
		!dr_9p_decode_Rwalk_advance(&qid, b, i, &pos) ||
		!dr_9p_decode_Rwalk_finish(i, pos));
      free(b);
    }
    dr_assert(!dr_9p_decode_Rwalk_iterator(&nwqid, buf, sizeof(buf) + 1, &pos) ||
	      !dr_9p_decode_Rwalk_advance(&qid, buf, sizeof(buf) + 1, &pos) ||
	      !dr_9p_decode_Rwalk_advance(&qid, buf, sizeof(buf) + 1, &pos) ||
	      !dr_9p_decode_Rwalk_finish(sizeof(buf) + 1, pos));
  }
  {
    const uint8_t buf[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x12, 0x34, 0x56, 0x78, 0xde };
    uint32_t fid;
    uint8_t mode;
    uint32_t pos = HEADER_OFFSET;
    dr_assert(dr_9p_decode_Topen(&fid, &mode, buf, sizeof(buf), &pos) &&
	      fid == 0x78563412 &&
	      mode == 0xde &&
	      pos == sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (i > 0) {
	memcpy(b, buf, i);
      }
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Topen(&fid, &mode, b, i, &pos));
      free(b);
    }
    dr_assert(!dr_9p_decode_Topen(&fid, &mode, buf, sizeof(buf) + 1, &pos));
  }
  {
    const uint8_t buf[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xca, 0xde, 0xad, 0xbe, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xca, 0xfe, 0xd0, 0x0d };
    struct dr_9p_qid qid;
    uint32_t iounit;
    uint32_t pos = HEADER_OFFSET;
    dr_assert(dr_9p_decode_Ropen(&qid, &iounit, buf, sizeof(buf), &pos) &&
	      qid.type == 0xca &&
	      qid.vers == 0xefbeadde &&
	      qid.path == 0xefcdab8967452301 &&
	      iounit == 0x0dd0feca &&
	      pos == sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (i > 0) {
	memcpy(b, buf, i);
      }
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Ropen(&qid, &iounit, b, i, &pos));
      free(b);
    }
    dr_assert(!dr_9p_decode_Ropen(&qid, &iounit, buf, sizeof(buf) + 1, &pos));
  }
  {
    const uint8_t buf[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xde, 0xad, 0xbe, 0xef, 0x04, 0x00, 'f', 'i', 'l', 'e', 0xca, 0xfe, 0xd0, 0x0d, 0xfe };
    uint32_t fid;
    struct dr_str name;
    uint32_t perm;
    uint8_t mode;
    uint32_t pos = HEADER_OFFSET;
    dr_assert(dr_9p_decode_Tcreate(&fid, &name, &perm, &mode, buf, sizeof(buf), &pos) &&
	      fid == 0xefbeadde &&
	      name.len == 4 &&
	      memcmp(name.buf, "file", 4) == 0 &&
	      perm == 0x0dd0feca &&
	      mode == 0xfe &&
	      pos == sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (i > 0) {
	memcpy(b, buf, i);
      }
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Tcreate(&fid, &name, &perm, &mode, b, i, &pos));
      free(b);
    }
    dr_assert(!dr_9p_decode_Tcreate(&fid, &name, &perm, &mode, buf, sizeof(buf) + 1, &pos));
  }
  {
    const uint8_t buf[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xca, 0xde, 0xad, 0xbe, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xde, 0xad, 0xbe, 0xef };
    struct dr_9p_qid qid;
    uint32_t iounit;
    uint32_t pos = HEADER_OFFSET;
    dr_assert(dr_9p_decode_Rcreate(&qid, &iounit, buf, sizeof(buf), &pos) &&
	      qid.type == 0xca &&
	      qid.vers == 0xefbeadde &&
	      qid.path == 0xefcdab8967452301 &&
	      iounit == 0xefbeadde &&
	      pos == sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (i > 0) {
	memcpy(b, buf, i);
      }
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Rcreate(&qid, &iounit, b, i, &pos));
      free(b);
    }
    dr_assert(!dr_9p_decode_Rcreate(&qid, &iounit, buf, sizeof(buf) + 1, &pos));
  }
  {
    const uint8_t buf[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x12, 0x34, 0x56, 0x78, 0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xba, 0xbe, 0x9a, 0xbc, 0xde, 0xf0 };
    uint32_t fid;
    uint64_t offset;
    uint32_t count;
    uint32_t pos = HEADER_OFFSET;
    dr_assert(dr_9p_decode_Tread(&fid, &offset, &count, buf, sizeof(buf), &pos) &&
	      fid == 0x78563412 &&
	      offset == 0xbebafecaefbeadde &&
	      count == 0xf0debc9a &&
	      pos == sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (i > 0) {
	memcpy(b, buf, i);
      }
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Tread(&fid, &offset, &count, b, i, &pos));
      free(b);
    }
    dr_assert(!dr_9p_decode_Tread(&fid, &offset, &count, buf, sizeof(buf) + 1, &pos));
  }
  {
    const uint8_t buf[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0x00, 'p', 'a', 'y', 'l', 'o', 'a', 'd' };
    uint32_t count;
    const void *restrict data;
    uint32_t pos = HEADER_OFFSET;
    dr_assert(dr_9p_decode_Rread(&count, &data, buf, sizeof(buf), &pos) &&
	      count == 7 &&
	      memcmp(data, "payload", 7) == 0 &&
	      pos == sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (i > 0) {
	memcpy(b, buf, i);
      }
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Rread(&count, &data, b, i, &pos));
      free(b);
    }
    dr_assert(!dr_9p_decode_Rread(&count, &data, buf, sizeof(buf) + 1, &pos));
  }
  {
    const uint8_t buf[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x12, 0x34, 0x56, 0x78, 0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xba, 0xbe, 0x07, 0x00, 0x00, 0x00, 'p', 'a', 'y', 'l', 'o', 'a', 'd' };
    uint32_t fid;
    uint64_t offset;
    uint32_t count;
    const void *restrict data;
    uint32_t pos = HEADER_OFFSET;
    dr_assert(dr_9p_decode_Twrite(&fid, &offset, &count, &data, buf, sizeof(buf), &pos) &&
	      fid == 0x78563412 &&
	      offset == 0xbebafecaefbeadde &&
	      count == 7 &&
	      memcmp(data, "payload", 7) == 0 &&
	      pos == sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (i > 0) {
	memcpy(b, buf, i);
      }
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Twrite(&fid, &offset, &count, &data, b, i, &pos));
      free(b);
    }
    dr_assert(!dr_9p_decode_Twrite(&fid, &offset, &count, &data, buf, sizeof(buf) + 1, &pos));
  }
  {
    const uint8_t buf[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xde, 0xad, 0xbe, 0xef };
    uint32_t count;
    uint32_t pos = HEADER_OFFSET;
    dr_assert(dr_9p_decode_Rwrite(&count, buf, sizeof(buf), &pos) &&
	      count == 0xefbeadde &&
	      pos == sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (i > 0) {
	memcpy(b, buf, i);
      }
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Rwrite(&count, b, i, &pos));
      free(b);
    }
    dr_assert(!dr_9p_decode_Rwrite(&count, buf, sizeof(buf) + 1, &pos));
  }
  {
    const uint8_t buf[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x12, 0x34, 0x56, 0x78 };
    uint32_t fid;
    uint32_t pos = HEADER_OFFSET;
    dr_assert(dr_9p_decode_Tclunk(&fid, buf, sizeof(buf), &pos) &&
	      fid == 0x78563412 &&
	      pos == sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (i > 0) {
	memcpy(b, buf, i);
      }
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Tclunk(&fid, b, i, &pos));
      free(b);
    }
    dr_assert(!dr_9p_decode_Tclunk(&fid, buf, sizeof(buf) + 1, &pos));
  }
  {
    uint32_t pos = HEADER_OFFSET;
    dr_assert(dr_9p_decode_Rclunk(7, &pos) &&
	      pos == 7);
    for (size_t i = 0; i < 7; ++i) {
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Rclunk(i, &pos));
    }
    dr_assert(!dr_9p_decode_Rclunk(7 + 1, &pos));
  }
  {
    const uint8_t buf[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x12, 0x34, 0x56, 0x78 };
    uint32_t fid;
    uint32_t pos = HEADER_OFFSET;
    dr_assert(dr_9p_decode_Tremove(&fid, buf, sizeof(buf), &pos) &&
	      fid == 0x78563412 &&
	      pos == sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (i > 0) {
	memcpy(b, buf, i);
      }
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Tremove(&fid, b, i, &pos));
      free(b);
    }
    dr_assert(!dr_9p_decode_Tremove(&fid, buf, sizeof(buf) + 1, &pos));
  }
  {
    uint32_t pos = HEADER_OFFSET;
    dr_assert(dr_9p_decode_Rremove(7, &pos) &&
	      pos == 7);
    for (size_t i = 0; i < 7; ++i) {
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Rremove(i, &pos));
    }
    dr_assert(!dr_9p_decode_Rremove(7 + 1, &pos));
  }
  {
    const uint8_t buf[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x12, 0x34, 0x56, 0x78 };
    uint32_t fid;
    uint32_t pos = HEADER_OFFSET;
    dr_assert(dr_9p_decode_Tstat(&fid, buf, sizeof(buf), &pos) &&
	      fid == 0x78563412 &&
	      pos == sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (i > 0) {
	memcpy(b, buf, i);
      }
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Tstat(&fid, b, i, &pos));
      free(b);
    }
    dr_assert(!dr_9p_decode_Tstat(&fid, buf, sizeof(buf) + 1, &pos));
  }
  {
    const uint8_t buf[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x40, 0x00, 0x3e, 0x00, 0xca, 0xfe, 0xde, 0xad, 0xbe, 0xef, 0xca, 0xef, 0xbe, 0xad, 0xde, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x0d, 0xd0, 0xfe, 0xca, 0x78, 0x56, 0x34, 0x12, 0x21, 0x43, 0x65, 0x87, 0x55, 0x34, 0x21, 0x13, 0x58, 0x23, 0x11, 0x00, 0x04, 0x00, 'f', 'i', 'l', 'e', 0x03, 0x00, 'o', 'w', 'n', 0x06, 0x00, 'o', 'g', 'r', 'o', 'u', 'p', 0x02, 0x00, 'i', 'd' };
    struct dr_9p_stat stat;
    uint32_t pos = HEADER_OFFSET;
    dr_assert(dr_9p_decode_Rstat(&stat, buf, sizeof(buf), &pos) &&
	      stat.type == 0xfeca &&
	      stat.dev == 0xefbeadde &&
	      stat.qid.type == 0xca &&
	      stat.qid.vers == 0xdeadbeef &&
	      stat.qid.path == 0x0807060504030201 &&
	      stat.mode == 0xcafed00d &&
	      stat.atime == 0x12345678 &&
	      stat.mtime == 0x87654321 &&
	      stat.length == 0x0011235813213455 &&
	      stat.name.len == 4 &&
	      memcmp(stat.name.buf, "file", 4) == 0 &&
	      stat.uid.len == 3 &&
	      memcmp(stat.uid.buf, "own", 3) == 0 &&
	      stat.gid.len == 6 &&
	      memcmp(stat.gid.buf, "ogroup", 6) == 0 &&
	      stat.muid.len == 2 &&
	      memcmp(stat.muid.buf, "id", 2) == 0 &&
	      pos == sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (i > 0) {
	memcpy(b, buf, i);
      }
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Rstat(&stat, b, i, &pos));
      free(b);
    }
    dr_assert(!dr_9p_decode_Rstat(&stat, buf, sizeof(buf) + 1, &pos));
  }
  {
    const uint8_t buf[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xde, 0xad, 0xbe, 0xef, 0x40, 0x00, 0x3e, 0x00, 0xca, 0xfe, 0xde, 0xad, 0xbe, 0xef, 0xca, 0xef, 0xbe, 0xad, 0xde, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x0d, 0xd0, 0xfe, 0xca, 0x78, 0x56, 0x34, 0x12, 0x21, 0x43, 0x65, 0x87, 0x55, 0x34, 0x21, 0x13, 0x58, 0x23, 0x11, 0x00, 0x04, 0x00, 'f', 'i', 'l', 'e', 0x03, 0x00, 'o', 'w', 'n', 0x06, 0x00, 'o', 'g', 'r', 'o', 'u', 'p', 0x02, 0x00, 'i', 'd' };
    uint32_t fid;
    struct dr_9p_stat stat;
    uint32_t pos = HEADER_OFFSET;
    dr_assert(dr_9p_decode_Twstat(&fid, &stat, buf, sizeof(buf), &pos) &&
	      fid == 0xefbeadde &&
	      stat.type == 0xfeca &&
	      stat.dev == 0xefbeadde &&
	      stat.qid.type == 0xca &&
	      stat.qid.vers == 0xdeadbeef &&
	      stat.qid.path == 0x0807060504030201 &&
	      stat.mode == 0xcafed00d &&
	      stat.atime == 0x12345678 &&
	      stat.mtime == 0x87654321 &&
	      stat.length == 0x0011235813213455 &&
	      stat.name.len == 4 &&
	      memcmp(stat.name.buf, "file", 4) == 0 &&
	      stat.uid.len == 3 &&
	      memcmp(stat.uid.buf, "own", 3) == 0 &&
	      stat.gid.len == 6 &&
	      memcmp(stat.gid.buf, "ogroup", 6) == 0 &&
	      stat.muid.len == 2 &&
	      memcmp(stat.muid.buf, "id", 2) == 0 &&
	      pos == sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (i > 0) {
	memcpy(b, buf, i);
      }
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Twstat(&fid, &stat, b, i, &pos));
      free(b);
    }
    dr_assert(!dr_9p_decode_Twstat(&fid, &stat, buf, sizeof(buf) + 1, &pos));
  }
  {
    uint32_t pos = HEADER_OFFSET;
    dr_assert(dr_9p_decode_Rwstat(7, &pos) &&
	      pos == 7);
    for (size_t i = 0; i < 7; ++i) {
      pos = HEADER_OFFSET;
      dr_assert(!dr_9p_decode_Rwstat(i, &pos));
    }
    dr_assert(!dr_9p_decode_Rwstat(7 + 1, &pos));
  }

  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    char version_9p2000[] = { '9','P','2','0','0','0' };
    const struct dr_str version = {
      .len = sizeof(version_9p2000),
      .buf = version_9p2000,
    };
    const uint8_t expected[] = { 0x13, 0x00, 0x00, 0x00, DR_TVERSION, 0x34, 0x12, 0x00, 0x20, 0x00, 0x00, 0x06, 0x00, '9', 'P', '2', '0', '0', '0' };
    dr_assert(dr_9p_encode_Tversion(buf, sizeof(buf), &pos, 0x1234, BUF_SIZE, &version) &&
	      pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    for (size_t i = 0; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      dr_assert(!dr_9p_encode_Tversion(b, i, &pos, 0x1234, BUF_SIZE, &version));
      free(b);
    }
  }
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    char version_9p2000[] = { '9','P','2','0','0','0' };
    const struct dr_str version = {
      .len = sizeof(version_9p2000),
      .buf = version_9p2000,
    };
    const uint8_t expected[] = { 0x13, 0x00, 0x00, 0x00, DR_RVERSION, 0x34, 0x12, 0x00, 0x20, 0x00, 0x00, 0x06, 0x00, '9', 'P', '2', '0', '0', '0' };
    dr_assert(dr_9p_encode_Rversion(buf, sizeof(buf), &pos, 0x1234, BUF_SIZE, &version) &&
	      pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    for (size_t i = 0; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      dr_assert(!dr_9p_encode_Rversion(b, i, &pos, 0x1234, BUF_SIZE, &version));
      free(b);
    }
  }
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    char ename_buf[] = { 'o', 'o', 'p', 's' };
    const struct dr_str ename = {
      .len = sizeof(ename_buf),
      .buf = ename_buf,
    };
    const uint8_t expected[] = { 0x0d, 0x00, 0x00, 0x00, DR_RERROR, 0x34, 0x12, 0x04, 0x00, 'o', 'o', 'p', 's' };
    dr_9p_encode_Rerror(buf, sizeof(buf), &pos, 0x1234, &ename);
    dr_assert(pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    // DR 0..8
    for (size_t i = 9; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      dr_9p_encode_Rerror(b, i, &pos, 0x1234, &ename);
      dr_assert(pos <= i);
      struct dr_str s;
      dr_assert(dr_9p_decode_Rerror(&s, b, pos, &pos) &&
		(s.len == 0 || memcmp(s.buf, ename_buf, s.len) == 0));
      free(b);
    }
  }
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    const struct dr_error err = {
      .domain = DR_ERR_ISO_C,
      .num = ENOENT,
    };
    const uint8_t expected[] = { 0x22, 0x00, 0x00, 0x00, DR_RERROR, 0x34, 0x12, 0x19, 0x00, 'N', 'o', ' ', 's', 'u', 'c', 'h', ' ', 'f', 'i', 'l', 'e', ' ', 'o', 'r', ' ', 'd', 'i', 'r', 'e', 'c', 't', 'o', 'r', 'y' };
    dr_9p_encode_Rerror_err(buf, sizeof(buf), &pos, 0x1234, &err);
    dr_assert(pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    // DR 0..8
    for (size_t i = 9; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      dr_9p_encode_Rerror_err(b, i, &pos, 0x1234, &err);
      dr_assert(pos <= i);
      struct dr_str ename;
      dr_assert(dr_9p_decode_Rerror(&ename, b, pos, &pos) &&
		(ename.len == 0 || memcmp(ename.buf, "No such file or directory", ename.len) == 0));
      free(b);
    }
  }
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    char uname_buf[] = { 'd', 'r', 'e', 'w', 'r', 'i', 'c', 'h', 'a', 'r', 'd', 's', 'o', 'n' };
    const struct dr_str uname = {
      .len = sizeof(uname_buf),
      .buf = uname_buf,
    };
    char aname_buf[] = { '/' };
    const struct dr_str aname = {
      .len = sizeof(aname_buf),
      .buf = aname_buf,
    };
    const uint8_t expected[] = { 0x22, 0x00, 0x00, 0x00, DR_TATTACH, 0x34, 0x12, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x0e, 0x00, 'd', 'r', 'e', 'w', 'r', 'i', 'c', 'h', 'a', 'r', 'd', 's', 'o', 'n', 0x01, 0x00, '/' };
    dr_assert(dr_9p_encode_Tattach(buf, sizeof(buf), &pos, 0x1234, 0x78563412, 0xf0debc9a, &uname, &aname) &&
	      pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    for (size_t i = 0; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      dr_assert(!dr_9p_encode_Tattach(b, i, &pos, 0x1234, 0x78563412, 0xf0debc9a, &uname, &aname));
      free(b);
    }
  }
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    const struct dr_file f = {
      .vers = 0xdeadbeef,
      .mode = 0xcafed00d,
    };
    const uint8_t expected[] = { 0x14, 0x00, 0x00, 0x00, DR_RATTACH, 0x34, 0x12, 0xca, 0xef, 0xbe, 0xad, 0xde, QID_PATH(&f) };
    dr_assert(dr_9p_encode_Rattach(buf, sizeof(buf), &pos, 0x1234, &f) &&
	      pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    for (size_t i = 0; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      dr_assert(!dr_9p_encode_Rattach(b, i, &pos, 0x1234, &f));
      free(b);
    }
  }
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    char name0_buf[] = { 'd', 'i', 'r' };
    const struct dr_str name0 = {
      .len = sizeof(name0_buf),
      .buf = name0_buf,
    };
    char name1_buf[] = { 'f', 'i', 'l', 'e' };
    const struct dr_str name1 = {
      .len = sizeof(name1_buf),
      .buf = name1_buf,
    };
    uint16_t nwname;
    const uint8_t expected[] = { 0x1c, 0x00, 0x00, 0x00, DR_TWALK, 0x34, 0x12, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x02, 0x00, 0x03, 0x00, 'd', 'i', 'r', 0x04, 0x00, 'f', 'i', 'l', 'e' };
    dr_assert(dr_9p_encode_Twalk_iterator(buf, sizeof(buf), &pos, 0x1234, 0x78563412, 0xf0debc9a, &nwname) &&
	      dr_9p_encode_Twalk_add(buf, sizeof(buf), &pos, &nwname, &name0) &&
	      dr_9p_encode_Twalk_add(buf, sizeof(buf), &pos, &nwname, &name1) &&
	      dr_9p_encode_Twalk_finish(buf, sizeof(buf), &pos, nwname) &&
	      pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    for (size_t i = 0; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      dr_assert(!dr_9p_encode_Twalk_iterator(b, i, &pos, 0x1234, 0x78563412, 0xf0debc9a, &nwname) ||
		!dr_9p_encode_Twalk_add(b, i, &pos, &nwname, &name0) ||
		!dr_9p_encode_Twalk_add(b, i, &pos, &nwname, &name1) ||
		!dr_9p_encode_Twalk_finish(b, i, &pos, nwname));
      free(b);
    }
  }
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    const struct dr_file f0 = {
      .vers = 0xdeadbeef,
      .mode = 0xcafed00d,
    };
    const struct dr_file f1 = {
      .vers = 0xcafed00d,
      .mode = 0xdeadbeef,
    };
    uint16_t nwqid;
    const uint8_t expected[] = { 0x23, 0x00, 0x00, 0x00, DR_RWALK, 0x34, 0x12, 0x02, 0x00, 0xca, 0xef, 0xbe, 0xad, 0xde, QID_PATH(&f0), 0xde, 0x0d, 0xd0, 0xfe, 0xca, QID_PATH(&f1) };
    dr_assert(dr_9p_encode_Rwalk_iterator(buf, sizeof(buf), &pos, 0x1234, &nwqid) &&
	      dr_9p_encode_Rwalk_add(buf, sizeof(buf), &pos, &nwqid, &f0) &&
	      dr_9p_encode_Rwalk_add(buf, sizeof(buf), &pos, &nwqid, &f1) &&
	      dr_9p_encode_Rwalk_finish(buf, sizeof(buf), &pos, nwqid) &&
	      pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    for (size_t i = 0; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      dr_assert(!dr_9p_encode_Rwalk_iterator(b, i, &pos, 0x1234, &nwqid) ||
		!dr_9p_encode_Rwalk_add(b, i, &pos, &nwqid, &f0) ||
		!dr_9p_encode_Rwalk_add(b, i, &pos, &nwqid, &f1) ||
		!dr_9p_encode_Rwalk_finish(b, i, &pos, nwqid));
      free(b);
    }
  }
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    const uint8_t expected[] = { 0x0c, 0x00, 0x00, 0x00, DR_TOPEN, 0x34, 0x12, 0x12, 0x34, 0x56, 0x78, 0xde };
    dr_assert(dr_9p_encode_Topen(buf, sizeof(buf), &pos, 0x1234, 0x78563412, 0xde) &&
	      pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    for (size_t i = 0; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      dr_assert(!dr_9p_encode_Topen(b, i, &pos, 0x1234, 0x78563412, 0xde));
      free(b);
    }
  }
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    const struct dr_file f = {
      .vers = 0xdeadbeef,
      .mode = 0xcafed00d,
    };
    const uint8_t expected[] = { 0x18, 0x00, 0x00, 0x00, DR_ROPEN, 0x34, 0x12, 0xca, 0xef, 0xbe, 0xad, 0xde, QID_PATH(&f), 0x0d, 0xd0, 0xfe, 0xca };
    dr_assert(dr_9p_encode_Ropen(buf, sizeof(buf), &pos, 0x1234, &f, 0xcafed00d) &&
	      pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    for (size_t i = 0; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      dr_assert(!dr_9p_encode_Ropen(b, i, &pos, 0x1234, &f, 0xcafed00d));
      free(b);
    }
  }
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    char name_buf[] = { 'f', 'i', 'l', 'e' };
    const struct dr_str name = {
      .len = sizeof(name_buf),
      .buf = name_buf,
    };
    const uint8_t expected[] = { 0x16, 0x00, 0x00, 0x00, DR_TCREATE, 0x34, 0x12, 0xde, 0xad, 0xbe, 0xef, 0x04, 0x00, 'f', 'i', 'l', 'e', 0xca, 0xfe, 0xd0, 0x0d, 0xfe };
    dr_assert(dr_9p_encode_Tcreate(buf, sizeof(buf), &pos, 0x1234, 0xefbeadde, &name, 0x0dd0feca, 0xfe) &&
	      pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    for (size_t i = 0; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      dr_assert(!dr_9p_encode_Tcreate(b, i, &pos, 0x1234, 0xefbeadde, &name, 0x0dd0feca, 0xfe));
      free(b);
    }
  }
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    const struct dr_file f = {
      .vers = 0xdeadbeef,
      .mode = 0xcafed00d,
    };
    const uint8_t expected[] = { 0x18, 0x00, 0x00, 0x00, DR_RCREATE, 0x34, 0x12, 0xca, 0xef, 0xbe, 0xad, 0xde, QID_PATH(&f), 0x0d, 0xd0, 0xfe, 0xca };
    dr_assert(dr_9p_encode_Rcreate(buf, sizeof(buf), &pos, 0x1234, &f, 0xcafed00d) &&
	      pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    for (size_t i = 0; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      dr_assert(!dr_9p_encode_Rcreate(b, i, &pos, 0x1234, &f, 0xcafed00d));
      free(b);
    }
  }
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    const uint8_t expected[] = { 0x17, 0x00, 0x00, 0x00, DR_TREAD, 0x34, 0x12, 0x12, 0x34, 0x56, 0x78, 0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xba, 0xbe, 0x9a, 0xbc, 0xde, 0xf0 };
    dr_assert(dr_9p_encode_Tread(buf, sizeof(buf), &pos, 0x1234, 0x78563412, 0xbebafecaefbeadde, 0xf0debc9a) &&
	      pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    for (size_t i = 0; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      dr_assert(!dr_9p_encode_Tread(b, i, &pos, 0x1234, 0x78563412, 0xbebafecaefbeadde, 0xf0debc9a));
      free(b);
    }
  }
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    const uint8_t expected[] = { 0x0f, 0x00, 0x00, 0x00, DR_RREAD, 0x34, 0x12, 0x04, 0x00, 0x00, 0x00, 0xde, 0xad, 0xbe, 0xef };
    const uint8_t data[] = { 0xde, 0xad, 0xbe, 0xef };
    dr_assert(dr_9p_encode_Rread_iterator(buf, sizeof(buf), &pos, 0x1234));
    memcpy(buf + pos, data, sizeof(data));
    dr_assert(dr_9p_encode_Rread_finish(buf, sizeof(buf), &pos, sizeof(data)) &&
	      pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    for (size_t i = 0; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (dr_9p_encode_Rread_iterator(b, i, &pos, 0x1234) &&
	  i >= pos + sizeof(data)) {
	memcpy(b + pos, data, sizeof(data));
	dr_assert(!dr_9p_encode_Rread_finish(b, i, &pos, sizeof(data)));
      }
      free(b);
    }
  }
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    const uint8_t expected[] = { 0x1b, 0x00, 0x00, 0x00, DR_TWRITE, 0x34, 0x12, 0x12, 0x34, 0x56, 0x78, 0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xba, 0xbe, 0x04, 0x00, 0x00, 0x00, 0xde, 0xad, 0xbe, 0xef };
    const uint8_t data[] = { 0xde, 0xad, 0xbe, 0xef };
    dr_assert(dr_9p_encode_Twrite_iterator(buf, sizeof(buf), &pos, 0x1234, 0x78563412, 0xbebafecaefbeadde));
    memcpy(buf + pos, data, sizeof(data));
    dr_assert(dr_9p_encode_Twrite_finish(buf, sizeof(buf), &pos, sizeof(data)) &&
	      pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    for (size_t i = 0; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      if (dr_9p_encode_Twrite_iterator(b, i, &pos, 0x1234, 0x78563412, 0xbebafecaefbeadde) &&
	  i >= pos + sizeof(data)) {
	memcpy(b + pos, data, sizeof(data));
	dr_assert(!dr_9p_encode_Twrite_finish(b, i, &pos, sizeof(data)));
      }
      free(b);
    }
  }
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    const uint8_t expected[] = { 0x0b, 0x00, 0x00, 0x00, DR_RWRITE, 0x34, 0x12, 0xef, 0xbe, 0xad, 0xde };
    dr_assert(dr_9p_encode_Rwrite(buf, sizeof(buf), &pos, 0x1234, 0xdeadbeef) &&
	      pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    for (size_t i = 0; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      dr_assert(!dr_9p_encode_Rwrite(b, i, &pos, 0x1234, 0xdeadbeef));
      free(b);
    }
  }
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    const uint8_t expected[] = { 0x0b, 0x00, 0x00, 0x00, DR_TCLUNK, 0x34, 0x12, 0xde, 0xad, 0xbe, 0xef };
    dr_assert(dr_9p_encode_Tclunk(buf, sizeof(buf), &pos, 0x1234, 0xefbeadde) &&
	      pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    for (size_t i = 0; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      dr_assert(!dr_9p_encode_Tclunk(b, i, &pos, 0x1234, 0xefbeadde));
      free(b);
    }
  }
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    const uint8_t expected[] = { 0x07, 0x00, 0x00, 0x00, DR_RCLUNK, 0x34, 0x12 };
    dr_assert(dr_9p_encode_Rclunk(buf, sizeof(buf), &pos, 0x1234) &&
	      pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    for (size_t i = 0; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      dr_assert(!dr_9p_encode_Rclunk(b, i, &pos, 0x1234));
      free(b);
    }
  }
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    const uint8_t expected[] = { 0x0b, 0x00, 0x00, 0x00, DR_TREMOVE, 0x34, 0x12, 0xde, 0xad, 0xbe, 0xef };
    dr_assert(dr_9p_encode_Tremove(buf, sizeof(buf), &pos, 0x1234, 0xefbeadde) &&
	      pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    for (size_t i = 0; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      dr_assert(!dr_9p_encode_Tremove(b, i, &pos, 0x1234, 0xefbeadde));
      free(b);
    }
  }
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    const uint8_t expected[] = { 0x07, 0x00, 0x00, 0x00, DR_RREMOVE, 0x34, 0x12 };
    dr_assert(dr_9p_encode_Rremove(buf, sizeof(buf), &pos, 0x1234) &&
	      pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    for (size_t i = 0; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      dr_assert(!dr_9p_encode_Rremove(b, i, &pos, 0x1234));
      free(b);
    }
  }
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    const uint8_t expected[] = { 0x0b, 0x00, 0x00, 0x00, DR_TSTAT, 0x34, 0x12, 0xde, 0xad, 0xbe, 0xef };
    dr_assert(dr_9p_encode_Tstat(buf, sizeof(buf), &pos, 0x1234, 0xefbeadde) &&
	      pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    for (size_t i = 0; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      dr_assert(!dr_9p_encode_Tstat(b, i, &pos, 0x1234, 0xefbeadde));
      free(b);
    }
  }
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    char gname_buf[] = { 'o', 'g', 'r', 'o', 'u', 'p' };
    struct dr_str gname = {
      .len = sizeof(gname_buf),
      .buf = gname_buf,
    };
    struct dr_group g = {
      .name = gname,
    };
    char fname_buf[] = { 'f', 'i', 'l', 'e' };
    const struct dr_file f = {
      .vers = 0xdeadbeef,
      .mode = 0xcafed00d,
      .atime = 0x12345678 * DR_NS_PER_S,
      .mtime = 0x87654321 * DR_NS_PER_S,
      .length = 0x0011235813213455,
      .name.len = sizeof(fname_buf),
      .name.buf = fname_buf,
      .uid = &u0,
      .gid = &g,
      .muid = &u1,
    };
    const uint8_t expected[] = { 0x49, 0x00, 0x00, 0x00, DR_RSTAT, 0x34, 0x12, 0x40, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xca, 0xef, 0xbe, 0xad, 0xde, QID_PATH(&f), 0x0d, 0xd0, 0xfe, 0xca, 0x78, 0x56, 0x34, 0x12, 0x21, 0x43, 0x65, 0x87, 0x55, 0x34, 0x21, 0x13, 0x58, 0x23, 0x11, 0x00, 0x04, 0x00, 'f', 'i', 'l', 'e', 0x03, 0x00, 'o', 'w', 'n', 0x06, 0x00, 'o', 'g', 'r', 'o', 'u', 'p', 0x02, 0x00, 'i', 'd' };
    dr_assert(dr_9p_encode_Rstat(buf, sizeof(buf), &pos, 0x1234, &f) &&
	      pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    for (size_t i = 0; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      dr_assert(!dr_9p_encode_Rstat(b, i, &pos, 0x1234, &f));
      free(b);
    }
  }
  // DR twstat
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    char gname_buf[] = { 'o', 'g', 'r', 'o', 'u', 'p' };
    struct dr_str gname = {
      .len = sizeof(gname_buf),
      .buf = gname_buf,
    };
    struct dr_group g = {
      .name = gname,
    };
    char fname_buf[] = { 'f', 'i', 'l', 'e' };
    const struct dr_9p_stat stat = {
      .type = 0xfeca,
      .dev = 0xefbeadde,
      .qid.type = 0xca,
      .qid.vers = 0xdeadbeef,
      .qid.path = 0x0807060504030201,
      .mode = 0xcafed00d,
      .atime = 0x12345678,
      .mtime = 0x87654321,
      .length = 0x0011235813213455,
      .name.len = sizeof(fname_buf),
      .name.buf = fname_buf,
      .uid = u0.name,
      .gid = g.name,
      .muid = u1.name,
    };
    const uint8_t expected[] = { 0x4d, 0x00, 0x00, 0x00, DR_TWSTAT, 0x34, 0x12, 0x21, 0x43, 0x65, 0x87, 0x40, 0x00, 0x3e, 0x00, 0xca, 0xfe, 0xde, 0xad, 0xbe, 0xef, 0xca, 0xef, 0xbe, 0xad, 0xde, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x0d, 0xd0, 0xfe, 0xca, 0x78, 0x56, 0x34, 0x12, 0x21, 0x43, 0x65, 0x87, 0x55, 0x34, 0x21, 0x13, 0x58, 0x23, 0x11, 0x00, 0x04, 0x00, 'f', 'i', 'l', 'e', 0x03, 0x00, 'o', 'w', 'n', 0x06, 0x00, 'o', 'g', 'r', 'o', 'u', 'p', 0x02, 0x00, 'i', 'd' };
    dr_assert(dr_9p_encode_Twstat(buf, sizeof(buf), &pos, 0x1234, 0x87654321, &stat) &&
	      pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    for (size_t i = 0; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      dr_assert(!dr_9p_encode_Twstat(b, i, &pos, 0x1234, 0x87654321, &stat));
      free(b);
    }
  }
  {
    uint8_t buf[BUF_SIZE];
    uint32_t pos;
    const uint8_t expected[] = { 0x07, 0x00, 0x00, 0x00, DR_RWSTAT, 0x34, 0x12 };
    dr_assert(dr_9p_encode_Rwstat(buf, sizeof(buf), &pos, 0x1234) &&
	      pos == sizeof(expected) &&
	      memcmp(buf, expected, sizeof(expected)) == 0);
    for (size_t i = 0; i < sizeof(expected); ++i) {
      uint8_t *restrict const b = (uint8_t *)malloc(i);
      dr_assert(!dr_9p_encode_Rwstat(b, i, &pos, 0x1234));
      free(b);
    }
  }
  printf("OK\n");
  return 0;
}
