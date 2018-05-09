// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#if !defined(DR_TYPES_IMPL_H)
#define DR_TYPES_IMPL_H

#include <stdbool.h>

#include "list.h"

#include "dr_version.h"
#include "dr_types_common.h"

#if defined(__linux__) || defined(DR_HAS_KEVENT) || defined(__sun)

#include <sys/socket.h>

#if defined(__linux__) || defined(DR_HAS_KEVENT)

#if defined(__linux__)

#include <sys/epoll.h>

typedef struct epoll_event dr_event_impl_t;

#elif defined(DR_HAS_KEVENT)

#include <sys/types.h>

#include <sys/event.h>

typedef struct kevent dr_event_impl_t;

#endif

#elif defined(__sun)

#include <port.h>

typedef port_event_t dr_event_impl_t;

#endif

typedef uint8_t dr_overlapped_impl_t;

#elif defined(_WIN32)

#include <winsock2.h>

#include <windows.h>

typedef OVERLAPPED_ENTRY dr_event_impl_t;

typedef OVERLAPPED dr_overlapped_impl_t;

#endif

typedef struct sockaddr_storage dr_sockaddr_impl_t;

#endif // DR_TYPES_IMPL_H
