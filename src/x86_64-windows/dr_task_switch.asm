; SPDX-License-Identifier: GPL-2.0-only
; Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

.code
dr_task_switch proc
	push	r15
	push	r14
	push	r13
	push	r12
	push	rdi
	push	rsi
	push	rbp
	push	rbx

	; _TIB
	mov	rbx,gs:[030h]
	; DeallocationStack
	mov	rax,[rbx+01478h]
	push	rax
	; SubSystemTib
	mov	rax,[rbx+00018h]
	push	rax
	; StackLimit
	mov	rax,[rbx+00010h]
	push	rax
	; StackBase
	mov	rax,[rbx+00008h]
	push	rax
	; ExceptionList
	mov	rax,[rbx+00000h]
	push	rax

	mov	[rcx],rsp
	mov	rsp,[rdx]

	; ExceptionList
	pop	rax
	mov	[rbx+00000h],rax
	; StackBase
	pop	rax
	mov	[rbx+00008h],rax
	; StackLimit
	pop	rax
	mov	[rbx+00010h],rax
	; SubSystemTib
	pop	rax
	mov	[rbx+00018h],rax
	; DeallocationStack
	pop	rax
	mov	[rbx+01478h],rax

	pop	rbx
	pop	rbp
	pop	rsi
	pop	rdi
	pop	r12
	pop	r13
	pop	r14
	pop	r15

	ret
dr_task_switch endp
end
