/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * dac driver (target, skeleton). Registers the DAC1/3 instance that writes the
 * comparator over-current threshold reference (#5: dac -> comparator, prio 20).
 * Threshold programming and the set_value op land in the dac ticket; here the
 * driver only gates its own peripheral clock so the instance is discoverable
 * by device_get_binding()/device_is_ready().
 */

#include "drivers/api/clock_control.h"

#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>

LOG_MODULE_REGISTER(dac_stm32, LOG_LEVEL_INF);

static int dac_stm32_init(const struct device *dev)
{
	const struct device *clock;

	ARG_UNUSED(dev);
	clock = device_get_binding("clock_control");
	__ASSERT(clock != NULL, "clock_control device not ready");
	if (clock == NULL) {
		return -1;
	}
	(void)clock_control_on(clock, PSG_CLOCK_DAC1);
	(void)clock_control_on(clock, PSG_CLOCK_DAC3);
	LOG_INF("dac ready");
	return 0;
}

DEVICE_DEFINE(dac, "dac", dac_stm32_init, NULL, NULL, NULL, PRE_KERNEL_2, 20,
	      NULL);
