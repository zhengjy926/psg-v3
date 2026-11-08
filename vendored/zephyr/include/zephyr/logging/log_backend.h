/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 *
 * Minimal portable closure of Zephyr's log backend registration.
 * Derived from Zephyr v4.5.0: include/zephyr/logging/log_backend.h
 *
 * Message packaging follows the MINIMAL path: the core formats each message
 * with vsnprintf() and hands the resulting bytes to every registered backend.
 */

#ifndef ZEPHYR_INCLUDE_LOGGING_LOG_BACKEND_H_
#define ZEPHYR_INCLUDE_LOGGING_LOG_BACKEND_H_

#include <stddef.h>
#include <stdint.h>

#include <zephyr/sys/iterable_sections.h>
#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Backend operations. */
struct log_backend_api {
	/** One-time backend setup, called by log_core_init(). */
	void (*init)(void);
	/** Emit a formatted message fragment. */
	void (*output)(const char *data, size_t length);
};

/** Backend control/description block, gathered in the log_backend section. */
struct log_backend {
	const char *name;
	const struct log_backend_api *api;
};

/** Register a log backend instance. */
#define LOG_BACKEND_DEFINE(_name, _api)                                        \
	const STRUCT_SECTION_ITERABLE_ALTERNATE(log_backend, log_backend,     \
						_name) = {                     \
		.name = STRINGIFY(_name), .api = (_api),                       \
	}

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_LOGGING_LOG_BACKEND_H_ */
