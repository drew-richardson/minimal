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
	mov	0x4(%esp), %eax
	mov	0x8(%esp), %ecx

	push	%edi
	push	%esi
	push	%ebp
	push	%ebx

	mov	%esp, (%eax)
	mov	(%ecx), %esp

	pop	%ebx
	pop	%ebp
	pop	%esi
	pop	%edi

	ret
#if !defined(__MACH__)
	.size	dr_task_switch, .-dr_task_switch
#endif
