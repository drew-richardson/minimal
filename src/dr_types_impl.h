// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#if !defined(DR_TYPES_IMPL_H)
#define DR_TYPES_IMPL_H

#include <stdbool.h>

#include "list.h"

#include "dr_version.h"
#include "dr_types_common.h"

#if defined(__linux__) || defined(HAS_KEVENT) || defined(__sun)

#if defined(__linux__) || defined(HAS_KEVENT)

#if defined(__linux__)

#include <sys/epoll.h>

typedef struct epoll_event dr_event_impl_t;

#elif defined(HAS_KEVENT)

#include <sys/types.h>

#include <sys/event.h>

typedef struct kevent dr_event_impl_t;

#endif

struct dr_equeue_handle {
  struct list_head changed_clients;
  dr_handle_t fd;
  unsigned int actual_events;
  unsigned int events;
};

#elif defined(__sun)

#include <port.h>

typedef port_event_t dr_event_impl_t;

struct dr_equeue_handle {
  struct list_head changed_clients;
  dr_handle_t fd;
  unsigned int events;
};

#endif

struct dr_equeue_impl {
  struct list_head changed_clients;
  dr_handle_t fd;
};

struct dr_equeue_server_impl {
  struct dr_equeue_handle h;
};

struct dr_equeue_client_impl {
  struct dr_equeue_handle h;
};

#elif defined(_WIN32)

#include <winsock2.h>

#include <windows.h>

typedef OVERLAPPED_ENTRY dr_event_impl_t;

struct dr_equeue_impl {
  dr_handle_t fd;
};

struct dr_equeue_server_impl {
  dr_handle_t sfd;
  dr_handle_t cfd;
  OVERLAPPED ol;
  char buf[2*(sizeof(struct sockaddr_storage) + 16)];
  bool subscribed;
};

struct dr_equeue_client_impl {
  dr_handle_t fd;
  OVERLAPPED rol;
  OVERLAPPED wol;
  bool subscribed;
};

#endif

#endif // DR_TYPES_IMPL_H
