// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"
#include "dr_io_internal.h"

#include <errno.h>

#if defined(__linux__)

#include <sys/epoll.h>

#elif defined(DR_HAS_KEVENT)

#include <sys/types.h>

#include <fcntl.h>
#include <sys/event.h>
#include <sys/time.h>

#elif defined(__sun)

#include <fcntl.h>
#include <poll.h>
#include <port.h>
#include <sys/types.h>
#include <unistd.h>

#elif defined(_WIN32)

#include <winsock2.h>

#include <windows.h>

#include <mswsock.h>

#else

#error Unimplemented

#endif

#include "dr_types_impl.h"

/*
 * epoll
 * - linux
 * - http://man7.org/linux/man-pages/man7/epoll.7.html
 * - epoll_create1
 * - epoll_ctl
 * - epoll_wait
 * - struct epoll_data, size: 12, align: 1 (but use 4)
 *
 * kqueue
 * - bsd
 * - https://www.freebsd.org/cgi/man.cgi?kqueue
 * - kqueue
 * - kevent
 * - kevent, size: 32, align: 8 (freebsd 64bit, openbsd 64bit), size: 28, align: 4 (minix 32bit), size: 40, align: 8? (netbsd 64bit)
 *
 * iocp
 * - windows
 * - https://msdn.microsoft.com/en-us/library/windows/desktop/aa365198(v=vs.85).aspx
 * - CreateIoCompletionPort
 * - GetQueuedCompletionStatusEx
 * - SetFileCompletionNotificationModes
 * - OVERLAPPED_ENTRY, size: 32, align: 8
 *
 * port_create
 * - solaris
 * - https://docs.oracle.com/cd/E23824_01/html/821-1465/port-create-3c.html
 * - port_create
 * - port_associate
 * - port_getn
 * - port_event_t, size: 16, align: 4? (32bit), size: 24, align: 8? (64bit)
 *
 * pollset_create
 * - aix
 * - https://www.ibm.com/support/knowledgecenter/en/ssw_aix_71/com.ibm.aix.basetrf1/pollset.htm
 * - nope, don't have access to aix
 *
 * /dev/poll
 * - ?
 * - http://docs.oracle.com/cd/E19253-01/816-5177/6mbbc4g9n/index.html
 * - nope, don't care about hpux and is deprecated on solaris
 *
 * select
 * - various
 * - nope, limited by FD_SETSIZE
 *
 * poll
 * - various
 * - nope, scales poorly
 */

static void dr_check_alignment(const void *restrict const ptr) {
  const uintptr_t val = (uintptr_t)ptr;
  dr_assert((val & 0x7) == 0);
}

#if defined(__linux__) || defined(DR_HAS_KEVENT) || defined(__sun)

#if defined(__linux__) || defined(DR_HAS_KEVENT)

#if defined(__linux__)

DR_WARN_UNUSED_RESULT static struct dr_result_handle dr_event_open(unsigned int flags) {
  if (dr_unlikely((flags & ~(DR_CLOEXEC)) != 0)) {
    return DR_RESULT_ERRNUM(handle, DR_ERR_ISO_C, EINVAL);
  }
  int epoll_flags = 0;
  if ((flags & DR_CLOEXEC) != 0) {
    epoll_flags |= EPOLL_CLOEXEC;
  }
  const int result = epoll_create1(epoll_flags);
  if (dr_unlikely(result < 0)) {
    return DR_RESULT_ERRNO(handle);
  }
  return DR_RESULT_OK(handle, result);
}

void *dr_event_key(dr_event_t *restrict const events, int i) {
  return ((struct epoll_event *)events)[i].data.ptr;
}

bool dr_event_is_read(dr_event_t *restrict const events, int i) {
  return ((struct epoll_event *)events)[i].events & EPOLLIN;
}

bool dr_event_is_write(dr_event_t *restrict const events, int i) {
  return ((struct epoll_event *)events)[i].events & EPOLLOUT;
}

