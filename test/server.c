// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define STACK_SIZE (1<<16)

struct client {
  struct list_head clients;
  struct dr_wait read_wait;
  struct dr_task read_task;
  struct dr_wait write_wait;
  struct dr_task write_task;
  struct dr_equeue_client c;
  size_t read_pos;
  size_t write_pos;
  bool cleanup;
  char buf[1<<10];
};

static struct list_head clients;
static struct dr_equeue equeue;
static bool cleanup;
static struct dr_equeue_server server;

static void read_func(void *restrict const arg);
static void write_func(void *restrict const arg);

static WARN_UNUSED_RESULT struct dr_result_void client_init(dr_handle_t fd) {
  struct client *restrict const c = (struct client *)malloc(sizeof(*c));
  if (c == NULL) {
    return DR_RESULT_ERRNO_VOID();
  }
  *c = (struct client) {
    .clients = LIST_HEAD_INIT(c->clients),
  };
  dr_equeue_client_init(&c->c, fd);
  dr_wait_init(&c->read_wait);
  {
    const struct dr_result_void r = dr_task_create(&c->read_task, STACK_SIZE, read_func, c);
    DR_IF_RESULT_ERR(r, err) {
      dr_wait_destroy(&c->read_wait);
      dr_equeue_client_destroy(&c->c);
      free(c);
      return DR_RESULT_ERROR_VOID(err);
    } DR_FI_RESULT;
  }
  dr_wait_init(&c->write_wait);
  {
    const struct dr_result_void r = dr_task_create(&c->write_task, STACK_SIZE, write_func, c);
    DR_IF_RESULT_ERR(r, err) {
      dr_wait_destroy(&c->write_wait);
      dr_task_destroy(&c->read_task);
      dr_wait_destroy(&c->read_wait);
      dr_equeue_client_destroy(&c->c);
      free(c);
      return DR_RESULT_ERROR_VOID(err);
    } DR_FI_RESULT;
  }
  list_add_tail(&c->clients, &clients);
  return DR_RESULT_OK_VOID();
}

static void client_destroy(struct client *restrict const c) {
  dr_log("Closing client");
  list_del(&c->clients);
  dr_task_destroy(&c->write_task);
  dr_wait_destroy(&c->write_wait);
  dr_task_destroy(&c->read_task);
  dr_wait_destroy(&c->read_wait);
  dr_equeue_client_destroy(&c->c);
  free(c);
}

static void read_func(void *restrict const arg) {
  struct client *restrict const c = (struct client *)arg;
  while (dr_likely(!cleanup)) {
    while (DR_QUEUE_WRITABLE(c) <= 0) {
      if (cleanup) {
	goto done;
      }
      dr_wait_wait(&c->read_wait);
    }
    size_t bytes;
    {
      const struct dr_result_size r = dr_equeue_read(&equeue, &c->c, c->buf + c->write_pos, DR_QUEUE_WRITABLE(c));
      DR_IF_RESULT_ERR(r, err) {
	dr_log_error("dr_equeue_read failed", err);
	goto done;
      } DR_ELIF_RESULT_OK(size_t, r, value) {
	bytes = value;
      } DR_FI_RESULT;
    }
    if (bytes == 0) {
      goto done; // DR ...
    }
    c->write_pos = (c->write_pos + bytes) % sizeof(c->buf);
    dr_wait_notify(&c->write_wait);
  }
 done:
  if (c->cleanup) {
    dr_task_exit(c, (void (*)(void *restrict const))client_destroy);
  }
  c->cleanup = true;
  dr_wait_notify(&c->write_wait);
  dr_log("Exiting");
}

static void write_func(void *restrict const arg) {
  struct client *restrict const c = (struct client *)arg;
  while (dr_likely(!cleanup)) {
    while (DR_QUEUE_READABLE(c) <= 0) {
      if (cleanup || c->cleanup) {
	goto done;
      }
      dr_wait_wait(&c->write_wait);
    }
    size_t bytes;
    {
      const struct dr_result_size r = dr_equeue_write(&equeue, &c->c, c->buf + c->read_pos, DR_QUEUE_READABLE(c));
      DR_IF_RESULT_ERR(r, err) {
	dr_log_error("dr_equeue_write failed", err);
	goto done;
      } DR_ELIF_RESULT_OK(size_t, r, value) {
	bytes = value;
      } DR_FI_RESULT;
    }
    if (bytes == 0) {
      goto done; // DR ...
    }
    c->read_pos = (c->read_pos + bytes) % sizeof(c->buf);
    dr_wait_notify(&c->read_wait);
  }
 done:
  if (c->cleanup) {
    dr_task_exit(c, (void (*)(void *restrict const))client_destroy);
  }
  c->cleanup = true;
  dr_wait_notify(&c->read_wait);
  dr_log("Exiting");
}

