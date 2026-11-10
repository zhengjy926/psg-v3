/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * fake gpio driver — the host "second adapter". Implements the same
 * gpio_driver_api ops structure as the target driver and registers through the
 * same device model, so tests exercise the real interface shape, not a
 * hand-written stub. No test drives the pins, so the ops are minimal.
 */

#include "drivers/include/gpio.h"

#include <device.h>

static gpio_port_value_t fake_port_value;

static int fake_gpio_pin_configure(const struct device *dev, gpio_pin_t pin,
				   gpio_flags_t flags)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(pin);
	ARG_UNUSED(flags);
	return 0;
}

static int fake_gpio_port_get_raw(const struct device *dev,
				  gpio_port_value_t *value)
{
	ARG_UNUSED(dev);
	*value = fake_port_value;
	return 0;
}

static int fake_gpio_port_set_masked_raw(const struct device *dev,
					 gpio_port_pins_t mask,
					 gpio_port_value_t value)
{
	ARG_UNUSED(dev);
	fake_port_value = (fake_port_value & ~mask) | (value & mask);
	return 0;
}

static int fake_gpio_port_set_bits_raw(const struct device *dev,
				       gpio_port_pins_t pins)
{
	ARG_UNUSED(dev);
	fake_port_value |= pins;
	return 0;
}

static int fake_gpio_port_clear_bits_raw(const struct device *dev,
					 gpio_port_pins_t pins)
{
	ARG_UNUSED(dev);
	fake_port_value &= ~pins;
	return 0;
}

static int fake_gpio_port_toggle_bits(const struct device *dev,
				      gpio_port_pins_t pins)
{
	ARG_UNUSED(dev);
	fake_port_value ^= pins;
	return 0;
}

static const struct gpio_driver_api fake_gpio_api = {
	.pin_configure = fake_gpio_pin_configure,
	.port_get_raw = fake_gpio_port_get_raw,
	.port_set_masked_raw = fake_gpio_port_set_masked_raw,
	.port_set_bits_raw = fake_gpio_port_set_bits_raw,
	.port_clear_bits_raw = fake_gpio_port_clear_bits_raw,
	.port_toggle_bits = fake_gpio_port_toggle_bits,
};

static int fake_gpio_init(const struct device *dev)
{
	ARG_UNUSED(dev);
	return 0;
}

DEVICE_DEFINE(gpio, "gpio", fake_gpio_init, NULL, NULL, PRE_KERNEL_1, 10,
	      &fake_gpio_api);
