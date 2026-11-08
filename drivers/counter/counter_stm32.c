/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * counter driver (target, skeleton). Registers TIM2 (the µs timestamp source,
 * used only by osal) at PRE_KERNEL_2 prio 50. Prescaler configuration and the
 * read op land in the counter ticket; here init only gates the TIM2 clock.
 */

#include "drivers/api/clock_control.h"

#include <zephyr/device.h>
#include <zephyr/errno.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>

LOG_MODULE_REGISTER(counter_stm32, LOG_LEVEL_INF);

static int counter_stm32_init(const struct device *dev)
{
	const struct device *clock;

	ARG_UNUSED(dev);
	clock = device_get_binding("clock_control");
	__ASSERT(clock != NULL, "clock_control device not ready");
	if (clock == NULL) {
		return -ENODEV;
	}
	(void)clock_control_on(clock, PSG_CLOCK_TIM2);
	LOG_INF("counter ready");
	return 0;
}

DEVICE_DEFINE(counter, "counter", counter_stm32_init, NULL, NULL, NULL,
	      PRE_KERNEL_2, 50, NULL);
