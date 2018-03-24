// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#if !defined(__MACH__)
	.section	.text,"ax",@progbits
	.p2align 4,,15
	.globl	dr_task_switch
	.type	dr_task_switch, @function
dr_task_switch:
#else
	.section	__TEXT,__text
	.globl  _dr_task_switch
	.align  4, 0x90
_dr_task_switch:
#endif
	push	%r15
	push	%r14
	push	%r13
	push	%r12
	push	%rbp
	push	%rbx

	mov	%rsp, (%rdi)
	mov	(%rsi), %rsp

	pop	%rbx
	pop	%rbp
	pop	%r12
	pop	%r13
	pop	%r14
	pop	%r15

	ret
#if !defined(__MACH__)
	.size	dr_task_switch, .-dr_task_switch
#endif
