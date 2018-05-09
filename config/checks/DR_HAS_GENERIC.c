// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#define neg(x) _Generic((x), long: negsl, default: negsi)(x)

long negsl(long l) {
  return -l;
}

int negsi(int l) {
  return -l;
}

int main(void) {
  long sl = -1;
  int si = -1;

  sl = neg(sl);
  si = neg(si);

  return sl + si;
}
