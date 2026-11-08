/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * clock_control driver (target). Implements the #5-resolved clock tree
 * (PWR -> regulator boost -> Flash wait states -> HSE 8 MHz -> PLL -> SYSCLK
 * 170 MHz -> AHB/APB) and the centralized peripheral clock gating. CMSIS
 * SystemInit() only does the Flash prefetch baseline; the PLL tree runs here
 * at PRE_KERNEL_1 prio 1, before any dependent driver initializes.
 */

#include "drivers/api/clock_control.h"

#include "board/stm32g474/stm32g474.h"
#include <zephyr/device.h>
#include <zephyr/errno.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(clock_stm32, LOG_LEVEL_INF);

/* HSE is an 8 MHz crystal on PF0/PF1 (docs/MCU引脚功能映射表.md). */
#define SYSCLK_FREQ_HZ 170000000UL
/* PLL: 8 MHz / 2 * 85 / 2 = 170 MHz. PLLM field = divider - 1. */
#define PLLM_DIV2_FIELD 1U
#define PLLN_VAL 85U
/* PLLR divider /2 -> field 0 (00 = /2). */

/* Peripheral gate map: bus enable-register selector + bit index. */
enum clock_bus { BUS_AHB1 = 0, BUS_AHB2, BUS_AHB3, BUS_APB1_1, BUS_APB1_2,
		 BUS_APB2 };

struct clock_gate {
	uint8_t bus;
	uint8_t bit;
};

static const struct clock_gate clock_gates[PSG_CLOCK_COUNT] = {
	[PSG_CLOCK_GPIOA] = { BUS_AHB2, RCC_AHB2ENR_GPIOAEN_Pos },
	[PSG_CLOCK_GPIOB] = { BUS_AHB2, RCC_AHB2ENR_GPIOBEN_Pos },
	[PSG_CLOCK_GPIOC] = { BUS_AHB2, RCC_AHB2ENR_GPIOCEN_Pos },
	[PSG_CLOCK_GPIOD] = { BUS_AHB2, RCC_AHB2ENR_GPIODEN_Pos },
	[PSG_CLOCK_GPIOE] = { BUS_AHB2, RCC_AHB2ENR_GPIOEEN_Pos },
	[PSG_CLOCK_GPIOF] = { BUS_AHB2, RCC_AHB2ENR_GPIOFEN_Pos },
	[PSG_CLOCK_GPIOG] = { BUS_AHB2, RCC_AHB2ENR_GPIOGEN_Pos },
	[PSG_CLOCK_DMA1] = { BUS_AHB1, RCC_AHB1ENR_DMA1EN_Pos },
	[PSG_CLOCK_ADC12] = { BUS_AHB2, RCC_AHB2ENR_ADC12EN_Pos },
	[PSG_CLOCK_DAC1] = { BUS_AHB2, RCC_AHB2ENR_DAC1EN_Pos },
	[PSG_CLOCK_DAC3] = { BUS_AHB2, RCC_AHB2ENR_DAC3EN_Pos },
	[PSG_CLOCK_HRTIM1] = { BUS_APB2, RCC_APB2ENR_HRTIM1EN_Pos },
	[PSG_CLOCK_USART1] = { BUS_APB2, RCC_APB2ENR_USART1EN_Pos },
	[PSG_CLOCK_USART2] = { BUS_APB1_1, RCC_APB1ENR1_USART2EN_Pos },
	[PSG_CLOCK_TIM2] = { BUS_APB1_1, RCC_APB1ENR1_TIM2EN_Pos },
	[PSG_CLOCK_FLASH] = { BUS_AHB1, RCC_AHB1ENR_FLASHEN_Pos },
	[PSG_CLOCK_PWR] = { BUS_APB1_1, RCC_APB1ENR1_PWREN_Pos },
	[PSG_CLOCK_SYSCFG] = { BUS_APB2, RCC_APB2ENR_SYSCFGEN_Pos },
};

static volatile uint32_t *clock_bus_reg(uint8_t bus)
{
	switch (bus) {
	case BUS_AHB1:
		return &RCC->AHB1ENR;
	case BUS_AHB2:
		return &RCC->AHB2ENR;
	case BUS_AHB3:
		return &RCC->AHB3ENR;
	case BUS_APB1_1:
		return &RCC->APB1ENR1;
	case BUS_APB1_2:
		return &RCC->APB1ENR2;
	case BUS_APB2:
		return &RCC->APB2ENR;
	default:
		return NULL;
	}
}

static int clock_stm32_on(const struct device *dev, uint32_t subsys)
{
	volatile uint32_t *reg;

	ARG_UNUSED(dev);
	if (subsys >= PSG_CLOCK_COUNT) {
		return -EINVAL;
	}
	reg = clock_bus_reg(clock_gates[subsys].bus);
	if (reg == NULL) {
		return -EINVAL;
	}
	*reg |= (1UL << clock_gates[subsys].bit);
	return 0;
}

