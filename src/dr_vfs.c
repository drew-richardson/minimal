// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

#include <errno.h>
#include <stdlib.h>

DR_WARN_UNUSED_RESULT const struct dr_group *restrict const *dr_user_get_groups(const struct dr_user *restrict const user) {
  return (const struct dr_group **)((char *)user + sizeof(*user));
}

DR_WARN_UNUSED_RESULT struct dr_file *restrict const *dr_dir_get_files(const struct dr_dir *restrict const dir) {
  return (struct dr_file **)((char *)dir + sizeof(*dir));
}

DR_WARN_UNUSED_RESULT static bool dr_is_user_member_of_group(const struct dr_user *restrict const user, const struct dr_group *restrict const group) {
  const struct dr_group *restrict const *restrict const groups = dr_user_get_groups(user);
  for (uint_fast32_t i = 0; i < user->group_count; ++i) {
    if (groups[i] == group) {
      return true;
    }
  }
  return false;
}

DR_WARN_UNUSED_RESULT static uint32_t dr_get_user_perm(const struct dr_user *restrict const user, const struct dr_file *restrict const file) {
  if (file->uid == user) {
    return (file->mode >> 6) & 0x7;
  }
  if (dr_is_user_member_of_group(user, file->gid)) {
    return (file->mode >> 3) & 0x7;
  }
  return file->mode & 0x7;
}

DR_WARN_UNUSED_RESULT static bool dr_user_has_perm_read(const struct dr_user *restrict const user, const struct dr_file *restrict const file) {
  return (dr_get_user_perm(user, file) & DR_AREAD) != 0;
}

DR_WARN_UNUSED_RESULT static bool dr_user_has_perm_write(const struct dr_user *restrict const user, const struct dr_file *restrict const file) {
  return (dr_get_user_perm(user, file) & DR_AWRITE) != 0;
}

DR_WARN_UNUSED_RESULT static bool dr_user_has_perm_exec(const struct dr_user *restrict const user, const struct dr_file *restrict const file) {
  return (dr_get_user_perm(user, file) & DR_AEXEC) != 0;
}

DR_WARN_UNUSED_RESULT static bool dr_is_dir(const struct dr_file *restrict const file) {
  return (file->mode & DR_DIR) != 0;
}

struct dr_result_file dr_vfs_walk(const struct dr_user *restrict const user, const struct dr_file *restrict const file, const struct dr_str *restrict const name) {
  if (dr_unlikely(!dr_is_dir(file))) {
    return DR_RESULT_ERRNUM(file, DR_ERR_ISO_C, ENOTDIR);
  }
  const struct dr_dir *restrict const dir = container_of_const(file, const struct dr_dir, file);
  if (dr_unlikely(!dr_user_has_perm_exec(user, file))) {
    return DR_RESULT_ERRNUM(file, DR_ERR_ISO_C, EACCES);
  }
  // DR add to vtbl to allow different impls
  struct dr_file *restrict const *restrict const files = dr_dir_get_files(dir);
  for (uint_fast32_t i = 0; i < dir->entry_count; ++i) {
    if (dr_str_eq(&files[i]->name, name)) {
      return DR_RESULT_OK(file, files[i]);
    }
  }
  if (name->len == 2 && name->buf[0] == '.' && name->buf[1] == '.') {
    return DR_RESULT_OK(file, &dir->parent->file);
  }
  return DR_RESULT_ERRNUM(file, DR_ERR_ISO_C, ENOENT);
}

struct dr_result_fd dr_vfs_open(const struct dr_user *restrict const user, struct dr_file *restrict const file, const uint8_t mode) {
  if (dr_unlikely(dr_is_dir(file) && mode != DR_OREAD)) {
    return DR_RESULT_ERRNUM(fd, DR_ERR_ISO_C, EISDIR);
  }
  const uint8_t masked_mode = mode & 0xf;
  // DR Ignoring OTRUNC and ORCLOSE
  const bool reading = masked_mode == DR_OREAD || masked_mode == DR_ORDWR;
  const bool writing = masked_mode == DR_OWRITE || masked_mode == DR_ORDWR;
  if (dr_unlikely((reading && !dr_user_has_perm_read(user, file)) ||
		  (writing && !dr_user_has_perm_write(user, file)) ||
		  (masked_mode == DR_OEXEC && !dr_user_has_perm_exec(user, file)))) {
    return DR_RESULT_ERRNUM(fd, DR_ERR_ISO_C, EACCES);
  }
  struct dr_fd *restrict const fd = (struct dr_fd *)malloc(sizeof(*fd));
  if (dr_unlikely(fd == NULL)) {
    return DR_RESULT_ERRNO(fd);
  }
  *fd = (struct dr_fd) {
    .file = file,
    .mode = (reading ? DR_AREAD : 0) | (writing ? DR_AWRITE : 0),
  };
  return DR_RESULT_OK(fd, fd);
}

struct dr_result_uint32 dr_9p_read_enosys(const struct dr_fd *restrict const fd, const uint64_t offset, const uint32_t count, void *restrict const buf) {
  (void)fd;
  (void)offset;
  (void)count;
  (void)buf;
  return DR_RESULT_ERRNUM(uint32, DR_ERR_ISO_C, ENOSYS);
}

struct dr_result_uint32 dr_vfs_read(const struct dr_fd *restrict const fd, const uint64_t offset, const uint32_t count, void *restrict const buf) {
  if (dr_unlikely((fd->mode & DR_AREAD) == 0)) {
    return DR_RESULT_ERRNUM(uint32, DR_ERR_ISO_C, EBADF);
  }
  return fd->file->vtbl->read(fd, offset, count, buf);
}

struct dr_result_uint32 dr_9p_write_enosys(const struct dr_fd *restrict const fd, const uint64_t offset, const uint32_t count, const void *restrict const buf) {
  (void)fd;
  (void)offset;
  (void)count;
  (void)buf;
  return DR_RESULT_ERRNUM(uint32, DR_ERR_ISO_C, ENOSYS);
}

struct dr_result_uint32 dr_vfs_write(const struct dr_fd *restrict const fd, const uint64_t offset, const uint32_t count, const void *restrict const buf) {
  if (dr_unlikely((fd->mode & DR_AWRITE) == 0)) {
    return DR_RESULT_ERRNUM(uint32, DR_ERR_ISO_C, EBADF);
  }
  return fd->file->vtbl->write(fd, offset, count, buf);
}

void dr_vfs_close(struct dr_fd *restrict const fd) {
  free(fd);
}
