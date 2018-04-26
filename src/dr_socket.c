// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"
#include "dr_io_internal.h"

#include <errno.h>

#if defined(_WIN32)

#include <winsock2.h>

#include <windows.h>
#include <ws2tcpip.h>

#else

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#endif

struct dr_result_void dr_socket_startup(void) {
#if defined(_WIN32)
  WSADATA wsaData;
  if (dr_unlikely(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)) {
    return DR_RESULT_WSAGETLASTERROR_VOID();
  }
#endif
  return DR_RESULT_OK_VOID();
}

struct dr_result_handle dr_socket(int domain, int type, int protocol, unsigned int flags) {
  if (dr_unlikely((flags & ~(DR_NONBLOCK | DR_CLOEXEC | DR_REUSEADDR)) != 0)) {
    return DR_RESULT_ERRNUM(handle, DR_ERR_ISO_C, EINVAL);
  }

#if defined(_WIN32)
  DWORD wsa_flags = 0;
  if ((flags & DR_NONBLOCK) != 0) {
    wsa_flags |= WSA_FLAG_OVERLAPPED;
  }
  const dr_handle_t result = WSASocketW(domain, type, protocol, NULL, 0, wsa_flags);
  if (dr_unlikely(result == INVALID_SOCKET)) {
    return DR_RESULT_WSAGETLASTERROR(handle);
  }
#else
#if defined(SOCK_NONBLOCK)
  if ((flags & DR_NONBLOCK) != 0) {
    type |= SOCK_NONBLOCK;
  }
#endif
#if defined(SOCK_CLOEXEC)
  if ((flags & DR_CLOEXEC) != 0) {
    type |= SOCK_CLOEXEC;
  }
#endif
  const dr_handle_t result = socket(domain, type, protocol);
  if (dr_unlikely(result < 0)) {
    return DR_RESULT_ERRNO(handle);
  }
#endif

#if !defined(_WIN32)
#if !defined(SOCK_NONBLOCK)
  if ((flags & DR_NONBLOCK) != 0) {
    const int sf = fcntl(result, F_GETFL); // DR Merge duplicate fcntl code?
    if (dr_unlikely(sf < 0 || fcntl(result, F_SETFL, sf | O_NONBLOCK) != 0)) {
      const int errnum = errno;
      dr_close(result);
      return DR_RESULT_ERRNUM(handle, DR_ERR_ISO_C, errnum);
    }
  }
#endif

#if !defined(SOCK_CLOEXEC)
  if ((flags & DR_CLOEXEC) != 0) {
    const int df = fcntl(result, F_GETFD); // DR Merge duplicate fcntl code?
    if (dr_unlikely(df < 0 || fcntl(result, F_SETFD, df | FD_CLOEXEC) != 0)) {
      const int errnum = errno;
      dr_close(result);
      return DR_RESULT_ERRNUM(handle, DR_ERR_ISO_C, errnum);
    }
  }
#endif
#endif

  if ((flags & DR_REUSEADDR) != 0) {
    const int on = 1;
#if defined(_WIN32)
    if (dr_unlikely(setsockopt((SOCKET)result, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on)) != 0)) {
      const int errnum = WSAGetLastError();
      dr_close(result);
      return DR_RESULT_ERRNUM(handle, DR_ERR_WIN, errnum);
    }
#else
    if (dr_unlikely(setsockopt(result, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on)) != 0)) {
      const int errnum = errno;
      dr_close(result);
      return DR_RESULT_ERRNUM(handle, DR_ERR_ISO_C, errnum);
    }
#endif
  }

  return DR_RESULT_OK(handle, result);
}

