/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Minimal STM32G474 device header — skeleton subset. This is NOT the full
 * CMSIS device header; it defines only the registers the build/test skeleton
 * needs (RCC, GPIO, FLASH, PWR, SysTick, NVIC/SCB). Replace with the real
 * STM32CubeG4 CMSIS headers (stm32g474xx.h) when the device pack is added.
 *
 * Register layout follows the STM32G4 reference manual (RM0440).
 */

#ifndef STM32G474_H_
#define STM32G474_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* --- Core peripherals (Cortex-M4) --- */

typedef struct {
	volatile uint32_t CTRL;
	volatile uint32_t LOAD;
	volatile uint32_t VAL;
	volatile uint32_t CALIB;
} SysTick_Type;

typedef struct {
	volatile uint32_t ISER[8];
	volatile uint32_t RESERVED0[24];
	volatile uint32_t ICER[8];
	volatile uint32_t RESERVED1[24];
	volatile uint32_t ISPR[8];
	volatile uint32_t RESERVED2[24];
	volatile uint32_t ICPR[8];
} NVIC_Type;

typedef struct {
	volatile uint32_t CPUID;
	volatile uint32_t ICSR;
	volatile uint32_t VTOR;
	volatile uint32_t AIRCR;
	volatile uint32_t SCR;
	volatile uint32_t CCR;
	volatile uint8_t SHPR[12];
	volatile uint32_t SHCSR;
	volatile uint32_t CFSR;
	volatile uint32_t HFSR;
	volatile uint32_t DFSR;
	volatile uint32_t MMFAR;
	volatile uint32_t BFAR;
	volatile uint32_t AFSR;
} SCB_Type;

#define SysTick ((SysTick_Type *)0xE000E010UL)
#define NVIC ((NVIC_Type *)0xE000E100UL)
#define SCB ((SCB_Type *)0xE000ED00UL)

/* SysTick CTRL bits */
#define SysTick_CTRL_CLKSOURCE_Pos 2U
#define SysTick_CTRL_TICKINT_Pos 1U
#define SysTick_CTRL_ENABLE_Pos 0U
#define SysTick_CTRL_CLKSOURCE_Msk (1UL << SysTick_CTRL_CLKSOURCE_Pos)
#define SysTick_CTRL_TICKINT_Msk (1UL << SysTick_CTRL_TICKINT_Pos)
#define SysTick_CTRL_ENABLE_Msk (1UL << SysTick_CTRL_ENABLE_Pos)

/* --- FLASH --- */
typedef struct {
	volatile uint32_t ACR;
	volatile uint32_t PDKEYR;
	volatile uint32_t KEYR;
	volatile uint32_t OPTKEYR;
	volatile uint32_t SR;
	volatile uint32_t CR;
	volatile uint32_t ECCR;
	volatile uint32_t RESERVED0;
	volatile uint32_t OPTR;
	volatile uint32_t PCROP1SR;
	volatile uint32_t PCROP1ER;
	volatile uint32_t WRP1AR;
	volatile uint32_t WRP1BR;
} FLASH_Type;

#define FLASH ((FLASH_Type *)0x40022000UL)

/* --- PWR --- */
typedef struct {
	volatile uint32_t CR1;
	volatile uint32_t CR2;
	volatile uint32_t CR3;
	volatile uint32_t CR4;
	volatile uint32_t SR1;
	volatile uint32_t SR2;
	volatile uint32_t SCR;
	volatile uint32_t RESERVED0;
	volatile uint32_t PUCRA;
	volatile uint32_t PDCRA;
	volatile uint32_t PUCRB;
	volatile uint32_t PDCRB;
	volatile uint32_t PUCRC;
	volatile uint32_t PDCRC;
	volatile uint32_t PUCRD;
	volatile uint32_t PDCRD;
	volatile uint32_t PUCRE;
	volatile uint32_t PDCRE;
	volatile uint32_t PUCRF;
	volatile uint32_t PDCRF;
	volatile uint32_t PUCRG;
	volatile uint32_t PDCRG;
} PWR_Type;

#define PWR ((PWR_Type *)0x40007000UL)

/* --- RCC --- */
typedef struct {
	volatile uint32_t CR;
	volatile uint32_t ICSCR;
	volatile uint32_t CFGR;
	volatile uint32_t PLLCFGR;
	volatile uint32_t RESERVED0;
	volatile uint32_t CIER;
	volatile uint32_t CIFR;
	volatile uint32_t CICR;
	volatile uint32_t RESERVED1;
	volatile uint32_t AHB1RSTR;
	volatile uint32_t AHB2RSTR;
	volatile uint32_t AHB3RSTR;
	volatile uint32_t RESERVED2;
	volatile uint32_t APB1RSTR1;
	volatile uint32_t APB1RSTR2;
	volatile uint32_t APB2RSTR;
	volatile uint32_t RESERVED3[2];
	volatile uint32_t AHB1ENR;
	volatile uint32_t AHB2ENR;
	volatile uint32_t AHB3ENR;
	volatile uint32_t RESERVED4;
	volatile uint32_t APB1ENR1;
	volatile uint32_t APB1ENR2;
	volatile uint32_t APB2ENR;
} RCC_Type;

#define RCC ((RCC_Type *)0x40021000UL)

/* RCC AHB2ENR GPIO enable bits */
#define RCC_AHB2ENR_GPIOAEN_Pos 0U
#define RCC_AHB2ENR_GPIOBEN_Pos 1U
#define RCC_AHB2ENR_GPIOCEN_Pos 2U
#define RCC_AHB2ENR_GPIODEN_Pos 3U
#define RCC_AHB2ENR_GPIOEEN_Pos 4U
#define RCC_AHB2ENR_GPIOFEN_Pos 5U

/* RCC CR HSE enable bit */
#define RCC_CR_HSERDY_Pos 17U
#define RCC_CR_HSEON_Pos 16U

/* --- GPIO --- */
typedef struct {
	volatile uint32_t MODER;
	volatile uint32_t OTYPER;
	volatile uint32_t OSPEEDR;
	volatile uint32_t PUPDR;
	volatile uint32_t IDR;
	volatile uint32_t ODR;
	volatile uint32_t BSRR;
	volatile uint32_t LCKR;
	volatile uint32_t AFR[2];
	volatile uint32_t BRR;
} GPIO_Type;

#define GPIOA ((GPIO_Type *)0x48000000UL)
#define GPIOB ((GPIO_Type *)0x48000400UL)
#define GPIOC ((GPIO_Type *)0x48000800UL)
#define GPIOD ((GPIO_Type *)0x48000C00UL)
#define GPIOE ((GPIO_Type *)0x48001000UL)
#define GPIOF ((GPIO_Type *)0x48001400UL)
#define GPIOG ((GPIO_Type *)0x48001800UL)

/* GPIO MODER: 2 bits per pin, 01 = output, 00 = input */
#define GPIO_MODER_MODER0_Pos 0U

/* --- CMSIS intrinsics --- */
static inline void __enable_irq(void)
{
	__asm volatile("cpsie i" ::: "memory");
}

static inline void __disable_irq(void)
{
	__asm volatile("cpsid i" ::: "memory");
}

static inline void __NOP(void)
{
	__asm volatile("nop");
}

static inline void __WFI(void)
{
	__asm volatile("wfi");
}

/* Board clock: 170 MHz (see system_stm32g474.c). */
extern uint32_t SystemCoreClock;

#ifdef __cplusplus
}
#endif

#endif /* STM32G474_H_ */
