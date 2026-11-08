/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * adc driver (target, skeleton). Registers ADC1/2 (PA0/PA1 current sample) at
 * PRE_KERNEL_2 prio 31, after gpio and dma. Calibration and the sample op land
 * in the adc ticket; here init only asserts its upstream dependencies and
 * gates the ADC12 clock.
 */

#include "drivers/api/clock_control.h"

#include <zephyr/device.h>
#include <zephyr/errno.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>

LOG_MODULE_REGISTER(adc_stm32, LOG_LEVEL_INF);

static int adc_stm32_init(const struct device *dev)
{
	const struct device *clock;
	const struct device *gpio;
	const struct device *dma;

	ARG_UNUSED(dev);
	clock = device_get_binding("clock_control");
	gpio = device_get_binding("gpio");
	dma = device_get_binding("dma");
	__ASSERT(clock != NULL, "clock_control device not ready");
	__ASSERT(gpio != NULL, "gpio device not ready");
	__ASSERT(dma != NULL, "dma device not ready");
	if ((clock == NULL) || (gpio == NULL) || (dma == NULL)) {
		return -ENODEV;
	}
	(void)clock_control_on(clock, PSG_CLOCK_ADC12);
	LOG_INF("adc ready");
	return 0;
}

DEVICE_DEFINE(adc, "adc", adc_stm32_init, NULL, NULL, NULL, PRE_KERNEL_2, 31,
	      NULL);
