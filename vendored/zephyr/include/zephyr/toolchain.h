/*
 * Copyright (c) 2015 Intel Corporation.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Minimal portable closure of the Zephyr toolchain abstraction layer.
 * Derived from Zephyr v4.5.0:
 *   include/zephyr/toolchain.h
 *   include/zephyr/toolchain/common.h
 *   include/zephyr/toolchain/gcc.h
 *
 * Only the GNU attribute surface needed by the device model, init and
 * logging frameworks is kept. armclang (AC6) reaches these definitions
 * through its clang -> GNU attribute mapping, so no armcc5 support exists.
 */

#ifndef ZEPHYR_INCLUDE_TOOLCHAIN_H_
#define ZEPHYR_INCLUDE_TOOLCHAIN_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Stringification with argument expansion. */
#define Z_STRINGIFY(x) #x
#define STRINGIFY(s) Z_STRINGIFY(s)

/* Token concatenation with argument expansion. */
#define _DO_CONCAT(x, y) x##y
#define _CONCAT(x, y) _DO_CONCAT(x, y)

/* --- GNU attribute surface (also valid under armclang AC6) --- */

#define __aligned(x)   __attribute__((__aligned__(x)))
#define __used         __attribute__((__used__))
#define __unused       __attribute__((__unused__))
#define __packed       __attribute__((__packed__))
#define __weak         __attribute__((__weak__))
#define __noinline     __attribute__((noinline))
#define ALWAYS_INLINE  inline __attribute__((always_inline))
#define __deprecated   __attribute__((deprecated))

/*
 * Keep scattered-structure instances out of AddressSanitizer global
 * instrumentation when it is enabled (GCC/clang). cppcheck does not define
 * __SANITIZE_ADDRESS__, so it treats __noasan as empty.
 */
#if defined(__SANITIZE_ADDRESS__)
#define __noasan __attribute__((no_sanitize_address))
#else
#define __noasan
#endif

/*
 * Place an object into the section ".<a>.<b>.<c>". Used by the iterable
 * sections macros to build ".<secname>.static.<postfix>" section names.
 */
#define ___in_section(a, b, c)                                                  \
	__attribute__((section("." Z_STRINGIFY(a) "." Z_STRINGIFY(b)            \
				       "." Z_STRINGIFY(c))))
#define __in_section(a, b, c) ___in_section(a, b, c)

/* Place an object into a fully-qualified section name string. */
#define __section(sec) __attribute__((section(STRINGIFY(sec))))

/*
 * Declare an object aligned to its type's natural alignment. The linker pads
 * scattered section segments to this alignment so the array walk is safe.
 */
#define Z_DECL_ALIGN(type) __aligned(__alignof(type)) type

/* Compile-time assertion (message swallowed by the common implementation). */
#ifndef BUILD_ASSERT
#define BUILD_ASSERT(EXPR, MSG...)                                             \
	enum _CONCAT(__build_assert_enum_, __COUNTER__) {                       \
		_CONCAT(__build_assert_, __COUNTER__) = 1 / !!(EXPR)            \
	}
#endif

#define likely(x)   (__builtin_expect(!!(x), 1))
#define unlikely(x) (__builtin_expect(!!(x), 0))

#define ARG_UNUSED(x) (void)(x)

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_TOOLCHAIN_H_ */
