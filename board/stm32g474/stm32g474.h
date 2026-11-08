/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Minimal STM32G474 device header — the subset the firmware needs. This is NOT
 * the full CMSIS device header; it defines the registers the boot skeleton and
 * drivers touch (RCC, PWR, FLASH, GPIO, IWDG, SysTick, NVIC/SCB). The full
 * STM32CubeG4 CMSIS header (stm32g474xx.h) may replace it once the device pack
 * is wired in; register layout and bit positions follow the STM32G4 reference
 * manual (RM0440) and the STM32G474 SVD (Keil.STM32G4xx_DFP).
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

/* FLASH_ACR bits */
#define FLASH_ACR_LATENCY_Pos 0U
#define FLASH_ACR_LATENCY_Msk (0xFUL << FLASH_ACR_LATENCY_Pos)
#define FLASH_ACR_PRFTEN_Pos 8U
#define FLASH_ACR_PRFTEN_Msk (1UL << FLASH_ACR_PRFTEN_Pos)
#define FLASH_ACR_ICEN_Pos 9U
#define FLASH_ACR_ICEN_Msk (1UL << FLASH_ACR_ICEN_Pos)
#define FLASH_ACR_DCEN_Pos 10U
#define FLASH_ACR_DCEN_Msk (1UL << FLASH_ACR_DCEN_Pos)

/* Flash wait states for 170 MHz (RM0440: 4 WS at VDD 2.7-3.6 V, Vcore range 1). */
#define FLASH_LATENCY_170MHZ 4U

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
	volatile uint32_t RESERVED1[10];
	volatile uint32_t CR5;
} PWR_Type;

#define PWR ((PWR_Type *)0x40007000UL)

/* PWR_CR1 bits */
#define PWR_CR1_DBP_Pos 8U
#define PWR_CR1_DBP_Msk (1UL << PWR_CR1_DBP_Pos)
#define PWR_CR1_VOS_Pos 9U
#define PWR_CR1_VOS_Msk (0x3UL << PWR_CR1_VOS_Pos)
#define PWR_CR1_VOS_RANGE1 (1UL << PWR_CR1_VOS_Pos) /* 0b01 = Range 1 (1.2 V) */

/* PWR_SR2 bits. VOSF is set while voltage scaling is in progress and cleared
 * by hardware once the regulator reaches the VOS target voltage. */
#define PWR_SR2_VOSF_Pos 10U
#define PWR_SR2_VOSF_Msk (1UL << PWR_SR2_VOSF_Pos)

/* PWR_CR5 bits. R1MODE = 0 enables Range 1 boost (up to 170 MHz); = 1 keeps
 * Range 1 normal mode (up to 150 MHz). */
#define PWR_CR5_R1MODE_Pos 8U
#define PWR_CR5_R1MODE_Msk (1UL << PWR_CR5_R1MODE_Pos)

/* --- RCC --- */

