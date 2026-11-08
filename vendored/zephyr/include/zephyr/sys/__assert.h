/*
 * Copyright (c) 2014 Wind River Systems, Inc.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Minimal portable closure of Zephyr's assertion helpers.
 * Derived from Zephyr v4.5.0: include/zephyr/sys/__assert.h
 */

#ifndef ZEPHYR_INCLUDE_SYS___ASSERT_H_
#define ZEPHYR_INCLUDE_SYS___ASSERT_H_

#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The assert post-handler is provided by the platform. The host test build
 * provides one that fails the running test; the firmware build provides one
 * that records the fault and halts.
 */
extern void z_assert_post(const char *file, unsigned int line);

#ifdef CONFIG_ASSERT
#define __ASSERT(expr, fmt, ...)                                               \
	do {                                                                   \
		if (!(expr)) {                                                 \
			z_assert_post(__FILE__, __LINE__);                     \
		}                                                              \
	} while (0)
#else
#define __ASSERT(expr, fmt, ...) ((void)0)
#endif /* CONFIG_ASSERT */

#define __ASSERT_NO_MSG(expr) __ASSERT(expr, "")

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_SYS___ASSERT_H_ */