struct dr_result_uint dr_equeue_dequeue(struct dr_equeue *restrict const e, dr_event_t *restrict const events, size_t bytes) {
  {
    struct dr_equeue_handle *restrict h;
    struct dr_equeue_handle *restrict n;
    list_for_each_entry_safe(h, n, &e->changed_clients, struct dr_equeue_handle, changed_clients) {
      list_del(&h->changed_clients);
      const int epoll_op = h->actual_events == 0 ? EPOLL_CTL_ADD : h->events != 0 ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
      if (dr_unlikely((h->events & ~(DR_EVENT_IN | DR_EVENT_OUT)) != 0)) {
	return DR_RESULT_ERRNUM(uint, DR_ERR_ISO_C, EINVAL);
      }
      dr_check_alignment(h);
      struct epoll_event event = {
	.events = 0,
	.data.ptr = h,
      };
      if ((h->events & DR_EVENT_IN) != 0) {
	event.events |= EPOLLIN;
      }
      if ((h->events & DR_EVENT_OUT) != 0) {
	event.events |= EPOLLOUT;
      }
      if (dr_unlikely(epoll_ctl(e->fd, epoll_op, h->fd, &event) != 0)) {
	return DR_RESULT_ERRNO(uint);
      }
      h->actual_events = h->events;
    }
  }
  dr_assert(sizeof(struct epoll_event) == sizeof(dr_event_t));
  const int count = epoll_wait(e->fd, (struct epoll_event *)events, bytes/sizeof(struct epoll_event), -1);
  if (dr_unlikely(count < 0)) {
    const int errnum = errno;
    if (errnum == EINTR) {
      return DR_RESULT_OK(uint, 0);
    }
    return DR_RESULT_ERRNUM(uint, DR_ERR_ISO_C, errnum);
  }
  return DR_RESULT_OK(uint, count);
}

#elif defined(DR_HAS_KEVENT)

DR_WARN_UNUSED_RESULT static struct dr_result_handle dr_event_open(unsigned int flags) {
  if (dr_unlikely((flags & ~(DR_CLOEXEC)) != 0)) {
    return DR_RESULT_ERRNUM(handle, DR_ERR_ISO_C, EINVAL);
  }
  const int result = kqueue();
  if (dr_unlikely(result < 0)) {
    return DR_RESULT_ERRNO(handle);
  }
  if ((flags & DR_CLOEXEC) != 0) {
    const int df = fcntl(result, F_GETFD); // DR Merge duplicate fcntl code?
    if (dr_unlikely(df < 0 || fcntl(result, F_SETFD, df | FD_CLOEXEC) != 0)) {
      goto fail_close;
    }
  }
  return DR_RESULT_OK(handle, result);
 fail_close:
  {
    const int errnum = errno;
    dr_close(result);
    return DR_RESULT_ERRNUM(handle, DR_ERR_ISO_C, errnum);
  }
}

void *dr_event_key(dr_event_t *restrict const events, int i) {
  return ((struct kevent *)events)[i].udata;
}

bool dr_event_is_read(dr_event_t *restrict const events, int i) {
  return ((struct kevent *)events)[i].filter == EVFILT_READ;
}

bool dr_event_is_write(dr_event_t *restrict const events, int i) {
  return ((struct kevent *)events)[i].filter == EVFILT_WRITE;
}

