/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * heartbeat service. Host-clean: must never include STM32 or other MCU
 * headers; it drives the on-board LED through the LED driver API.
 */

#ifndef PSG_SERVICE_HEARTBEAT_HEARTBEAT_H_
#define PSG_SERVICE_HEARTBEAT_HEARTBEAT_H_

#include <stdint.h>
struct device;

struct heartbeat_config {
	/** LED controller device (injected by the composition root). */
	const struct device *led;
	/** LED index on the controller. */
	uint32_t led_index;
	/** Half-period of the blink, in milliseconds. */
	uint32_t half_period_ms;
};

int heartbeat_init(const struct heartbeat_config *cfg);

/** Advance the heartbeat; toggles the LED at the configured rate. */
void heartbeat_step(void);

#endif /* PSG_SERVICE_HEARTBEAT_HEARTBEAT_H_ */
