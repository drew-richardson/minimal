// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

#include <limits.h>
#include <string.h>

#define DR_UINT64_MAX 0xffffffffffffffff
#define DR_UINT64_MAX_DEC "18446744073709551615"
#define DR_UINT64_MAX_OCT "1777777777777777777777"
#define DR_UINT64_MAX_HEXL "ffffffffffffffff"
#define DR_UINT64_MAX_HEXU "FFFFFFFFFFFFFFFF"
#define DR_INT64_MIN 0x8000000000000000
#define DR_INT64_MIN_DEC "-9223372036854775808"
#define DR_INT64_MAX 0x7fffffffffffffff
#define DR_INT64_MAX_DEC "9223372036854775807"

#define DR_UINT32_MAX 0xffffffff
#define DR_UINT32_MAX_DEC "4294967295"
#define DR_UINT32_MAX_OCT "37777777777"
#define DR_UINT32_MAX_HEXL "ffffffff"
#define DR_UINT32_MAX_HEXU "FFFFFFFF"
#define DR_INT32_MIN 0x80000000
#define DR_INT32_MIN_DEC "-2147483648"
#define DR_INT32_MAX 0x7fffffff
#define DR_INT32_MAX_DEC "2147483647"

#define DR_UINT16_MAX 0xffff
#define DR_UINT16_MAX_DEC "65535"
#define DR_UINT16_MAX_OCT "177777"
#define DR_UINT16_MAX_HEXL "ffff"
#define DR_UINT16_MAX_HEXU "FFFF"
#define DR_INT16_MIN 0x8000
#define DR_INT16_MIN_DEC "-32768"
#define DR_INT16_MAX 0x7fff
#define DR_INT16_MAX_DEC "32767"

#define DR_UINT8_MAX 0xff
#define DR_UINT8_MAX_DEC "255"
#define DR_UINT8_MAX_OCT "377"
#define DR_UINT8_MAX_HEXL "ff"
#define DR_UINT8_MAX_HEXU "FF"
#define DR_INT8_MIN 0x80
#define DR_INT8_MIN_DEC "-128"
#define DR_INT8_MAX 0x7f
#define DR_INT8_MAX_DEC "127"

#if ULLONG_MAX == DR_UINT64_MAX
#define DR_ULLONG_MAX_DEC DR_UINT64_MAX_DEC
#define DR_ULLONG_MAX_OCT DR_UINT64_MAX_OCT
#define DR_ULLONG_MAX_HEXL DR_UINT64_MAX_HEXL
#define DR_ULLONG_MAX_HEXU DR_UINT64_MAX_HEXU
#else
#error Unrecognized ULLONG_MAX
#endif

#if LLONG_MAX == DR_INT64_MAX
#define DR_LLONG_MIN_DEC DR_INT64_MIN_DEC
#define DR_LLONG_MAX_DEC DR_INT64_MAX_DEC
#else
#error Unrecognized LLONG_MAX
#endif

#if ULONG_MAX == DR_UINT64_MAX
#define DR_ULONG_MAX_DEC DR_UINT64_MAX_DEC
#define DR_ULONG_MAX_OCT DR_UINT64_MAX_OCT
#define DR_ULONG_MAX_HEXL DR_UINT64_MAX_HEXL
#define DR_ULONG_MAX_HEXU DR_UINT64_MAX_HEXU
#elif ULONG_MAX == DR_UINT32_MAX
#define DR_ULONG_MAX_DEC DR_UINT32_MAX_DEC
#define DR_ULONG_MAX_OCT DR_UINT32_MAX_OCT
#define DR_ULONG_MAX_HEXL DR_UINT32_MAX_HEXL
#define DR_ULONG_MAX_HEXU DR_UINT32_MAX_HEXU
#else
#error Unrecognized ULONG_MAX
#endif

#if LONG_MAX == DR_INT64_MAX
#define DR_LONG_MIN_DEC DR_INT64_MIN_DEC
#define DR_LONG_MAX_DEC DR_INT64_MAX_DEC
#elif LONG_MAX == DR_INT32_MAX
#define DR_LONG_MIN_DEC DR_INT32_MIN_DEC
#define DR_LONG_MAX_DEC DR_INT32_MAX_DEC
#else
#error Unrecognized LONG_MAX
#endif

#if UINT_MAX == DR_UINT32_MAX
#define DR_UINT_MAX_DEC DR_UINT32_MAX_DEC
#define DR_UINT_MAX_OCT DR_UINT32_MAX_OCT
#define DR_UINT_MAX_HEXL DR_UINT32_MAX_HEXL
#define DR_UINT_MAX_HEXU DR_UINT32_MAX_HEXU
#else
#error Unrecognized UINT_MAX
#endif

#if INT_MAX == DR_INT32_MAX
#define DR_INT_MIN_DEC DR_INT32_MIN_DEC
#define DR_INT_MAX_DEC DR_INT32_MAX_DEC
#else
#error Unrecognized INT_MAX
#endif

