/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * comparator driver (target, skeleton). Registers COMP1/3 (the two channel
 * over-current comparators) at PRE_KERNEL_2 prio 21, after the dac that writes
 * its threshold reference. COMP has no RCC clock gate; the enable + output
 * wiring land in the comparator ticket. Here init only asserts its upstream
 * dependency is ready.
 */

#include <zephyr/device.h>
#include <zephyr/errno.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>

LOG_MODULE_REGISTER(comparator_stm32, LOG_LEVEL_INF);

static int comparator_stm32_init(const struct device *dev)
{
	const struct device *dac;

	ARG_UNUSED(dev);
	dac = device_get_binding("dac");
	__ASSERT(dac != NULL, "dac device not ready");
	if (dac == NULL) {
		return -ENODEV;
	}
	LOG_INF("comparator ready");
	return 0;
}

DEVICE_DEFINE(comparator, "comparator", comparator_stm32_init, NULL, NULL, NULL,
	      PRE_KERNEL_2, 21, NULL);
