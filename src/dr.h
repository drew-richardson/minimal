// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#if !defined(DR_H)
#define DR_H

#include "dr_config.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dr_version.h"
#include "dr_types.h"
#include "dr_compiler.h"

extern char *restrict dr_optarg;
extern int dr_optind, dr_opterr, dr_optopt, dr_optreset;

DR_WARN_UNUSED_RESULT int dr_getopt(int argc, char * const argv[], const char *optstring);
DR_WARN_UNUSED_RESULT int dr_getopt_long(int argc, char **argv, const char *optstring, const struct dr_option *longopts, int *idx);

DR_WARN_UNUSED_RESULT struct dr_result_size dr_write_all_fn(struct dr_io *restrict const io, dr_io_write_fn_t write, const void *restrict const buf, size_t count);
DR_WARN_UNUSED_RESULT struct dr_result_size dr_write_all(struct dr_io *restrict const io, const void *restrict const buf, size_t count);

#define DR_ARGMAX 9
DR_WARN_UNUSED_RESULT struct dr_result_size dr_vfprintf(struct dr_io *restrict const io, const char *restrict const fmt, va_list ap);
DR_WARN_UNUSED_RESULT struct dr_result_size dr_vsnprintf(char *restrict const s, size_t n, const char *restrict const fmt, va_list ap);
DR_WARN_UNUSED_RESULT DR_FORMAT_PRINTF(3, 4) struct dr_result_size dr_snprintf(char *restrict const s, size_t n, const char *restrict const fmt, ...);
DR_WARN_UNUSED_RESULT DR_FORMAT_PRINTF(2, 3) struct dr_result_size dr_fprintf(struct dr_io *restrict const io, const char *restrict const fmt, ...);
DR_WARN_UNUSED_RESULT DR_FORMAT_PRINTF(1, 2) struct dr_result_size dr_printf(const char *restrict const fmt, ...);
DR_WARN_UNUSED_RESULT struct dr_result_size dr_fputs(const char *restrict const s, struct dr_io *restrict const io);
DR_WARN_UNUSED_RESULT struct dr_result_size dr_puts(const char *restrict const s);

#define dr_max_tname_impl(TYPE, TNAME) \
  DR_WARN_UNUSED_RESULT static inline TYPE dr_max_##TNAME##_impl(TYPE lhs, TYPE rhs) { return lhs > rhs ? lhs : rhs; }

dr_max_tname_impl(size_t, size);

#define dr_min_tname_impl(TYPE, TNAME) \
  DR_WARN_UNUSED_RESULT static inline TYPE dr_min_##TNAME##_impl(TYPE lhs, TYPE rhs) { return lhs < rhs ? lhs : rhs; }

dr_min_tname_impl(size_t, size);

#if defined(DR_HAS_GENERIC)

void dr_max_mismatch(int lhs, int rhs);

#define dr_max_size(lhs, rhs) _Generic((lhs), size_t: _Generic((rhs), size_t: dr_max_size_impl, const size_t: dr_max_size_impl, default: dr_max_mismatch), const size_t: _Generic((rhs), size_t: dr_max_size_impl, const size_t: dr_max_size_impl, default: dr_max_mismatch), default: dr_max_mismatch)(lhs, rhs)

void dr_min_mismatch(int lhs, int rhs);

#define dr_min_size(lhs, rhs) _Generic((lhs), size_t: _Generic((rhs), size_t: dr_min_size_impl, const size_t: dr_min_size_impl, default: dr_min_mismatch), const size_t: _Generic((rhs), size_t: dr_min_size_impl, const size_t: dr_min_size_impl, default: dr_min_mismatch), default: dr_min_mismatch)(lhs, rhs)

#else

#define dr_max_size(lhs, rhs) dr_max_size_impl(lhs, rhs)

#define dr_min_size(lhs, rhs) dr_min_size_impl(lhs, rhs)

#endif

enum {
  DR_ERR_ISO_C = 1,
#if defined(_WIN32)
  DR_ERR_WIN   = 2,
#endif
  DR_ERR_GAI   = 3,
};

#define DR_RESULT_OK(TNAME, VALUE) \
  (struct dr_result_##TNAME) { \
    .private_is_err = false, \
    .private_u.private_value = VALUE, \
  }

