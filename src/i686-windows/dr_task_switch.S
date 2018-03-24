// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

	.section	.text
	.p2align 4,,15
	.globl	_dr_task_switch
	.def	_dr_task_switch;	.scl	2;	.type	32;	.endef
_dr_task_switch:
	mov	0x4(%esp), %eax
	mov	0x8(%esp), %ecx

	push	%edi
	push	%esi
	push	%ebp
	push	%ebx

	// _TIB
	mov	%fs:(0x18), %ebx
	// DeallocationStack
	mov	0xe0c(%ebx), %ebp
	push	%ebp
	// SubSystemTib
	mov	0x010(%ebx), %ebp
	push	%ebp
	// StackLimit
	mov	0x008(%ebx), %ebp
	push	%ebp
	// StackBase
	mov	0x004(%ebx), %ebp
	push	%ebp
	// ExceptionList
	mov	0x000(%ebx), %ebp
	push	%ebp

	mov	%esp, (%eax)
	mov	(%ecx), %esp

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

	ret
