/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * board/devices.c — composition root (target). Binds the concrete devices to
 * service configuration and hands the result to app_init(). This is the only
 * layer that touches device binding; app never calls device_get_binding().
 */

#include "app/app.h"
#include "board/board.h"
#include "drivers/api/gpio.h"

#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(devices, LOG_LEVEL_INF);

static int devices_compose(void)
{
	struct app_config cfg;
	const struct device *gpio = device_get_binding("gpio");

	cfg.heartbeat.gpio = gpio;
	cfg.heartbeat.led_pin = BOARD_LED_PIN;
	cfg.heartbeat.half_period_ms = 500U;

	app_init(&cfg);
	LOG_INF("composition root ready");
	return 0;
}

SYS_INIT(devices_compose, APPLICATION, 90);
