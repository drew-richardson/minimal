; SPDX-License-Identifier: GPL-2.0-only
; Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

.code
dr_task_destroy_on_do proc
	mov	rsp,[rdx]

	; _TIB
	mov	rbx,gs:[030h]
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

	jmp	r8
dr_task_destroy_on_do endp
end
