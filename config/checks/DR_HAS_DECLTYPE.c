// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

int main(void) {
  int a = 0;
  decltype(a) b = a;
  return b;
}
