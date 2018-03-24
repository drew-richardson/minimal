// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#if !defined(__MACH__)
	.section	.text,"ax",@progbits
	.p2align 4,,15
	.globl	dr_task_destroy_on_do
	.type	dr_task_destroy_on_do, @function
dr_task_destroy_on_do:
#else
	.section	__TEXT,__text
	.globl  _dr_task_destroy_on_do
	.align  4, 0x90
_dr_task_destroy_on_do:
#endif
	mov	0x4(%esp), %eax
	mov	0x8(%esp), %ecx
	mov	0xc(%esp), %edx

	mov	(%ecx), %esp

	pop	%ebx
	pop	%ebp
	pop	%esi
	pop	%edi

	mov	%eax, 0x4(%esp)
	jmp	*%edx
#if !defined(__MACH__)
	.size	dr_task_destroy_on_do, .-dr_task_destroy_on_do
#endif
