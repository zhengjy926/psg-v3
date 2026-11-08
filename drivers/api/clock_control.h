/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * clock_control driver API. This is the host-clean seam: it must never include
 * STM32 or other MCU headers. Target and host (fake) drivers implement the same
 * ops structure.
 */

#ifndef PSG_DRIVERS_API_CLOCK_CONTROL_H_
#define PSG_DRIVERS_API_CLOCK_CONTROL_H_

#include <stdint.h>
#include <zephyr/device.h>

struct clock_control_driver_api {
	/** Enable the peripheral clock gate (centralized gating). */
	int (*on)(const struct device *dev, uint32_t peripheral);
	/** Return the clock rate in Hz for the device (HRTIM ns resolution). */
	uint32_t (*get_rate)(const struct device *dev);
};

/** Convenience wrappers. */
static inline int clock_control_on(const struct device *dev, uint32_t peripheral)
{
	const struct clock_control_driver_api *api =
		DEVICE_API_GET(clock_control, dev);

	return api->on(dev, peripheral);
}

static inline uint32_t clock_control_get_rate(const struct device *dev)
{
	const struct clock_control_driver_api *api =
		DEVICE_API_GET(clock_control, dev);

	return api->get_rate(dev);
}

#endif /* PSG_DRIVERS_API_CLOCK_CONTROL_H_ */
