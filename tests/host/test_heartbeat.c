/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Verifies heartbeat timing through the LED device API seam.
 */

#include "app/app.h"
#include "drivers/include/led.h"
#include "osal/osal.h"

#include <device.h>
#include <errno.h>
#include <stdint.h>

#include "unity.h"

static uint8_t fake_led_brightness;

static int fake_led_set_brightness(const struct device *dev, uint32_t led,
				   uint8_t value)
{
	(void)dev;
	if ((led != 0U) || (value > LED_BRIGHTNESS_MAX)) {
		return -EINVAL;
	}

	fake_led_brightness = value;
	return 0;
}

static DEVICE_API(led, fake_led_api) = {
	.set_brightness = fake_led_set_brightness,
};

static struct device fake_led = {
	.name = "fake-led",
	.api = &fake_led_api,
};

static void tick_ms(int ms)
{
	for (int i = 0; i < ms; ++i) {
		osal_tick();
	}
}

void test_heartbeat(void)
{
	struct app_config cfg = {
		.heartbeat = {
			.led = &fake_led,
			.led_index = 0U,
			.half_period_ms = 500U,
		},
	};

	fake_led_brightness = 0U;
	TEST_ASSERT_EQUAL_INT(0, app_init(&cfg));
	TEST_ASSERT_EQUAL_UINT8(0U, fake_led_brightness);

	tick_ms(499);
	app_step();
	TEST_ASSERT_EQUAL_UINT8(0U, fake_led_brightness);

	tick_ms(1);
	app_step();
	TEST_ASSERT_EQUAL_UINT8(LED_BRIGHTNESS_MAX, fake_led_brightness);

	tick_ms(500);
	app_step();
	TEST_ASSERT_EQUAL_UINT8(0U, fake_led_brightness);
}