struct dr_result_uint dr_equeue_dequeue(struct dr_equeue *restrict const e, dr_event_t *restrict const events, size_t bytes) {
  // DR only call kevent once, or at least less often, and on freebsd, netbsd, openbsd, and macOS, the same kevent arrays can be the same
  {
    struct dr_equeue_handle *restrict h;
    struct dr_equeue_handle *restrict n;
    list_for_each_entry_safe(h, n, &e->changed_clients, struct dr_equeue_handle, changed_clients) {
      list_del(&h->changed_clients);
      const int kqueue_op = h->actual_events == 0 ? EV_ADD : h->events != 0 ? EV_ADD : EV_DELETE;
      if (dr_unlikely((h->events & ~(DR_EVENT_IN | DR_EVENT_OUT)) != 0)) {
	return DR_RESULT_ERRNUM(uint, DR_ERR_ISO_C, EINVAL);
      }
      int kevent_events = 0;
      if ((h->events & DR_EVENT_IN) != 0) {
	kevent_events |= EVFILT_READ;
      }
      if ((h->events & DR_EVENT_OUT) != 0) {
	kevent_events |= EVFILT_WRITE;
      }
      struct kevent kev;
      dr_check_alignment(h);
      EV_SET(&kev, h->fd, kevent_events, kqueue_op, 0, 0, h);
      if (dr_unlikely(kevent(e->fd, &kev, 1, NULL, 0, NULL) != 0)) {
	return DR_RESULT_ERRNO(uint);
      }
      h->actual_events = h->events;
    }
  }
  dr_assert(sizeof(struct kevent) == sizeof(dr_event_t));
  const int count = kevent(e->fd, NULL, 0, (struct kevent *)events, bytes/sizeof(struct kevent), NULL);
  if (dr_unlikely(count < 0)) {
    return DR_RESULT_ERRNO(uint);
  }
  return DR_RESULT_OK(uint, count);
}

#endif

static void dr_event_subscribe(struct dr_equeue *restrict const e, struct dr_equeue_handle *restrict const c, const unsigned int f) {
  c->events |= f;
  if (c->events != c->actual_events && c->changed_clients.next == NULL) {
    list_add_tail(&c->changed_clients, &e->changed_clients);
  } else if (c->events == c->actual_events && c->changed_clients.next != NULL) {
    list_del(&c->changed_clients);
  }
}

static void dr_event_unsubscribe(struct dr_equeue *restrict const e, struct dr_equeue_handle *restrict const c, const unsigned int f) {
  c->events &= ~f;
  if (c->events != c->actual_events && c->changed_clients.next == NULL) {
    list_add_tail(&c->changed_clients, &e->changed_clients);
  } else if (c->events == c->actual_events && c->changed_clients.next != NULL) {
    list_del(&c->changed_clients);
  }
}

#elif defined(__sun)

DR_WARN_UNUSED_RESULT static struct dr_result_handle dr_event_open(unsigned int flags) {
  if (dr_unlikely((flags & ~(DR_CLOEXEC)) != 0)) {
    return DR_RESULT_ERRNUM(handle, DR_ERR_ISO_C, EINVAL);
  }
  const int result = port_create();
  if (dr_unlikely(result < 0)) {
    return DR_RESULT_ERRNO(handle);
  }
  if ((flags & DR_CLOEXEC) != 0) {
    const int df = fcntl(result, F_GETFD); // DR Merge duplicate fcntl code?
    if (dr_unlikely(df < 0 || fcntl(result, F_SETFD, df | FD_CLOEXEC) != 0)) {
      goto fail_close;
    }
  }
  return DR_RESULT_OK(handle, result);
 fail_close:
  {
    const int errnum = errno;
    dr_close(result);
    return DR_RESULT_ERRNUM(handle, DR_ERR_ISO_C, errnum);
  }
}

void *dr_event_key(dr_event_t *restrict const events, int i) {
  return ((port_event_t *)events)[i].portev_user;
}

bool dr_event_is_read(dr_event_t *restrict const events, int i) {
  return ((port_event_t *)events)[i].portev_events & POLLIN;
}

bool dr_event_is_write(dr_event_t *restrict const events, int i) {
  return ((port_event_t *)events)[i].portev_events & POLLOUT;
}

static void dr_event_subscribe(struct dr_equeue *restrict const e, struct dr_equeue_handle *restrict const c, const unsigned int f) {
  c->events |= f;
  if (c->changed_clients.next == NULL) {
    list_add_tail(&c->changed_clients, &e->changed_clients);
  }
}

