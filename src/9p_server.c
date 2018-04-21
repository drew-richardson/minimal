// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

static bool debug;

static char dr_nobody_name[] = {'n','o','b','o','d','y'};
static char dr_group_name[] = {'u','s','e','r','s'};
static char dr_user_name[] = {'d','r','e','w','r','i','c','h','a','r','d','s','o','n'};
static char dr_file_name[] = {'w','o','r','l','d'};
static char dr_dir_name[] = {'h','e','l','l','o'};
static char dr_root_name[] = {'.'};

static struct dr_group dr_group = {
  .name.len = sizeof(dr_group_name),
  .name.buf = dr_group_name,
};

static struct dr_user dr_user = {
  .name.len = sizeof(dr_user_name),
  .name.buf = dr_user_name,
  .group_count = 1,
  .groups = { &dr_group },
};

static struct dr_user dr_nobody = {
  .name.len = sizeof(dr_nobody_name),
  .name.buf = dr_nobody_name,
};

// date -u +'%s%N'
#define DR_TIME 1511944580543223696

static struct dr_result_uint32 file_read(const struct dr_fd *restrict const fd, const uint64_t offset, const uint32_t count, void *restrict const buf);
static struct dr_result_uint32 file_write(const struct dr_fd *restrict const fd, const uint64_t offset, const uint32_t count, const void *restrict const buf);

static const struct dr_file_vtbl dr_file_vtbl = {
  .read = file_read,
  .write = file_write,
};

static struct dr_file dr_file = {
  .vers = 0,
  .mode = DR_APPEND | 0666,
  .atime = DR_TIME,
  .mtime = DR_TIME,
  .length = 0,
  .name.len = sizeof(dr_file_name),
  .name.buf = dr_file_name,
  .uid = &dr_user,
  .gid = &dr_group,
  .muid = &dr_user,
  .vtbl = &dr_file_vtbl,
};

static struct dr_dir dr_root;

static const struct dr_file_vtbl dr_dir_vtbl = {
  .read = dr_dir_read,
  .write = dr_9p_write_enosys,
};

static struct dr_dir dr_dir = {
  .file = {
    .vers = 0,
    .mode = DR_DIR | 0777,
    .atime = DR_TIME,
    .mtime = DR_TIME,
    .length = 0,
    .name.len = sizeof(dr_dir_name),
    .name.buf = dr_dir_name,
    .uid = &dr_user,
    .gid = &dr_group,
    .muid = &dr_user,
    .vtbl = &dr_dir_vtbl,
  },
  .parent = &dr_root,
  .entry_count = 1,
  .entries = { &dr_file },
};

static struct dr_dir dr_root = {
  .file = {
    .vers = 0,
    .mode = DR_DIR | 0777,
    .atime = DR_TIME,
    .mtime = DR_TIME,
    .length = 0,
    .name.len = sizeof(dr_root_name),
    .name.buf = dr_root_name,
    .uid = &dr_user,
    .gid = &dr_group,
    .muid = &dr_user,
    .vtbl = &dr_dir_vtbl,
  },
  .parent = &dr_root,
  .entry_count = 1,
  .entries = { &dr_dir.file },
};

struct dr_fid {
  struct list_head fids;
  struct dr_user *restrict user;
  union {
    struct dr_file *restrict file;
    struct dr_fd *restrict fd;
  } u;
  uint32_t id;
  uint32_t open;
};

