/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * board/devices.c — composition root (target). Runs at APPLICATION (last step
 * before main()): performs the "all devices ready" final check (#5 §6), binds
 * the concrete devices to service configuration and hands the result to
 * app_init(). This is the only layer that touches device binding; app never
 * calls device_get_binding().
 */

#include "app/app.h"
#include "board/board.h"
#include "drivers/api/gpio.h"
#include "service/fault/fault.h"

#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(devices, LOG_LEVEL_INF);

/*
 * The 11 required driver classes (#5). device_get_binding() returns NULL for a
 * device that is not found or not ready, so a NULL here means "not ready" and
 * sets fault word bit0.
 */
static const char *const required_devices[] = {
	"clock_control", "gpio",   "dac",  "comparator", "hrtim", "dma",
	"adc",           "uart",   "counter", "flash",    "watchdog",
};

static int devices_compose(void)
{
	struct app_config cfg;
	const struct device *gpio;
	const struct device *watchdog;
	uint32_t i;

	/* Final readiness check (#5 §6). Boot continues on failure; bit0 is
	 * cleared only by a later successful self-test (diag). */
	for (i = 0U; i < ARRAY_SIZE(required_devices); i++) {
		if (device_get_binding(required_devices[i]) == NULL) {
			LOG_ERR("required device not ready: %s",
				required_devices[i]);
			fault_set(BIT(FAULT_BIT_NOT_READY));
		}
	}

	gpio = device_get_binding("gpio");
	watchdog = device_get_binding("watchdog");

	cfg.heartbeat.gpio = gpio;
	cfg.heartbeat.led_pin = BOARD_LED_PIN;
	cfg.heartbeat.half_period_ms = 500U;
	cfg.watchdog = watchdog;

	app_init(&cfg);
	LOG_INF("composition root ready");
	return 0;
}

SYS_INIT(devices_compose, APPLICATION, 90);
