/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * heartbeat service (see heartbeat.h). Blinks the LED at a fixed rate; does
 * not carry fault semantics (F-015).
 */

#include "heartbeat.h"

#include "drivers/api/gpio.h"
#include "osal/osal.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(heartbeat, LOG_LEVEL_INF);

static struct heartbeat_config hb_cfg;
static uint32_t hb_last_toggle;
static int hb_state;

void heartbeat_init(const struct heartbeat_config *cfg)
{
	if (cfg == NULL) {
		return;
	}
	hb_cfg = *cfg;
	hb_last_toggle = osal_uptime_ms();
	hb_state = 0;

	/* PB9 is active-low; start with the LED off (pin high). */
	if (hb_cfg.gpio != NULL) {
		(void)gpio_set(hb_cfg.gpio, hb_cfg.led_pin, GPIO_HIGH);
	}
}

void heartbeat_step(void)
{
	uint32_t now = osal_uptime_ms();
	uint32_t period;

	if (hb_cfg.gpio == NULL) {
		return;
	}
	period = hb_cfg.half_period_ms;
	if (period == 0U) {
		period = 500U;
	}

	if ((now - hb_last_toggle) >= period) {
		hb_last_toggle = now;
		hb_state = (hb_state == 0) ? 1 : 0;
		(void)gpio_set(hb_cfg.gpio, hb_cfg.led_pin,
			       hb_state ? GPIO_LOW : GPIO_HIGH);
	}
}
