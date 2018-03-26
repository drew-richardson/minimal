// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#include "dr.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(USE_VALGRIND)
#include <valgrind/valgrind.h>
#endif

#include "list.h"

// 1MB
static const uintptr_t dr_task_guard_size = 1U<<20;

#if !defined(_WIN32) && defined(__x86_64__)

// Linux/BSD/macOS x86_64
struct dr_task_frame {
  uintptr_t rbx;
  uintptr_t rbp;
  uintptr_t r12;
  uintptr_t r13;
  uintptr_t r14;
  uintptr_t r15;
  uintptr_t rip;
};

#define PC rip

static uintptr_t align_sp(const uintptr_t sp) {
  return ((sp - 8) & -64) + 8;
}

#elif defined(_WIN32) && (defined(__x86_64__) || defined(_M_X64))

// Windows x86_64
struct dr_task_frame {
  uintptr_t exception_list;
  uintptr_t stack_base;
  uintptr_t stack_limit;
  uintptr_t sub_system_tib;
  uintptr_t deallocation_stack;
  uintptr_t rbx;
  uintptr_t rbp;
  uintptr_t rsi;
  uintptr_t rdi;
  uintptr_t r12;
  uintptr_t r13;
  uintptr_t r14;
  uintptr_t r15;
  uintptr_t rip;
};

#define PC rip

static uintptr_t align_sp(const uintptr_t sp) {
  return ((sp - 8) & -64) + 8;
}

#elif !defined(_WIN32) && defined(__i386)

// Linux/BSD i686
struct dr_task_frame {
  uintptr_t ebx;
  uintptr_t ebp;
  uintptr_t esi;
  uintptr_t edi;
  uintptr_t eip;
};

#define PC eip

static uintptr_t align_sp(const uintptr_t sp) {
  return ((sp - 4) & -64) + 4;
}

#elif defined(_WIN32) && (defined(__i386) || defined(_M_IX86))

// Windows i686
struct dr_task_frame {
  uintptr_t exception_list;
  uintptr_t stack_base;
  uintptr_t stack_limit;
  uintptr_t sub_system_tib;
  uintptr_t deallocation_stack;
  uintptr_t ebx;
  uintptr_t ebp;
  uintptr_t esi;
  uintptr_t edi;
  uintptr_t eip;
};

#define PC eip

static uintptr_t align_sp(const uintptr_t sp) {
  return ((sp - 4) & -64) + 4;
}

#elif !defined(_WIN32) && defined(__aarch64__)

// Linux arm64
struct dr_task_frame {
  uintptr_t x19;
  uintptr_t x20;
  uintptr_t x21;
  uintptr_t x22;
  uintptr_t x23;
  uintptr_t x24;
  uintptr_t x25;
  uintptr_t x26;
  uintptr_t x27;
  uintptr_t x28;
  uintptr_t x29;
  uintptr_t pc;
};

#define PC pc

static uintptr_t align_sp(const uintptr_t sp) {
  return sp & -64;
}

#elif !defined(_WIN32) && defined(__arm__)

// Linux arm
struct dr_task_frame {
  uintptr_t r4;
  uintptr_t r5;
  uintptr_t r6;
  uintptr_t r7;
  uintptr_t r8;
  uintptr_t r9;
  uintptr_t r10;
  uintptr_t r11;
  uintptr_t pc;
};

#define PC pc

static uintptr_t align_sp(const uintptr_t sp) {
  return sp & -64;
}

#else

#error Unsupported arch

#endif

struct dr_task_start_args {
  dr_task_start_t func;
  void *restrict arg;
};

static struct dr_task dr_task_parent;
static struct list_head dr_runnable;
static struct list_head dr_sleeping;

extern void dr_task_switch(struct dr_task *restrict const cur, struct dr_task *restrict const next);
NORETURN
extern void dr_task_destroy_on_do(void *restrict const arg, struct dr_task *restrict const next, void (*func)(void *restrict const));

#if defined(_WIN32)

#include <windows.h>

WARN_UNUSED_RESULT static size_t dr_read_page_size(void) {
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  return si.dwPageSize;
}

WARN_UNUSED_RESULT static struct dr_result_voidp dr_task_alloc_stack(const size_t guard_size, const size_t alloc_size) {
  void *restrict const stack = VirtualAlloc(NULL, alloc_size, MEM_COMMIT, PAGE_READWRITE);
  if (dr_unlikely(stack == NULL)) {
    return DR_RESULT_GETLASTERROR(voidp);
  }
  DWORD oldProtect;
  const int result = VirtualProtect(stack, guard_size, PAGE_NOACCESS, &oldProtect);
  if (dr_unlikely(result == 0)) {
    const DWORD errnum = GetLastError();
    VirtualFree(stack, alloc_size, MEM_RELEASE);
    return DR_RESULT_ERRNUM(voidp, DR_ERR_WIN, errnum);
  }
  return DR_RESULT_OK(voidp, stack);
}

static void dr_task_free_stack(struct dr_task *restrict const task) {
  VirtualFree(task->stack, task->alloc_size, MEM_RELEASE);
}

#else

#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>

WARN_UNUSED_RESULT static size_t dr_read_page_size(void) {
  return sysconf(_SC_PAGESIZE);
}

