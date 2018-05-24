// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAND_BUF_LEN (1<<20)
static unsigned char *restrict rand_buf;

static void prepare_random(void) {
  // Use reproducible random number generator
  unsigned int seed;
  {
    const struct dr_result_int64 r = dr_system_time_ns();
    DR_IF_RESULT_ERR(r, err) {
      dr_log_error("dr_system_time_ns failed", err);
      exit(-1);
    } DR_ELIF_RESULT_OK(size_t, r, value) {
      seed = value/100;
    } DR_FI_RESULT;
  }
  printf("seed: %u\n", seed);
  srand(seed);
  rand_buf = (unsigned char *)malloc(RAND_BUF_LEN);
  if (rand_buf == NULL) {
    dr_log("malloc failed");
    exit(-1);
  }
  for (size_t i = 0; i < RAND_BUF_LEN; i += sizeof(int)) {
    *(int*)(rand_buf + i) = rand();
  }
}

static void test_queue(void) {
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
}

static void test_ro_fixed(void) {
  struct dr_io_ro_fixed io;
  const char hello_world[] = {'H','e','l','l','o',' ','w','o','r','l','d'};
  dr_io_ro_fixed_init(&io, hello_world, sizeof(hello_world));
  char buf[16];
  {
    const struct dr_result_size r = io.io.vtbl->read(&io.io, buf, 2);
    DR_IF_RESULT_ERR(r, err) {
      (void)err;
      dr_assert(false);
    } DR_ELIF_RESULT_OK(size_t, r, value) {
      dr_assert(value == 2);
      dr_assert(memcmp(buf, "He", 2) == 0);
    } DR_FI_RESULT;
  }
  {
    const struct dr_result_size r = io.io.vtbl->read(&io.io, buf, 3);
    DR_IF_RESULT_ERR(r, err) {
      (void)err;
      dr_assert(false);
    } DR_ELIF_RESULT_OK(size_t, r, value) {
      dr_assert(value == 3);
      dr_assert(memcmp(buf, "llo", 3) == 0);
    } DR_FI_RESULT;
  }
  {
    const struct dr_result_size r = io.io.vtbl->read(&io.io, buf, 7);
    DR_IF_RESULT_ERR(r, err) {
      (void)err;
      dr_assert(false);
    } DR_ELIF_RESULT_OK(size_t, r, value) {
      dr_assert(value == 6);
      dr_assert(memcmp(buf, " world", 6) == 0);
    } DR_FI_RESULT;
  }
  {
    const struct dr_result_size r = io.io.vtbl->read(&io.io, buf, 11);
    DR_IF_RESULT_ERR(r, err) {
      (void)err;
      dr_assert(false);
    } DR_ELIF_RESULT_OK(size_t, r, value) {
      dr_assert(value == 0);
    } DR_FI_RESULT;
  }
  {
    const struct dr_result_size r = io.io.vtbl->write(&io.io, buf, sizeof(buf));
    dr_assert(DR_IS_RESULT_ERR(r));
  }
  io.io.vtbl->close(&io.io);
}

