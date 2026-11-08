/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 *
 * Minimal portable closure of Zephyr's preprocessor utility macros.
 * Derived from Zephyr v4.5.0: include/zephyr/sys/util_macro.h and
 * include/zephyr/sys/util_internal.h.
 */

#ifndef ZEPHYR_INCLUDE_SYS_UTIL_MACRO_H_
#define ZEPHYR_INCLUDE_SYS_UTIL_MACRO_H_

#include <zephyr/toolchain.h>

/*
 * IS_ENABLED(CONFIG_X): expands to 1 when the flag expands to the integer
 * literal 1, otherwise 0 (flags that are undefined or 0 count as disabled).
 */
#define Z_IS_ENABLED1(config_macro) Z_IS_ENABLED2(_XXXX##config_macro)
#define _XXXX1 _YYYY,
#define Z_IS_ENABLED2(one_or_two_args) Z_IS_ENABLED3(one_or_two_args 1, 0)
#define Z_IS_ENABLED3(ignore_this, val, ...) val
#define IS_ENABLED(config_macro) Z_IS_ENABLED1(config_macro)

/*
 * COND_CODE_1(flag, if_1, else): expands to if_1 when flag is the integer
 * literal 1, else otherwise. Both branches must be wrapped in parentheses.
 */
#define Z_COND_CODE_1(_flag, _if_1_code, _else_code)                           \
	__COND_CODE(_XXXX##_flag, _if_1_code, _else_code)
#define __COND_CODE(one_or_two_args, _if_code, _else_code)                     \
	__GET_ARG2_DEBRACKET(one_or_two_args _if_code, _else_code)
#define __GET_ARG2_DEBRACKET(ignore_this, val, ...) __DEBRACKET val
#define __DEBRACKET(...) __VA_ARGS__
#define COND_CODE_1(_flag, _if_1_code, _else_code)                             \
	Z_COND_CODE_1(_flag, _if_1_code, _else_code)

/* COND_CODE_0(flag, if_0, else): like COND_CODE_1 but selects on flag == 0. */
#define Z_COND_CODE_0(_flag, _if_0_code, _else_code)                           \
	__COND_CODE(_ZZZZ##_flag, _if_0_code, _else_code)
#define _ZZZZ0 _YYYY,
#define COND_CODE_0(_flag, _if_0_code, _else_code)                             \
	Z_COND_CODE_0(_flag, _if_0_code, _else_code)

#define IF_ENABLED(_flag, _code) COND_CODE_1(_flag, (_code), ())
#define IF_DISABLED(_flag, _code) COND_CODE_1(_flag, (), (_code))

#endif /* ZEPHYR_INCLUDE_SYS_UTIL_MACRO_H_ */
