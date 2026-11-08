/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * heartbeat service. Host-clean: must never include STM32 or other MCU
 * headers; it drives the on-board LED through the gpio driver API seam.
 */

#ifndef PSG_SERVICE_HEARTBEAT_HEARTBEAT_H_
#define PSG_SERVICE_HEARTBEAT_HEARTBEAT_H_

#include <stdint.h>
#include <zephyr/device.h>

struct heartbeat_config {
	/** gpio device driving the LED (injected by the composition root). */
	const struct device *gpio;
	/** LED pin id (board-specific, e.g. PB9). */
	uint32_t led_pin;
	/** Half-period of the blink, in milliseconds. */
	uint32_t half_period_ms;
};

void heartbeat_init(const struct heartbeat_config *cfg);

/** Advance the heartbeat; toggles the LED at the configured rate. */
void heartbeat_step(void);

#endif /* PSG_SERVICE_HEARTBEAT_HEARTBEAT_H_ */
