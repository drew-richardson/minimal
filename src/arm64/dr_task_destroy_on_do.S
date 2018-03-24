// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

	.text
	.align	2
	.global	dr_task_destroy_on_do
	.type	dr_task_destroy_on_do, %function
dr_task_destroy_on_do:
	ldr	x9, [x1]
	mov	sp, x9
	ldp	x19, x20, [sp], #16
	ldp	x21, x22, [sp], #16
	ldp	x23, x24, [sp], #16
	ldp	x25, x26, [sp], #16
	ldp	x27, x28, [sp], #16
	ldp	x29, x30, [sp], #16
	br	x2
	.size	dr_task_destroy_on_do, .-dr_task_destroy_on_do
