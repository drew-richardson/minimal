// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

	.arm
	.section	.text,"ax",%progbits
	.align	2
	.global	dr_task_switch
	.type	dr_task_switch, %function
dr_task_switch:
	push	{r4 - r11, lr}
	str	sp, [r0]
	ldr	sp, [r1]
	pop	{r4 - r11, pc}
	.size	dr_task_switch, .-dr_task_switch
