/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * heartbeat service (see heartbeat.h). Blinks the LED at a fixed rate; does
 * not carry fault semantics (F-015).
 */

#include "heartbeat.h"

#include "../../drivers/include/led.h"
#include "osal/osal.h"
#include <errno.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(heartbeat, LOG_LEVEL_INF);

static struct heartbeat_config hb_cfg;
static uint32_t hb_last_toggle;
static int hb_state;
static bool hb_initialized;

int heartbeat_init(const struct heartbeat_config *cfg)
{
	int err;

	hb_initialized = false;
	if ((cfg == NULL) || (cfg->led == NULL)) {
		return -EINVAL;
	}

	hb_cfg = *cfg;
	hb_state = 0;
	err = led_off(hb_cfg.led, hb_cfg.led_index);
	if (err != 0) {
		LOG_ERR("Failed to turn off heartbeat LED (%d)", err);
		return err;
	}

	hb_last_toggle = osal_uptime_ms();
	hb_initialized = true;
	return 0;
}

void heartbeat_step(void)
{
	uint32_t now;
	uint32_t period;
	int err;

	if (!hb_initialized) {
		return;
	}

	now = osal_uptime_ms();
	period = hb_cfg.half_period_ms;
	if (period == 0U) {
		period = 500U;
	}

	if ((now - hb_last_toggle) < period) {
		return;
	}

	err = (hb_state == 0) ? led_on(hb_cfg.led, hb_cfg.led_index)
			     : led_off(hb_cfg.led, hb_cfg.led_index);
	if (err != 0) {
		LOG_ERR("Failed to toggle heartbeat LED (%d)", err);
		hb_last_toggle = now;
		return;
	}

	hb_state = (hb_state == 0) ? 1 : 0;
	hb_last_toggle = now;
}