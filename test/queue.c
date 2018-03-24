// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

#include <stdio.h>

int main(void) {
  struct {
    size_t read_pos;
    size_t write_pos;
    char buf[16];
  } q = {
    .read_pos = 0,
  };

  // 0123456789012345
  // ????????????????
  // | read_pos
  // | write_pos
  // readable = 0
  // writable = 15
  q.read_pos = 0;
  q.write_pos = 0;
  dr_assert(DR_QUEUE_READABLE(&q) == 0);
  dr_assert(DR_QUEUE_WRITABLE(&q) == 15);

  // 0123456789012345
  // ????????????????
  //  | read_pos
  //  | write_pos
  // readable = 0
  // writable = 15
  q.read_pos = 1;
  q.write_pos = 1;
  dr_assert(DR_QUEUE_READABLE(&q) == 0);
  dr_assert(DR_QUEUE_WRITABLE(&q) == 15);

  // 0123456789012345
  // ????????????????
  //       | read_pos
  //       | write_pos
  // readable = 0
  // writable = 15
  q.read_pos = 6;
  q.write_pos = 6;
  dr_assert(DR_QUEUE_READABLE(&q) == 0);
  dr_assert(DR_QUEUE_WRITABLE(&q) == 10);

  // 0123456789012345
  // ????????????????
  //               | read_pos
  //               | write_pos
  // readable = 0
  // writable = 2
  q.read_pos = 14;
  q.write_pos = 14;
  dr_assert(DR_QUEUE_READABLE(&q) == 0);
  dr_assert(DR_QUEUE_WRITABLE(&q) == 2);

  // 0123456789012345
  // ????????????????
  //                | read_pos
  //                | write_pos
  // readable = 0
  // writable = 1
  q.read_pos = 15;
  q.write_pos = 15;
  dr_assert(DR_QUEUE_READABLE(&q) == 0);
  dr_assert(DR_QUEUE_WRITABLE(&q) == 1);

  // 0123456789012345
  // xxxxx???????????
  // | read_pos
  //      | write_pos
  // readable = 5
  // writable = 10
  q.read_pos = 0;
  q.write_pos = 5;
  dr_assert(DR_QUEUE_READABLE(&q) == 5);
  dr_assert(DR_QUEUE_WRITABLE(&q) == 10);

  // 0123456789012345
  // ?xxxxx??????????
  //  | read_pos
  //       | write_pos
  // readable = 5
  // writable = 10
  q.read_pos = 1;
  q.write_pos = 6;
  dr_assert(DR_QUEUE_READABLE(&q) == 5);
  dr_assert(DR_QUEUE_WRITABLE(&q) == 10);

  // 0123456789012345
  // ???xxxxx????????
  //    | read_pos
  //         | write_pos
  // readable = 5
  // writable = 8
  q.read_pos = 3;
  q.write_pos = 8;
  dr_assert(DR_QUEUE_READABLE(&q) == 5);
  dr_assert(DR_QUEUE_WRITABLE(&q) == 8);

  // 0123456789012345
  // ?????????xxxxx??
  //          | read_pos
  //               | write_pos
  // readable = 5
  // writable = 2
  q.read_pos = 9;
  q.write_pos = 14;
  dr_assert(DR_QUEUE_READABLE(&q) == 5);
  dr_assert(DR_QUEUE_WRITABLE(&q) == 2);

  // 0123456789012345
  // ??????????xxxxx?
  //           | read_pos
  //                | write_pos
  // readable = 5
  // writable = 1
  q.read_pos = 10;
  q.write_pos = 15;
  dr_assert(DR_QUEUE_READABLE(&q) == 5);
  dr_assert(DR_QUEUE_WRITABLE(&q) == 1);

  // 0123456789012345
  // ???????????xxxxx
  //            | read_pos
  // | write_pos
  // readable = 5
  // writable = 10
  q.read_pos = 11;
  q.write_pos = 0;
  dr_assert(DR_QUEUE_READABLE(&q) == 5);
  dr_assert(DR_QUEUE_WRITABLE(&q) == 10);

  // 0123456789012345
  // x???????????xxxx
  //             | read_pos
  //  | write_pos
  // readable = 4
  // writable = 10
  q.read_pos = 12;
  q.write_pos = 1;
  dr_assert(DR_QUEUE_READABLE(&q) == 4);
  dr_assert(DR_QUEUE_WRITABLE(&q) == 10);

  // 0123456789012345
  // xxx???????????xx
  //               | read_pos
  //    | write_pos
  // readable = 2
  // writable = 10
  q.read_pos = 14;
  q.write_pos = 3;
  dr_assert(DR_QUEUE_READABLE(&q) == 2);
  dr_assert(DR_QUEUE_WRITABLE(&q) == 10);

  // 0123456789012345
  // xxxx???????????x
  //                | read_pos
  //     | write_pos
  // readable = 1
  // writable = 10
  q.read_pos = 15;
  q.write_pos = 4;
  dr_assert(DR_QUEUE_READABLE(&q) == 1);
  dr_assert(DR_QUEUE_WRITABLE(&q) == 10);

  // 0123456789012345
  // xxxxxxxxxxxxxxx?
  // | read_pos
  //                | write_pos
  // readable = 15
  // writable = 0
  q.read_pos = 0;
  q.write_pos = 15;
  dr_assert(DR_QUEUE_READABLE(&q) == 15);
  dr_assert(DR_QUEUE_WRITABLE(&q) == 0);

  // 0123456789012345
  // ?xxxxxxxxxxxxxxx
  //  | read_pos
  // | write_pos
  // readable = 15
  // writable = 0
  q.read_pos = 1;
  q.write_pos = 0;
  dr_assert(DR_QUEUE_READABLE(&q) == 15);
  dr_assert(DR_QUEUE_WRITABLE(&q) == 0);

  // 0123456789012345
  // x?xxxxxxxxxxxxxx
  //   | read_pos
  //  | write_pos
  // readable = 14
  // writable = 0
  q.read_pos = 2;
  q.write_pos = 1;
  dr_assert(DR_QUEUE_READABLE(&q) == 14);
  dr_assert(DR_QUEUE_WRITABLE(&q) == 0);

  // 0123456789012345
  // xxxxxx?xxxxxxxxx
  //        | read_pos
  //       | write_pos
  // readable = 9
  // writable = 0
  q.read_pos = 7;
  q.write_pos = 6;
  dr_assert(DR_QUEUE_READABLE(&q) == 9);
  dr_assert(DR_QUEUE_WRITABLE(&q) == 0);

  // 0123456789012345
  // xxxxxxxxxxxxx?xx
  //               | read_pos
  //              | write_pos
  // readable = 2
  // writable = 0
  q.read_pos = 14;
  q.write_pos = 13;
  dr_assert(DR_QUEUE_READABLE(&q) == 2);
  dr_assert(DR_QUEUE_WRITABLE(&q) == 0);

  // 0123456789012345
  // xxxxxxxxxxxxxx?x
  //                | read_pos
  //               | write_pos
  // readable = 1
  // writable = 0
  q.read_pos = 15;
  q.write_pos = 14;
  dr_assert(DR_QUEUE_READABLE(&q) == 1);
  dr_assert(DR_QUEUE_WRITABLE(&q) == 0);

  printf("OK\n");

  return 0;
}
