/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * board — board init (target). Configures the LED/run pins, starts the 1 ms
 * SysTick, and provides the console log backend hook.
 */

#include "board/board.h"
#include "board/stm32g474/stm32g474.h"
#include "drivers/api/gpio.h"
#include "osal/osal.h"

#include <stddef.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_backend.h>

LOG_MODULE_REGISTER(board, LOG_LEVEL_INF);

/* --- SysTick (1 ms) --- */

void SysTick_Handler(void)
{
	osal_tick();
}

static void sys_tick_start(void)
{
	SysTick->LOAD = (SystemCoreClock / 1000U) - 1U;
	SysTick->VAL = 0U;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk |
			SysTick_CTRL_ENABLE_Msk;
}

/* --- console log backend (SEGGER RTT hook point) --- */

/*
 * The firmware routes log output to SEGGER RTT (N-006). The actual RTT write
 * lands in a later ticket; this weak hook keeps the skeleton linkable and lets
 * the RTT backend replace it without touching the logging core.
 */
__attribute__((weak)) void z_log_hw_write(const char *data, size_t length)
{
	(void)data;
	(void)length;
}

static void console_output(const char *data, size_t length)
{
	z_log_hw_write(data, length);
}

static void console_init(void)
{
}

static const struct log_backend_api console_api = {
	.init = console_init,
	.output = console_output,
};

LOG_BACKEND_DEFINE(console, &console_api);

/* --- board init --- */

static int board_init(void)
{
	const struct device *gpio = device_get_binding("gpio");

	if (gpio != NULL) {
		(void)gpio_configure(gpio, BOARD_LED_PIN, GPIO_OUTPUT);
		(void)gpio_configure(gpio, BOARD_RUN_PIN, GPIO_OUTPUT);
		(void)gpio_set(gpio, BOARD_LED_PIN, GPIO_HIGH);
		(void)gpio_set(gpio, BOARD_RUN_PIN, GPIO_LOW);
	}
	sys_tick_start();
	LOG_INF("board ready");
	return 0;
}

SYS_INIT(board_init, POST_KERNEL, 10);
