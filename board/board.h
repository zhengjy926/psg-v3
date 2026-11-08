/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * board — board-level constants and init. Target-only: may include STM32
 * headers. Pin assignments follow docs/MCU引脚功能映射表.md.
 */

#ifndef PSG_BOARD_BOARD_H_
#define PSG_BOARD_BOARD_H_

#include <stdint.h>

/* On-board LED (PB9), active-low (F-015). */
#define BOARD_LED_PIN 9U
/* Run indicator (PB5), high while running (F-014). */
#define BOARD_RUN_PIN 5U

/*
 * Protocol UART (compile-time selection, #15): 1 = USART1 (single-board debug),
 * 2 = USART2 (release). Only the selected port builds its protocol stack.
 */
#ifndef PSG_PROTO_UART
#define PSG_PROTO_UART 2
#endif

#endif /* PSG_BOARD_BOARD_H_ */
