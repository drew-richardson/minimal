; SPDX-License-Identifier: GPL-2.0-only
; Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

.686
.model flat, c
.code
dr_task_switch proc
	mov	eax,[esp+04h]
	mov	ecx,[esp+08h]

	push	edi
	push	esi
	push	ebp
	push	ebx

	; _TIB
	assume	fs:nothing
	mov	ebx,fs:[018h]
	assume	fs:error
	; DeallocationStack
	mov	ebp,[ebx+0e0ch]
	push	ebp
	; SubSystemTib
	mov	ebp,[ebx+0010h]
	push	ebp
	; StackLimit
	mov	ebp,[ebx+0008h]
	push	ebp
	; StackBase
	mov	ebp,[ebx+0004h]
	push	ebp
	; ExceptionList
	mov	ebp,[ebx+0000h]
	push	ebp

	mov	[eax],esp
	mov	esp,[ecx]

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

	ret
dr_task_switch endp
end
