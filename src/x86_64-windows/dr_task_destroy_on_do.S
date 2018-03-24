// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

	.section	.text
	.p2align 4,,15
	.globl	dr_task_destroy_on_do
	.def	dr_task_destroy_on_do;	.scl	2;	.type	32;	.endef
dr_task_destroy_on_do:
	mov	(%rdx), %rsp

	// _TIB
	mov	%gs:(0x30), %rbx
	// ExceptionList
	pop	%rax
	mov	%rax, 0x0000(%rbx)
	// StackBase
	pop	%rax
	mov	%rax, 0x0008(%rbx)
	// StackLimit
	pop	%rax
	mov	%rax, 0x0010(%rbx)
	// SubSystemTib
	pop	%rax
	mov	%rax, 0x0018(%rbx)
	// DeallocationStack
	pop	%rax
	mov	%rax, 0x1478(%rbx)

	pop	%rbx
	pop	%rbp
	pop	%rsi
	pop	%rdi
	pop	%r12
	pop	%r13
	pop	%r14
	pop	%r15

	jmp	*%r8

/*
#/bin/sh
set -e

i686-w64-mingw32-gcc src/i686-win/dr_task_switch.S -c -o build/obj/dr_task_switch.o
i686-w64-mingw32-objdump -d --disassembler-options=intel build/obj/dr_task_switch.o | cut -b 28- > src/i686-win/dr_task_switch.asm

i686-w64-mingw32-gcc src/i686-win/dr_task_destroy_on_do.S -c -o build/obj/dr_task_destroy_on_do.o
i686-w64-mingw32-objdump -d --disassembler-options=intel build/obj/dr_task_destroy_on_do.o | cut -b 28- > src/i686-win/dr_task_destroy_on_do.asm

x86_64-w64-mingw32-gcc src/x86_64-win/dr_task_switch.S -c -o build/obj/dr_task_switch.o
x86_64-w64-mingw32-objdump -d --disassembler-options=intel build/obj/dr_task_switch.o | cut -b 28- > src/x86_64-win/dr_task_switch.asm

x86_64-w64-mingw32-gcc src/x86_64-win/dr_task_destroy_on_do.S -c -o build/obj/dr_task_destroy_on_do.o
x86_64-w64-mingw32-objdump -d --disassembler-options=intel build/obj/dr_task_destroy_on_do.o | cut -b 28- > src/x86_64-win/dr_task_destroy_on_do.asm
 */
