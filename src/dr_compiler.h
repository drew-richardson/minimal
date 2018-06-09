// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#if !defined(DR_COMPILER_H)
#define DR_COMPILER_H

#include "dr_config.h"
#include "dr_version.h"

#if !defined(DR_HAS_COMPOUND_LITERALS)
#error Compiler does not support compound literals
#endif

#if !defined(DR_HAS_DESIGNATED_INITIALIZERS)
#error Compiler does not support designated initializers
#endif

#if !defined(DR_HAS_MIXED_DECLARATIONS)
#error Compiler does not support mixed declarations
#endif

#if !defined(DR_HAS_VARIADIC_MACROS)
#error Compiler does not support variadic macros
#endif

#if !defined(DR_HAS_RESTRICT)
// DR Check for __restrict as well?
#define restrict
#endif

#if defined(DR_HAS_BUILTIN_EXPECT)

#define dr_likely(cond) __builtin_expect(!!(cond), 1)
#define dr_unlikely(cond) __builtin_expect(!!(cond), 0)

#else

#define dr_likely(cond) (cond)
#define dr_unlikely(cond) (cond)

#endif

#if defined(DR_HAS_ATTRIBUTE_WARN_UNUSED_RESULT)
#define DR_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define DR_WARN_UNUSED_RESULT
#endif

#if defined(DR_HAS_NORETURN)
#define DR_NORETURN _Noreturn
#elif defined(DR_HAS_ATTRIBUTE_NORETURN)
#define DR_NORETURN __attribute__((noreturn))
#elif defined(DR_HAS_DECLSPEC_NORETURN)
#define DR_NORETURN __declspec(noreturn)
#else
#define DR_NORETURN
#endif

#if defined(DR_HAS_ATTRIBUTE_FORMAT_PRINTF)
#define DR_FORMAT_PRINTF(FORMAT_IND, ARG_IND) __attribute__((__format__(__printf__, FORMAT_IND, ARG_IND)))
#else
#define DR_FORMAT_PRINTF(FORMAT_IND, ARG_IND)
#endif

#if defined(DR_HAS_ATTRIBUTE_ALIGNED)
#define DR_ALIGNED(ALIGNMENT) __attribute__((aligned(ALIGNMENT)))
#else
#define DR_ALIGNED(ALIGNMENT)
#endif

#endif // DR_COMPILER_H
