/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * app — application layer (see app.h).
 */

#include "app.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

void app_init(const struct app_config *cfg)
{
	struct app_config local_cfg;

	if (cfg == NULL) {
		return;
	}
	local_cfg = *cfg;
	heartbeat_init(&local_cfg.heartbeat);
	LOG_INF("app ready");
}

void app_step(void)
{
	heartbeat_step();
}
