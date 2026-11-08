/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * watchdog driver API. Host-clean seam: must never include STM32 or other MCU
 * headers. The target driver implements the IWDG; a host fake can implement
 * the same ops.
 */

#ifndef PSG_DRIVERS_API_WATCHDOG_H_
#define PSG_DRIVERS_API_WATCHDOG_H_

#include <zephyr/device.h>

struct watchdog_driver_api {
	/** Reload the watchdog counter (feed). */
	void (*feed)(const struct device *dev);
};

/** Reload the watchdog counter. */
static inline void watchdog_feed(const struct device *dev)
{
	const struct watchdog_driver_api *api = DEVICE_API_GET(watchdog, dev);

	api->feed(dev);
}

#endif /* PSG_DRIVERS_API_WATCHDOG_H_ */