static const char HELLO_WORLD[] = { 'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '\n' };

struct dr_result_uint32 file_read(const struct dr_fd *restrict const fd, const uint64_t offset, const uint32_t count, void *restrict const buf) {
  (void)fd;
  if (dr_unlikely(offset > sizeof(HELLO_WORLD))) {
    return DR_RESULT_ERRNUM(uint32, DR_ERR_ISO_C, EINVAL);
  }
  const uint32_t bytes = offset + count <= sizeof(HELLO_WORLD) ? count : sizeof(HELLO_WORLD) - offset;
  memcpy(buf, HELLO_WORLD + offset, bytes);
  return DR_RESULT_OK(uint32, bytes);
}

struct dr_result_uint32 file_write(const struct dr_fd *restrict const fd, const uint64_t offset, const uint32_t count, const void *restrict const buf) {
  (void)fd;
  (void)offset;
  printf("'%.*s'\n", count, (const char *)buf);
  return DR_RESULT_OK(uint32, count);
}

WARN_UNUSED_RESULT static struct dr_fid *dr_fid_get(const struct list_head *restrict const fids, const uint32_t fid) {
  struct dr_fid *restrict f;
  list_for_each_entry(f, fids, struct dr_fid, fids) {
    if (f->id == fid) {
      return f;
    }
  }
  return NULL;
}

WARN_UNUSED_RESULT static struct dr_fid *dr_fid_init(struct list_head *restrict const fids, struct dr_user *restrict const user, struct dr_file *restrict const file, const uint32_t id) {
  struct dr_fid *restrict const f = (struct dr_fid *)malloc(sizeof(*f));
  if (dr_unlikely(f == NULL)) {
    return NULL;
  }
  *f = (struct dr_fid) {
    .user = user,
    .u.file = file,
    .id = id,
    .open = false,
  };
  list_add(&f->fids, fids);
  return f;
}

static void dr_fid_destroy(struct dr_fid *restrict const f) {
  list_del(&f->fids);
  if (f->open) {
    dr_vfs_close(f->u.fd);
  }
  free(f);
}

#define DR_9P_BUF_SIZE (1<<13)

static bool dr_handle_request(struct list_head *restrict const fids, const uint8_t *restrict const tbuf, const uint32_t tsize, uint8_t *restrict const rbuf, const uint32_t rsize, uint32_t *restrict const rpos) {
  uint32_t tpos;
  uint8_t type;
  uint16_t tag;
  if (dr_unlikely(!dr_9p_decode_header(&type, &tag, tbuf, tsize, &tpos))) {
    dr_log("dr_9p_decode_header failed");
    return false;
  }
  switch (type) {
  case DR_TVERSION: {
    uint32_t msize;
    struct dr_str version;
    if (dr_unlikely(!dr_9p_decode_Tversion(&msize, &version, tbuf, tsize, &tpos))) {
      dr_log("dr_9p_decode_Tversion failed");
      return false;
    }
    if (debug) {
      printf("Tversion %" PRIu16 " %" PRIu32 " '%.*s'\n", tag, msize, version.len, version.buf);
    }
    if (msize > DR_9P_BUF_SIZE) {
      msize = DR_9P_BUF_SIZE;
    }
    // DR Set and use msize instead of sizeof(rbuf)
    char version_9p2000[] = {'9','P','2','0','0','0'};
    const struct dr_str rversion = {
      .len = sizeof(version_9p2000),
      .buf = version_9p2000,
    };
    if (dr_unlikely(!dr_9p_encode_Rversion(rbuf, rsize, rpos, tag, msize, &rversion))) {
      dr_log("dr_9p_encode_Rversion failed");
      return false;
    }
    return true;
  }
  case DR_TAUTH: {
    uint32_t afid;
    struct dr_str uname;
    struct dr_str aname;
    if (dr_unlikely(!dr_9p_decode_Tauth(&afid, &uname, &aname, tbuf, tsize, &tpos))) {
      dr_log("dr_9p_decode_Tauth failed");
      return false;
    }
    if (debug) {
      printf("Tauth %" PRIu16 " %" PRIu32 " '%.*s' '%.*s'\n", tag, afid, uname.len, uname.buf, aname.len, aname.buf);
    }
    struct dr_str ename = {
      .len = 0,
      .buf = NULL,
    };
    dr_9p_encode_Rerror(rbuf, rsize, rpos, tag, &ename);
    return true;
  }
  case DR_TATTACH: {
    uint32_t fid;
    uint32_t afid;
    struct dr_str uname;
    struct dr_str aname;
    if (dr_unlikely(!dr_9p_decode_Tattach(&fid, &afid, &uname, &aname, tbuf, tsize, &tpos))) {
      dr_log("dr_9p_decode_Tattach failed");
      return false;
    }
    if (debug) {
      printf("Tattach %" PRIu16 " %" PRIu32 " %" PRIu32 " '%.*s' '%.*s'\n", tag, fid, afid, uname.len, uname.buf, aname.len, aname.buf);
    }
    if (dr_unlikely(afid != DR_NOFID)) {
      dr_log("Afid is invalid");
      return false;
    }
    if (dr_unlikely(dr_fid_get(fids, fid) != NULL)) {
      dr_log("Fid already in use");
      return false;
    }
    const struct dr_fid *restrict const f = dr_fid_init(fids, dr_str_eq(&uname, &dr_user.name) ? &dr_user : &dr_nobody, &dr_root.file, fid);
    if (dr_unlikely(f == NULL)) {
      dr_log("dr_fid_init failed");
      return false;
    }
    if (dr_unlikely(!dr_9p_encode_Rattach(rbuf, rsize, rpos, tag, f->u.file))) {
      dr_log("dr_9p_encode_Rattach failed");
      return false;
    }
    return true;
  }
  case DR_TWALK: {
    uint32_t fid;
    uint32_t newfid;
    uint16_t nwname;
    if (dr_unlikely(!dr_9p_decode_Twalk_iterator(&fid, &newfid, &nwname, tbuf, tsize, &tpos))) {
      dr_log("dr_9p_decode_Twalk_iterator failed");
      return false;
    }
    if (debug) {
      printf("Twalk %" PRIu16 " %" PRIu32 " %" PRIu32 " %" PRIu16, tag, fid, newfid, nwname);
    }
    uint16_t nwqid;
    if (dr_unlikely(!dr_9p_encode_Rwalk_iterator(rbuf, rsize, rpos, tag, &nwqid))) {
      dr_log("dr_9p_encode_Rwalk_iterator failed");
      return false;
    }
    struct dr_fid *restrict const fidp = dr_fid_get(fids, fid);
    if (dr_unlikely(fidp == NULL)) {
      dr_log("Unable to find fid");
      return false;
    }
    if (dr_unlikely(newfid != fid && dr_fid_get(fids, newfid) != NULL)) {
      dr_log("Newfid already in use");
      return false;
    }
    struct dr_file *restrict f = fidp->open ? fidp->u.fd->file : fidp->u.file;
    for (uint_fast16_t i = 0; i < nwname; ++i) {
      struct dr_str wname;
      if (dr_unlikely(!dr_9p_decode_Twalk_advance(&wname, tbuf, tsize, &tpos))) {
	dr_log("dr_9p_decode_Twalk_advance failed");
	return false;
      }
      if (debug) {
	printf(" '%.*s'", wname.len, wname.buf);
      }
      {
	const struct dr_result_file r = dr_vfs_walk(fidp->user, f, &wname);
	DR_IF_RESULT_ERR(r, err) {
	  dr_log_error("dr_vfs_walk failed", err);
	  if (i > 0) {
	    break;
	  }
	  dr_9p_encode_Rerror_err(rbuf, rsize, rpos, tag, err);
	  return true;
	} DR_ELIF_RESULT_OK(struct dr_file *restrict, r, value) {
	  f = value;
	} DR_FI_RESULT;
      }
      if (dr_unlikely(!dr_9p_encode_Rwalk_add(rbuf, rsize, rpos, &nwqid, f))) {
	dr_log("dr_9p_encode_Rwalk_add failed");
	return false;
      }
    }
    if (debug) {
      printf("\n");
    }
    if (nwname == nwqid) {
      if (fid == newfid) {
	fidp->u.file = f;
      } else if (dr_unlikely(dr_fid_init(fids, fidp->user, f, newfid) == NULL)) {
	dr_log("dr_fid_init failed");
	return false;
      }
    }
    if (dr_unlikely(!dr_9p_decode_Twalk_finish(tsize, tpos))) {
      dr_log("dr_9p_decode_Twalk_finish failed");
      return false;
    }
    if (dr_unlikely(!dr_9p_encode_Rwalk_finish(rbuf, rsize, rpos, nwqid))) {
      dr_log("dr_9p_encode_Rwalk_finish failed");
      return false;
    }
    return true;
  }
  case DR_TOPEN: {
    uint32_t fid;
    uint8_t mode;
    if (dr_unlikely(!dr_9p_decode_Topen(&fid, &mode, tbuf, tsize, &tpos))) {
      dr_log("dr_9p_decode_Topen failed");
      return false;
    }
    if (debug) {
      printf("Topen %" PRIu16 " %" PRIu32 " %" PRIu8 "\n", tag, fid, mode);
    }
    struct dr_fid *restrict const fidp = dr_fid_get(fids, fid);
    if (dr_unlikely(fidp == NULL)) {
      dr_log("Unable to find fid");
      return false;
    }
    if (dr_unlikely(fidp->open)) {
      dr_log("Fid is open");
      return false;
    }
    struct dr_fd *restrict fd;
    {
      const struct dr_result_fd r = dr_vfs_open(fidp->user, fidp->u.file, mode);
      DR_IF_RESULT_ERR(r, err) {
	dr_log_error("dr_vfs_open failed", err);
	dr_9p_encode_Rerror_err(rbuf, rsize, rpos, tag, err);
	return true;
      } DR_ELIF_RESULT_OK(struct dr_fd *restrict, r, value) {
	fd = value;
      } DR_FI_RESULT;
    }
    fidp->open = true;
    fidp->u.fd = fd;
    if (dr_unlikely(!dr_9p_encode_Ropen(rbuf, rsize, rpos, tag, fd->file, 0))) {
      dr_log("dr_9p_encode_Ropen failed");
      return false;
    }
    return true;
  }
  case DR_TCREATE: {
    uint32_t fid;
    struct dr_str name;
    uint32_t perm;
    uint8_t mode;
    if (dr_unlikely(!dr_9p_decode_Tcreate(&fid, &name, &perm, &mode, tbuf, tsize, &tpos))) {
      dr_log("dr_9p_decode_Tcreate failed");
      return false;
    }
    if (debug) {
      printf("Tcreate %" PRIu16 " %" PRIu32 " '%.*s' %" PRIu32 " %" PRIu8 "\n", tag, fid, name.len, name.buf, perm, mode);
    }
    const struct dr_error err = {
      .domain = DR_ERR_ISO_C,
      .num = EACCES,
    };
    dr_9p_encode_Rerror_err(rbuf, rsize, rpos, tag, &err);
    return true;
  }
  case DR_TREAD: {
    uint32_t fid;
    uint64_t offset;
    uint32_t count;
    if (dr_unlikely(!dr_9p_decode_Tread(&fid, &offset, &count, tbuf, tsize, &tpos))) {
      dr_log("dr_9p_decode_Tread failed");
      return false;
    }
    if (debug) {
      printf("Tread %" PRIu16 " %" PRIu32 " %" PRIu64 " %" PRIu32 "\n", tag, fid, offset, count);
    }
    struct dr_fid *restrict const fidp = dr_fid_get(fids, fid);
    if (dr_unlikely(fidp == NULL)) {
      dr_log("Unable to find fid");
      return false;
    }
    if (dr_unlikely(!fidp->open)) {
      dr_log("Fid is not open");
      return false;
    }
    if (dr_unlikely(!dr_9p_encode_Rread_iterator(rbuf, rsize, rpos, tag))) {
      dr_log("dr_9p_decode_Rread_iterator failed");
      return false;
    }
    if (count > rsize - *rpos) {
      // DR Is this the proper logic?
      count = rsize - *rpos;
    }
    uint32_t bytes_read;
    {
      const struct dr_result_uint32 r = dr_vfs_read(fidp->u.fd, offset, count, rbuf + *rpos);
      DR_IF_RESULT_ERR(r, err) {
	dr_log_error("dr_vfs_read failed", err);
	dr_9p_encode_Rerror_err(rbuf, rsize, rpos, tag, err);
	return true;
      } DR_ELIF_RESULT_OK(uint32_t, r, value) {
	bytes_read = value;
      } DR_FI_RESULT;
    }
    *rpos += bytes_read;
    if (dr_unlikely(!dr_9p_encode_Rread_finish(rbuf, rsize, rpos, bytes_read))) {
      dr_log("dr_9p_decode_Rread_finish failed");
      return false;
    }
    return true;
  }
  case DR_TWRITE: {
    uint32_t fid;
    uint64_t offset;
    uint32_t count;
    const void *restrict data;
    if (dr_unlikely(!dr_9p_decode_Twrite(&fid, &offset, &count, &data, tbuf, tsize, &tpos))) {
      dr_log("dr_9p_decode_Twrite failed");
      return false;
    }
    if (debug) {
      printf("Twrite %" PRIu16 " %" PRIu32 " %" PRIu64 " %" PRIu32 "\n", tag, fid, offset, count);
    }
    struct dr_fid *restrict const fidp = dr_fid_get(fids, fid);
    if (dr_unlikely(fidp == NULL)) {
      dr_log("Unable to find fid");
      return false;
    }
    if (dr_unlikely(!fidp->open)) {
      dr_log("Fid is not open");
      return false;
    }
    uint32_t bytes_written;
    {
      const struct dr_result_uint32 r = dr_vfs_write(fidp->u.fd, offset, count, data);
      DR_IF_RESULT_ERR(r, err) {
	dr_log_error("dr_vfs_write failed", err);
	dr_9p_encode_Rerror_err(rbuf, rsize, rpos, tag, err);
	return true;
      } DR_ELIF_RESULT_OK(uint32_t, r, value) {
	bytes_written = value;
      } DR_FI_RESULT;
    }
    if (dr_unlikely(!dr_9p_encode_Rwrite(rbuf, rsize, rpos, tag, bytes_written))) {
      dr_log("dr_9p_encode_Rwrite failed");
      return false;
    }
    return true;
  }
  case DR_TCLUNK: {
    uint32_t fid;
    if (dr_unlikely(!dr_9p_decode_Tclunk(&fid, tbuf, tsize, &tpos))) {
      dr_log("dr_9p_decode_Tclunk failed");
      return false;
    }
    if (debug) {
      printf("Tclunk %" PRIu16 " %" PRIu32 "\n", tag, fid);
    }
    struct dr_fid *restrict const f = dr_fid_get(fids, fid);
    if (dr_unlikely(f == NULL)) {
      dr_log("dr_fid_get failed");
      return false;
    }
    dr_fid_destroy(f);
    if (dr_unlikely(!dr_9p_encode_Rclunk(rbuf, rsize, rpos, tag))) {
      dr_log("dr_9p_encode_Rclunk failed");
      return false;
    }
    return true;
  }
  case DR_TREMOVE: {
    uint32_t fid;
    if (dr_unlikely(!dr_9p_decode_Tremove(&fid, tbuf, tsize, &tpos))) {
      dr_log("dr_9p_decode_Tremove failed");
      return false;
    }
    if (debug) {
      printf("Tremove %" PRIu16 " %" PRIu32 "\n", tag, fid);
    }
    struct dr_fid *restrict const f = dr_fid_get(fids, fid);
    if (dr_unlikely(f == NULL)) {
      dr_log("dr_fid_get failed");
      return false;
    }
    dr_fid_destroy(f);
    const struct dr_error err = {
      .domain = DR_ERR_ISO_C,
      .num = EACCES,
    };
    dr_9p_encode_Rerror_err(rbuf, rsize, rpos, tag, &err);
    return true;
  }
  case DR_TSTAT: {
    uint32_t fid;
    if (dr_unlikely(!dr_9p_decode_Tstat(&fid, tbuf, tsize, &tpos))) {
      dr_log("dr_9p_decode_Tstat failed");
      return false;
    }
    if (debug) {
      printf("Tstat %" PRIu16 " %" PRIu32 "\n", tag, fid);
    }
    struct dr_fid *restrict const fidp = dr_fid_get(fids, fid);
    if (dr_unlikely(fidp == NULL)) {
      dr_log("dr_fid_get failed");
      return false;
    }
    if (dr_unlikely(!dr_9p_encode_Rstat(rbuf, rsize, rpos, tag, fidp->open ? fidp->u.fd->file : fidp->u.file))) {
      dr_log("dr_9p_encode_Rstat failed");
      return false;
    }
    return true;
  }
  case DR_TWSTAT: {
    uint32_t fid;
    struct dr_9p_stat stat;
    if (dr_unlikely(!dr_9p_decode_Twstat(&fid, &stat, tbuf, tsize, &tpos))) {
      dr_log("dr_9p_decode_Twstat failed");
      return false;
    }
    if (debug) {
      printf("Twstat %" PRIu16 " %" PRIu32 " %" PRIu16 " %" PRIu32 " %" PRIu8 " %" PRIu32 " %" PRIu64 " %" PRIu32 " %" PRIu32 " %" PRIu32 " %" PRIu64 " '%.*s' '%.*s' '%.*s' '%.*s'\n", tag, fid, stat.type, stat.dev, stat.qid.type, stat.qid.vers, stat.qid.path, stat.mode, stat.atime, stat.mtime, stat.length, stat.name.len, stat.name.buf, stat.uid.len, stat.uid.buf, stat.gid.len, stat.gid.buf, stat.muid.len, stat.muid.buf);
    }
    const struct dr_error err = {
      .domain = DR_ERR_ISO_C,
      .num = EACCES,
    };
    dr_9p_encode_Rerror_err(rbuf, rsize, rpos, tag, &err);
    return true;
  }
  default: {
    dr_log("Unrecognized message");
    const struct dr_error err = {
      .domain = DR_ERR_ISO_C,
      .num = ENOSYS,
    };
    dr_9p_encode_Rerror_err(rbuf, rsize, rpos, tag, &err);
    return true;
  }
  }
}

static const size_t STACK_SIZE = 1<<16;

struct client {
  struct list_head clients;
  struct dr_task task;
  struct dr_equeue_client c;
  struct list_head fids;
};

static struct list_head clients;
static struct dr_equeue equeue;
static struct dr_equeue_server server;

static void client_func(void *restrict const arg);

static WARN_UNUSED_RESULT struct dr_result_void client_init(dr_handle_t fd) {
  struct client *restrict const c = (struct client *)malloc(sizeof(*c));
  if (c == NULL) {
    return DR_RESULT_ERRNO_VOID();
  }
  *c = (struct client) {
    .fids = LIST_HEAD_INIT(c->fids),
  };
  dr_equeue_client_init(&c->c, fd);
  {
    const struct dr_result_void r = dr_task_create(&c->task, STACK_SIZE, client_func, c);
    DR_IF_RESULT_ERR(r, err) {
      dr_equeue_client_destroy(&c->c);
      free(c);
      return DR_RESULT_ERROR_VOID(err);
    } DR_FI_RESULT;
  }
  list_add_tail(&c->clients, &clients);
  return DR_RESULT_OK_VOID();
}

static void client_destroy(struct client *restrict const c) {
  dr_log("Closing client");
  list_del(&c->clients);
  {
    struct dr_fid *restrict f;
    struct dr_fid *restrict n;
    list_for_each_entry_safe(f, n, &c->fids, struct dr_fid, fids) {
      dr_fid_destroy(f);
    }
  }
  dr_task_destroy(&c->task);
  dr_equeue_client_destroy(&c->c);
  free(c);
}

static void client_func(void *restrict const arg) {
  struct client *restrict const c = (struct client *)arg;
  while (true) {
    uint8_t tbuf[DR_9P_BUF_SIZE];
    size_t bytes;
    {
      const struct dr_result_size r = dr_equeue_read(&equeue, &c->c, tbuf, sizeof(tbuf));
      DR_IF_RESULT_ERR(r, err) {
	dr_log_error("dr_equeue_read failed", err);
	break;
      } DR_ELIF_RESULT_OK(size_t, r, value) {
	bytes = value;
      } DR_FI_RESULT;
    }
    if (bytes == 0) {
      break;
    }
    if (bytes < sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint16_t)) {
      dr_log("Short read");
      break;
    }
    const uint32_t tsize = dr_decode_uint32(tbuf);
    if (bytes != tsize) {
      dr_log("Short read");
      break;
    }
    uint8_t rbuf[DR_9P_BUF_SIZE];
    uint32_t rpos;
    if (!dr_handle_request(&c->fids, tbuf, tsize, rbuf, sizeof(rbuf), &rpos)) {
      break;
    }
    {
      const struct dr_result_size r = dr_equeue_write(&equeue, &c->c, rbuf, rpos);
      DR_IF_RESULT_ERR(r, err) {
	dr_log_error("dr_equeue_write failed", err);
	break;
      } DR_ELIF_RESULT_OK(size_t, r, value) {
	bytes = value;
      } DR_FI_RESULT;
    }
    if (bytes != rpos) {
      dr_log("Short write");
      break;
    }
  }
  dr_task_exit(c, (void (*)(void *restrict const))client_destroy);
}

