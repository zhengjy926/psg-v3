/**
 *******************************************************************************
 * @copyright : Copyright To Hangzhou Dinova EP Technology Co.,Ltd
 * @file      : app.c
 * @author    : ZJY
 * @version   : V1.0
 * @date      : 2026-11-06
 * @brief     :
 * @attention : None
 *******************************************************************************
 * @history   :
 *       V1.0 : 初始版本
 *******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "app.h"
#include "../board/board.h"

#include <device.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* Private user code ---------------------------------------------------------*/
int main(void)
{
	const struct device *led = device_get_binding(BOARD_LED_DEVICE_NAME);
	const struct app_config cfg = {
		.heartbeat = {
			.led = led,
			.led_index = 0U,
			.half_period_ms = 500U,
		},
	};
	int err;

	if (led == NULL) {
		LOG_ERR("LED device '%s' is not ready", BOARD_LED_DEVICE_NAME);
		for (;;) {
		}
	}

	err = app_init(&cfg);
	if (err != 0) {
		LOG_ERR("Application initialization failed (%d)", err);
		for (;;) {
		}
	}

	for (;;) {
		app_step();
	}
}