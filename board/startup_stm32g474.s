/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * STM32G474 reset handler + vector table (armclang AC6, GNU assembler syntax).
 *
 * Boot flow: set SP -> zero .bss -> SystemInit() -> z_sys_init_run_all()
 * (walks the level+priority-sorted init sections) -> main().
 *
 * The skeleton has no initialized .data (all writable globals are zero-init),
 * so the classic .data copy step is omitted; add it back using
 * Load$$RW_IRAM1$$Base / Image$$RW_IRAM1$$Base when .data appears.
 */

	.syntax unified
	.cpu cortex-m4
	.thumb
	.eabi_attribute 25, 1

	.global g_pfnVectors
	.global Default_Handler
	.global Reset_Handler

	.section .isr_vector, "a", %progbits
	.type g_pfnVectors, %object
	.size g_pfnVectors, .-g_pfnVectors

g_pfnVectors:
	.word Image$$RW_IRAM1$$Limit      /* initial SP = top of RAM */
	.word Reset_Handler
	.word NMI_Handler
	.word HardFault_Handler
	.word MemManage_Handler
	.word BusFault_Handler
	.word UsageFault_Handler
	.word 0
	.word 0
	.word 0
	.word 0
	.word SVC_Handler
	.word DebugMon_Handler
	.word 0
	.word PendSV_Handler
	.word SysTick_Handler
	.rept 100                         /* STM32G4 device IRQs */
	.word Default_Handler
	.endr

	.section .text.Reset_Handler, "ax", %progbits
	.thumb_func
	.type Reset_Handler, %function
Reset_Handler:
	ldr sp, =Image$$RW_IRAM1$$Limit

	/* Enable FPU (Cortex-M4F): CPACR[20:23] = 0xF, required for hard-float ABI. */
	ldr r0, =0xE000ED88
	ldr r1, [r0]
	orr r1, r1, #(0xF << 20)
	str r1, [r0]
	dsb
	isb

	/* Zero .bss. */
	ldr r0, =Image$$RW_IRAM1$$ZI$$Base
	ldr r1, =Image$$RW_IRAM1$$ZI$$Limit
	movs r2, #0
	b 2f
1:
	str r2, [r0], #4
2:
	cmp r0, r1
	bne 1b

	bl SystemInit
	bl z_sys_init_run_all
	bl main
	b .

	.section .text, "ax", %progbits
	.thumb_func
	.type Default_Handler, %function
Default_Handler:
	b .

	/* Weak aliases so every vector resolves to Default_Handler. */
	.weak NMI_Handler
	.thumb_set NMI_Handler, Default_Handler
	.weak HardFault_Handler
	.thumb_set HardFault_Handler, Default_Handler
	.weak MemManage_Handler
	.thumb_set MemManage_Handler, Default_Handler
	.weak BusFault_Handler
	.thumb_set BusFault_Handler, Default_Handler
	.weak UsageFault_Handler
	.thumb_set UsageFault_Handler, Default_Handler
	.weak SVC_Handler
	.thumb_set SVC_Handler, Default_Handler
	.weak DebugMon_Handler
	.thumb_set DebugMon_Handler, Default_Handler
	.weak PendSV_Handler
	.thumb_set PendSV_Handler, Default_Handler
	.weak SysTick_Handler
	.thumb_set SysTick_Handler, Default_Handler

	.end
