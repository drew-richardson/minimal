; SPDX-License-Identifier: GPL-2.0-only
; Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

.686
.model flat, c
.code
dr_task_destroy_on_do proc
	mov	eax,[esp+04h]
	mov	ecx,[esp+08h]
	mov	edx,[esp+0ch]

	mov	esp,[ecx]

	; _TIB
	assume	fs:nothing
	mov	ebx,fs:[018h]
	assume	fs:error
	; ExceptionList
	pop	ebp
	mov	[ebx+0000h],ebp
	; StackBase
	pop	ebp
	mov	[ebx+0004h],ebp
	; StackLimit
	pop	ebp
	mov	[ebx+0008h],ebp
	; SubSystemTib
	pop	ebp
	mov	[ebx+0010h],ebp
	; DeallocationStack
	pop	ebp
	mov	[ebx+0e0ch],ebp

	pop	ebx
	pop	ebp
	pop	esi
	pop	edi

	mov	[esp+04h],eax
	jmp	edx
dr_task_destroy_on_do endp
end