struct dr_result_void dr_ioserver_sock_accept_handle(struct dr_ioserver_handle *restrict const ihserver, struct dr_io_handle *restrict const ih, size_t iolen, dr_sockaddr_t *restrict const addr, dr_socklen_t *restrict const addrlen, unsigned int flags) {
  if (dr_unlikely(sizeof(*ih) < iolen)) {
    return DR_RESULT_ERRNUM_VOID(DR_ERR_ISO_C, ENOMEM);
  }
  if (dr_unlikely((flags & ~(DR_NONBLOCK | DR_CLOEXEC)) != 0)) {
    return DR_RESULT_ERRNUM_VOID(DR_ERR_ISO_C, EINVAL);
  }

#if defined(DR_HAS_ACCEPT4)
  unsigned int f = 0;
  if ((flags & DR_NONBLOCK) != 0) {
    f |= SOCK_NONBLOCK;
  }
  if ((flags & DR_CLOEXEC) != 0) {
    f |= SOCK_CLOEXEC;
  }
  const dr_handle_t result = accept4(ihserver->fd, (struct sockaddr *)addr, addrlen, f);
  if (dr_unlikely(result < 0)) {
    return DR_RESULT_ERRNO_VOID();
  }
  dr_io_handle_init(ih, result);
  return DR_RESULT_OK_VOID();

#else
  const dr_handle_t result = accept(ihserver->fd, (struct sockaddr *)addr, addrlen);
#if defined(_WIN32)
  if (dr_unlikely(result == INVALID_SOCKET)) {
    return DR_RESULT_WSAGETLASTERROR_VOID();
  }
#else
  if (dr_unlikely(result < 0)) {
    return DR_RESULT_ERRNO_VOID();
  }
#endif

#if !defined(_WIN32)
  // DR How to set DR_NONBLOCK on windows?
  if ((flags & DR_NONBLOCK) != 0) {
    const int sf = fcntl(result, F_GETFL); // DR Merge duplicate fcntl code?
    if (dr_unlikely(sf < 0 || fcntl(result, F_SETFL, sf | O_NONBLOCK) != 0)) {
      const int errnum = errno;
      dr_close(result);
      return DR_RESULT_ERRNUM_VOID(DR_ERR_ISO_C, errnum);
    }
  }

  if ((flags & DR_CLOEXEC) != 0) {
    const int df = fcntl(result, F_GETFD); // DR Merge duplicate fcntl code?
    if (dr_unlikely(df < 0 || fcntl(result, F_SETFD, df | FD_CLOEXEC) != 0)) {
      const int errnum = errno;
      dr_close(result);
      return DR_RESULT_ERRNUM_VOID(DR_ERR_ISO_C, errnum);
    }
  }
#endif

  dr_io_handle_init(ih, result);
  return DR_RESULT_OK_VOID();
#endif
}

DR_WARN_UNUSED_RESULT static struct dr_result_void dr_ioserver_sock_accept(struct dr_ioserver *restrict const ioserver, struct dr_io *restrict const io, size_t iolen, dr_sockaddr_t *restrict const addr, dr_socklen_t *restrict const addrlen, unsigned int flags) {
  struct dr_ioserver_handle *restrict const ihserver = container_of(ioserver, struct dr_ioserver_handle, ioserver);
  struct dr_io_handle *restrict const ih = container_of(io, struct dr_io_handle, io);
  dr_assert((uintptr_t)io == (uintptr_t)ih);
  return dr_ioserver_sock_accept_handle(ihserver, ih, iolen, addr, addrlen, flags);
}

static const struct dr_ioserver_handle_vtbl dr_ioserver_sock_vtbl = {
  .ioserver.accept = dr_ioserver_sock_accept,
  .ioserver.close = dr_ioserver_handle_close,
  .accept_handle = dr_ioserver_sock_accept_handle,
};

void dr_ioserver_sock_init(struct dr_ioserver_handle *restrict const ihserver, dr_handle_t fd) {
  *ihserver = (struct dr_ioserver_handle) {
    .ioserver.vtbl = &dr_ioserver_sock_vtbl.ioserver,
    .fd = fd,
  };
}

struct dr_result_void dr_bind(dr_handle_t sockfd, const dr_sockaddr_t *restrict const addr, dr_socklen_t addrlen) {
  if (dr_unlikely(bind(sockfd, (const struct sockaddr *)addr, addrlen) != 0)) {
#if defined(_WIN32)
    return DR_RESULT_WSAGETLASTERROR_VOID();
#else
    return DR_RESULT_ERRNO_VOID();
#endif
  }
  return DR_RESULT_OK_VOID();
}

struct dr_result_void dr_connect(dr_handle_t sockfd, const dr_sockaddr_t *restrict const addr, dr_socklen_t addrlen) {
  if (dr_unlikely(connect(sockfd, (const struct sockaddr *)addr, addrlen) != 0)) {
#if defined(_WIN32)
    return DR_RESULT_WSAGETLASTERROR_VOID();
#else
    return DR_RESULT_ERRNO_VOID();
#endif
  }
  return DR_RESULT_OK_VOID();
}

