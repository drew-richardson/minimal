// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#if !defined(DR_TYPES_COMMON_H)
#define DR_TYPES_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "list.h"

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

#if defined(_WIN32)
typedef uintptr_t dr_handle_t;
typedef int dr_socklen_t;
#else
typedef int dr_handle_t;
typedef unsigned int dr_socklen_t;
#endif

struct dr_task {
  struct dr_task_frame *restrict frame;
  void *restrict stack;
  struct list_head tasks;
  size_t alloc_size;
#if defined(USE_VALGRIND)
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
  WARN_UNUSED_RESULT struct dr_result_uint32 (*read)(const struct dr_fd *restrict const, const uint64_t, const uint32_t, void *restrict const);
  WARN_UNUSED_RESULT struct dr_result_uint32 (*write)(const struct dr_fd *restrict const, const uint64_t, const uint32_t, const void *restrict const);
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

DR_RESULT_DECL(dr_handle_t, handle);
DR_RESULT_DECL(size_t, size);
DR_RESULT_DECL(uint32_t, uint32);
DR_RESULT_DECL(int64_t, int64);
DR_RESULT_DECL(unsigned int, uint);
DR_RESULT_DECL(void *restrict, voidp);
DR_RESULT_DECL(struct dr_file *restrict, file);
DR_RESULT_DECL(struct dr_fd *restrict, fd);

#endif // DR_TYPES_COMMON_H
