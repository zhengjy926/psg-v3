/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * watchdog driver (target): IWDG on LSI. Setup (~1 s) and start happen in
 * init, at APPLICATION (the last device before the main loop, #5). The main
 * loop feeds it once per iteration through diag.feed_watchdog() later; for
 * now app_step() calls watchdog_feed() directly.
 */

#include "drivers/api/watchdog.h"

#include "board/stm32g474/stm32g474.h"
#include <zephyr/device.h>
#include <zephyr/errno.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(watchdog_stm32, LOG_LEVEL_INF);

static int watchdog_wait_ready(volatile uint32_t *reg, uint32_t mask)
{
	volatile uint32_t timeout = 100000U;

	while ((*reg & mask) == 0U) {
		if (timeout == 0U) {
			return -ETIMEDOUT;
		}
		timeout--;
	}
	return 0;
}

/* Wait for an IWDG register update to complete (SR.PVU/RVU clears). */
static int iwdg_wait_update(uint32_t mask)
{
	volatile uint32_t timeout = 100000U;

	while ((IWDG->SR & mask) != 0U) {
		if (timeout == 0U) {
			return -ETIMEDOUT;
		}
		timeout--;
	}
	return 0;
}

static void watchdog_stm32_feed(const struct device *dev)
{
	ARG_UNUSED(dev);
	IWDG->KR = IWDG_KR_KEY_RELOAD;
}

static const struct watchdog_driver_api watchdog_stm32_api = {
	.feed = watchdog_stm32_feed,
};

static int watchdog_stm32_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	/* IWDG runs from LSI (32 kHz); enable it and wait for it to settle. */
	RCC->CSR |= RCC_CSR_LSION_Msk;
	if (watchdog_wait_ready(&RCC->CSR, RCC_CSR_LSIRDY_Msk) != 0) {
		LOG_ERR("LSI not ready");
		return -ETIMEDOUT;
	}

	/* Unlock, prescale /256 -> 125 Hz, reload 125 -> ~1 s, then start. */
	IWDG->KR = IWDG_KR_KEY_ENABLE;
	IWDG->PR = IWDG_PR_DIV256;
	if (iwdg_wait_update(IWDG_SR_PVU_Msk) != 0) {
		LOG_ERR("IWDG prescaler update failed");
		return -ETIMEDOUT;
	}
	IWDG->RLR = IWDG_RELOAD_1S;
	if (iwdg_wait_update(IWDG_SR_RVU_Msk) != 0) {
		LOG_ERR("IWDG reload update failed");
		return -ETIMEDOUT;
	}
	IWDG->KR = IWDG_KR_KEY_START;
	IWDG->KR = IWDG_KR_KEY_RELOAD;

	LOG_INF("watchdog ready");
	return 0;
}

DEVICE_DEFINE(watchdog, "watchdog", watchdog_stm32_init, NULL, NULL, NULL,
	      APPLICATION, 80, &watchdog_stm32_api);
