// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

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

struct dr_result_handle dr_accept(dr_handle_t sockfd, struct sockaddr *restrict const addr, dr_socklen_t *restrict const addrlen, unsigned int flags) {
  if (dr_unlikely((flags & ~(DR_NONBLOCK | DR_CLOEXEC)) != 0)) {
    return DR_RESULT_ERRNUM(handle, DR_ERR_ISO_C, EINVAL);
  }

#if defined(HAS_ACCEPT4)
  unsigned int f = 0;
  if ((flags & DR_NONBLOCK) != 0) {
    f |= SOCK_NONBLOCK;
  }
  if ((flags & DR_CLOEXEC) != 0) {
    f |= SOCK_CLOEXEC;
  }
  const dr_handle_t result = accept4(sockfd, addr, addrlen, f);
  if (dr_unlikely(result < 0)) {
    return DR_RESULT_ERRNO(handle);
  }
  return DR_RESULT_OK(handle, result);

#else
  const dr_handle_t result = accept(sockfd, addr, addrlen);
#if defined(_WIN32)
  if (dr_unlikely(result == INVALID_SOCKET)) {
    return DR_RESULT_WSAGETLASTERROR(handle);
  }
#else
  if (dr_unlikely(result < 0)) {
    return DR_RESULT_ERRNO(handle);
  }
#endif

#if !defined(_WIN32)
  // DR How to set DR_NONBLOCK on windows?
  if ((flags & DR_NONBLOCK) != 0) {
    const int sf = fcntl(result, F_GETFL); // DR Merge duplicate fcntl code?
    if (dr_unlikely(sf < 0 || fcntl(result, F_SETFL, sf | O_NONBLOCK) != 0)) {
      const int errnum = errno;
      dr_close(result);
      return DR_RESULT_ERRNUM(handle, DR_ERR_ISO_C, errnum);
    }
  }

  if ((flags & DR_CLOEXEC) != 0) {
    const int df = fcntl(result, F_GETFD); // DR Merge duplicate fcntl code?
    if (dr_unlikely(df < 0 || fcntl(result, F_SETFD, df | FD_CLOEXEC) != 0)) {
      const int errnum = errno;
      dr_close(result);
      return DR_RESULT_ERRNUM(handle, DR_ERR_ISO_C, errnum);
    }
  }
#endif

  return DR_RESULT_OK(handle, result);
#endif
}

struct dr_result_void dr_bind(dr_handle_t sockfd, const struct sockaddr *restrict const addr, dr_socklen_t addrlen) {
  if (dr_unlikely(bind(sockfd, addr, addrlen) != 0)) {
#if defined(_WIN32)
    return DR_RESULT_WSAGETLASTERROR_VOID();
#else
    return DR_RESULT_ERRNO_VOID();
#endif
  }
  return DR_RESULT_OK_VOID();
}

struct dr_result_void dr_connect(dr_handle_t sockfd, const struct sockaddr *restrict const addr, dr_socklen_t addrlen) {
  if (dr_unlikely(connect(sockfd, addr, addrlen) != 0)) {
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

struct dr_result_handle dr_sock_connect(const char *restrict const hostname, const char *restrict const port, unsigned int flags) {
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
      return DR_RESULT_ERRNUM(handle, DR_ERR_GAI, errnum);
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
      const struct dr_result_void r = dr_connect(fd, ai->ai_addr, ai->ai_addrlen);
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
    return DR_RESULT_ERROR(handle, &last_error);
  }
  return DR_RESULT_OK(handle, fd);
}

struct dr_result_handle dr_sock_listen(const char *restrict const hostname, const char *restrict const port, unsigned int flags) {
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
      return DR_RESULT_ERRNUM(handle, DR_ERR_GAI, errnum);
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
      const struct dr_result_void r = dr_bind(fd, ai->ai_addr, ai->ai_addrlen);
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
    return DR_RESULT_ERROR(handle, &last_error);
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
