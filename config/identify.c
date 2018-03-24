// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include <stdio.h>

/*
 * gcc: gcc/config/i386/i386-c.c
 * clang(cfe): lib/Basic/Targets.cpp
 * https://sourceforge.net/p/predef/wiki/Architectures/
 *
 * __amd64
 * __amd64__
 * __x86_64
 * __x86_64__
 *
 * x32:
 * _ILP32
 * __ILP32__
 *
 * https://msdn.microsoft.com/en-us/library/b0084kay.aspx
 *
 * _M_AMD64
 * _M_X64
 */
#if defined(__x86_64__) || defined(_M_X64)
#define MACHINE "x86_64"
// Ignore x32 for now

/*
 * gcc: gcc/config/i386/i386-c.c
 * clang(cfe): lib/Basic/Targets.cpp
 * https://sourceforge.net/p/predef/wiki/Architectures/
 *
 * __i386
 * __i386__ # Not in sun studio
 * i386
 *
 * https://msdn.microsoft.com/en-us/library/b0084kay.aspx
 *
 * _M_IX86
 */
#elif defined(__i386) || defined(_M_IX86)
#define MACHINE "i686"

/*
 * gcc: gcc/config/aarch64/aarch64.h
 * clang(cfe): lib/Basic/Targets.cpp
 * https://sourceforge.net/p/predef/wiki/Architectures/
 *
 * __aarch64__
 *
 * https://msdn.microsoft.com/en-us/library/b0084kay.aspx
 *
 * _M_ARM64
 */
#elif defined(__aarch64__) || defined(_M_ARM64)
#define MACHINE "arm64"

/*
 * gcc: gcc/config/arm/arm.h
 * clang(cfe): lib/Basic/Targets.cpp
 * https://sourceforge.net/p/predef/wiki/Architectures/
 *
 * __arm # clang only
 * __arm__
 *
 * https://msdn.microsoft.com/en-us/library/b0084kay.aspx
 *
 * _M_ARM
 */
#elif defined(__arm__) || defined(_M_ARM)
#define MACHINE "arm"

#else
#define MACHINE "unknown"
#endif

/*
 * gcc: gcc/config/linux.h:
 * clang(cfe): lib/Basic/Targets.cpp
 * https://sourceforge.net/p/predef/wiki/OperatingSystems/
 *
 * __linux
 * __linux__
 * __gnu_linux__ # glibc only
 * linux
 */
#if defined(__linux__)
#define OS "linux"

#elif defined(__FreeBSD__)
/*
 * gcc: gcc/config/freebsd-spec.h
 * clang(cfe): lib/Basic/Targets.cpp
 * https://sourceforge.net/p/predef/wiki/OperatingSystems/
 *
 * __FreeBSD__
 */
#define OS "freebsd"

#elif defined(__OpenBSD__)
/*
 * gcc: gcc/config/openbsd.h
 * clang(cfe): lib/Basic/Targets.cpp
 * https://sourceforge.net/p/predef/wiki/OperatingSystems/
 *
 * __OpenBSD__
 */
#define OS "openbsd"

#elif defined(__NetBSD__)
/*
 * gcc: gcc/config/netbsd.h
 * clang(cfe): lib/Basic/Targets.cpp
 * https://sourceforge.net/p/predef/wiki/OperatingSystems/
 *
 * __NetBSD__
 */
#define OS "netbsd"

#elif defined(__DragonFly__)
/*
 * gcc: gcc/config/dragonfly.h
 * clang(cfe): lib/Basic/Targets.cpp
 * https://sourceforge.net/p/predef/wiki/OperatingSystems/
 *
 * __DragonFly__
 */
#define OS "dragonfly"

#elif defined(__minix)
/*
 * clang(cfe): lib/Basic/Targets.cpp
 * https://sourceforge.net/p/predef/wiki/OperatingSystems/
 *
 * __minix
 */
#define OS "minix"

/*
 * gcc: gcc/config/darwin-c.c
 * clang(cfe): lib/Basic/Targets.cpp
 * https://sourceforge.net/p/predef/wiki/OperatingSystems/
 *
 * __APPLE__
 * __MACH__
 */

#elif defined(__MACH__)
#define OS "darwin"

/*
 * gcc: gcc/config/i386/mingw32.h
 * clang(cfe): lib/Basic/Targets.cpp
 * https://sourceforge.net/p/predef/wiki/OperatingSystems/
 *
 * _WIN32
 */
#elif defined(_WIN32)
#define OS "windows"

/*
 * gcc: gcc-5.4.0/gcc/config/i386/cygwin.h
 * clang(cfe): lib/Basic/Targets.cpp
 * https://sourceforge.net/p/predef/wiki/OperatingSystems/
 *
 * __CYGWIN__
 */
#elif defined(__CYGWIN__)
#define OS "cygwin"

/*
 * gcc: gcc/config/sol2.h
 * clang(cfe): lib/Basic/Targets.cpp
 * https://sourceforge.net/p/predef/wiki/OperatingSystems/
 *
 * sun
 * __sun
 */
#elif defined(__sun)
#define OS "solaris"

#else
#define OS "unknown"
#endif

int main(void) {
  puts("\n" "MACHINE=" MACHINE "\n");
  puts("\n" "OS=" OS "\n");
  return 0;
}
