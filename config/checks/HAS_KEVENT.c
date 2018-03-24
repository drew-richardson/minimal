// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

int main(void) {
  int kq = 0;
  struct kevent changelist[2];
  int nchanges = 0;
  struct kevent eventlist[2];
  int nevents = 0;
  const struct timespec timeout;
  int result = kevent(kq, changelist, nchanges, eventlist, nevents, &timeout);
  return result;
}
