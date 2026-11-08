/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * hrtim driver (target, skeleton). Registers HRTIM1 at PRE_KERNEL_2 prio 22,
 * after gpio and comparator (EEV source = COMP, X1 safety loop). Output stays
 * disabled; the EEV/output configuration lands in the hrtim ticket. Here init
 * only asserts its upstream dependencies and gates its peripheral clock.
 */

#include "drivers/api/clock_control.h"

#include <zephyr/device.h>
#include <zephyr/errno.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>

LOG_MODULE_REGISTER(hrtim_stm32, LOG_LEVEL_INF);

static int hrtim_stm32_init(const struct device *dev)
{
	const struct device *clock;
	const struct device *gpio;
	const struct device *comparator;

	ARG_UNUSED(dev);
	clock = device_get_binding("clock_control");
	gpio = device_get_binding("gpio");
	comparator = device_get_binding("comparator");
	__ASSERT(clock != NULL, "clock_control device not ready");
	__ASSERT(gpio != NULL, "gpio device not ready");
	__ASSERT(comparator != NULL, "comparator device not ready");
	if ((clock == NULL) || (gpio == NULL) || (comparator == NULL)) {
		return -ENODEV;
	}
	(void)clock_control_on(clock, PSG_CLOCK_HRTIM1);
	LOG_INF("hrtim ready (output disabled)");
	return 0;
}

DEVICE_DEFINE(hrtim, "hrtim", hrtim_stm32_init, NULL, NULL, NULL, PRE_KERNEL_2,
	      22, NULL);
