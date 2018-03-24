// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

#include <errno.h>

struct dr_waiter {
  struct list_head waiters;
  struct dr_task *restrict task;
};

void dr_wait_init(struct dr_wait *restrict const wait) {
  *wait = (struct dr_wait) {
    .waiters = LIST_HEAD_INIT(wait->waiters),
  };
}

void dr_wait_destroy(struct dr_wait *restrict const wait) {
  (void)wait;
  // Nothing to do
}

void dr_wait_notify(struct dr_wait *restrict const wait) {
  if (!list_empty(&wait->waiters)) {
    struct dr_waiter *restrict const waiter = list_first_entry(&wait->waiters, struct dr_waiter, waiters);
    list_del(&waiter->waiters);
    dr_task_runnable(waiter->task);
  }
}

void dr_wait_wait(struct dr_wait *restrict const wait) {
  struct dr_waiter waiter;
  list_add_tail(&waiter.waiters, &wait->waiters);
  waiter.task = dr_task_self();
  dr_schedule(true);
}

static const unsigned int dr_sem_value_max = 0x7fffffff;

struct dr_result_void dr_sem_init(struct dr_sem *restrict const sem, unsigned int value) {
  if (value > dr_sem_value_max) {
    return DR_RESULT_ERRNUM_VOID(DR_ERR_ISO_C, EINVAL);
  }
  *sem = (struct dr_sem) {
    .value = value,
  };
  dr_wait_init(&sem->wait);
  return DR_RESULT_OK_VOID();
}

void dr_sem_destroy(struct dr_sem *restrict const sem) {
  dr_wait_destroy(&sem->wait);
}

struct dr_result_void dr_sem_post(struct dr_sem *restrict const sem) {
  if (sem->value > dr_sem_value_max) {
    return DR_RESULT_ERRNUM_VOID(DR_ERR_ISO_C, EOVERFLOW);
  }
  ++sem->value;
  dr_wait_notify(&sem->wait);
  return DR_RESULT_OK_VOID();
}

struct dr_result_void dr_sem_wait(struct dr_sem *restrict const sem) {
  while (sem->value <= 0) {
    dr_wait_wait(&sem->wait);
  }
  --sem->value;
  return DR_RESULT_OK_VOID();
}
