// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#if !defined(DR_UTIL_H)
#define DR_UTIL_H

#define DR_XINTXX2(SIZE, IS_UNSIGNED)					\
  IS_UNSIGNED ? 'u' : ' ',						\
  'i','n','t',								\
  SIZE == 1 ? '8'  : SIZE == 2 ? '1' : SIZE == 4 ? '3' : SIZE == 8 ? '6' : '?', \
  SIZE == 1 ? '_'  : SIZE == 2 ? '6' : SIZE == 4 ? '2' : SIZE == 8 ? '4' : '?', \
  SIZE == 1 ? 't'  : SIZE == 2 ? '_' : SIZE == 4 ? '_' : SIZE == 8 ? '_' : '?', \
  SIZE == 1 ? ' '  : SIZE == 2 ? 't' : SIZE == 4 ? 't' : SIZE == 8 ? 't' : '?'

#define DR_XINTXX(TYPE) DR_XINTXX2(sizeof(TYPE), (TYPE)~0 > 0)

#define DR_HEXCHAR(VAL)				\
  (VAL) == 0x0 ? '0' :				\
  (VAL) == 0x1 ? '1' :				\
  (VAL) == 0x2 ? '2' :				\
  (VAL) == 0x3 ? '3' :				\
  (VAL) == 0x4 ? '4' :				\
  (VAL) == 0x5 ? '5' :				\
  (VAL) == 0x6 ? '6' :				\
  (VAL) == 0x7 ? '7' :				\
  (VAL) == 0x8 ? '8' :				\
  (VAL) == 0x9 ? '9' :				\
  (VAL) == 0xa ? 'a' :				\
  (VAL) == 0xb ? 'b' :				\
  (VAL) == 0xc ? 'c' :				\
  (VAL) == 0xd ? 'd' :				\
  (VAL) == 0xe ? 'e' :				\
  (VAL) == 0xf ? 'f' :				\
  '?'

#define DR_HEXSTR(VAL)				\
  '0','x',					\
  DR_HEXCHAR(((VAL) >> (7*4)) & 0xf),		\
  DR_HEXCHAR(((VAL) >> (6*4)) & 0xf),		\
  DR_HEXCHAR(((VAL) >> (5*4)) & 0xf),		\
  DR_HEXCHAR(((VAL) >> (4*4)) & 0xf),		\
  DR_HEXCHAR(((VAL) >> (3*4)) & 0xf),		\
  DR_HEXCHAR(((VAL) >> (2*4)) & 0xf),		\
  DR_HEXCHAR(((VAL) >> (1*4)) & 0xf),		\
  DR_HEXCHAR(((VAL) >> (0*4)) & 0xf)

#define DR_BSORT2(arg0, arg1)			\
  (arg0) < (arg1) ? (arg0) : (arg1),		\
  (arg0) < (arg1) ? (arg1) : (arg0)

#define DR_BSORT3(arg0, arg1, arg2)					\
  (arg0) < (arg1) ? ((arg0) < (arg2) ? ((arg1) < (arg2) ? (arg0) : (arg0)) : (arg2)) : (arg0) < (arg2) ? (arg1) : (arg1) < (arg2) ? (arg1) : (arg2), \
  (arg0) < (arg1) ? ((arg0) < (arg2) ? ((arg1) < (arg2) ? (arg1) : (arg2)) : (arg0)) : (arg0) < (arg2) ? (arg0) : (arg1) < (arg2) ? (arg2) : (arg1), \
  (arg0) < (arg1) ? ((arg0) < (arg2) ? ((arg1) < (arg2) ? (arg2) : (arg1)) : (arg1)) : (arg0) < (arg2) ? (arg2) : (arg1) < (arg2) ? (arg0) : (arg0)

