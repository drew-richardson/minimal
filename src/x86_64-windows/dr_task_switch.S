// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

	.section	.text
	.p2align 4,,15
	.globl	dr_task_switch
	.def	dr_task_switch;	.scl	2;	.type	32;	.endef
dr_task_switch:
	push	%r15
	push	%r14
	push	%r13
	push	%r12
	push	%rdi
	push	%rsi
	push	%rbp
	push	%rbx

	// _TIB
	mov	%gs:(0x30), %rbx
	// DeallocationStack
	mov	0x1478(%rbx), %rax
	push	%rax
	// SubSystemTib
	mov	0x0018(%rbx), %rax
	push	%rax
	// StackLimit
	mov	0x0010(%rbx), %rax
	push	%rax
	// StackBase
	mov	0x0008(%rbx), %rax
	push	%rax
	// ExceptionList
	mov	0x0000(%rbx), %rax
	push	%rax

	mov	%rsp, (%rcx)
	mov	(%rdx), %rsp

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

	ret
