/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * uart driver (target, skeleton). Registers the protocol UART at PRE_KERNEL_2
 * prio 40. The port is chosen at compile time (#15: release = USART2, debug =
 * USART1); only the selected port builds its stack. Baud/pin configuration and
 * the poll_in/poll_out ops land in the ecsp transport ticket; here init only
 * gates the selected peripheral clock.
 */

#include "board/board.h"
#include "drivers/api/clock_control.h"

#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>

LOG_MODULE_REGISTER(uart_stm32, LOG_LEVEL_INF);

static int uart_stm32_init(const struct device *dev)
{
	const struct device *clock;
	const struct device *gpio;
	uint32_t subsys;

	ARG_UNUSED(dev);
	clock = device_get_binding("clock_control");
	gpio = device_get_binding("gpio");
	__ASSERT(clock != NULL, "clock_control device not ready");
	__ASSERT(gpio != NULL, "gpio device not ready");
	if ((clock == NULL) || (gpio == NULL)) {
		return -1;
	}

#if PSG_PROTO_UART == 1
	subsys = PSG_CLOCK_USART1;
#elif PSG_PROTO_UART == 2
	subsys = PSG_CLOCK_USART2;
#else
#error "PSG_PROTO_UART must be 1 (USART1) or 2 (USART2)"
#endif
	(void)clock_control_on(clock, subsys);
	LOG_INF("uart ready (USART%d)", PSG_PROTO_UART);
	return 0;
}

DEVICE_DEFINE(uart, "uart", uart_stm32_init, NULL, NULL, NULL, PRE_KERNEL_2, 40,
	      NULL);