#define DR_BSORT4(arg0, arg1, arg2, arg3)				\
  (arg0) < (arg1) ? ((arg2) < (arg3) ? ((arg0) < (arg2) ? ((arg1) < (arg2) ? (arg0) : (arg1) < (arg3) ? (arg0) : (arg0)) : (arg1) < (arg3) ? (arg2) : (arg0) < (arg3) ? (arg2) : (arg2)) : (arg1) < (arg2) ? ((arg1) < (arg3) ? (arg0) : (arg0) < (arg3) ? (arg0) : (arg3)) : (arg0) < (arg3) ? (arg0) : (arg0) < (arg2) ? (arg3) : (arg3)) : (arg2) < (arg3) ? ((arg0) < (arg3) ? ((arg0) < (arg2) ? (arg1) : (arg1) < (arg2) ? (arg1) : (arg2)) : (arg1) < (arg2) ? (arg1) : (arg1) < (arg3) ? (arg2) : (arg2)) : (arg0) < (arg2) ? ((arg0) < (arg3) ? (arg1) : (arg1) < (arg3) ? (arg1) : (arg3)) : (arg1) < (arg3) ? (arg1) : (arg1) < (arg2) ? (arg3) : (arg3), \
  (arg0) < (arg1) ? ((arg2) < (arg3) ? ((arg0) < (arg2) ? ((arg1) < (arg2) ? (arg1) : (arg1) < (arg3) ? (arg2) : (arg2)) : (arg1) < (arg3) ? (arg0) : (arg0) < (arg3) ? (arg0) : (arg3)) : (arg1) < (arg2) ? ((arg1) < (arg3) ? (arg1) : (arg0) < (arg3) ? (arg3) : (arg0)) : (arg0) < (arg3) ? (arg3) : (arg0) < (arg2) ? (arg0) : (arg2)) : (arg2) < (arg3) ? ((arg0) < (arg3) ? ((arg0) < (arg2) ? (arg0) : (arg1) < (arg2) ? (arg2) : (arg1)) : (arg1) < (arg2) ? (arg2) : (arg1) < (arg3) ? (arg1) : (arg3)) : (arg0) < (arg2) ? ((arg0) < (arg3) ? (arg0) : (arg1) < (arg3) ? (arg3) : (arg1)) : (arg1) < (arg3) ? (arg3) : (arg1) < (arg2) ? (arg1) : (arg2), \
  (arg0) < (arg1) ? ((arg2) < (arg3) ? ((arg0) < (arg2) ? ((arg1) < (arg2) ? (arg2) : (arg1) < (arg3) ? (arg1) : (arg3)) : (arg1) < (arg3) ? (arg1) : (arg0) < (arg3) ? (arg3) : (arg0)) : (arg1) < (arg2) ? ((arg1) < (arg3) ? (arg3) : (arg0) < (arg3) ? (arg1) : (arg1)) : (arg0) < (arg3) ? (arg2) : (arg0) < (arg2) ? (arg2) : (arg0)) : (arg2) < (arg3) ? ((arg0) < (arg3) ? ((arg0) < (arg2) ? (arg2) : (arg1) < (arg2) ? (arg0) : (arg0)) : (arg1) < (arg2) ? (arg3) : (arg1) < (arg3) ? (arg3) : (arg1)) : (arg0) < (arg2) ? ((arg0) < (arg3) ? (arg3) : (arg1) < (arg3) ? (arg0) : (arg0)) : (arg1) < (arg3) ? (arg2) : (arg1) < (arg2) ? (arg2) : (arg1), \
  (arg0) < (arg1) ? ((arg2) < (arg3) ? ((arg0) < (arg2) ? ((arg1) < (arg2) ? (arg3) : (arg1) < (arg3) ? (arg3) : (arg1)) : (arg1) < (arg3) ? (arg3) : (arg0) < (arg3) ? (arg1) : (arg1)) : (arg1) < (arg2) ? ((arg1) < (arg3) ? (arg2) : (arg0) < (arg3) ? (arg2) : (arg2)) : (arg0) < (arg3) ? (arg1) : (arg0) < (arg2) ? (arg1) : (arg1)) : (arg2) < (arg3) ? ((arg0) < (arg3) ? ((arg0) < (arg2) ? (arg3) : (arg1) < (arg2) ? (arg3) : (arg3)) : (arg1) < (arg2) ? (arg0) : (arg1) < (arg3) ? (arg0) : (arg0)) : (arg0) < (arg2) ? ((arg0) < (arg3) ? (arg2) : (arg1) < (arg3) ? (arg2) : (arg2)) : (arg1) < (arg3) ? (arg0) : (arg1) < (arg2) ? (arg0) : (arg0)

#endif // DR_UTIL_H
