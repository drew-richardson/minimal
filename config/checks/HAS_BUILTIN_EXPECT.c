// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

int main(void) {
  int i = 0;
  if (__builtin_expect(i, 0)) {
    return -1;
  }
  return 0;
}
