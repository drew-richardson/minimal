// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#if defined(_WIN32)

#include <windows.h>

typedef int socklen_t;

#else

#include <sys/types.h>
#include <sys/socket.h>

#endif

int main(void) {
  int fd;
  int sockfd = 0;
  struct sockaddr addr;
  socklen_t addrlen = 0;
  fd = accept(sockfd, &addr, &addrlen);
  return fd;
}
