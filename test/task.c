// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

#include <stdio.h>

#define STACK_SIZE (1<<16)

#define DEFS \
  int v00 = buf[0x00]; int v01 = buf[0x01]; \
  int v02 = buf[0x02]; int v03 = buf[0x03]; \
  int v04 = buf[0x04]; int v05 = buf[0x05]; \
  int v06 = buf[0x06]; int v07 = buf[0x07]; \
  int v08 = buf[0x08]; int v09 = buf[0x09]; \
  int v0a = buf[0x0a]; int v0b = buf[0x0b]; \
  int v0c = buf[0x0c]; int v0d = buf[0x0d]; \
  int v0e = buf[0x0e]; int v0f = buf[0x0f]; \
  int v10 = buf[0x10]; int v11 = buf[0x11]; \
  int v12 = buf[0x12]; int v13 = buf[0x13]; \
  int v14 = buf[0x14]; int v15 = buf[0x15]; \
  int v16 = buf[0x16]; int v17 = buf[0x17]; \
  int v18 = buf[0x18]; int v19 = buf[0x19]; \
  int v1a = buf[0x1a]; int v1b = buf[0x1b]; \
  int v1c = buf[0x1c]; int v1d = buf[0x1d]; \
  int v1e = buf[0x1e]; int v1f = buf[0x1f]

#define INC \
  v00 += v01; v01 += v02; v02 += v03; v03 += v04; \
  v04 += v05; v05 += v06; v06 += v07; v07 += v08; \
  v08 += v09; v09 += v0a; v0a += v0b; v0b += v0c; \
  v0c += v0d; v0d += v0e; v0e += v0f; v0f += v10; \
  v10 += v11; v11 += v12; v12 += v13; v13 += v14; \
  v14 += v15; v15 += v16; v16 += v17; v17 += v18; \
  v18 += v19; v19 += v1a; v1a += v1b; v1b += v1c; \
  v1c += v1d; v1d += v1e; v1e += v1f; v1f += v00

#define INCx02 INC;    INC
#define INCx04 INCx02; INCx02
#define INCx08 INCx04; INCx04
#define INCx10 INCx08; INCx08
#define INCx20 INCx10; INCx10

static void client_func(void *restrict const arg) {
  const int *restrict const argc = (const int *)arg;
  char buf[32];
  snprintf(buf, sizeof(buf), "one%itwo%ithree%ifour%ifive%isix%isev", *argc + 1, *argc + 2, *argc + 3, *argc + 4, *argc + 5, *argc + 6);
  DEFS;
  INCx20;
  printf("b%s%i%i", buf, *argc, v00 + 444858010);
  dr_schedule(false);
  INCx20;
  printf("e%s%i%i", buf, *argc, v00 + 80230891);
}

static void exit_func(void *restrict const arg) {
  char *restrict const s = (char *)arg;
  printf("hCleanup%s", s);
}

static void sleep_func(void *restrict const arg) {
  char *restrict const s = (char *)arg;
  printf("cSleeping%s", s);
  dr_schedule(true);
  printf("gExiting%s", s);
  dr_task_exit(arg, exit_func);
}

// Declared outside of main so gdb knows about them
static struct dr_task t0;
static struct dr_task t1;
static char foo[] = "foo";

int main(int argc, char *argv[]) {
  (void)argv;

  {
    const struct dr_result_void r = dr_task_create(&t0, STACK_SIZE, client_func, &argc);
    DR_IF_RESULT_ERR(r, err) {
      dr_log_error("dr_task_create failed", err);
      return -1;
    } DR_FI_RESULT;
  }
  {
    const struct dr_result_void r = dr_task_create(&t1, STACK_SIZE, sleep_func, foo);
    DR_IF_RESULT_ERR(r, err) {
      dr_log_error("dr_task_create failed", err);
      return -1;
    } DR_FI_RESULT;
  }

  char buf[32];
  snprintf(buf, sizeof(buf), "one%itwo%ithree%ifour%ifive%isix%isev", argc + 1, argc + 2, argc + 3, argc + 4, argc + 5, argc + 6);
  DEFS;
  INCx20;
  printf("a%s%i%i", buf, argc, v00 + 444858010);
  dr_schedule(false);
  INCx20;
  printf("d%s%i%i", buf, argc, v00 + 80230891);
  dr_schedule(false);
  INCx20;
  printf("f%s%i%i", buf, argc, v00 - 1026488990);
  dr_task_runnable(&t1);
  dr_schedule(false);
  printf("iBack");

  dr_task_destroy(&t1);
  dr_task_destroy(&t0);

  return 0;
}
