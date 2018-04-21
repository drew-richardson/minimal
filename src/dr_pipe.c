// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

#include <errno.h>

#if !defined(_WIN32)

#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

struct dr_result_handle dr_pipe_listen(const char *restrict const name, unsigned int flags) {
  struct sockaddr_un addr = {
    .sun_family = AF_UNIX,
  };
  const size_t name_len = strlen(name) + 1;
  if (dr_unlikely(name_len > sizeof(addr.sun_path))) {
    return DR_RESULT_ERRNUM(handle, DR_ERR_ISO_C, EINVAL);
  }
  memcpy(addr.sun_path, name, name_len);
  (void)unlink(addr.sun_path);
  dr_handle_t fd;
  {
    const struct dr_result_handle r = dr_socket(AF_UNIX, SOCK_STREAM, 0, flags);
    DR_IF_RESULT_ERR(r, err) {
      return DR_RESULT_ERROR(handle, err);
    } DR_ELIF_RESULT_OK(dr_handle_t, r, value) {
      fd = value;
    } DR_FI_RESULT;
  }
  {
    const struct dr_result_void r = dr_bind(fd, (struct sockaddr *)&addr, offsetof(struct sockaddr_un, sun_path) + name_len);
    DR_IF_RESULT_ERR(r, err) {
      dr_close(fd);
      return DR_RESULT_ERROR(handle, err);
    } DR_FI_RESULT;
  }
  {
    const struct dr_result_void r = dr_listen(fd, 16);
    DR_IF_RESULT_ERR(r, err) {
      dr_close(fd);
      return DR_RESULT_ERROR(handle, err);
    } DR_FI_RESULT;
  }
  return DR_RESULT_OK(handle, fd);
}

struct dr_result_handle dr_pipe_connect(const char *restrict const name, unsigned int flags) {
  struct sockaddr_un addr = {
    .sun_family = AF_UNIX,
  };
  const size_t name_len = strlen(name) + 1;
  if (dr_unlikely(name_len > sizeof(addr.sun_path))) {
    return DR_RESULT_ERRNUM(handle, DR_ERR_ISO_C, EINVAL);
  }
  memcpy(addr.sun_path, name, name_len);
  dr_handle_t fd;
  {
    const struct dr_result_handle r = dr_socket(AF_UNIX, SOCK_STREAM, 0, flags);
    DR_IF_RESULT_ERR(r, err) {
      return DR_RESULT_ERROR(handle, err);
    } DR_ELIF_RESULT_OK(dr_handle_t, r, value) {
      fd = value;
    } DR_FI_RESULT;
  }
  {
    const struct dr_result_void r = dr_connect(fd, (struct sockaddr *)&addr, offsetof(struct sockaddr_un, sun_path) + name_len);
    DR_IF_RESULT_ERR(r, err) {
      dr_close(fd);
      return DR_RESULT_ERROR(handle, err);
    } DR_FI_RESULT;
  }
  return DR_RESULT_OK(handle, fd);
}

#else

#include <stdio.h>
#include <windows.h>

struct dr_result_handle dr_pipe_listen(const char *restrict const name, unsigned int flags) {
  // DR Although this returns a valid named pipe, the resulting value cannot be used with dr_listen or dr_accept.
  if (dr_unlikely((flags & ~(DR_NONBLOCK | DR_CLOEXEC | DR_REUSEADDR)) != 0)) {
    return DR_RESULT_ERRNUM(handle, DR_ERR_ISO_C, EINVAL);
  }
  char buf[108 + 9]; // sizeof(sun_path) + strlen("\\\\.\\pipe\\")
  const int buf_len = snprintf(buf, sizeof(buf), "\\\\.\\pipe\\%s", name);
  if (dr_unlikely(buf_len + 1 > (int)sizeof(buf))) { // + 1 for '\0'
    return DR_RESULT_ERRNUM(handle, DR_ERR_ISO_C, EINVAL);
  }
  int mode = PIPE_ACCESS_DUPLEX;
  if ((flags & DR_NONBLOCK) != 0) {
    mode |= FILE_FLAG_OVERLAPPED;
  }
  const HANDLE fd = CreateNamedPipeA(buf, mode, PIPE_REJECT_REMOTE_CLIENTS, PIPE_UNLIMITED_INSTANCES, 1 << 16, 1 << 16, 0, NULL);
  if (dr_unlikely(fd == INVALID_HANDLE_VALUE)) {
    return DR_RESULT_GETLASTERROR(handle);
  }
  return DR_RESULT_OK(handle, (dr_handle_t)fd);
}

struct dr_result_handle dr_pipe_connect(const char *restrict const name, unsigned int flags) {
  return DR_RESULT_ERRNUM(handle, DR_ERR_ISO_C, ENOSYS);
}

#endif
