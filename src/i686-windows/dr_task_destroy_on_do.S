// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

	.section	.text
	.p2align 4,,15
	.globl	_dr_task_destroy_on_do
	.def	_dr_task_destroy_on_do;	.scl	2;	.type	32;	.endef
_dr_task_destroy_on_do:
	mov	0x4(%esp), %eax
	mov	0x8(%esp), %ecx
	mov	0xc(%esp), %edx

	mov	(%ecx), %esp

	// _TIB
	mov	%fs:(0x18), %ebx
	// ExceptionList
	pop	%ebp
	mov	%ebp, 0x000(%ebx)
	// StackBase
	pop	%ebp
	mov	%ebp, 0x004(%ebx)
	// StackLimit
	pop	%ebp
	mov	%ebp, 0x008(%ebx)
	// SubSystemTib
	pop	%ebp
	mov	%ebp, 0x010(%ebx)
	// DeallocationStack
	pop	%ebp
	mov	%ebp, 0xe0c(%ebx)

	pop	%ebx
	pop	%ebp
	pop	%esi
	pop	%edi

	mov	%eax, 0x4(%esp)
	jmp	*%edx