static void server_func(void *restrict const arg) {
  const char *restrict const port = (char *)arg;
  {
    dr_handle_t sfd;
    {
      const struct dr_result_handle r = dr_sock_bind("localhost", port, DR_CLOEXEC | DR_NONBLOCK | DR_REUSEADDR);
      DR_IF_RESULT_ERR(r, err) {
	dr_log_error("dr_sock_bind failed", err);
	goto fail;
      } DR_ELIF_RESULT_OK(dr_handle_t, r, value) {
	sfd = value;
      } DR_FI_RESULT;
    }
    {
      const struct dr_result_void r = dr_listen(sfd, 16);
      DR_IF_RESULT_ERR(r, err) {
	dr_log_error("dr_listen failed", err);
	dr_close(sfd);
	goto fail;
      } DR_FI_RESULT;
    }
    dr_equeue_server_init(&server, sfd);
  }
  while (dr_likely(!cleanup)) {
    dr_handle_t cfd;
    {
      const struct dr_result_handle r = dr_equeue_accept(&equeue, &server);
      DR_IF_RESULT_ERR(r, err) {
	dr_log_error("dr_equeue_accept failed", err);
	goto fail_equeue_server_destroy;
      } DR_ELIF_RESULT_OK(dr_handle_t, r, value) {
	cfd = value;
      } DR_FI_RESULT;
    }
    dr_log("Accepted client");
    {
      const struct dr_result_void r = client_init(cfd);
      DR_IF_RESULT_ERR(r, err) {
	dr_log_error("client_init failed", err);
	goto fail_equeue_server_destroy;
      } DR_FI_RESULT;
    }
  }
 fail_equeue_server_destroy:
  dr_equeue_server_destroy(&server);
 fail:
  cleanup = true;
}

/*
#if defined(WIN32)

#include <windows.h>

static int handler(unsigned long type) {
  (void)type;
  static bool subsequent;
  if (subsequent) {
    return false;
  }
  subsequent = true;
  dr_log("Cleaning up");
  cleanup = true;
  // PostQueuedCompletionStatus(iocp, 0, 0, NULL)
  return true;
}

// DR ...
static struct dr_result_void foo(void) {
  if (dr_unlikely(SetConsoleCtrlHandler(handler, true) == 0)) {
    return DR_RESULT_GETLASTERROR_VOID();
  }
  return DR_RESULT_OK_VOID();
}

#else

#include <signal.h>

static void handler(int signum) {
  (void)signum;
  dr_log("Cleaning up");
  cleanup = true;
  // Write to pipe
}

// DR ...
static struct dr_result_void foo(void) {
  struct sigaction act = {
    .sa_handler = handler,
    .sa_flags = SA_RESETHAND,
  };
  if (dr_unlikely(sigaction(SIGINT, &act, NULL) ||
		  sigaction(SIGTERM, &act, NULL))) {
    return DR_RESULT_ERRNO_VOID();
  }
  return DR_RESULT_OK_VOID();
}

#endif
*/

// DR struct dr_result_void dr_equeue_dispatch(struct dr_equeue *restrict const e)

int main(int argc, char *argv[]) {
  int result = -1;
  char *restrict port = NULL;
  {
    static struct dr_option longopts[] = {
      {"port", 1, 0, 'p'},
      {0, 0, 0, 0},
    };
    dr_optind = 0;
    while (true) {
      int opt = dr_getopt_long(argc, argv, "+p:", longopts, NULL);
      if (opt == -1) {
	break;
      }
      switch (opt) {
      case 'p':
	port = dr_optarg;
	break;
      default:
	// DR Add help
	break;
      }
    }
  }
  if (port == NULL) {
    // DR ...
    goto fail;
  }
  INIT_LIST_HEAD(&clients);
  {
    const struct dr_result_void r = dr_socket_startup();
    DR_IF_RESULT_ERR(r, err) {
      dr_log_error("dr_socket_startup failed", err);
      goto fail;
    } DR_FI_RESULT;
  }
  //foo();
  {
    const struct dr_result_void r = dr_equeue_init(&equeue);
    DR_IF_RESULT_ERR(r, err) {
      dr_log_error("dr_equeue_init failed", err);
      goto fail;
    } DR_FI_RESULT;
  }
  struct dr_task server_task;
  {
    const struct dr_result_void r = dr_task_create(&server_task, STACK_SIZE, server_func, port);
    DR_IF_RESULT_ERR(r, err) {
      dr_log_error("dr_task_create failed", err);
      goto fail_equeue_destroy;
    } DR_FI_RESULT;
  }
  // Switch to allow server_func to run for the first time
  dr_schedule(true);
  while (dr_likely(!cleanup)) {
    // DR This stuff should likely be a method in dr_event
    struct dr_event events[16];
    unsigned int count;
    {
      const struct dr_result_uint r = dr_equeue_dequeue(&equeue, events, sizeof(events));
      DR_IF_RESULT_ERR(r, err) {
	dr_log_error("dr_equeue_dequeue failed", err);
	goto done;
      } DR_ELIF_RESULT_OK(unsigned int, r, value) {
	count = value;
      } DR_FI_RESULT;
    }
    if (count == 0) {
      continue;
    }
    for (unsigned int i = 0; i < count; ++i) {
      void *restrict const key = dr_event_key(events, i);
      if (key == &server) {
	dr_task_runnable(&server_task);
      } else {
	struct client *restrict const c = container_of(key, struct client, c);
	if (dr_event_is_read(events, i)) {
	  dr_task_runnable(&c->read_task);
	}
	if (dr_event_is_write(events, i)) {
	  dr_task_runnable(&c->write_task);
	}
      }
    }
    dr_schedule(true);
  }
 done:
  cleanup = true;
  {
    struct client *restrict c;
    struct client *restrict n;
    list_for_each_entry_safe(c, n, &clients, struct client, clients) {
      client_destroy(c);
    }
  }
  dr_task_destroy(&server_task);
 fail_equeue_destroy:
  dr_equeue_destroy(&equeue);
 fail:
  return result;
}
