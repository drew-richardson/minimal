// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

#if defined(_WIN32)
#include <ws2tcpip.h>

#include <windows.h>
#else
#include <netdb.h>
#endif

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void dr_log_prolog(const char *restrict const func, const char *restrict const file, const int line) {
  int width = 0;
  {
    const struct dr_result_int64 r = dr_system_time_ns();
    DR_IF_RESULT_OK(int64_t, r, value) {
      width += printf("%" PRIi64 ".%09" PRIi64 " ", value/DR_NS_PER_S, value%DR_NS_PER_S);
    } DR_FI_RESULT;
  }
  width += printf("%s(%s:%i)", func, file, line);
  static int max_width;
  if (dr_unlikely(width > max_width)) {
    max_width = width;
  }
  printf("%*s : ", max_width - width, "");
}

static void dr_log_epilog(void) {
  fflush(stdout);
}

void dr_log_impl(const char *restrict const func, const char *restrict const file, const int line, const char *restrict const msg) {
  dr_log_prolog(func, file, line);
  printf("%s\n", msg);
  dr_log_epilog();
}

void dr_log_error_impl(const char *restrict const func, const char *restrict const file, const int line, const char *restrict const msg, const struct dr_error *restrict const error) {
  dr_log_prolog(func, file, line);
  char buf[1<<6];
  const char *restrict bufp;
  bool newline = true;
  switch (error->domain) {
  case DR_ERR_ISO_C:
    bufp = strerror(error->num);
    break;
#if defined(_WIN32)
  case DR_ERR_WIN:
    // DR What happens on truncation? Is the trailing \0 added?
    if (dr_unlikely(FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error->num, 0, buf, sizeof(buf) - 1, NULL) == 0)) {
      snprintf(buf, sizeof(buf), "An unknown error occurred\n");
    }
    bufp = buf;
    newline = false;
    break;
#endif
  case DR_ERR_GAI:
    bufp = gai_strerror(error->num);
    break;
  default:
    snprintf(buf, sizeof(buf), "An unknown error occurred");
    bufp = buf;
    break;
  }
  printf("at %s(%s:%i): %s: %s%s", error->func, error->file, error->line, msg, bufp, newline ? "\n" : "");
  dr_log_epilog();
}

// This ommits func/file/line, but is convienent where it's being used. If that's not desireable, multiple versions could be created
int dr_log_format(char *restrict const buf, size_t size, const struct dr_error *restrict const error) {
  const char *restrict str;
  switch (error->domain) {
  case DR_ERR_ISO_C:
    str = strerror(error->num);
    goto print;
#if defined(_WIN32)
  case DR_ERR_WIN: {
    // DR What happens on truncation? Is the trailing \0 added?
    if (dr_unlikely(FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error->num, 0, buf, size - 1, NULL) == 0)) {
      str = "An unknown error occurred";
      goto print;
    }
    return strlen(buf);
  }
#endif
  case DR_ERR_GAI:
    str = gai_strerror(error->num);
    goto print;
  default:
    str = "An unknown error occurred";
    goto print;
  }
 print:
  return snprintf(buf, size, "%s", str);
}

void dr_assert_fail(const char *restrict const func, const char *restrict const file, const int line, const char *restrict const cond) {
  dr_log_prolog(func, file, line);
  printf("`%s` failed\n", cond);
  dr_log_epilog();
  abort();
}
