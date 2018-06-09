// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#if !defined(DR_TYPES_COMMON_H)
#define DR_TYPES_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "list.h"

#include "dr_identify.h"
#include "dr_version.h"
#include "dr_compiler.h"

struct dr_option {
  const char *restrict name;
  int has_arg;
  int *restrict flag;
  int val;
};

struct dr_error {
  const char *restrict func;
  const char *restrict file;
  int line;
  int domain;
  int num;
};

#define DR_RESULT_DECL(TYPE, TNAME) \
  struct dr_result_##TNAME { \
    union { \
      TYPE private_value; \
      struct dr_error private_error; \
    } private_u; \
    bool private_is_err; \
  }

struct dr_result_void {
  union {
    struct dr_error private_error;
  } private_u;
  bool private_is_err;
};

#if defined(DR_OS_WINDOWS)
typedef uintptr_t dr_handle_t;
typedef int dr_socklen_t;
#else
typedef int dr_handle_t;
typedef unsigned int dr_socklen_t;
#endif

struct dr_io_vtbl;

struct dr_io {
  const struct dr_io_vtbl *restrict vtbl;
};

typedef DR_WARN_UNUSED_RESULT struct dr_result_size (*dr_io_write_fn_t)(struct dr_io *restrict const io, const void *restrict const buf, size_t count);

struct dr_io_vtbl {
  DR_WARN_UNUSED_RESULT struct dr_result_size (*read)(struct dr_io *restrict const io, void *restrict const buf, size_t count);
  dr_io_write_fn_t write;
  void (*close)(struct dr_io *restrict const io);
};

struct dr_ioserver_vtbl;

struct dr_ioserver {
  const struct dr_ioserver_vtbl *restrict vtbl;
};

struct dr_ioserver_vtbl {
  DR_WARN_UNUSED_RESULT struct dr_result_void (*accept)(struct dr_ioserver *restrict const ioserver, struct dr_io *restrict const io, size_t iolen, dr_sockaddr_t *restrict const addr, dr_socklen_t *restrict const addrlen, unsigned int flags);
  void (*close)(struct dr_ioserver *restrict const ioserver);
};

struct dr_io_ro_fixed {
  struct dr_io io;
  size_t count;
  size_t pos;
  const uint8_t *restrict buf;
};

struct dr_io_wo {
  struct dr_io io;
  size_t count;
  size_t pos;
  uint8_t *restrict buf;
};

struct dr_io_rw {
  struct dr_io io;
  size_t count;
  size_t read_pos;
  size_t write_pos;
  uint8_t *restrict buf;
};

struct dr_io_handle {
  struct dr_io io;
  dr_handle_t fd;
};

struct dr_io_handle_wo_buf {
  struct dr_io_handle ih;
  size_t count;
  size_t pos;
  uint8_t *restrict buf;
};

struct dr_io_handle_wo_buf_vtbl {
  struct dr_io_vtbl io;
  DR_WARN_UNUSED_RESULT struct dr_result_void (*flush)(struct dr_io_handle_wo_buf *restrict const ih_wo);
};

struct dr_ioserver_handle {
  struct dr_ioserver ioserver;
  dr_handle_t fd;
};

struct dr_ioserver_handle_vtbl {
  struct dr_ioserver_vtbl ioserver;
  DR_WARN_UNUSED_RESULT struct dr_result_void (*accept_handle)(struct dr_ioserver_handle *restrict const ihserver, struct dr_io_handle *restrict const ih, size_t iolen, dr_sockaddr_t *restrict const addr, dr_socklen_t *restrict const addrlen, unsigned int flags);
};

#if defined(DR_OS_WINDOWS)

struct dr_equeue {
  dr_handle_t fd;
};

struct dr_equeue_server {
  struct dr_ioserver_handle ihserver;
  dr_handle_t cfd;
  dr_overlapped_t ol;
  struct dr_equeue *restrict e;
  char buf[2*(sizeof(dr_sockaddr_t) + 16)];
  bool subscribed;
};

