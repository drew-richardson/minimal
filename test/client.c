// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

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

  {
    const struct dr_result_void r = dr_socket_startup();
    DR_IF_RESULT_ERR(r, err) {
      dr_log_error("dr_socket_startup failed", err);
      goto fail;
    } DR_FI_RESULT;
  }

  {
    const struct dr_result_void r = dr_console_startup();
    DR_IF_RESULT_ERR(r, err) {
      dr_log_error("dr_console_startup failed", err);
      goto fail;
    } DR_FI_RESULT;
  }

  struct dr_io_handle ih;
  for (int i = 0;; ++i) {
    const struct dr_result_void r = dr_sock_connect(&ih, "localhost", port, DR_CLOEXEC);
    DR_IF_RESULT_ERR(r, err) {
      if (i < 2) {
	const struct dr_result_void r1 = dr_system_sleep_ns(10*DR_NS_PER_MS);
	(void)r1;
	continue;
      }
      dr_log_error("dr_sock_connect failed", err);
      goto fail;
    } DR_FI_RESULT;
    break;
  }

  char buf[64];
  while (true) {
    {
      size_t bytes;
      {
	const struct dr_result_size r = dr_read(dr_stdin, buf, sizeof(buf));
	DR_IF_RESULT_ERR(r, err) {
#if defined(_WIN32)
	  if (err->domain == DR_ERR_WIN && err->num == 109/*ERROR_BROKEN_PIPE*/) {
	    bytes = 0;
	  } else
#endif
	    {
	      dr_log_error("dr_read failed", err);
	      goto fail_close_cfd;
	    }
	} DR_ELIF_RESULT_OK(size_t, r, value) {
	  bytes = value;
	} DR_FI_RESULT;
      }
      if (bytes == 0) {
	break;
      }

      {
	const struct dr_result_size r = ih.io.vtbl->write(&ih.io, buf, bytes);
	DR_IF_RESULT_ERR(r, err) {
	  dr_log_error("dr_io::write failed", err);
	  goto fail_close_cfd;
	} DR_FI_RESULT;
      }
    }
    {
      size_t bytes;
      {
	const struct dr_result_size r = ih.io.vtbl->read(&ih.io, buf, sizeof(buf));
	DR_IF_RESULT_ERR(r, err) {
	  dr_log_error("dr_io::read failed", err);
	  goto fail_close_cfd;
	} DR_ELIF_RESULT_OK(size_t, r, value) {
	  bytes = value;
	} DR_FI_RESULT;
      }
      if (bytes == 0) {
	break;
      }

      {
	const struct dr_result_size r = dr_write(dr_stdout, buf, bytes);
	DR_IF_RESULT_ERR(r, err) {
	  dr_log_error("dr_write failed", err);
	  goto fail_close_cfd;
	} DR_FI_RESULT;
      }
    }
  }

  result = 0;
 fail_close_cfd:
  ih.io.vtbl->close(&ih.io);
 fail:
  return result;
}
