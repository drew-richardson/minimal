// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include <stdio.h>

#include "dr_util.h"
#include "dr_config.h"

// Temporary definitions to satisfy dr_types_common.h usages of dr_types.h definitions
typedef char dr_event_t;
typedef char dr_overlapped_t;
typedef char dr_sockaddr_t;

#include "dr_types_impl.h"

#if defined(DR_HAS___ALIGNOF)
#define ALIGN(TYPE) __alignof(TYPE)
#elif defined(DR_HAS___ALIGNOF__)
#define ALIGN(TYPE) __alignof__(TYPE)
#else
#error missing alignof
#endif

int main(void) {
  static const char buf[] = {
    '\n',

    't','y','p','e','d','e','f',' ','s','t','r','u','c','t',' ','{',' ',
    DR_XINTXX2(ALIGN(dr_event_impl_t), 1),
    ' ','_','_','p','r','i','v','a','t','e','[',
    DR_HEXSTR((sizeof(dr_event_impl_t) + ALIGN(dr_event_impl_t) - 1)/ALIGN(dr_event_impl_t)),
    ']',';',' ','}',' ','d','r','_','e','v','e','n','t','_','t',';','\n',

    't','y','p','e','d','e','f',' ','s','t','r','u','c','t',' ','{',' ',
    DR_XINTXX2(ALIGN(dr_overlapped_impl_t), 1),
    ' ','_','_','p','r','i','v','a','t','e','[',
    DR_HEXSTR((sizeof(dr_overlapped_impl_t) + ALIGN(dr_overlapped_impl_t) - 1)/ALIGN(dr_overlapped_impl_t)),
    ']',';',' ','}',' ','d','r','_','o','v','e','r','l','a','p','p','e','d','_','t',';','\n',

    't','y','p','e','d','e','f',' ','s','t','r','u','c','t',' ','{',' ',
    DR_XINTXX2(ALIGN(dr_sockaddr_impl_t), 1),
    ' ','_','_','p','r','i','v','a','t','e','[',
    DR_HEXSTR((sizeof(dr_sockaddr_impl_t) + ALIGN(dr_sockaddr_impl_t) - 1)/ALIGN(dr_sockaddr_impl_t)),
    ']',';',' ','}',' ','d','r','_','s','o','c','k','a','d','d','r','_','t',';','\n',

    '\n','\0',
  };
  printf("%s", buf);
  return 0;
}