typedef struct {
	volatile uint32_t CR;          /* 0x00 */
	volatile uint32_t ICSCR;       /* 0x04 */
	volatile uint32_t CFGR;        /* 0x08 */
	volatile uint32_t PLLCFGR;     /* 0x0C */
	volatile uint32_t RESERVED0[2];/* 0x10, 0x14 */
	volatile uint32_t CIER;        /* 0x18 */
	volatile uint32_t CIFR;        /* 0x1C */
	volatile uint32_t CICR;        /* 0x20 */
	volatile uint32_t RESERVED1;   /* 0x24 */
	volatile uint32_t AHB1RSTR;    /* 0x28 */
	volatile uint32_t AHB2RSTR;    /* 0x2C */
	volatile uint32_t AHB3RSTR;    /* 0x30 */
	volatile uint32_t RESERVED2;   /* 0x34 */
	volatile uint32_t APB1RSTR1;   /* 0x38 */
	volatile uint32_t APB1RSTR2;   /* 0x3C */
	volatile uint32_t APB2RSTR;    /* 0x40 */
	volatile uint32_t RESERVED3;   /* 0x44 */
	volatile uint32_t AHB1ENR;     /* 0x48 */
	volatile uint32_t AHB2ENR;     /* 0x4C */
	volatile uint32_t AHB3ENR;     /* 0x50 */
	volatile uint32_t RESERVED4;   /* 0x54 */
	volatile uint32_t APB1ENR1;    /* 0x58 */
	volatile uint32_t APB1ENR2;    /* 0x5C */
	volatile uint32_t APB2ENR;     /* 0x60 */
	volatile uint32_t RESERVED5;   /* 0x64 */
	volatile uint32_t AHB1SMENR;   /* 0x68 */
	volatile uint32_t AHB2SMENR;   /* 0x6C */
	volatile uint32_t AHB3SMENR;   /* 0x70 */
	volatile uint32_t RESERVED6;   /* 0x74 */
	volatile uint32_t APB1SMENR1;  /* 0x78 */
	volatile uint32_t APB1SMENR2;  /* 0x7C */
	volatile uint32_t APB2SMENR;   /* 0x80 */
	volatile uint32_t RESERVED7;   /* 0x84 */
	volatile uint32_t CCIPR;       /* 0x88 */
	volatile uint32_t RESERVED8;   /* 0x8C */
	volatile uint32_t BDCR;        /* 0x90 */
	volatile uint32_t CSR;         /* 0x94 */
	volatile uint32_t CRRCR;       /* 0x98 */
	volatile uint32_t CCIPR2;      /* 0x9C */
} RCC_Type;

#define RCC ((RCC_Type *)0x40021000UL)

/* RCC_CR bits */
#define RCC_CR_HSION_Pos 8U
#define RCC_CR_HSION_Msk (1UL << RCC_CR_HSION_Pos)
#define RCC_CR_HSIRDY_Pos 10U
#define RCC_CR_HSIRDY_Msk (1UL << RCC_CR_HSIRDY_Pos)
#define RCC_CR_HSEON_Pos 16U
#define RCC_CR_HSEON_Msk (1UL << RCC_CR_HSEON_Pos)
#define RCC_CR_HSERDY_Pos 17U
#define RCC_CR_HSERDY_Msk (1UL << RCC_CR_HSERDY_Pos)
#define RCC_CR_HSEBYP_Pos 18U
#define RCC_CR_HSEBYP_Msk (1UL << RCC_CR_HSEBYP_Pos)
#define RCC_CR_PLLON_Pos 24U
#define RCC_CR_PLLON_Msk (1UL << RCC_CR_PLLON_Pos)
#define RCC_CR_PLLRDY_Pos 25U
#define RCC_CR_PLLRDY_Msk (1UL << RCC_CR_PLLRDY_Pos)

/* RCC_CFGR bits */
#define RCC_CFGR_SW_Pos 0U
#define RCC_CFGR_SW_Msk (0x3UL << RCC_CFGR_SW_Pos)
#define RCC_CFGR_SW_PLL (0x3UL << RCC_CFGR_SW_Pos)
#define RCC_CFGR_SWS_Pos 2U
#define RCC_CFGR_SWS_Msk (0x3UL << RCC_CFGR_SWS_Pos)
#define RCC_CFGR_SWS_PLL (0x3UL << RCC_CFGR_SWS_Pos)
#define RCC_CFGR_HPRE_Pos 4U
#define RCC_CFGR_HPRE_Msk (0xFUL << RCC_CFGR_HPRE_Pos)
#define RCC_CFGR_PPRE1_Pos 8U
#define RCC_CFGR_PPRE1_Msk (0x7UL << RCC_CFGR_PPRE1_Pos)
#define RCC_CFGR_PPRE2_Pos 11U
#define RCC_CFGR_PPRE2_Msk (0x7UL << RCC_CFGR_PPRE2_Pos)

