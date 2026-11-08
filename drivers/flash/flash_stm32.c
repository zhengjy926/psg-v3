/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * flash driver (target, skeleton). Registers the internal Flash at
 * PRE_KERNEL_2 prio 60 (read/erase/program/page_size, used by calib #12).
 * The read/erase/program ops land in the calib ticket; here init only gates
 * the Flash interface clock.
 */

#include "drivers/api/clock_control.h"

#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>

LOG_MODULE_REGISTER(flash_stm32, LOG_LEVEL_INF);

static int flash_stm32_init(const struct device *dev)
{
	const struct device *clock;

	ARG_UNUSED(dev);
	clock = device_get_binding("clock_control");
	__ASSERT(clock != NULL, "clock_control device not ready");
	if (clock == NULL) {
		return -1;
	}
	(void)clock_control_on(clock, PSG_CLOCK_FLASH);
	LOG_INF("flash ready");
	return 0;
}

DEVICE_DEFINE(flash, "flash", flash_stm32_init, NULL, NULL, NULL, PRE_KERNEL_2,
	      60, NULL);
