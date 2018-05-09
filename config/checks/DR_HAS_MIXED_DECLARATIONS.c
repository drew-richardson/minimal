// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

int main(void) {
  int a;
  a = 2;
  int b = a;
  for (int i = 0; i < 10; ++i) {
    b += i;
  }
  return 0;
}