static void test_wo_fixed(void) {
  struct dr_io_wo io;
  char buf[8];
  dr_io_wo_fixed_init(&io, buf, sizeof(buf));
  {
    const struct dr_result_size r = io.io.vtbl->write(&io.io, "dr", 2);
    DR_IF_RESULT_ERR(r, err) {
      (void)err;
      dr_assert(false);
    } DR_ELIF_RESULT_OK(size_t, r, value) {
      dr_assert(value == 2);
      dr_assert(io.pos == 2);
      dr_assert(memcmp(buf, "dr", 2) == 0);
    } DR_FI_RESULT;
  }
  {
    const struct dr_result_size r = io.io.vtbl->write(&io.io, "ewr", 3);
    DR_IF_RESULT_ERR(r, err) {
      (void)err;
      dr_assert(false);
    } DR_ELIF_RESULT_OK(size_t, r, value) {
      dr_assert(value == 3);
      dr_assert(io.pos == 5);
      dr_assert(memcmp(buf, "drewr", 5) == 0);
    } DR_FI_RESULT;
  }
  {
    const struct dr_result_size r = io.io.vtbl->write(&io.io, "ichar", 5);
    DR_IF_RESULT_ERR(r, err) {
      (void)err;
      dr_assert(false);
    } DR_ELIF_RESULT_OK(size_t, r, value) {
      dr_assert(value == 3);
      dr_assert(io.pos == 8);
      dr_assert(memcmp(buf, "drewrich", 8) == 0);
    } DR_FI_RESULT;
  }
  {
    const struct dr_result_size r = io.io.vtbl->write(&io.io, "dson", 4);
    DR_IF_RESULT_ERR(r, err) {
      (void)err;
      dr_assert(false);
    } DR_ELIF_RESULT_OK(size_t, r, value) {
      dr_assert(value == 0);
      dr_assert(io.pos == 8);
      dr_assert(memcmp(buf, "drewrich", 8) == 0);
    } DR_FI_RESULT;
  }
  {
    const struct dr_result_size r = io.io.vtbl->read(&io.io, buf, sizeof(buf));
    dr_assert(DR_IS_RESULT_ERR(r));
  }
  io.io.vtbl->close(&io.io);
}

static void test_wo_resize(void) {
  struct dr_io_wo io;
  {
    const struct dr_result_void r = dr_io_wo_resize_init(&io, 32);
    dr_assert(DR_IS_RESULT_OK(r));
  }
  {
    const struct dr_result_size r = io.io.vtbl->write(&io.io, rand_buf + 0, 17);
    DR_IF_RESULT_ERR(r, err) {
      (void)err;
      dr_assert(false);
    } DR_ELIF_RESULT_OK(size_t, r, value) {
      dr_assert(value == 17);
      dr_assert(io.pos == 17);
      dr_assert(io.count >= io.pos);
    } DR_FI_RESULT;
  }
  {
    const struct dr_result_size r = io.io.vtbl->write(&io.io, rand_buf + 17, 208);
    DR_IF_RESULT_ERR(r, err) {
      (void)err;
      dr_assert(false);
    } DR_ELIF_RESULT_OK(size_t, r, value) {
      dr_assert(value == 208);
      dr_assert(io.pos == 225);
      dr_assert(io.count >= io.pos);
    } DR_FI_RESULT;
  }
  {
    const struct dr_result_size r = io.io.vtbl->write(&io.io, rand_buf + 225, 77);
    DR_IF_RESULT_ERR(r, err) {
      (void)err;
      dr_assert(false);
    } DR_ELIF_RESULT_OK(size_t, r, value) {
      dr_assert(value == 77);
      dr_assert(io.pos == 302);
      dr_assert(io.count >= io.pos);
    } DR_FI_RESULT;
  }
  {
    const struct dr_result_size r = io.io.vtbl->write(&io.io, rand_buf + 302, 494);
    DR_IF_RESULT_ERR(r, err) {
      (void)err;
      dr_assert(false);
    } DR_ELIF_RESULT_OK(size_t, r, value) {
      dr_assert(value == 494);
      dr_assert(io.pos == 796);
      dr_assert(io.count >= io.pos);
    } DR_FI_RESULT;
  }
  dr_assert(memcmp(io.buf, rand_buf, 796) == 0);
  io.io.vtbl->close(&io.io);
}

