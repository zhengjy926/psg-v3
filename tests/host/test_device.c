/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Verifies the device model: device_get_binding() returns the ready fake gpio
 * device and NULL for unknown names; device_is_ready() reflects init status.
 */

#include <device.h>

#include "drivers/include/gpio.h"
#include "unity.h"

void test_device_model(void)
{
	const struct device *gpio = device_get_binding("gpio");

	TEST_ASSERT_NOT_NULL(gpio);
	TEST_ASSERT_TRUE(device_is_ready(gpio));
	TEST_ASSERT_EQUAL_STRING("gpio", gpio->name);
	TEST_ASSERT_NULL(device_get_binding("no_such_device"));
	TEST_ASSERT_NULL(device_get_binding(""));
	TEST_ASSERT_NULL(device_get_binding(NULL));
}
