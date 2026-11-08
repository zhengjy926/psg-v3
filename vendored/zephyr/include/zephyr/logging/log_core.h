/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 *
 * Minimal portable closure of Zephyr's log core.
 * Derived from Zephyr v4.5.0: include/zephyr/logging/log_core.h
 */

#ifndef ZEPHYR_INCLUDE_LOGGING_LOG_CORE_H_
#define ZEPHYR_INCLUDE_LOGGING_LOG_CORE_H_

#include <stdint.h>

#include <zephyr/autoconf.h>
#include <zephyr/logging/log_instance.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Log levels (numeric severity, lower = more severe). */
#define LOG_LEVEL_NONE 0U
#define LOG_LEVEL_ERR  1U
#define LOG_LEVEL_WRN  2U
#define LOG_LEVEL_INF  3U
#define LOG_LEVEL_DBG  4U

/**
 * Format a message (MINIMAL path: vsnprintf) and emit it through every
 * registered backend. Callers should gate this behind Z_LOG_LEVEL_CHECK() so
 * disabled levels are removed at compile time.
 */
void z_log_printf(const struct log_source_const_data *src, uint32_t level,
		  const char *fmt, ...);

/** Initialize the logging subsystem (iterates and inits registered backends). */
void log_core_init(void);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_LOGGING_LOG_CORE_H_ */