static void test_rw_fixed(void) {
  struct dr_io_rw io;
  char io_buf[32];
  dr_io_rw_fixed_init(&io, io_buf, sizeof(io_buf));
  size_t read_pos = 0;
  size_t write_pos = 0;
  while (true) {
    const int rint = rand();
    const bool do_read = (rint & 1) == 1;
    const size_t bytes = (rint >> 1)%(sizeof(io_buf) - 1) + 1;
    if (do_read) {
      char buf[sizeof(io_buf)];
      {
	const struct dr_result_size r = io.io.vtbl->read(&io.io, buf, bytes);
	DR_IF_RESULT_ERR(r, err) {
	  dr_assert(read_pos == write_pos);
	  dr_assert(err->domain == DR_ERR_ISO_C);
	  dr_assert(err->num == EAGAIN);
	} DR_ELIF_RESULT_OK(size_t, r, value) {
	  dr_assert(value > 0);
	  dr_assert(value <= bytes);
	  dr_assert(value == bytes || value == write_pos - read_pos);
	  dr_assert(memcmp(rand_buf + read_pos, buf, value) == 0);
	  read_pos += value;
	} DR_FI_RESULT;
      }
    } else {
      if (write_pos + bytes > RAND_BUF_LEN) {
	break;
      }
      {
	const struct dr_result_size r = io.io.vtbl->write(&io.io, rand_buf + write_pos, bytes);
	DR_IF_RESULT_ERR(r, err) {
	  dr_assert(read_pos + 32 == write_pos);
	  dr_assert(err->domain == DR_ERR_ISO_C);
	  dr_assert(err->num == EAGAIN);
	} DR_ELIF_RESULT_OK(size_t, r, value) {
	  dr_assert(value > 0);
	  dr_assert(value <= bytes);
	  dr_assert(value == bytes || value == sizeof(io_buf) + read_pos - write_pos);
	  write_pos += value;
	} DR_FI_RESULT;
      }
    }
    dr_assert(read_pos <= write_pos);
    dr_assert(write_pos <= RAND_BUF_LEN);
  }
  io.io.vtbl->close(&io.io);
}

static void test_rw_resize(void) {
  struct dr_io_rw io;
  {
    const struct dr_result_void r = dr_io_rw_resize_init(&io, 32);
    dr_assert(DR_IS_RESULT_OK(r));
  }
  size_t read_pos = 0;
  size_t write_pos = 0;
  size_t size = 32;
  while (true) {
    const int rint = rand();
    const bool do_read = (rint & 1) == 1;
    const size_t bytes = (rint >> 1)%size + 1;
    size += 1;
    if (do_read) {
      char buf[4096];
      dr_assert(bytes <= sizeof(buf));
      {
	const struct dr_result_size r = io.io.vtbl->read(&io.io, buf, bytes);
	DR_IF_RESULT_ERR(r, err) {
	  dr_assert(read_pos == write_pos);
	  dr_assert(err->domain == DR_ERR_ISO_C);
	  dr_assert(err->num == EAGAIN);
	} DR_ELIF_RESULT_OK(size_t, r, value) {
	  dr_assert(value > 0);
	  dr_assert(value <= bytes);
	  dr_assert(value == bytes || value == write_pos - read_pos);
	  dr_assert(memcmp(rand_buf + read_pos, buf, value) == 0);
	  read_pos += value;
	} DR_FI_RESULT;
      }
    } else {
      if (write_pos + bytes > RAND_BUF_LEN) {
	break;
      }
      {
	const struct dr_result_size r = io.io.vtbl->write(&io.io, rand_buf + write_pos, bytes);
	DR_IF_RESULT_ERR(r, err) {
	  (void)err;
	  dr_assert(false);
	} DR_ELIF_RESULT_OK(size_t, r, value) {
	  dr_assert(value == bytes);
	  write_pos += value;
	} DR_FI_RESULT;
      }
    }
    dr_assert(read_pos <= write_pos);
    dr_assert(write_pos <= RAND_BUF_LEN);
  }
  io.io.vtbl->close(&io.io);
}

int main(void) {
  prepare_random();
  test_queue();
  test_ro_fixed();
  test_wo_fixed();
  test_wo_resize();
  test_rw_fixed();
  test_rw_resize();
  printf("OK\n");
  return 0;
}