WARN_UNUSED_RESULT static struct dr_result_voidp dr_task_alloc_stack(const size_t guard_size, const size_t alloc_size) {
  void *restrict const stack = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
  if (dr_unlikely(stack == MAP_FAILED)) {
    return DR_RESULT_ERRNO(voidp);
  }
  const int result = mprotect(stack, guard_size, PROT_NONE);
  if (dr_unlikely(result != 0)) {
    const int errnum = errno;
    munmap(stack, alloc_size);
    return DR_RESULT_ERRNUM(voidp, DR_ERR_ISO_C, errnum);
  }
  return DR_RESULT_OK(voidp, stack);
}

static void dr_task_free_stack(struct dr_task *restrict const task) {
  munmap(task->stack, task->alloc_size);
}

#endif

WARN_UNUSED_RESULT static size_t dr_get_page_size(void) {
  static size_t page_size;
  if (dr_unlikely(page_size == 0)) {
    page_size = dr_read_page_size();
  }
  return page_size;
}

void dr_task_destroy(struct dr_task *restrict const task) {
  if (dr_unlikely(task->stack != NULL)) {
    list_del(&task->tasks);
#if defined(USE_VALGRIND)
    VALGRIND_STACK_DEREGISTER(task->valgrind_stack_id);
#endif
    dr_task_free_stack(task);
    task->stack = NULL;
  }
}

static struct dr_task *dr_get_next_runnable(void) {
  if (dr_unlikely(list_empty(&dr_runnable))) {
    dr_task_runnable(&dr_task_parent);
  }
  return list_first_entry(&dr_runnable, struct dr_task, tasks);
}

NORETURN static void dr_task_start_do(void) {
  struct dr_task *restrict const current = dr_task_self();
  const uintptr_t stack_end = (uintptr_t)current->stack + dr_task_guard_size;
  struct dr_task_start_args *restrict const args = (struct dr_task_start_args *)stack_end;
  args->func(args->arg);
  dr_task_exit(current, (void (*)(void *restrict const))dr_task_destroy);
}

NORETURN void dr_task_exit(void *restrict const arg, void (*cleanup)(void *restrict const)) {
  struct dr_task *restrict const current = dr_task_self();
  list_move_tail(&current->tasks, &dr_sleeping);
  struct dr_task *restrict const next = dr_get_next_runnable();
  dr_task_destroy_on_do(arg, next, cleanup);
}

struct dr_result_void dr_task_create(struct dr_task *restrict const task, const size_t stack_size, const dr_task_start_t func, void *restrict const arg) {
  if (dr_unlikely(dr_runnable.next == NULL)) {
    INIT_LIST_HEAD(&dr_runnable);
    INIT_LIST_HEAD(&dr_sleeping);
    list_add(&dr_task_parent.tasks, &dr_runnable);
  }
  const size_t page_size = dr_get_page_size();
  const size_t alloc_size = (stack_size + page_size - 1 + dr_task_guard_size)/page_size*page_size;
  void *restrict stack;
  {
    const struct dr_result_voidp r = dr_task_alloc_stack(dr_task_guard_size, alloc_size);
    DR_IF_RESULT_ERR(r, err) {
      return DR_RESULT_ERROR_VOID(err);
    } DR_ELIF_RESULT_OK(void *, r, value) {
      stack = value;
    } DR_FI_RESULT;
  }
  const uintptr_t sp = align_sp((uintptr_t)stack + alloc_size);
  struct dr_task_frame *restrict const frame = (struct dr_task_frame *)(sp - sizeof(*frame));
  task->frame = frame;
  task->stack = stack;
  task->alloc_size = alloc_size;
#if defined(USE_VALGRIND)
  task->valgrind_stack_id = VALGRIND_STACK_REGISTER(stack, sp);
#endif
  const uintptr_t stack_end = (uintptr_t)stack + dr_task_guard_size;
  *(struct dr_task_start_args *)stack_end = (struct dr_task_start_args) {
    .func = func,
    .arg = arg,
  };
  frame->PC = (uintptr_t)dr_task_start_do;
#if defined(_WIN32)
  frame->exception_list = -1;
  frame->stack_base = sp;
  frame->stack_limit = stack_end;
  frame->sub_system_tib = 0;
  frame->deallocation_stack = stack_end;
#endif
  task->runnable = true;
  list_add_tail(&task->tasks, &dr_runnable);
  return DR_RESULT_OK_VOID();
}

struct dr_task *dr_task_self(void) {
  return list_first_entry(&dr_runnable, struct dr_task, tasks);
}

/*
void dr_task_run(struct dr_task *restrict const next, const bool sleep) {
  struct dr_task *restrict const prev = dr_task_self();
  list_move_tail(&prev->tasks, sleep ? &dr_sleeping : &dr_runnable);
  list_move(&next->tasks, &dr_runnable);
  dr_task_switch(prev, next);
}
*/

void dr_task_runnable(struct dr_task *restrict const task) {
  if (!task->runnable) {
    list_move_tail(&task->tasks, &dr_runnable);
    task->runnable = true;
  }
}

void dr_schedule(const bool sleep) {
  struct dr_task *restrict const prev = dr_task_self();
  list_move_tail(&prev->tasks, sleep ? &dr_sleeping : &dr_runnable);
  prev->runnable = !sleep;
  if (dr_unlikely(list_empty(&dr_runnable))) {
    dr_task_runnable(&dr_task_parent);
  }
  struct dr_task *restrict const next = dr_get_next_runnable();
  dr_task_switch(prev, next);
}
