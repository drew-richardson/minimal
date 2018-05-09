// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

int main(void) {
  int i = ({
      int j = 1;
      int k = 2;
      j + k;
    });
  return i;
}
