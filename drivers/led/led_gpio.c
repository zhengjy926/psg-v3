/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Statically configured GPIO LED controller
 */

#include <device.h>
#include <errno.h>
#include <init.h>
#include <led.h>
#include <stdbool.h>
#include <stdint.h>
#include <stm32g4xx.h>
#include <stm32_ll_bus.h>
#include <stm32_ll_gpio.h>

#include "../../board/board.h"

struct led_gpio_config {
	GPIO_TypeDef *port;
	uint32_t pin;
	uint32_t clock_enable;
	bool active_low;
};

static int led_gpio_set_brightness(const struct device *dev, uint32_t led,
				   uint8_t value)
{
	const struct led_gpio_config *config = dev->config;
	bool on;
	bool high;

	if ((led != 0U) || (value > LED_BRIGHTNESS_MAX)) {
		return -EINVAL;
	}

	on = value != 0U;
	high = on != config->active_low;
	if (high) {
		LL_GPIO_SetOutputPin(config->port, config->pin);
	} else {
		LL_GPIO_ResetOutputPin(config->port, config->pin);
	}

	return 0;
}

static int led_gpio_init(const struct device *dev)
{
	const struct led_gpio_config *config = dev->config;

	LL_AHB2_GRP1_EnableClock(config->clock_enable);
	LL_GPIO_SetOutputPin(config->port, config->pin);
	LL_GPIO_SetPinOutputType(config->port, config->pin,
				 LL_GPIO_OUTPUT_PUSHPULL);
	LL_GPIO_SetPinPull(config->port, config->pin, LL_GPIO_PULL_NO);
	LL_GPIO_SetPinSpeed(config->port, config->pin, LL_GPIO_SPEED_FREQ_LOW);
	LL_GPIO_SetPinMode(config->port, config->pin, LL_GPIO_MODE_OUTPUT);

	return 0;
}

static DEVICE_API(led, led_gpio_api) = {
	.set_brightness = led_gpio_set_brightness,
};

static const struct led_gpio_config board_led_config = {
	.port = GPIOB,
	.pin = 1U << BOARD_LED_PIN,
	.clock_enable = LL_AHB2_GRP1_PERIPH_GPIOB,
	.active_low = true,
};

DEVICE_DEFINE(board_led, BOARD_LED_DEVICE_NAME, led_gpio_init, NULL,
	      &board_led_config, POST_KERNEL, 5, &led_gpio_api);