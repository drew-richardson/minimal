// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#if !defined(DR_COMPILER_H)
#define DR_COMPILER_H

#include "dr_config.h"
#include "dr_version.h"

#if !defined(HAS_RESTRICT)
// DR Check for __restrict as well?
#define restrict
#endif

#if defined(HAS_BUILTIN_EXPECT)

#define dr_likely(cond) __builtin_expect(!!(cond), 1)
#define dr_unlikely(cond) __builtin_expect(!!(cond), 0)

#else

#define dr_likely(cond) (cond)
#define dr_unlikely(cond) (cond)

#endif

#if defined(HAS_ATTRIBUTE_WARN_UNUSED_RESULT)
#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define WARN_UNUSED_RESULT
#endif

#if defined(HAS_ATTRIBUTE_NORETURN)
#define NORETURN __attribute__((noreturn))
#elif defined(HAS_DECLSPEC_NORETURN)
#define NORETURN __declspec(noreturn)
#else
#define NORETURN
#endif

#endif // DR_COMPILER_H
