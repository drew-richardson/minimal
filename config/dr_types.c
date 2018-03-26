// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include <stdio.h>

#include "dr_util.h"
#include "dr_config.h"
#include "dr_types_impl.h"

#if defined(HAS___ALIGNOF)
#define ALIGN(TYPE) __alignof(TYPE)
#elif defined(HAS___ALIGNOF__)
#define ALIGN(TYPE) __alignof__(TYPE)
#else
#error missing alignof
#endif

int main(void) {
  static const char buf[] = {
    '\n',

    's','t','r','u','c','t',' ','d','r','_','e','v','e','n','t',' ','{',' ',
    DR_XINTXX2(ALIGN(dr_event_impl_t), 1),
    ' ','_','_','p','r','i','v','a','t','e','[',
    DR_HEXSTR((sizeof(dr_event_impl_t) + ALIGN(dr_event_impl_t) - 1)/ALIGN(dr_event_impl_t)),
    ']',';',' ','}',';','\n',

    's','t','r','u','c','t',' ','d','r','_','e','q','u','e','u','e',' ','{',' ',
    DR_XINTXX2(ALIGN(struct dr_equeue_impl), 1),
    ' ','_','_','p','r','i','v','a','t','e','[',
    DR_HEXSTR((sizeof(struct dr_equeue_impl) + ALIGN(struct dr_equeue_impl) - 1)/ALIGN(struct dr_equeue_impl)),
    ']',';',' ','}',';','\n',

    's','t','r','u','c','t',' ','d','r','_','e','q','u','e','u','e','_','s','e','r','v','e','r',' ','{',' ',
    DR_XINTXX2(ALIGN(struct dr_equeue_server_impl), 1),
    ' ','_','_','p','r','i','v','a','t','e','[',
    DR_HEXSTR((sizeof(struct dr_equeue_server_impl) + ALIGN(struct dr_equeue_server_impl) - 1)/ALIGN(struct dr_equeue_server_impl)),
    ']',';',' ','}',';','\n',

    's','t','r','u','c','t',' ','d','r','_','e','q','u','e','u','e','_','c','l','i','e','n','t',' ','{',' ',
    DR_XINTXX2(ALIGN(struct dr_equeue_client_impl), 1),
    ' ','_','_','p','r','i','v','a','t','e','[',
    DR_HEXSTR((sizeof(struct dr_equeue_client_impl) + ALIGN(struct dr_equeue_client_impl) - 1)/ALIGN(struct dr_equeue_client_impl)),
    ']',';',' ','}',';','\n',

    '\n','\0',
  };
  printf("%s", buf);
  return 0;
}
