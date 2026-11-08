/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Hand-written configuration header. In upstream Zephyr this file is generated
 * from Kconfig; the bare-metal port fixes the relevant switches here. These
 * match the "software device + minimal logging" closure described in
 * docs/research ticket #2.
 */

#ifndef ZEPHYR_INCLUDE_AUTOCONF_H_
#define ZEPHYR_INCLUDE_AUTOCONF_H_

/* Assertions (off in release builds). */
#define CONFIG_ASSERT 1

/* Logging: minimal mode (printk-style, no cbprintf). */
#define CONFIG_LOG 1
#define CONFIG_LOG_MODE_MINIMAL 1
#define CONFIG_LOG_DEFAULT_LEVEL 4 /* DBG */
#define CONFIG_LOG_MAX_LEVEL 4     /* DBG */
#define CONFIG_LOG_RUNTIME_FILTERING 0

/* Device model trims. */
#define CONFIG_DEVICE_DEPS 0
#define CONFIG_DEVICE_DEPS_DYNAMIC 0
#define CONFIG_DEVICE_DEINIT_SUPPORT 0
#define CONFIG_PM_DEVICE 0
#define CONFIG_DEVICE_MUTABLE 0

#endif /* ZEPHYR_INCLUDE_AUTOCONF_H_ */
