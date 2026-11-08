/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * fake gpio driver — the host "second adapter". Implements the same
 * gpio_driver_api ops structure as the target driver and registers through the
 * same device model, so tests exercise the real interface shape, not a
 * hand-written stub.
 */

#include "drivers/api/gpio.h"

#include <zephyr/device.h>

static uint32_t fake_pin_mode[16];
static uint32_t fake_pin_value[16];

static int fake_gpio_configure(const struct device *dev, uint32_t pin,
			       uint32_t mode)
{
	ARG_UNUSED(dev);
	if (pin >= 16U) {
		return -1;
	}
	fake_pin_mode[pin] = mode;
	return 0;
}

static int fake_gpio_set(const struct device *dev, uint32_t pin, uint32_t value)
{
	ARG_UNUSED(dev);
	if (pin >= 16U) {
		return -1;
	}
	fake_pin_value[pin] = (value != 0U) ? 1U : 0U;
	return 0;
}

static int fake_gpio_get(const struct device *dev, uint32_t pin)
{
	ARG_UNUSED(dev);
	if (pin >= 16U) {
		return 0;
	}
	return (int)fake_pin_value[pin];
}

static const struct gpio_driver_api fake_gpio_api = {
	.configure = fake_gpio_configure,
	.set = fake_gpio_set,
	.get = fake_gpio_get,
};

static int fake_gpio_init(const struct device *dev)
{
	ARG_UNUSED(dev);
	return 0;
}

DEVICE_DEFINE(gpio, "gpio", fake_gpio_init, NULL, NULL, NULL, PRE_KERNEL_1, 10,
	      &fake_gpio_api);
