/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * app — application layer (see app.h).
 */

#include "app.h"

#include <errno.h>

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

int app_init(const struct app_config *cfg)
{
	if (cfg == NULL) {
		return -EINVAL;
	}

	return heartbeat_init(&cfg->heartbeat);
}

void app_step(void)
{
	heartbeat_step();
}