#if UCHAR_MAX == DR_UINT8_MAX
#define DR_UCHAR_MAX_DEC DR_UINT8_MAX_DEC
#define DR_UCHAR_MAX_OCT DR_UINT8_MAX_OCT
#define DR_UCHAR_MAX_HEXL DR_UINT8_MAX_HEXL
#define DR_UCHAR_MAX_HEXU DR_UINT8_MAX_HEXU
#else
#error Unrecognized UCHAR_MAX
#endif

#if CHAR_MAX == DR_INT8_MAX
#define DR_CHAR_MIN_DEC DR_INT8_MIN_DEC
#define DR_CHAR_MAX_DEC DR_INT8_MAX_DEC
#else
#error Unrecognized CHAR_MAX
#endif

#if SIZE_MAX == DR_UINT64_MAX
#define DR_UPTR_MAX DR_UINT64_MAX
#define DR_UPTR_MAX_DEC DR_UINT64_MAX_DEC
#define DR_UPTR_MAX_OCT DR_UINT64_MAX_OCT
#define DR_UPTR_MAX_HEXL DR_UINT64_MAX_HEXL
#define DR_UPTR_MAX_HEXU DR_UINT64_MAX_HEXU
#define DR_PTR_MIN DR_INT64_MIN
#define DR_PTR_MIN_DEC DR_INT64_MIN_DEC
#define DR_PTR_MAX DR_INT64_MAX
#define DR_PTR_MAX_DEC DR_INT64_MAX_DEC
#elif SIZE_MAX == DR_UINT32_MAX
#define DR_UPTR_MAX DR_UINT32_MAX
#define DR_UPTR_MAX_DEC DR_UINT32_MAX_DEC
#define DR_UPTR_MAX_OCT DR_UINT32_MAX_OCT
#define DR_UPTR_MAX_HEXL DR_UINT32_MAX_HEXL
#define DR_UPTR_MAX_HEXU DR_UINT32_MAX_HEXU
#define DR_PTR_MIN DR_INT32_MIN
#define DR_PTR_MIN_DEC DR_INT32_MIN_DEC
#define DR_PTR_MAX DR_INT32_MAX
#define DR_PTR_MAX_DEC DR_INT32_MAX_DEC
#else
#error Unrecognized ULONG_MAX
#endif