#define DR_RESULT_ERRNUM(TNAME, DOMAIN, NUM) \
  (struct dr_result_##TNAME) { \
    .private_is_err = true, \
    .private_u.private_error.func = __func__, \
    .private_u.private_error.file = __FILE__, \
    .private_u.private_error.line = __LINE__, \
    .private_u.private_error.domain = (DOMAIN), \
    .private_u.private_error.num = (NUM), \
  }

#define DR_RESULT_ERRNO(TNAME) DR_RESULT_ERRNUM(TNAME, DR_ERR_ISO_C, errno)
#define DR_RESULT_GETLASTERROR(TNAME) DR_RESULT_ERRNUM(TNAME, DR_ERR_WIN, GetLastError())
#define DR_RESULT_WSAGETLASTERROR(TNAME) DR_RESULT_ERRNUM(TNAME, DR_ERR_WIN, WSAGetLastError())

#define DR_RESULT_ERROR(TNAME, ERROR) \
  (struct dr_result_##TNAME) { \
    .private_is_err = true, \
    .private_u.private_error = *(ERROR), \
  }

#define DR_IS_RESULT_ERR(R) ((R).private_is_err)
#define DR_IS_RESULT_OK(R) (!DR_IS_RESULT_ERR(R))

#define DR_IF_RESULT_ERR(R, ERROR) \
  if (dr_unlikely(DR_IS_RESULT_ERR(R))) { \
    const struct dr_error *restrict const ERROR = &(R).private_u.private_error; \

#define DR_ELIF_RESULT_OK(TYPE, R, VALUE) \
  } else { \
    TYPE const VALUE = (R).private_u.private_value; \

#define DR_IF_RESULT_OK(TYPE, R, VALUE) \
  if (dr_likely(DR_IS_RESULT_OK(R))) { \
    TYPE const VALUE = (R).private_u.private_value; \

#define DR_ELIF_RESULT_ERR(R, ERROR) \
  } else { \
    const struct dr_error *restrict const ERROR = &(R).private_u.private_error; \

#define DR_FI_RESULT }

#define DR_RESULT_OK_VOID() \
  (struct dr_result_void) { \
    .private_is_err = false, \
  }

#define DR_RESULT_ERRNUM_VOID(DOMAIN, NUM) \
  (struct dr_result_void) { \
    .private_is_err = true, \
    .private_u.private_error.func = __func__, \
    .private_u.private_error.file = __FILE__, \
    .private_u.private_error.line = __LINE__, \
    .private_u.private_error.domain = (DOMAIN), \
    .private_u.private_error.num = (NUM), \
  }

#define DR_RESULT_ERRNO_VOID() DR_RESULT_ERRNUM_VOID(DR_ERR_ISO_C, errno)
#define DR_RESULT_GETLASTERROR_VOID() DR_RESULT_ERRNUM_VOID(DR_ERR_WIN, GetLastError())
#define DR_RESULT_WSAGETLASTERROR_VOID() DR_RESULT_ERRNUM_VOID(DR_ERR_WIN, WSAGetLastError())

#define DR_RESULT_ERROR_VOID(ERROR) \
  (struct dr_result_void) { \
    .private_is_err = true, \
    .private_u.private_error = *(ERROR), \
  }

#define DR_ELIF_RESULT_OK_VOID(R) \
  } else {

#define DR_IF_RESULT_OK_VOID(R) \
  if (dr_likely(DR_IS_RESULT_OK(R))) {

#define dr_log(msg) dr_log_impl(__func__, __FILE__, __LINE__, msg)
//__attribute__((noinline,cold))
void dr_log_impl(const char *restrict const func, const char *restrict const file, const int line, const char *restrict const msg);

#define dr_log_error(msg, error) dr_log_error_impl(__func__, __FILE__, __LINE__, msg, error)
//__attribute__((noinline,cold))
void dr_log_error_impl(const char *restrict const func, const char *restrict const file, const int line, const char *restrict const msg, const struct dr_error *restrict const error);

#define dr_assert(cond) (dr_likely(cond) ? (void)(0) : dr_assert_fail(__func__, __FILE__, __LINE__, #cond))
//__attribute__((noinline,cold))
DR_NORETURN void dr_assert_fail(const char *restrict const func, const char *restrict const file, const int line, const char *restrict const cond);

int dr_log_format(char *restrict const buf, size_t size, const struct dr_error *restrict const error);

DR_WARN_UNUSED_RESULT bool dr_str_eq(const struct dr_str *restrict const a, const struct dr_str *restrict const b);

static const int64_t DR_NS_PER_S = 1000000000;
static const int64_t DR_NS_PER_MS = 1000000;

// 2^63/1000000000 = 9223372036 -> Sep 21 00:12:44 UTC 1677 - Apr 11 23:47:16 UTC 2262
DR_WARN_UNUSED_RESULT struct dr_result_int64 dr_system_time_ns(void);
DR_WARN_UNUSED_RESULT struct dr_result_void dr_system_sleep_ns(const int64_t time);

void dr_close(dr_handle_t fd);

void dr_io_handle_init(struct dr_io_handle *restrict const ih, dr_handle_t fd);
void dr_io_handle_wo_fixed_init(struct dr_io_handle_wo_buf *restrict const ih_wo, dr_handle_t fd, void *restrict const buf, size_t count);

void dr_io_ro_fixed_init(struct dr_io_ro_fixed *restrict const ro_fixed, const void *restrict const buf, size_t count);
void dr_io_wo_fixed_init(struct dr_io_wo *restrict const wo_fixed, void *restrict const buf, size_t count);
DR_WARN_UNUSED_RESULT struct dr_result_void dr_io_wo_resize_init(struct dr_io_wo *restrict const wo_resize, size_t count);
void dr_io_rw_fixed_init(struct dr_io_rw *restrict const rw_fixed, void *restrict const buf, size_t count);
DR_WARN_UNUSED_RESULT struct dr_result_void dr_io_rw_resize_init(struct dr_io_rw *restrict const rw_resize, size_t count);

#define DR_QUEUE_READABLE(c) ((c)->write_pos >= (c)->read_pos ? (c)->write_pos - (c)->read_pos : sizeof((c)->buf) - (c)->read_pos)
#define DR_QUEUE_WRITABLE(c) ((c)->write_pos < (c)->read_pos ? (c)->read_pos - 1 - (c)->write_pos : (c)->read_pos == 0 ? sizeof((c)->buf) - 1 - (c)->write_pos : sizeof((c)->buf) - (c)->write_pos)

DR_WARN_UNUSED_RESULT struct dr_result_void dr_console_startup(void);
extern struct dr_io_handle dr_stdin;
extern struct dr_io_handle dr_stdout;
extern struct dr_io_handle dr_stderr;

const char *dr_source_revision(void);
const char *dr_source_status(void);
const char *dr_source_date(void);
int dr_get_version_short(char *restrict const str, const size_t size);
int dr_get_version_long(char *restrict const str, const size_t size);

enum {
  DR_NONBLOCK  = 1U<<0, // DR Should this just always be the default? Doesn't play well on windows
  DR_CLOEXEC   = 1U<<1,
  DR_REUSEADDR = 1U<<2,
};

DR_WARN_UNUSED_RESULT struct dr_result_void dr_socket_startup(void);
DR_WARN_UNUSED_RESULT struct dr_result_handle dr_socket(int domain, int type, int protocol, unsigned int flags);
DR_WARN_UNUSED_RESULT struct dr_result_void dr_bind(dr_handle_t sockfd, const dr_sockaddr_t *restrict const addr, dr_socklen_t addrlen);
DR_WARN_UNUSED_RESULT struct dr_result_void dr_sock_listen(struct dr_ioserver_handle *restrict const ihserver, const char *restrict const hostname, const char *restrict const port, unsigned int flags);
DR_WARN_UNUSED_RESULT struct dr_result_void dr_connect(dr_handle_t sockfd, const dr_sockaddr_t *restrict const addr, dr_socklen_t addrlen);
DR_WARN_UNUSED_RESULT struct dr_result_void dr_sock_connect(struct dr_io_handle *restrict const ih, const char *restrict const hostname, const char *restrict const port, unsigned int flags);
DR_WARN_UNUSED_RESULT struct dr_result_void dr_listen(dr_handle_t sockfd, int backlog);

DR_WARN_UNUSED_RESULT struct dr_result_void dr_pipe_listen(struct dr_ioserver_handle *restrict const ihserver, const char *restrict const name, unsigned int flags);
DR_WARN_UNUSED_RESULT struct dr_result_void dr_pipe_connect(struct dr_io_handle *restrict const ih, const char *restrict const name, unsigned int flags);

void dr_ioserver_sock_init(struct dr_ioserver_handle *restrict const ihserver, dr_handle_t fd);

enum {
  DR_EVENT_IN  = 1U<<0,
  DR_EVENT_OUT = 1U<<1,
};

enum {
  DR_EVENT_ADD = 0,
  DR_EVENT_MOD = 1,
  DR_EVENT_DEL = 2,
};

DR_WARN_UNUSED_RESULT void *dr_event_key(dr_event_t *restrict const events, int i);
DR_WARN_UNUSED_RESULT bool dr_event_is_read(dr_event_t *restrict const events, int i);
DR_WARN_UNUSED_RESULT bool dr_event_is_write(dr_event_t *restrict const events, int i);

DR_WARN_UNUSED_RESULT struct dr_result_void dr_equeue_init(struct dr_equeue *restrict const e);
void dr_equeue_destroy(struct dr_equeue *restrict const e);

DR_WARN_UNUSED_RESULT struct dr_result_uint dr_equeue_dequeue(struct dr_equeue *restrict const e, dr_event_t *restrict const events, size_t bytes);

void dr_equeue_server_init(struct dr_equeue_server *restrict const s, struct dr_equeue *restrict const e, struct dr_ioserver_handle *restrict const ihserver);

void dr_equeue_client_init(struct dr_equeue_client *restrict const c, struct dr_equeue *restrict const e, struct dr_io_handle *restrict const ih);

DR_WARN_UNUSED_RESULT struct dr_result_void dr_equeue_dispatch(struct dr_equeue *restrict const e);

DR_WARN_UNUSED_RESULT struct dr_result_void dr_task_create(struct dr_task *restrict const task, const size_t stack_size, const dr_task_start_t func, void *restrict const arg);
DR_WARN_UNUSED_RESULT struct dr_task *dr_task_self(void);
void dr_task_destroy(struct dr_task *restrict const task);
void dr_task_runnable(struct dr_task *restrict const task);
DR_NORETURN void dr_task_exit(void *restrict const arg, void (*cleanup)(void *restrict const));
void dr_schedule(const bool sleep);

void dr_wait_init(struct dr_wait *restrict const wait);
void dr_wait_destroy(struct dr_wait *restrict const wait);
void dr_wait_notify(struct dr_wait *restrict const wait);
void dr_wait_wait(struct dr_wait *restrict const wait);

DR_WARN_UNUSED_RESULT struct dr_result_void dr_sem_init(struct dr_sem *restrict const sem, unsigned int value);
void dr_sem_destroy(struct dr_sem *restrict const sem);
DR_WARN_UNUSED_RESULT struct dr_result_void dr_sem_post(struct dr_sem *restrict const sem);
DR_WARN_UNUSED_RESULT struct dr_result_void dr_sem_wait(struct dr_sem *restrict const sem);

// mode
enum {
  DR_DIR    = 0x80000000,
  DR_APPEND = 0x40000000,
  DR_EXCL   = 0x20000000,
  DR_AUTH   = 0x08000000,
  DR_TMP    = 0x04000000,
  DR_RUSR   = 0x00000100,
  DR_WUSR   = 0x00000080,
  DR_XUSR   = 0x00000040,
  DR_RGRP   = 0x00000020,
  DR_WGRP   = 0x00000010,
  DR_XGRP   = 0x00000008,
  DR_ROTH   = 0x00000004,
  DR_WOTH   = 0x00000002,
  DR_XOTH   = 0x00000001,
};

// qid type
enum {
  DR_QTDIR    = 0x80,
  DR_QTAPPEND = 0x40,
  DR_QTEXCL   = 0x20,
  DR_QTAUTH   = 0x08,
  DR_QTTMP    = 0x04,
};

enum {
  DR_AEXEC  = 0x1,
  DR_AWRITE = 0x2,
  DR_AREAD  = 0x4,
};

enum {
  DR_OREAD  = 0,
  DR_OWRITE = 1,
  DR_ORDWR  = 2,
  DR_OEXEC  = 3,
  DR_OTRUNC = 0x10,
};

DR_WARN_UNUSED_RESULT struct dr_result_file dr_vfs_walk(const struct dr_user *restrict const user, const struct dr_file *restrict const file, const struct dr_str *restrict const name);
DR_WARN_UNUSED_RESULT struct dr_result_fd dr_vfs_open(const struct dr_user *restrict const user, struct dr_file *restrict const file, const uint8_t mode);
DR_WARN_UNUSED_RESULT struct dr_result_uint32 dr_vfs_read(const struct dr_fd *restrict const fd, const uint64_t offset, const uint32_t count, void *restrict const buf);
DR_WARN_UNUSED_RESULT struct dr_result_uint32 dr_vfs_write(const struct dr_fd *restrict const fd, const uint64_t offset, const uint32_t count, const void *restrict const buf);
void dr_vfs_close(struct dr_fd *restrict const fd);

DR_WARN_UNUSED_RESULT struct dr_result_uint32 dr_9p_read_enosys(const struct dr_fd *restrict const fd, const uint64_t offset, const uint32_t count, void *restrict const buf);
DR_WARN_UNUSED_RESULT struct dr_result_uint32 dr_9p_write_enosys(const struct dr_fd *restrict const fd, const uint64_t offset, const uint32_t count, const void *restrict const buf);
DR_WARN_UNUSED_RESULT struct dr_result_uint32 dr_dir_read(const struct dr_fd *restrict const fd, const uint64_t offset, const uint32_t count, void *restrict const buf);

enum {
  DR_TVERSION = 100,
  DR_RVERSION = 101,
  DR_TAUTH    = 102,

  DR_TATTACH  = 104,
  DR_RATTACH  = 105,

  DR_RERROR   = 107,

  DR_TWALK    = 110,
  DR_RWALK    = 111,
  DR_TOPEN    = 112,
  DR_ROPEN    = 113,
  DR_TCREATE  = 114,
  DR_RCREATE  = 115,
  DR_TREAD    = 116,
  DR_RREAD    = 117,
  DR_TWRITE   = 118,
  DR_RWRITE   = 119,
  DR_TCLUNK   = 120,
  DR_RCLUNK   = 121,
  DR_TREMOVE  = 122,
  DR_RREMOVE  = 123,
  DR_TSTAT    = 124,
  DR_RSTAT    = 125,
  DR_TWSTAT   = 126,
  DR_RWSTAT   = 127,
};

static const uint32_t DR_NOFID = ~0U;

DR_WARN_UNUSED_RESULT uint8_t dr_decode_uint8(const uint8_t *restrict const buf);
DR_WARN_UNUSED_RESULT uint16_t dr_decode_uint16(const uint8_t *restrict const buf);
DR_WARN_UNUSED_RESULT uint32_t dr_decode_uint32(const uint8_t *restrict const buf);
DR_WARN_UNUSED_RESULT uint64_t dr_decode_uint64(const uint8_t *restrict const buf);

void dr_encode_uint8(uint8_t *restrict const buf, const uint8_t val);
void dr_encode_uint16(uint8_t *restrict const buf, const uint16_t val);
void dr_encode_uint32(uint8_t *restrict const buf, const uint32_t val);
void dr_encode_uint64(uint8_t *restrict const buf, const uint64_t val);

static const uint32_t DR_FAIL_UINT32 = ~0U;
DR_WARN_UNUSED_RESULT uint32_t dr_9p_decode_stat(struct dr_9p_stat *restrict const stat, const uint8_t *restrict const buf, const uint32_t size);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_header(uint8_t *restrict const type, uint16_t *restrict const tag, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Tversion(uint32_t *restrict const msize, struct dr_str *restrict const version, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Rversion(uint32_t *restrict const msize, struct dr_str *restrict const version, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Tauth(uint32_t *restrict const afid, struct dr_str *restrict const uname, struct dr_str *restrict const aname, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Rerror(struct dr_str *restrict const ename, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Tattach(uint32_t *restrict const fid, uint32_t *restrict const afid, struct dr_str *restrict const uname, struct dr_str *restrict const aname, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Rattach(struct dr_9p_qid *restrict const qid, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Twalk_iterator(uint32_t *restrict const fid, uint32_t *restrict const newfid, uint16_t *restrict const nwname, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Twalk_advance(struct dr_str *restrict const wname, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Twalk_finish(const uint32_t size, const uint32_t pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Rwalk_iterator(uint16_t *restrict const nwqid, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Rwalk_advance(struct dr_9p_qid *restrict const qid, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Rwalk_finish(const uint32_t size, const uint32_t pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Topen(uint32_t *restrict const fid, uint8_t *restrict const mode, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Ropen(struct dr_9p_qid *restrict const qid, uint32_t *restrict const iounit, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Tcreate(uint32_t *restrict const fid, struct dr_str *restrict const name, uint32_t *restrict const perm, uint8_t *restrict const mode, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Rcreate(struct dr_9p_qid *restrict const qid, uint32_t *restrict const iounit, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Tread(uint32_t *restrict const fid, uint64_t *restrict const offset, uint32_t *restrict const count, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Rread(uint32_t *restrict const count, const void *restrict *restrict const data, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Twrite(uint32_t *restrict const fid, uint64_t *restrict const offset, uint32_t *restrict const count, const void *restrict *restrict const data, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Rwrite(uint32_t *restrict const count, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Tclunk(uint32_t *restrict const fid, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Rclunk(const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Tremove(uint32_t *restrict const fid, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Rremove(const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Tstat(uint32_t *restrict const fid, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Rstat(struct dr_9p_stat *restrict const stat, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Twstat(uint32_t *restrict const fid, struct dr_9p_stat *restrict const stat, const uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos);
DR_WARN_UNUSED_RESULT bool dr_9p_decode_Rwstat(const uint32_t size, uint32_t *restrict const pos);

DR_WARN_UNUSED_RESULT bool dr_9p_encode_Tversion(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t msize, const struct dr_str *restrict const version);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Rversion(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t msize, const struct dr_str *restrict const version);
void dr_9p_encode_Rerror(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const struct dr_str *restrict const ename);
void dr_9p_encode_Rerror_err(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const struct dr_error *restrict const error);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Tattach(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t fid, const uint32_t afid, const struct dr_str *restrict const uname, const struct dr_str *restrict const aname);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Rattach(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const struct dr_file *restrict const f);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Twalk_iterator(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t fid, const uint32_t newfid, uint16_t *restrict const nwname);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Twalk_add(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, uint16_t *restrict const nwname, const struct dr_str *restrict const name);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Twalk_finish(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t nwname);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Rwalk_iterator(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, uint16_t *restrict const nwqid);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Rwalk_add(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, uint16_t *restrict const nwqid, const struct dr_file *restrict const f);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Rwalk_finish(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t nwqid);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Topen(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t fid, const uint8_t mode);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Ropen(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const struct dr_file *restrict const f, const uint32_t iounit);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Tcreate(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t fid, const struct dr_str *restrict const name, const uint32_t perm, const uint8_t mode);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Rcreate(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const struct dr_file *restrict const f, const uint32_t iounit);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Tread(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t fid, const uint64_t offset, const uint32_t count);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Rread_iterator(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Rread_finish(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint32_t count);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Twrite_iterator(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t fid, const uint64_t offset);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Twrite_finish(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint32_t count);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Rwrite(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t count);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Tclunk(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t fid);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Rclunk(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Tremove(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t fid);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Rremove(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Tstat(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t fid);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Rstat(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const struct dr_file *restrict const f);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Twstat(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag, const uint32_t fid, const struct dr_9p_stat *restrict const stat);
DR_WARN_UNUSED_RESULT bool dr_9p_encode_Rwstat(uint8_t *restrict const buf, const uint32_t size, uint32_t *restrict const pos, const uint16_t tag);

#endif // DR_H