static void dr_event_unsubscribe(struct dr_equeue *restrict const e, struct dr_equeue_handle *restrict const c, const unsigned int f) {
  (void)e;
  (void)c;
  (void)f;
}

struct dr_result_uint dr_equeue_dequeue(struct dr_equeue *restrict const e, dr_event_t *restrict const events, size_t bytes) {
  {
    struct dr_equeue_handle *restrict h;
    struct dr_equeue_handle *restrict n;
    list_for_each_entry_safe(h, n, &e->changed_clients, struct dr_equeue_handle, changed_clients) {
      list_del(&h->changed_clients);
      if (dr_unlikely((h->events & ~(DR_EVENT_IN | DR_EVENT_OUT)) != 0)) {
	return DR_RESULT_ERRNUM(uint, DR_ERR_ISO_C, EINVAL);
      }
      int port_events = 0;
      if ((h->events & DR_EVENT_IN) != 0) {
	port_events |= POLLIN;
      }
      if ((h->events & DR_EVENT_OUT) != 0) {
	port_events |= POLLOUT;
      }
      dr_check_alignment(h);
      if (dr_unlikely(port_associate(e->fd, PORT_SOURCE_FD, h->fd, port_events, h) != 0)) {
	return DR_RESULT_ERRNO(uint);
      }
    }
  }
  uint_t count = 1;
  dr_assert(sizeof(port_event_t) == sizeof(dr_event_t));
  if (dr_unlikely(port_getn(e->fd, (port_event_t *)events, bytes/sizeof(port_event_t), &count, NULL) != 0)) {
    return DR_RESULT_ERRNO(uint);
  }
  return DR_RESULT_OK(uint, count);
}

#endif

struct dr_result_void dr_equeue_accept(struct dr_io_handle *restrict const ih, struct dr_equeue *restrict const e, struct dr_equeue_server *restrict const s) {
  {
    const struct dr_result_void r = dr_ioserver_sock_accept_handle(&s->ihserver, ih, sizeof(*ih), NULL, NULL, DR_NONBLOCK | DR_CLOEXEC);
    DR_IF_RESULT_ERR(r, err) {
      if (dr_unlikely(err->num != EAGAIN)) {
	return DR_RESULT_ERROR_VOID(err);
      }
    } DR_ELIF_RESULT_OK_VOID(r) {
      return DR_RESULT_OK_VOID();
    } DR_FI_RESULT;
  }
  dr_event_subscribe(e, &s->h, DR_EVENT_IN);
  dr_schedule(true);
  dr_event_unsubscribe(e, &s->h, DR_EVENT_IN);
  return dr_ioserver_sock_accept_handle(&s->ihserver, ih, sizeof(*ih), NULL, NULL, DR_NONBLOCK | DR_CLOEXEC);
}

struct dr_result_size dr_equeue_read(struct dr_equeue *restrict const e, struct dr_equeue_client *restrict const c, void *restrict const buf, const size_t count) {
  {
    const struct dr_result_size r = dr_io_handle_read(&c->ih.io, buf, count);
    DR_IF_RESULT_ERR(r, err) {
      if (dr_unlikely(err->num != EAGAIN)) {
	return DR_RESULT_ERROR(size, err);
      }
    } DR_ELIF_RESULT_OK(size_t, r, value) {
      return DR_RESULT_OK(size, value);
    } DR_FI_RESULT;
  }
  dr_event_subscribe(e, &c->h, DR_EVENT_IN);
  dr_schedule(true);
  dr_event_unsubscribe(e, &c->h, DR_EVENT_IN);
  return dr_io_handle_read(&c->ih.io, buf, count);
}

