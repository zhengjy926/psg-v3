/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 *
 * Minimal portable closure of Zephyr's log instance registration.
 * Derived from Zephyr v4.5.0: include/zephyr/logging/log_instance.h
 */

#ifndef ZEPHYR_INCLUDE_LOGGING_LOG_INSTANCE_H_
#define ZEPHYR_INCLUDE_LOGGING_LOG_INSTANCE_H_

#include <stdint.h>

/** Constant (ROM) per-module logging data, gathered in the log_const section. */
struct log_source_const_data {
	const char *name;
	uint8_t level;
};

#endif /* ZEPHYR_INCLUDE_LOGGING_LOG_INSTANCE_H_ */