struct dr_result_void dr_listen(dr_handle_t sockfd, int backlog) {
  if (dr_unlikely(listen(sockfd, backlog) != 0)) {
#if defined(_WIN32)
    return DR_RESULT_WSAGETLASTERROR_VOID();
#else
    return DR_RESULT_ERRNO_VOID();
#endif
  }
  return DR_RESULT_OK_VOID();
}

struct dr_result_void dr_sock_connect(struct dr_io_handle *restrict const ih, const char *restrict const hostname, const char *restrict const port, unsigned int flags) {
  struct addrinfo hints = {
    .ai_family = AF_UNSPEC,
    .ai_socktype = SOCK_STREAM,
    .ai_protocol = 0,
    .ai_flags = 0,
  };
  struct addrinfo *res;
  {
    const int errnum = getaddrinfo(hostname, port, &hints, &res);
    if (dr_unlikely(errnum != 0)) {
      return DR_RESULT_ERRNUM_VOID(DR_ERR_GAI, errnum);
    }
  }

  dr_handle_t fd;
  struct dr_error last_error = {
    .line = 0,
  };
  struct addrinfo *restrict ai;
  for (ai = res; ai != NULL; ai = ai->ai_next) {
    {
      const struct dr_result_handle r = dr_socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol, flags);
      DR_IF_RESULT_ERR(r, err) {
	last_error = *err;
	continue;
      } DR_ELIF_RESULT_OK(dr_handle_t, r, value) {
	fd = value;
      } DR_FI_RESULT;
    }
    {
      const struct dr_result_void r = dr_connect(fd, (dr_sockaddr_t *)ai->ai_addr, ai->ai_addrlen);
      DR_IF_RESULT_ERR(r, err) {
	last_error = *err;
      } DR_ELIF_RESULT_OK_VOID(r) {
	break;
      } DR_FI_RESULT;
    }
    dr_close(fd);
  }

  freeaddrinfo(res);
  if (dr_unlikely(ai == NULL)) {
    return DR_RESULT_ERROR_VOID(&last_error);
  }
  dr_io_handle_init(ih, fd);
  return DR_RESULT_OK_VOID();
}

struct dr_result_void dr_sock_listen(struct dr_ioserver_handle *restrict const ihserver, const char *restrict const hostname, const char *restrict const port, unsigned int flags) {
  const struct addrinfo hints = {
    .ai_family = AF_UNSPEC,
    .ai_socktype = SOCK_STREAM,
    .ai_protocol = 0,
    .ai_flags = AI_PASSIVE,
  };
  struct addrinfo *res;
  {
    const int errnum = getaddrinfo(hostname, port, &hints, &res);
    if (dr_unlikely(errnum != 0)) {
      return DR_RESULT_ERRNUM_VOID(DR_ERR_GAI, errnum);
    }
  }

  dr_handle_t fd;
  struct dr_error last_error = {
    .line = 0,
  };
  struct addrinfo *restrict ai;
  for (ai = res; ai != NULL; ai = ai->ai_next) {
    {
      const struct dr_result_handle r = dr_socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol, flags);
      DR_IF_RESULT_ERR(r, err) {
	last_error = *err;
	continue;
      } DR_ELIF_RESULT_OK(dr_handle_t, r, value) {
	fd = value;
      } DR_FI_RESULT;
    }
    {
      const struct dr_result_void r = dr_bind(fd, (dr_sockaddr_t *)ai->ai_addr, ai->ai_addrlen);
      DR_IF_RESULT_ERR(r, err) {
	last_error = *err;
      } DR_ELIF_RESULT_OK_VOID(r) {
	break;
      } DR_FI_RESULT;
    }
    dr_close(fd);
  }

  freeaddrinfo(res);
  if (dr_unlikely(ai == NULL)) {
    return DR_RESULT_ERROR_VOID(&last_error);
  }

  {
    const struct dr_result_void r = dr_listen(fd, 16);
    DR_IF_RESULT_ERR(r, err) {
      dr_close(fd);
      return DR_RESULT_ERROR_VOID(err);
    } DR_FI_RESULT;
  }

  dr_ioserver_sock_init(ihserver, fd);
  return DR_RESULT_OK_VOID();
}