static void server_func(void *restrict const arg) {
  char *restrict const port = (char *)arg;
  {
    dr_handle_t sfd;
    {
      const struct dr_result_handle r = dr_sock_listen(NULL, port, DR_CLOEXEC | DR_NONBLOCK | DR_REUSEADDR);
      //const struct dr_result_handle r = dr_pipe_listen("/tmp/9p_server", DR_CLOEXEC | DR_NONBLOCK | DR_REUSEADDR);
      DR_IF_RESULT_ERR(r, err) {
	dr_log_error("dr_sock_listen failed", err);
	//dr_log_error("dr_pipe_listen failed", err);
	goto fail;
      } DR_ELIF_RESULT_OK(dr_handle_t, r, value) {
	sfd = value;
      } DR_FI_RESULT;
    }
    dr_equeue_server_init(&server, sfd);
  }
  while (true) {
    dr_handle_t cfd;
    {
      const struct dr_result_handle r = dr_equeue_accept(&equeue, &server);
      DR_IF_RESULT_ERR(r, err) {
	dr_log_error("dr_equeue_accept failed", err);
	goto fail_equeue_server_destroy;
      } DR_ELIF_RESULT_OK(dr_handle_t, r, value) {
	cfd = value;
      } DR_FI_RESULT;
    }
    dr_log("Accepted client");
    {
      const struct dr_result_void r = client_init(cfd);
      DR_IF_RESULT_ERR(r, err) {
	dr_log_error("client_init failed", err);
	goto fail_equeue_server_destroy;
      } DR_FI_RESULT;
    }
  }
 fail_equeue_server_destroy:
  dr_equeue_server_destroy(&server);
 fail:
  return;
}

