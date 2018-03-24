// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

int main(void) {
  int i = 0;
  int *restrict p = &i;
  return *p;
}
