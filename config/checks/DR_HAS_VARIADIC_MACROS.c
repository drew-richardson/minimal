// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#define foo(ARG, ...) foo_impl(ARG, __VA_ARGS__)

int foo_impl(int arg, ...) {
  return arg;
}

int main(void) {
  return foo(1, 2, 3);
}
