/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 *
 * Minimal portable closure of Zephyr's utility helpers.
 * Derived from Zephyr v4.5.0: include/zephyr/sys/util.h
 */

#ifndef ZEPHYR_INCLUDE_SYS_UTIL_H_
#define ZEPHYR_INCLUDE_SYS_UTIL_H_

#include <stddef.h>
#include <stdint.h>
#include <zephyr/toolchain.h>
#include <zephyr/sys/util_macro.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

#define BIT(n) (1UL << (n))
#define BIT64(n) (1ULL << (n))

#define POINTER_TO_UINT(x) ((uintptr_t)(x))
#define UINT_TO_POINTER(x) ((void *)(uintptr_t)(x))

/* Round a value up to the next multiple of a power-of-two alignment. */
#define ROUND_UP(x, align)                                                     \
	(((unsigned long)(x) + ((unsigned long)(align) - 1)) &                  \
	 ~((unsigned long)(align) - 1))
#define ROUND_DOWN(x, align)                                                   \
	((unsigned long)(x) & ~((unsigned long)(align) - 1))

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_SYS_UTIL_H_ */