/* RCC_PLLCFGR bits */
#define RCC_PLLCFGR_PLLSRC_Pos 0U
#define RCC_PLLCFGR_PLLSRC_Msk (0x3UL << RCC_PLLCFGR_PLLSRC_Pos)
#define RCC_PLLCFGR_PLLSRC_HSE (0x3UL << RCC_PLLCFGR_PLLSRC_Pos)
#define RCC_PLLCFGR_PLLM_Pos 4U
#define RCC_PLLCFGR_PLLM_Msk (0xFUL << RCC_PLLCFGR_PLLM_Pos)
#define RCC_PLLCFGR_PLLN_Pos 8U
#define RCC_PLLCFGR_PLLN_Msk (0x7FUL << RCC_PLLCFGR_PLLN_Pos)
#define RCC_PLLCFGR_PLLPEN_Pos 16U
#define RCC_PLLCFGR_PLLPEN_Msk (1UL << RCC_PLLCFGR_PLLPEN_Pos)
#define RCC_PLLCFGR_PLLP_Pos 17U
#define RCC_PLLCFGR_PLLP_Msk (1UL << RCC_PLLCFGR_PLLP_Pos)
#define RCC_PLLCFGR_PLLQEN_Pos 20U
#define RCC_PLLCFGR_PLLQEN_Msk (1UL << RCC_PLLCFGR_PLLQEN_Pos)
#define RCC_PLLCFGR_PLLQ_Pos 21U
#define RCC_PLLCFGR_PLLQ_Msk (0x3UL << RCC_PLLCFGR_PLLQ_Pos)
#define RCC_PLLCFGR_PLLREN_Pos 24U
#define RCC_PLLCFGR_PLLREN_Msk (1UL << RCC_PLLCFGR_PLLREN_Pos)
#define RCC_PLLCFGR_PLLR_Pos 25U
#define RCC_PLLCFGR_PLLR_Msk (0x3UL << RCC_PLLCFGR_PLLR_Pos)

/* RCC_CSR bits */
#define RCC_CSR_LSION_Pos 0U
#define RCC_CSR_LSION_Msk (1UL << RCC_CSR_LSION_Pos)
#define RCC_CSR_LSIRDY_Pos 1U
#define RCC_CSR_LSIRDY_Msk (1UL << RCC_CSR_LSIRDY_Pos)

/* RCC_AHB1ENR enable bits */
#define RCC_AHB1ENR_DMA1EN_Pos 0U
#define RCC_AHB1ENR_DMA1EN_Msk (1UL << RCC_AHB1ENR_DMA1EN_Pos)
#define RCC_AHB1ENR_FLASHEN_Pos 8U
#define RCC_AHB1ENR_FLASHEN_Msk (1UL << RCC_AHB1ENR_FLASHEN_Pos)

/* RCC_AHB2ENR enable bits */
#define RCC_AHB2ENR_GPIOAEN_Pos 0U
#define RCC_AHB2ENR_GPIOBEN_Pos 1U
#define RCC_AHB2ENR_GPIOCEN_Pos 2U
#define RCC_AHB2ENR_GPIODEN_Pos 3U
#define RCC_AHB2ENR_GPIOEEN_Pos 4U
#define RCC_AHB2ENR_GPIOFEN_Pos 5U
#define RCC_AHB2ENR_GPIOGEN_Pos 6U
#define RCC_AHB2ENR_GPIOAEN_Msk (1UL << RCC_AHB2ENR_GPIOAEN_Pos)
#define RCC_AHB2ENR_GPIOBEN_Msk (1UL << RCC_AHB2ENR_GPIOBEN_Pos)
#define RCC_AHB2ENR_GPIOCEN_Msk (1UL << RCC_AHB2ENR_GPIOCEN_Pos)
#define RCC_AHB2ENR_GPIODEN_Msk (1UL << RCC_AHB2ENR_GPIODEN_Pos)
#define RCC_AHB2ENR_GPIOEEN_Msk (1UL << RCC_AHB2ENR_GPIOEEN_Pos)
#define RCC_AHB2ENR_GPIOFEN_Msk (1UL << RCC_AHB2ENR_GPIOFEN_Pos)
#define RCC_AHB2ENR_GPIOGEN_Msk (1UL << RCC_AHB2ENR_GPIOGEN_Pos)
#define RCC_AHB2ENR_ADC12EN_Pos 13U
#define RCC_AHB2ENR_ADC12EN_Msk (1UL << RCC_AHB2ENR_ADC12EN_Pos)
#define RCC_AHB2ENR_DAC1EN_Pos 16U
#define RCC_AHB2ENR_DAC1EN_Msk (1UL << RCC_AHB2ENR_DAC1EN_Pos)
#define RCC_AHB2ENR_DAC3EN_Pos 18U
#define RCC_AHB2ENR_DAC3EN_Msk (1UL << RCC_AHB2ENR_DAC3EN_Pos)

