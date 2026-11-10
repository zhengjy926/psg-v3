/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * board — board init (target). Configures the LED/run pins, starts the 1 ms
 * SysTick, and provides the console log backend hook.
 */

#include "board.h"
#include <stm32_ll_rcc.h>
#include <stm32_ll_flash.h>
#include <stm32_ll_utils.h>

#include "gpio.h"
#include "osal.h"

#include <stddef.h>
#include <device.h>
#include <init.h>
#include <logging/log.h>
#include <logging/log_backend.h>

LOG_MODULE_REGISTER(board, LOG_LEVEL_INF);

/* --- SysTick (1 ms) --- */

void SysTick_Handler(void)
{
	osal_tick();
}

void SystemClock_Config(void);

/* --- board init --- */

static int board_init(void)
{
    return 0;
}

SYS_INIT(board_init, POST_KERNEL, 10);