int main(void) {
  {
    const struct dr_result_void r = dr_console_startup();
    DR_IF_RESULT_ERR(r, err) {
      // If dr_log is changed to use dr_*printf, which uses dr_std*, we have a problem here. But perhaps the solution is that things are logged to multiple endpoints, console, file, socket and hopefully at least one of them works.
      dr_log_error("dr_console_startup failed", err);
      return -1;
    } DR_FI_RESULT;
  }

  char buf[23];

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%d", INT_MIN);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_INT_MIN_DEC) == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%d", INT_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_INT_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%7d", 1234);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "   1234") == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%2d", 1234);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "1234") == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%*d", 6, 1234);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "  1234") == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%.7d", 1234);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "0001234") == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%.2d", 1234);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "1234") == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%.*d", 6, 1234);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "001234") == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%07d", 1234);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "0001234") == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%02d", 1234);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "1234") == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%-7d", 1234);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "1234   ") == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%-2d", 1234);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "1234") == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%-.7d", 1234);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "0001234") == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%-.2d", 1234);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "1234") == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "% 7d", 1234);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "   1234") == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "% 2d", 1234);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, " 1234") == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "% .7d", 1234);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, " 0001234") == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "% .2d", 1234);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, " 1234") == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%+d", INT_MIN);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_INT_MIN_DEC) == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%+d", INT_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "+" DR_INT_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%i", INT_MIN);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_INT_MIN_DEC) == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%i", INT_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_INT_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%o", UINT_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UINT_MAX_OCT) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%#o", UINT_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "0" DR_UINT_MAX_OCT) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%u", UINT_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UINT_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%x", UINT_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UINT_MAX_HEXL) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%#x", UINT_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "0x" DR_UINT_MAX_HEXL) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%X", UINT_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UINT_MAX_HEXU) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%#X", UINT_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "0X" DR_UINT_MAX_HEXU) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%c", '!');
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "!") == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%s", "Hello world");
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "Hello world") == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%.5s", "Hello world");
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "Hello") == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%.30s", "Hello world");
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "Hello world") == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%p", (void *)DR_UPTR_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "0x" DR_UPTR_MAX_HEXL) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%ld", LONG_MIN);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_LONG_MIN_DEC) == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%ld", LONG_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_LONG_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%li", LONG_MIN);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_LONG_MIN_DEC) == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%li", LONG_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_LONG_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%lo", ULONG_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_ULONG_MAX_OCT) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%lu", ULONG_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_ULONG_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%lx", ULONG_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_ULONG_MAX_HEXL) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%lX", ULONG_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_ULONG_MAX_HEXU) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%lld", LLONG_MIN);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_LLONG_MIN_DEC) == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%lld", LLONG_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_LLONG_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%lli", LLONG_MIN);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_LLONG_MIN_DEC) == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%lli", LLONG_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_LLONG_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%llo", ULLONG_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_ULLONG_MAX_OCT) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%llu", ULLONG_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_ULLONG_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%llx", ULLONG_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_ULLONG_MAX_HEXL) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%llX", ULLONG_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_ULLONG_MAX_HEXU) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%hd", (short)DR_INT16_MIN);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_INT16_MIN_DEC) == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%hd", (short)DR_INT16_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_INT16_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%hi", (short)DR_INT16_MIN);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_INT16_MIN_DEC) == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%hi", (short)DR_INT16_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_INT16_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%ho", (unsigned short)DR_UINT16_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UINT16_MAX_OCT) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%hu", (unsigned short)DR_UINT16_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UINT16_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%hx", (unsigned short)DR_UINT16_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UINT16_MAX_HEXL) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%hX", (unsigned short)DR_UINT16_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UINT16_MAX_HEXU) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%hhd", (signed char)CHAR_MIN);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_CHAR_MIN_DEC) == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%hhd", (signed char)CHAR_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_CHAR_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%hhi", (signed char)CHAR_MIN);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_CHAR_MIN_DEC) == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%hhi", (signed char)CHAR_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_CHAR_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%hho", (unsigned char)UCHAR_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UCHAR_MAX_OCT) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%hhu", (unsigned char)UCHAR_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UCHAR_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%hhx", (unsigned char)UCHAR_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UCHAR_MAX_HEXL) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%hhX", (unsigned char)UCHAR_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UCHAR_MAX_HEXU) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%zd", (size_t)DR_PTR_MIN);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_PTR_MIN_DEC) == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%zd", (size_t)DR_PTR_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_PTR_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%zi", (size_t)DR_PTR_MIN);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_PTR_MIN_DEC) == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%zi", (size_t)DR_PTR_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_PTR_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%zo", (size_t)DR_UPTR_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UPTR_MAX_OCT) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%zu", (size_t)DR_UPTR_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UPTR_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%zx", (size_t)DR_UPTR_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UPTR_MAX_HEXL) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%zX", (size_t)DR_UPTR_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UPTR_MAX_HEXU) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%td", (ptrdiff_t)DR_PTR_MIN);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_PTR_MIN_DEC) == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%td", (ptrdiff_t)DR_PTR_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_PTR_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%ti", (ptrdiff_t)DR_PTR_MIN);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_PTR_MIN_DEC) == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%ti", (ptrdiff_t)DR_PTR_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_PTR_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%to", (ptrdiff_t)DR_UPTR_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UPTR_MAX_OCT) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%tu", (ptrdiff_t)DR_UPTR_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UPTR_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%tx", (ptrdiff_t)DR_UPTR_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UPTR_MAX_HEXL) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%tX", (ptrdiff_t)DR_UPTR_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UPTR_MAX_HEXU) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%jd", (intmax_t)DR_INT64_MIN);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_INT64_MIN_DEC) == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%jd", (intmax_t)DR_INT64_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_INT64_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%ji", (intmax_t)DR_INT64_MIN);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_INT64_MIN_DEC) == 0);
  }
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%ji", (intmax_t)DR_INT64_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_INT64_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%jo", (uintmax_t)DR_UINT64_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UINT64_MAX_OCT) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%ju", (uintmax_t)DR_UINT64_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UINT64_MAX_DEC) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%jx", (uintmax_t)DR_UINT64_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UINT64_MAX_HEXL) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%jX", (uintmax_t)DR_UINT64_MAX);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, DR_UINT64_MAX_HEXU) == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%%");
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "%") == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "a %s b %c c %i d", "one", 't', 3);
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "a one b t c 3 d") == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f');
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "0123456789abcdef") == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "a %3$s b %1$c c %2$i d", 'o', 2, "three");
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "a three b o c 2 d") == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%9$c%8$c%7$c%6$c%5$c%4$c%3$c%2$c%1$c", '0', '1', '2', '3', '4', '5', '6', '7', '8');
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "876543210") == 0);
  }

  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf), "%10$c%9$c%8$c%7$c%6$c%5$c%4$c%3$c%2$c%1$c", '0', '1', '2', '3', '4', '5', '6', '7', '8', '9');
    dr_assert(DR_IS_RESULT_ERR(r));
  }

  buf[sizeof(buf) - 1] = '\0';
  {
    const struct dr_result_size r = dr_snprintf(buf, sizeof(buf) - 1, "Something that is definitely way too long");
    dr_assert(DR_IS_RESULT_OK(r));
    dr_assert(strcmp(buf, "Something that is def") == 0);
  }

  dr_log("OK");
  return 0;
}