/* RCC_APB1ENR1 enable bits */
#define RCC_APB1ENR1_TIM2EN_Pos 0U
#define RCC_APB1ENR1_TIM2EN_Msk (1UL << RCC_APB1ENR1_TIM2EN_Pos)
#define RCC_APB1ENR1_USART2EN_Pos 17U
#define RCC_APB1ENR1_USART2EN_Msk (1UL << RCC_APB1ENR1_USART2EN_Pos)
#define RCC_APB1ENR1_PWREN_Pos 28U
#define RCC_APB1ENR1_PWREN_Msk (1UL << RCC_APB1ENR1_PWREN_Pos)

/* RCC_APB2ENR enable bits */
#define RCC_APB2ENR_SYSCFGEN_Pos 0U
#define RCC_APB2ENR_SYSCFGEN_Msk (1UL << RCC_APB2ENR_SYSCFGEN_Pos)
#define RCC_APB2ENR_USART1EN_Pos 14U
#define RCC_APB2ENR_USART1EN_Msk (1UL << RCC_APB2ENR_USART1EN_Pos)
#define RCC_APB2ENR_HRTIM1EN_Pos 26U
#define RCC_APB2ENR_HRTIM1EN_Msk (1UL << RCC_APB2ENR_HRTIM1EN_Pos)

/* --- IWDG --- */

typedef struct {
	volatile uint32_t KR;   /* 0x00 key */
	volatile uint32_t PR;   /* 0x04 prescaler */
	volatile uint32_t RLR;  /* 0x08 reload */
	volatile uint32_t SR;   /* 0x0C status */
	volatile uint32_t WINR; /* 0x10 window */
} IWDG_Type;

#define IWDG ((IWDG_Type *)0x40003000UL)

#define IWDG_KR_KEY_ENABLE 0x5555U /* unlock PR/RLR */
#define IWDG_KR_KEY_RELOAD 0xAAAAU /* reload counter (feed) */
#define IWDG_KR_KEY_START 0xCCCCU  /* start watchdog */

#define IWDG_SR_PVU_Pos 0U
#define IWDG_SR_PVU_Msk (1UL << IWDG_SR_PVU_Pos)
#define IWDG_SR_RVU_Pos 1U
#define IWDG_SR_RVU_Msk (1UL << IWDG_SR_RVU_Pos)

/* LSI is 32 kHz. /256 -> 125 Hz; reload 125 -> ~1 s watchdog period. */
#define IWDG_PR_DIV256 6U
#define IWDG_RELOAD_1S 125U

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

/* Board clock: 170 MHz after the clock_control driver runs the PLL tree
 * (see drivers/clock_control/clock_stm32.c). Reset value reflects HSI16. */
extern uint32_t SystemCoreClock;

#ifdef __cplusplus
}
#endif

#endif /* STM32G474_H_ */
