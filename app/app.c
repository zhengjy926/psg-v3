/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * app — application layer (see app.h).
 */

#include "app.h"

#include "drivers/api/watchdog.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

static struct app_config app_cfg;

void app_init(const struct app_config *cfg)
{
	if (cfg == NULL) {
		return;
	}
	app_cfg = *cfg;
	heartbeat_init(&app_cfg.heartbeat);
	LOG_INF("app ready");
}

void app_step(void)
{
	heartbeat_step();

	/* Feed the watchdog once, unconditionally, at the end of the loop
	 * (#5 §3). Routes through diag.feed_watchdog() once the diag service
	 * lands; for now app feeds it directly. */
	if (app_cfg.watchdog != NULL) {
		watchdog_feed(app_cfg.watchdog);
	}
}
