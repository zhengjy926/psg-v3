/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Verifies the heartbeat service drives the LED through the injected (fake)
 * gpio device: LED starts off, toggles on after one half-period, off after two.
 */

#include "app/app.h"
#include "drivers/api/gpio.h"
#include "osal/osal.h"

#include <zephyr/device.h>

#include "unity.h"

#define LED_PIN 9U

static void tick_ms(int ms)
{
	int i;

	for (i = 0; i < ms; i++) {
		osal_tick();
	}
}

void test_heartbeat(void)
{
	const struct device *gpio = device_get_binding("gpio");
	struct app_config cfg;

	TEST_ASSERT_NOT_NULL(gpio);

	cfg.heartbeat.gpio = gpio;
	cfg.heartbeat.led_pin = LED_PIN;
	cfg.heartbeat.half_period_ms = 500U;

	app_init(&cfg);

	/* LED starts off (active-low -> pin high). */
	TEST_ASSERT_EQUAL_INT(GPIO_HIGH, gpio_get(gpio, LED_PIN));

	/* One half-period -> on. */
	tick_ms(500);
	app_step();
	TEST_ASSERT_EQUAL_INT(GPIO_LOW, gpio_get(gpio, LED_PIN));

	/* Second half-period -> off. */
	tick_ms(500);
	app_step();
	TEST_ASSERT_EQUAL_INT(GPIO_HIGH, gpio_get(gpio, LED_PIN));
}
