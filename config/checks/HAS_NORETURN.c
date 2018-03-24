// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include <stdnoreturn.h>

_Noreturn
void foo(void) {
  for (;;);
}

int main(void) {
  foo();
}