struct dr_result_size dr_equeue_write(struct dr_equeue *restrict const e, struct dr_equeue_client *restrict const c, const void *restrict const buf, const size_t count) {
  {
    const struct dr_result_size r = dr_io_handle_write(&c->ih.io, buf, count);
    DR_IF_RESULT_ERR(r, err) {
      if (dr_unlikely(err->num != EAGAIN)) {
	return DR_RESULT_ERROR(size, err);
      }
    } DR_ELIF_RESULT_OK(size_t, r, value) {
      return DR_RESULT_OK(size, value);
    } DR_FI_RESULT;
  }
  dr_event_subscribe(e, &c->h, DR_EVENT_OUT);
  dr_schedule(true);
  dr_event_unsubscribe(e, &c->h, DR_EVENT_OUT);
  return dr_io_handle_write(&c->ih.io, buf, count);
}

struct dr_result_void dr_equeue_init(struct dr_equeue *restrict const e) {
  const struct dr_result_handle r = dr_event_open(DR_CLOEXEC);
  DR_IF_RESULT_ERR(r, err) {
    return DR_RESULT_ERROR_VOID(err);
  } DR_ELIF_RESULT_OK(dr_handle_t, r, value) {
    *e = (struct dr_equeue) {
      .fd = value,
      .changed_clients = LIST_HEAD_INIT(e->changed_clients),
    };
    return DR_RESULT_OK_VOID();
  } DR_FI_RESULT;
}

DR_WARN_UNUSED_RESULT static struct dr_equeue_handle dr_equeue_handle_init(dr_handle_t fd) {
  return (struct dr_equeue_handle) {
    .fd = fd,
  };
}

static void dr_equeue_handle_destroy(struct dr_equeue_handle *restrict const h) {
  if (h->changed_clients.next != NULL) {
    list_del(&h->changed_clients);
  }
}

void dr_equeue_server_init(struct dr_equeue_server *restrict const s, struct dr_ioserver_handle *restrict const ihserver) {
  *s = (struct dr_equeue_server) {
    .h = dr_equeue_handle_init(ihserver->fd),
    .ihserver = *ihserver,
  };
}

void dr_equeue_server_destroy(struct dr_equeue_server *restrict const s) {
  dr_equeue_handle_destroy(&s->h);
  dr_ioserver_handle_close(&s->ihserver.ioserver);
}

void dr_equeue_client_init(struct dr_equeue_client *restrict const c, struct dr_io_handle *restrict const ih) {
  *c = (struct dr_equeue_client) {
    .h = dr_equeue_handle_init(ih->fd),
    .ih = *ih,
  };
}

void dr_equeue_client_destroy(struct dr_equeue_client *restrict const c) {
  dr_equeue_handle_destroy(&c->h);
  dr_io_handle_close(&c->ih.io);
}

#elif defined(_WIN32)

DR_WARN_UNUSED_RESULT static struct dr_result_void dr_event_associate(dr_handle_t efd, dr_handle_t fd, void *restrict const key) {
  dr_check_alignment(key);
  if (dr_unlikely(CreateIoCompletionPort((HANDLE)fd, (HANDLE)efd, (ULONG_PTR)key, 0) == NULL)) {
    return DR_RESULT_GETLASTERROR_VOID();
  }
  if (dr_unlikely(SetFileCompletionNotificationModes((HANDLE)fd, FILE_SKIP_COMPLETION_PORT_ON_SUCCESS) == 0)) {
    return DR_RESULT_GETLASTERROR_VOID();
  }
  return DR_RESULT_OK_VOID();
}

void *dr_event_key(dr_event_t *restrict const events, int i) {
  return (void *)((OVERLAPPED_ENTRY *)events)[i].lpCompletionKey;
}

DR_WARN_UNUSED_RESULT static uintptr_t dr_overlapped_err(dr_overlapped_t *restrict const ol) {
  return ((OVERLAPPED *)ol)->Internal;
}

DR_WARN_UNUSED_RESULT static uintptr_t dr_overlapped_count(dr_overlapped_t *restrict const ol) {
  return ((OVERLAPPED *)ol)->InternalHigh;
}

