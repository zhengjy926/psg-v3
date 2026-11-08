/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * dma driver (target, skeleton). Registers DMA1 at PRE_KERNEL_2 prio 30 (the
 * ADC sample data path). Channel/stream configuration lands in the dma ticket;
 * here init only gates its peripheral clock.
 */

#include "drivers/api/clock_control.h"

#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>

LOG_MODULE_REGISTER(dma_stm32, LOG_LEVEL_INF);

static int dma_stm32_init(const struct device *dev)
{
	const struct device *clock;

	ARG_UNUSED(dev);
	clock = device_get_binding("clock_control");
	__ASSERT(clock != NULL, "clock_control device not ready");
	if (clock == NULL) {
		return -1;
	}
	(void)clock_control_on(clock, PSG_CLOCK_DMA1);
	LOG_INF("dma ready");
	return 0;
}

DEVICE_DEFINE(dma, "dma", dma_stm32_init, NULL, NULL, NULL, PRE_KERNEL_2, 30,
	      NULL);