static uint32_t clock_stm32_get_rate(const struct device *dev)
{
	ARG_UNUSED(dev);
	return SystemCoreClock;
}

static uint32_t clock_stm32_get_status(const struct device *dev)
{
	ARG_UNUSED(dev);
	if ((RCC->CR & RCC_CR_PLLRDY_Msk) == 0U) {
		return CLOCK_STATUS_PLL_NOT_LOCKED;
	}
	return 0U;
}

/* Poll until every bit in mask is set, or a countdown timeout expires. */
static int clock_wait_ready(volatile uint32_t *reg, uint32_t mask)
{
	volatile uint32_t timeout = 100000U;

	while ((*reg & mask) != mask) {
		if (timeout == 0U) {
			return -ETIMEDOUT;
		}
		timeout--;
	}
	return 0;
}

/* Poll until every bit in mask is cleared, or a countdown timeout expires. */
static int clock_wait_clear(volatile uint32_t *reg, uint32_t mask)
{
	volatile uint32_t timeout = 100000U;

	while ((*reg & mask) != 0U) {
		if (timeout == 0U) {
			return -ETIMEDOUT;
		}
		timeout--;
	}
	return 0;
}

/*
 * Full clock tree (RM0440). Runs from HSI16 (default) and switches to HSE ->
 * PLL -> 170 MHz SYSCLK. On failure returns a negative errno; do_device_init
 * records it and boot continues (fault word bit0 is set later by the
 * composition root's readiness check).
 */
static int clock_stm32_configure(void)
{
	/* 1. PWR clock, then Range 1 boost (R1MODE=0) + VOS=Range 1. VOSF is set
	 * while the regulator scales and cleared when it reaches the target. */
	RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN_Msk;
	PWR->CR5 &= ~PWR_CR5_R1MODE_Msk;
	PWR->CR1 = (PWR->CR1 & ~PWR_CR1_VOS_Msk) | PWR_CR1_VOS_RANGE1;
	if (clock_wait_clear(&PWR->SR2, PWR_SR2_VOSF_Msk) != 0) {
		return -ETIMEDOUT;
	}

	/* 2. Flash: 4 wait states + prefetch + instruction/data caches. */
	FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY_Msk) |
		     (FLASH_LATENCY_170MHZ << FLASH_ACR_LATENCY_Pos) |
		     FLASH_ACR_PRFTEN_Msk | FLASH_ACR_ICEN_Msk | FLASH_ACR_DCEN_Msk;

	/* 3. HSE on (crystal, not bypass). */
	RCC->CR |= RCC_CR_HSEON_Msk;
	if (clock_wait_ready(&RCC->CR, RCC_CR_HSERDY_Msk) != 0) {
		return -ETIMEDOUT;
	}

	/* 4. PLL = HSE / 2 * 85, R = /2 -> 170 MHz. */
	RCC->PLLCFGR = RCC_PLLCFGR_PLLSRC_HSE |
		       ((uint32_t)PLLM_DIV2_FIELD << RCC_PLLCFGR_PLLM_Pos) |
		       ((uint32_t)PLLN_VAL << RCC_PLLCFGR_PLLN_Pos) |
		       RCC_PLLCFGR_PLLREN_Msk;
	RCC->CR |= RCC_CR_PLLON_Msk;
	if (clock_wait_ready(&RCC->CR, RCC_CR_PLLRDY_Msk) != 0) {
		return -ETIMEDOUT;
	}

	/* 5. AHB/APB prescalers = /1 (G4 APB1/APB2 max is 170 MHz). TIMPRE has
	 * no G4 equivalent: with no APB divider the timer clock = APB = 170 MHz. */
	RCC->CFGR &= ~(RCC_CFGR_HPRE_Msk | RCC_CFGR_PPRE1_Msk |
		       RCC_CFGR_PPRE2_Msk);

	/* 6. Switch SYSCLK to PLL and wait for the switch. */
	RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | RCC_CFGR_SW_PLL;
	if (clock_wait_ready(&RCC->CFGR, RCC_CFGR_SWS_PLL) != 0) {
		return -ETIMEDOUT;
	}

	SystemCoreClock = SYSCLK_FREQ_HZ;
	return 0;
}

static int clock_stm32_init(const struct device *dev)
{
	ARG_UNUSED(dev);
	if (clock_stm32_configure() != 0) {
		LOG_ERR("clock tree failed");
		return -ETIMEDOUT;
	}
	LOG_INF("clock ready: %u Hz", (unsigned int)SystemCoreClock);
	return 0;
}

static const struct clock_control_driver_api clock_stm32_api = {
	.on = clock_stm32_on,
	.get_rate = clock_stm32_get_rate,
	.get_status = clock_stm32_get_status,
};

DEVICE_DEFINE(clock_control, "clock_control", clock_stm32_init, NULL, NULL, NULL,
	      PRE_KERNEL_1, 1, &clock_stm32_api);
