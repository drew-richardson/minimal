// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#if defined(__linux__)
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/socket.h>

int main(void) {
  int fd = 0;
  int sockfd = 0;
  struct sockaddr addr;
  socklen_t addrlen = 0;
  int flags = 0;
  fd = accept4(sockfd, &addr, &addrlen, flags);
  return fd;
}