WARN_UNUSED_RESULT static int print_version(void) {
  char buf[256];
  dr_get_version_long(buf, sizeof(buf));
  printf("9p_server %s\n", buf);
  return 0;
}

WARN_UNUSED_RESULT static int print_usage(void) {
  printf("Usage: 9p_server [OPTIONS]...\n"
	 "\n"
	 "Options:\n"
	 "  -p, --port     TCP/IP port name to connect to\n"
	 "  -d, --debug    Print received messages\n"
	 "  -v, --version  Print version information\n"
	 "  -h, --help     Print this help\n");
  return -1;
}

int main(int argc, char *argv[]) {
  int result = -1;
  char *restrict port = 0;
  {
    static struct dr_option longopts[] = {
      {"port", 1, 0, 'p'},
      {"debug", 0, 0, 'd'},
      {"version", 0, 0, 'v'},
      {"help", 0, 0, 'h'},
      {0, 0, 0, 0},
    };
    dr_optind = 0;
    while (true) {
      int opt = dr_getopt_long(argc, argv, "+p:dvh", longopts, NULL);
      if (opt == -1) {
	break;
      }
      switch (opt) {
      case 'p':
	port = dr_optarg;
	break;
      case 'd':
	debug = true;
	break;
      case 'v':
	return print_version();
      default:
      case 'h':
	return print_usage();
      }
    }
  }
  if (port == NULL) {
    return print_usage();
  }
  {
    char buf[256];
    int written = snprintf(buf, sizeof(buf), "9p_server ");
    dr_get_version_long(buf + written, sizeof(buf) - written);
    dr_log(buf);
  }
  INIT_LIST_HEAD(&clients);
  {
    const struct dr_result_void r = dr_socket_startup();
    DR_IF_RESULT_ERR(r, err) {
      dr_log_error("dr_socket_startup failed", err);
      goto fail;
    } DR_FI_RESULT;
  }
  {
    const struct dr_result_void r = dr_equeue_init(&equeue);
    DR_IF_RESULT_ERR(r, err) {
      dr_log_error("dr_equeue_init failed", err);
      goto fail;
    } DR_FI_RESULT;
  }
  struct dr_task server_task;
  {
    const struct dr_result_void r = dr_task_create(&server_task, STACK_SIZE, server_func, port);
    DR_IF_RESULT_ERR(r, err) {
      dr_log_error("dr_task_create failed", err);
      goto fail_equeue_destroy;
    } DR_FI_RESULT;
  }
  // Switch to allow server_func to run for the first time
  dr_schedule(true);
  dr_log("Listening for clients");
  while (true) {
    struct dr_event events[16];
    unsigned int count;
    {
      const struct dr_result_uint r = dr_equeue_dequeue(&equeue, events, sizeof(events));
      DR_IF_RESULT_ERR(r, err) {
	dr_log_error("dr_equeue_dequeue failed", err);
	goto done;
      } DR_ELIF_RESULT_OK(unsigned int, r, value) {
	count = value;
      } DR_FI_RESULT;
    }
    if (count == 0) {
      continue;
    }
    for (unsigned int i = 0; i < count; ++i) {
      void *restrict const key = dr_event_key(events, i);
      if (key == &server) {
	dr_task_runnable(&server_task);
      } else {
	struct client *restrict const c = container_of(key, struct client, c);
	dr_task_runnable(&c->task);
      }
    }
    dr_schedule(true);
  }
 done:
  {
    struct client *restrict c;
    struct client *restrict n;
    list_for_each_entry_safe(c, n, &clients, struct client, clients) {
      client_destroy(c);
    }
  }
  dr_task_destroy(&server_task);
 fail_equeue_destroy:
  dr_equeue_destroy(&equeue);
 fail:
  return result;
}