struct dr_result_void dr_equeue_accept(struct dr_io_handle *restrict const ih, struct dr_equeue *restrict const e, struct dr_equeue_server *restrict const s) {
  dr_handle_t cfd;
  {
    {
      const struct dr_result_handle r = dr_socket(AF_INET6, SOCK_STREAM, 0, DR_NONBLOCK | DR_CLOEXEC); // DR ...
      DR_IF_RESULT_ERR(r, err) {
	return DR_RESULT_ERROR_VOID(err);
      } DR_ELIF_RESULT_OK(dr_handle_t, r, value) {
	cfd = value;
      } DR_FI_RESULT;
    }
    DWORD bytes;
    dr_assert(sizeof(dr_overlapped_t) == sizeof(OVERLAPPED));
    dr_assert(sizeof(dr_sockaddr_t) == sizeof(struct sockaddr_storage));
    // DR Create & use accept_ol
    if (AcceptEx(s->ihserver.fd, cfd, s->buf, 0, sizeof(struct sockaddr_storage) + 16, sizeof(struct sockaddr_storage) + 16, &bytes, (OVERLAPPED *)&s->ol) != 0) {
      goto ok;
    }
    {
      const DWORD errnum = WSAGetLastError();
      if (dr_unlikely(errnum != ERROR_IO_PENDING)) {
	closesocket(cfd);
	return DR_RESULT_ERRNUM_VOID(DR_ERR_WIN, errnum);
      }
    }
    if (!s->subscribed) {
      const struct dr_result_void r = dr_event_associate(e->fd, s->ihserver.fd, s);
      DR_IF_RESULT_ERR(r, err) {
	closesocket(cfd);
	return DR_RESULT_ERROR_VOID(err);
      } DR_FI_RESULT;
      s->subscribed = true;
    }
  }
  s->cfd = cfd;
  dr_schedule(true);
  s->cfd = INVALID_SOCKET;
  if (dr_unlikely(dr_overlapped_err(&s->ol) != 0)) {
    closesocket(cfd);
    return DR_RESULT_ERRNUM_VOID(DR_ERR_WIN, dr_overlapped_err(&s->ol));
  }
 ok:
  dr_io_handle_init(ih, cfd);
  return DR_RESULT_OK_VOID();
}

struct dr_result_size dr_equeue_read(struct dr_equeue *restrict const e, struct dr_equeue_client *restrict const c, void *restrict const buf, const size_t count) {
  {
    {
      const struct dr_result_size r = dr_io_handle_read_ol(&c->ih, buf, count, &c->rol);
      DR_IF_RESULT_ERR(r, err) {
	if (dr_unlikely(err->num != ERROR_IO_PENDING)) {
	  return DR_RESULT_ERROR(size, err);
	}
      } DR_ELIF_RESULT_OK(size_t, r, value) {
	return DR_RESULT_OK(size, value);
      } DR_FI_RESULT;
    }
    if (!c->subscribed) {
      const struct dr_result_void r = dr_event_associate(e->fd, c->ih.fd, c);
      DR_IF_RESULT_ERR(r, err) {
	return DR_RESULT_ERROR(size, err);
      } DR_FI_RESULT;
      c->subscribed = true;
    }
  }
  dr_schedule(true);
  if (dr_unlikely(dr_overlapped_err(&c->rol) != 0)) {
    return DR_RESULT_ERRNUM(size, DR_ERR_WIN, dr_overlapped_err(&c->rol));
  }
  return DR_RESULT_OK(size, dr_overlapped_count(&c->rol));
}