struct dr_equeue_client {
  struct dr_io_handle ih;
  dr_overlapped_t rol;
  dr_overlapped_t wol;
  struct dr_equeue *restrict e;
  bool subscribed;
};

#else

struct dr_equeue_handle {
  struct list_head changed_clients;
  dr_handle_t fd;
#if !defined(DR_OS_SOLARIS)
  unsigned int actual_events;
#endif
  unsigned int events;
};

struct dr_equeue {
  struct list_head changed_clients;
  dr_handle_t fd;
};

struct dr_equeue_server {
  // h must be the first object because it's assumed it's at the same offset as the client
  struct dr_equeue_handle h;
  struct dr_ioserver_handle ihserver;
  struct dr_equeue *restrict e;
};

struct dr_equeue_client {
  // h must be the first object because it's assumed it's at the same offset as the client
  struct dr_equeue_handle h;
  struct dr_io_handle ih;
  struct dr_equeue *restrict e;
};

#endif

struct dr_io_equeue_server_vtbl {
  struct dr_ioserver_handle_vtbl ihserver;
  DR_WARN_UNUSED_RESULT struct dr_result_void (*accept_equeue)(struct dr_equeue_server *restrict const s, struct dr_equeue_client *restrict const c, size_t iolen, dr_sockaddr_t *restrict const addr, dr_socklen_t *restrict const addrlen, unsigned int flags);
};

struct dr_task {
  struct dr_task_frame *restrict frame;
  void *restrict stack;
  struct list_head tasks;
  size_t alloc_size;
#if defined(DR_USE_VALGRIND)
  unsigned int valgrind_stack_id;
#endif
  bool runnable;
};

typedef void (*dr_task_start_t)(void *restrict const);

struct dr_wait {
  struct list_head waiters;
};

struct dr_sem {
  struct dr_wait wait;
  unsigned int value;
};

struct dr_str {
  char *restrict buf;
  uint16_t len;
};

struct dr_group {
  struct dr_str name;
};

struct dr_user {
  struct dr_str name;
  uint16_t group_count;
  struct dr_group *restrict groups[];
};

struct dr_file_vtbl;

struct dr_file {
  uint32_t vers;
  uint32_t mode;
  uint64_t atime;
  uint64_t mtime;
  uint64_t length;
  struct dr_str name;
  struct dr_user *restrict uid;
  struct dr_group *restrict gid;
  struct dr_user *restrict muid;
  const struct dr_file_vtbl *restrict vtbl;
};

struct dr_dir {
  struct dr_file file;
  struct dr_dir *restrict parent;
  uint16_t entry_count;
  struct dr_file *restrict entries[];
};

struct dr_fd {
  struct dr_file *restrict file;
  int mode;
};

struct dr_file_vtbl {
  DR_WARN_UNUSED_RESULT struct dr_result_uint32 (*read)(const struct dr_fd *restrict const, const uint64_t, const uint32_t, void *restrict const);
  DR_WARN_UNUSED_RESULT struct dr_result_uint32 (*write)(const struct dr_fd *restrict const, const uint64_t, const uint32_t, const void *restrict const);
};

struct dr_9p_qid {
  uint64_t path;
  uint32_t vers;
  uint8_t type;
};

struct dr_9p_stat {
  uint64_t length;
  struct dr_9p_qid qid;
  struct dr_str name;
  struct dr_str uid;
  struct dr_str gid;
  struct dr_str muid;
  uint32_t dev;
  uint32_t mode;
  uint32_t atime;
  uint32_t mtime;
  uint16_t type;
};

struct dr_print {
  char *restrict s;
  size_t n;
  size_t pos;
};

DR_RESULT_DECL(dr_handle_t, handle);
DR_RESULT_DECL(size_t, size);
DR_RESULT_DECL(uint32_t, uint32);
DR_RESULT_DECL(int64_t, int64);
DR_RESULT_DECL(unsigned int, uint);
DR_RESULT_DECL(void *restrict, voidp);
DR_RESULT_DECL(struct dr_file *restrict, file);
DR_RESULT_DECL(struct dr_fd *restrict, fd);

#endif // DR_TYPES_COMMON_H
