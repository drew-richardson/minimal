// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

struct bar {
  int a;
  int b;
};

int main(void) {
  struct bar bar = { .b = 1 };
  return bar.b;
}
