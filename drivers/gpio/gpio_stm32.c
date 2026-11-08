/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * gpio driver (target). Skeleton: one instance driving GPIOB (board LED PB9,
 * run indicator PB5). Demonstrates the driver seam: the same ops structure is
 * implemented by the host fake driver.
 */

#include "drivers/api/clock_control.h"
#include "drivers/api/gpio.h"

#include "board/stm32g474/stm32g474.h"
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>

LOG_MODULE_REGISTER(gpio_stm32, LOG_LEVEL_INF);

struct gpio_stm32_config {
	GPIO_Type *port;
	uint32_t clock_subsys;
};

static const struct gpio_stm32_config gpio_stm32_cfg = {
	.port = GPIOB,
	.clock_subsys = PSG_CLOCK_GPIOB,
};

static int gpio_stm32_configure(const struct device *dev, uint32_t pin,
				uint32_t mode)
{
	const struct gpio_stm32_config *cfg = dev->config;

	if (pin > 15U) {
		return -1;
	}
	if (mode == GPIO_OUTPUT) {
		cfg->port->MODER &= ~(3UL << (pin * 2U));
		cfg->port->MODER |= (1UL << (pin * 2U));
	} else {
		cfg->port->MODER &= ~(3UL << (pin * 2U));
	}
	return 0;
}

static int gpio_stm32_set(const struct device *dev, uint32_t pin, uint32_t value)
{
	const struct gpio_stm32_config *cfg = dev->config;

	if (pin > 15U) {
		return -1;
	}
	if (value != 0U) {
		cfg->port->BSRR = (1UL << pin);
	} else {
		cfg->port->BRR = (1UL << pin);
	}
	return 0;
}

static int gpio_stm32_get(const struct device *dev, uint32_t pin)
{
	const struct gpio_stm32_config *cfg = dev->config;

	if (pin > 15U) {
		return 0;
	}
	return (int)((cfg->port->IDR >> pin) & 1UL);
}

static const struct gpio_driver_api gpio_stm32_api = {
	.configure = gpio_stm32_configure,
	.set = gpio_stm32_set,
	.get = gpio_stm32_get,
};

static int gpio_stm32_init(const struct device *dev)
{
	const struct gpio_stm32_config *cfg = dev->config;
	const struct device *clock = device_get_binding("clock_control");

	__ASSERT(clock != NULL, "clock_control device not ready");
	if (clock == NULL) {
		return -1;
	}
	(void)clock_control_on(clock, cfg->clock_subsys);
	LOG_INF("gpio ready");
	return 0;
}

DEVICE_DEFINE(gpio, "gpio", gpio_stm32_init, NULL, NULL, &gpio_stm32_cfg,
	      PRE_KERNEL_1, 10, &gpio_stm32_api);
