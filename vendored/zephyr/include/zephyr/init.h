/*
 * Copyright (c) 2015 Intel Corporation.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Minimal portable closure of Zephyr's system-initialization infrastructure.
 * Derived from Zephyr v4.5.0: include/zephyr/init.h
 *
 * Difference from upstream: init-entry section names carry a fixed-width
 * (3-digit) priority and sub-priority (`.z_init_<LEVEL>_P_<prio>_SUB_<sub>_`).
 * armlink sorts input sections lexicographically by section name but has no
 * GNU-ld `SORT()` equivalent, so zero-padding makes lexical order equal
 * numeric order; see the scatter file and docs/research ticket #2.
 */

#ifndef ZEPHYR_INCLUDE_INIT_H_
#define ZEPHYR_INCLUDE_INIT_H_

#include <stdint.h>
#include <stddef.h>

#include <zephyr/sys/util.h>
#include <zephyr/sys/init_prio_str.h>
#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

struct device;

/**
 * Initialization entry. SYS_INIT entries carry init_fn and leave dev NULL;
 * device entries carry dev and leave init_fn NULL.
 */
struct init_entry {
	int (*init_fn)(void);
	const struct device *dev;
};

/* Init level ordinals (must stay in execution order). */
#define Z_INIT_ORD_EARLY         0
#define Z_INIT_ORD_PRE_KERNEL_1  1
#define Z_INIT_ORD_PRE_KERNEL_2  2
#define Z_INIT_ORD_POST_KERNEL   3
#define Z_INIT_ORD_APPLICATION   4

#define INIT_LEVEL_ORD(level) Z_INIT_ORD_##level

/* Name of the init-entry object for a given id. */
#define Z_INIT_ENTRY_NAME(init_id) _CONCAT(__init_, init_id)

/*
 * Section an init entry is placed into. Zero-padded priority/sub-priority
 * keeps lexical section-name order equal to numeric order.
 */
#define Z_INIT_ENTRY_SECTION(level, prio, sub_prio)                             \
	__attribute__((__section__(".z_init_" #level "_P_" Z_PRIO_STR(prio)     \
				   "_SUB_" Z_PRIO_STR(sub_prio) "_")))

/**
 * Register an initialization function. Called at boot according to the given
 * level and priority. The return value is ignored.
 *
 * @param init_fn Initialization function (int fn(void)).
 * @param level   Initialization level (EARLY, PRE_KERNEL_1, PRE_KERNEL_2,
 *                POST_KERNEL, APPLICATION).
 * @param prio    Priority within the level, 0..999 (literal integer).
 */
#define SYS_INIT(init_fn, level, prio) SYS_INIT_NAMED(init_fn, init_fn, level, prio)

/**
 * Named variant of SYS_INIT, for when the same function is used by several
 * init entries (or when a stable symbol name is required).
 */
#define SYS_INIT_NAMED(name, init_fn_, level, prio)                             \
	static const Z_DECL_ALIGN(struct init_entry)                           \
		Z_INIT_ENTRY_SECTION(level, prio, 0) __used __noasan           \
		Z_INIT_ENTRY_NAME(name) = {.init_fn = (init_fn_), .dev = NULL}

/* --- bare-metal boot driver (implemented in src/init.c) --- */

/** Run every init entry of one level (level = Z_INIT_ORD_<LEVEL>). */
void z_sys_init_run_level(uint32_t level);

/** Run all five init levels in order (EARLY .. APPLICATION). */
void z_sys_init_run_all(void);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_INIT_H_ */