struct dr_result_size dr_equeue_write(struct dr_equeue *restrict const e, struct dr_equeue_client *restrict const c, const void *restrict const buf, const size_t count) {
  {
    {
      const struct dr_result_size r = dr_io_handle_write_ol(&c->ih, buf, count, &c->wol);
      DR_IF_RESULT_ERR(r, err) {
	if (dr_unlikely(err->num != ERROR_IO_PENDING)) {
	  return DR_RESULT_ERROR(size, err);
	}
      } DR_ELIF_RESULT_OK(size_t, r, value) {
	return DR_RESULT_OK(size, value);
      } DR_FI_RESULT;
    }
    if (!c->subscribed) {
      const struct dr_result_void r = dr_event_associate(e->fd, c->ih.fd, c);
      DR_IF_RESULT_ERR(r, err) {
	return DR_RESULT_ERROR(size, err);
      } DR_FI_RESULT;
      c->subscribed = true;
    }
  }
  dr_schedule(true);
  if (dr_unlikely(dr_overlapped_err(&c->wol) != 0)) {
    return DR_RESULT_ERRNUM(size, DR_ERR_WIN, dr_overlapped_err(&c->wol));
  }
  return DR_RESULT_OK(size, dr_overlapped_count(&c->wol));
}

bool dr_event_is_read(dr_event_t *restrict const events, int i) {
  OVERLAPPED_ENTRY *restrict const e = ((OVERLAPPED_ENTRY *)events) +i;
  return (char *)e->lpOverlapped - (char *)e->lpCompletionKey == offsetof(struct dr_equeue_client, rol);
}

bool dr_event_is_write(dr_event_t *restrict const events, int i) {
  OVERLAPPED_ENTRY *restrict const e = ((OVERLAPPED_ENTRY *)events) +i;
  return (char *)e->lpOverlapped - (char *)e->lpCompletionKey == offsetof(struct dr_equeue_client, wol);
}

struct dr_result_void dr_equeue_init(struct dr_equeue *restrict const e) {
  const HANDLE result = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
  if (dr_unlikely(result == NULL)) {
    return DR_RESULT_GETLASTERROR_VOID();
  }
  *e = (struct dr_equeue) {
    .fd = (dr_handle_t)result,
  };
  return DR_RESULT_OK_VOID();
}

struct dr_result_uint dr_equeue_dequeue(struct dr_equeue *restrict const e, dr_event_t *restrict const events, size_t bytes) {
  DWORD count;
  dr_assert(sizeof(OVERLAPPED_ENTRY) == sizeof(dr_event_t));
#if 0 // DR ...
  if (dr_unlikely(GetQueuedCompletionStatusEx((HANDLE)e->fd, (OVERLAPPED_ENTRY *)events, bytes/sizeof(OVERLAPPED_ENTRY), &count, INFINITE, TRUE) == 0))
#else
  if (dr_unlikely(bytes < sizeof(OVERLAPPED_ENTRY))) {
    return DR_RESULT_ERRNUM(uint, DR_ERR_WIN, ERROR_INVALID_HANDLE);
  }
  count = 1;
  if (dr_unlikely(GetQueuedCompletionStatus((HANDLE)e->fd, &((OVERLAPPED_ENTRY *)events)[0].dwNumberOfBytesTransferred, &((OVERLAPPED_ENTRY *)events)[0].lpCompletionKey, &((OVERLAPPED_ENTRY *)events)[0].lpOverlapped, INFINITE) == 0))
#endif
  {
    return DR_RESULT_GETLASTERROR(uint);
  }
  return DR_RESULT_OK(uint, count);
}

void dr_equeue_server_init(struct dr_equeue_server *restrict const s, struct dr_ioserver_handle *restrict const ihserver) {
  *s = (struct dr_equeue_server) {
    .ihserver = *ihserver,
  };
}

void dr_equeue_server_destroy(struct dr_equeue_server *restrict const s) {
  dr_close(s->cfd);
  dr_ioserver_handle_close(&s->ihserver.ioserver);
}

void dr_equeue_client_init(struct dr_equeue_client *restrict const c, struct dr_io_handle *restrict const ih) {
  *c = (struct dr_equeue_client) {
    .ih = *ih,
  };
}

void dr_equeue_client_destroy(struct dr_equeue_client *restrict const c) {
  dr_io_handle_close(&c->ih.io);
}

#endif

void dr_equeue_destroy(struct dr_equeue *restrict const e) {
  dr_close(e->fd);
}
