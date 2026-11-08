/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * clock_control driver (target). Skeleton: enables AHB2 peripheral clock gates
 * (GPIO ports) on demand; get_rate() reports SystemCoreClock. Full PLL tree
 * configuration lives in board/stm32g474/system_stm32g474.c.
 */

#include "drivers/api/clock_control.h"

#include "board/stm32g474/stm32g474.h"
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(clock_stm32, LOG_LEVEL_INF);

/* Peripheral gate ids: AHB2ENR bit masks for GPIO ports. */
#define CLOCK_GPIOA (1UL << RCC_AHB2ENR_GPIOAEN_Pos)
#define CLOCK_GPIOB (1UL << RCC_AHB2ENR_GPIOBEN_Pos)
#define CLOCK_GPIOC (1UL << RCC_AHB2ENR_GPIOCEN_Pos)
#define CLOCK_GPIOD (1UL << RCC_AHB2ENR_GPIODEN_Pos)
#define CLOCK_GPIOE (1UL << RCC_AHB2ENR_GPIOEEN_Pos)
#define CLOCK_GPIOF (1UL << RCC_AHB2ENR_GPIOFEN_Pos)

static int clock_stm32_on(const struct device *dev, uint32_t peripheral)
{
	ARG_UNUSED(dev);
	RCC->AHB2ENR |= peripheral;
	return 0;
}

static uint32_t clock_stm32_get_rate(const struct device *dev)
{
	ARG_UNUSED(dev);
	return SystemCoreClock;
}

static const struct clock_control_driver_api clock_stm32_api = {
	.on = clock_stm32_on,
	.get_rate = clock_stm32_get_rate,
};

static int clock_stm32_init(const struct device *dev)
{
	ARG_UNUSED(dev);
	LOG_INF("clock ready");
	return 0;
}

DEVICE_DEFINE(clock_control, "clock_control", clock_stm32_init, NULL, NULL, NULL,
	      PRE_KERNEL_1, 1, &clock_stm32_api);
