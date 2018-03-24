// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

__attribute__((warn_unused_result))
int foo(void) {
  return 0;
}

int main(void) {
  return foo();
}
