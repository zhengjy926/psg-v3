/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 *
 * Minimal portable closure of Zephyr's logging API.
 * Derived from Zephyr v4.5.0: include/zephyr/logging/log.h
 */

#ifndef ZEPHYR_INCLUDE_LOGGING_LOG_H_
#define ZEPHYR_INCLUDE_LOGGING_LOG_H_

#include <stdint.h>

#include <zephyr/logging/log_backend.h>
#include <zephyr/logging/log_core.h>
#include <zephyr/logging/log_instance.h>
#include <zephyr/sys/iterable_sections.h>
#include <zephyr/sys/util_macro.h>
#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Register a logging module. Places its constant descriptor in the log_const
 * iterable section and exposes a per-file `__log_level` used for compile-time
 * level gating.
 *
 * @param name  Module name token (used verbatim as its display name).
 * @param level Compile-time default level (LOG_LEVEL_ERR..LOG_LEVEL_DBG).
 */
#define LOG_MODULE_REGISTER(_name, _level)                                     \
	static const uint8_t __log_level __unused = (_level);                            \
	static const STRUCT_SECTION_ITERABLE_ALTERNATE(log_const,              \
		log_source_const_data, _CONCAT(_name, _log_source)) =            \
		{.name = STRINGIFY(_name), .level = (_level)};                  \
	static const struct log_source_const_data *const __log_source __unused =        \
		&_CONCAT(_name, _log_source)

/* Emit a message if the level passes the compile-time gate. */
#define Z_LOG(level, fmt, ...)                                                 \
	do {                                                                   \
		if (IS_ENABLED(CONFIG_LOG) &&                                  \
		    ((uint32_t)(level) <= (uint32_t)(__log_level)) &&           \
		    ((uint32_t)(level) <= (uint32_t)CONFIG_LOG_MAX_LEVEL)) {   \
			z_log_printf(__log_source, (uint32_t)(level), fmt,     \
				     ##__VA_ARGS__);                           \
		}                                                              \
	} while (0)

#define LOG_ERR(fmt, ...) Z_LOG(LOG_LEVEL_ERR, fmt, ##__VA_ARGS__)
#define LOG_WRN(fmt, ...) Z_LOG(LOG_LEVEL_WRN, fmt, ##__VA_ARGS__)
#define LOG_INF(fmt, ...) Z_LOG(LOG_LEVEL_INF, fmt, ##__VA_ARGS__)
#define LOG_DBG(fmt, ...) Z_LOG(LOG_LEVEL_DBG, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_LOGGING_LOG_H_ */
