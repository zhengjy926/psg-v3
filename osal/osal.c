/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * osal — minimal OS abstraction layer (see osal.h).
 */

#include "osal.h"

#include <zephyr/init.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(osal, LOG_LEVEL_INF);

static volatile uint32_t osal_ticks;

int osal_init(void)
{
	osal_ticks = 0U;
	LOG_INF("osal ready");
	return 0;
}

void osal_tick(void)
{
	osal_ticks++;
}

uint32_t osal_uptime_ms(void)
{
	return osal_ticks;
}

SYS_INIT(osal_init, POST_KERNEL, 1);
