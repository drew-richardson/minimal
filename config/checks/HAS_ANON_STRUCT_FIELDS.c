// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

struct bar {
  union {
    int i;
    double d;
  };
  struct {
    int a;
    int b;
  };
};

int main(void) {
  struct bar bar;
  bar.d = 2.3;
  bar.a = 4;
  bar.b = 5;
  return bar.i + bar.a + bar.b;
}
