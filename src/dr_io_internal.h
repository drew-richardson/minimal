// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#if !defined(DR_IO_INTERNAL_H)
#define DR_IO_INTERNAL_H

#include "dr.h"

void dr_ioserver_handle_close(struct dr_ioserver *restrict const ioserver);

DR_WARN_UNUSED_RESULT struct dr_result_void dr_ioserver_sock_accept_handle(struct dr_ioserver_handle *restrict const ihserver, struct dr_io_handle *restrict const ih, size_t iolen, dr_sockaddr_t *restrict const addr, dr_socklen_t *restrict const addrlen, unsigned int flags);

void dr_io_handle_close(struct dr_io *restrict const io);

#if defined(_WIN32)

DR_WARN_UNUSED_RESULT struct dr_result_size dr_io_handle_read_ol(struct dr_io_handle *restrict const ih, void *restrict const buf, size_t count, dr_overlapped_t *restrict const ol);
DR_WARN_UNUSED_RESULT struct dr_result_size dr_io_handle_write_ol(struct dr_io_handle *restrict const ih, const void *restrict const buf, size_t count, dr_overlapped_t *restrict const ol);

#else

DR_WARN_UNUSED_RESULT struct dr_result_size dr_io_handle_read(struct dr_io *restrict const io, void *restrict const buf, size_t count);
DR_WARN_UNUSED_RESULT struct dr_result_size dr_io_handle_write(struct dr_io *restrict const io, const void *restrict const buf, size_t count);

#endif

#endif // DR_IO_INTERNAL_H
