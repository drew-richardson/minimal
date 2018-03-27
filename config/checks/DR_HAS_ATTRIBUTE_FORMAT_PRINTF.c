// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

__attribute__((__format__(__printf__, 1, 2)))
int foo(const char *restrict const fmt, ...) {
  return 0;
}

int main(void) {
  foo("%s\n", "Hello world");
}
