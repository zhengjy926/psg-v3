/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * clock_control driver API. This is the host-clean seam: it must never include
 * STM32 or other MCU headers. Target and host (fake) drivers implement the same
 * ops structure.
 *
 * The `on` op centralizes peripheral clock gating (#5): every driver's
 * ops.init calls clock_control_on() with its own subsys id, and the target
 * driver maps that id to the correct RCC enable bit.
 */

#ifndef PSG_DRIVERS_API_CLOCK_CONTROL_H_
#define PSG_DRIVERS_API_CLOCK_CONTROL_H_

#include <stdint.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>

/**
 * Peripheral clock-gate ids. Opaque to the host-clean layers; the target
 * clock_control driver maps each to (bus enable register, bit).
 */
enum psg_clock_subsys {
	PSG_CLOCK_GPIOA = 0,
	PSG_CLOCK_GPIOB,
	PSG_CLOCK_GPIOC,
	PSG_CLOCK_GPIOD,
	PSG_CLOCK_GPIOE,
	PSG_CLOCK_GPIOF,
	PSG_CLOCK_GPIOG,
	PSG_CLOCK_DMA1,
	PSG_CLOCK_ADC12,
	PSG_CLOCK_DAC1,
	PSG_CLOCK_DAC3,
	PSG_CLOCK_HRTIM1,
	PSG_CLOCK_USART1,
	PSG_CLOCK_USART2,
	PSG_CLOCK_TIM2,
	PSG_CLOCK_FLASH,
	PSG_CLOCK_PWR,
	PSG_CLOCK_SYSCFG,
	PSG_CLOCK_COUNT,
};

/* get_status() contract: 0 = healthy; each set bit is a failure. */
#define CLOCK_STATUS_PLL_NOT_LOCKED BIT(0)

struct clock_control_driver_api {
	/** Enable the peripheral clock gate (centralized gating). */
	int (*on)(const struct device *dev, uint32_t subsys);
	/** Return the clock rate in Hz (HRTIM ns resolution). */
	uint32_t (*get_rate)(const struct device *dev);
	/** Return a clock health bitmask (0 = healthy). */
	uint32_t (*get_status)(const struct device *dev);
};

/** Convenience wrappers. */
static inline int clock_control_on(const struct device *dev, uint32_t subsys)
{
	const struct clock_control_driver_api *api =
		DEVICE_API_GET(clock_control, dev);

	return api->on(dev, subsys);
}

static inline uint32_t clock_control_get_rate(const struct device *dev)
{
	const struct clock_control_driver_api *api =
		DEVICE_API_GET(clock_control, dev);

	return api->get_rate(dev);
}

static inline uint32_t clock_control_get_status(const struct device *dev)
{
	const struct clock_control_driver_api *api =
		DEVICE_API_GET(clock_control, dev);

	return api->get_status(dev);
}

#endif /* PSG_DRIVERS_API_CLOCK_CONTROL_H_ */
