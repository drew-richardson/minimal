// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

	.arm
	.section	.text,"ax",%progbits
	.align	2
	.global	dr_task_destroy_on_do
	.type	dr_task_destroy_on_do, %function
dr_task_destroy_on_do:
	ldr	sp, [r1]
	pop	{r4 - r11, lr}
	bx	r2
	.size	dr_task_destroy_on_do, .-dr_task_destroy_on_do
