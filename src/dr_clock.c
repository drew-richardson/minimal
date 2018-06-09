// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

#if defined(DR_OS_WINDOWS)

#include <windows.h>

struct dr_result_int64 dr_system_time_ns(void) {
  FILETIME filetime;
  GetSystemTimeAsFileTime(&filetime);
  //GetSystemTimePreciseAsFileTime(&filetime); // Windows 8 and later
  const int64_t time = ((int64_t)filetime.dwHighDateTime << 32 | filetime.dwLowDateTime) - 116444736000000000;
  return DR_RESULT_OK(int64, time*100);
}

struct dr_result_void dr_system_sleep_ns(const int64_t time) {
  Sleep(time/DR_NS_PER_MS);
  return DR_RESULT_OK_VOID();
}

#else

#include <errno.h>
#include <time.h>

struct dr_result_int64 dr_system_time_ns(void) {
  struct timespec res;
  if (dr_unlikely(clock_gettime(CLOCK_REALTIME, &res) != 0)) {
    return DR_RESULT_ERRNO(int64);
  }
  return DR_RESULT_OK(int64, DR_NS_PER_S*(int64_t)res.tv_sec + res.tv_nsec);
}

struct dr_result_void dr_system_sleep_ns(const int64_t time) {
  struct timespec request = {
    .tv_sec = time/DR_NS_PER_S,
    .tv_nsec = time%DR_NS_PER_S,
  };
  if (dr_unlikely(nanosleep(&request, NULL) != 0)) {
    return DR_RESULT_ERRNO_VOID();
  }
  return DR_RESULT_OK_VOID();
}

#endif
