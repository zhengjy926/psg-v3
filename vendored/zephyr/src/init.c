/*
 * Copyright (c) 2015 Intel Corporation.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Bare-metal boot init: walks the init-entry sections in level+priority order.
 * Derived from Zephyr v4.5.0: kernel/init.c (z_sys_init_run_level).
 *
 * Level boundaries are linker-provided:
 *  - armlink: each level lives in its own execution region; boundaries are the
 *    region's Image$$<region>$$Base / $$Limit symbols.
 *  - GNU ld: the host linker script emits __init_<LEVEL>_start/end symbols
 *    around each level's KEEP(*(SORT(.z_init_<LEVEL>_P_*))).
 */

#include <stddef.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/sys/util.h>

struct z_init_level {
	const struct init_entry *start;
	const struct init_entry *end;
};

#if defined(__ARMCOMPILER_VERSION)

extern char Image$$ER_INIT_EARLY$$Base;
extern char Image$$ER_INIT_EARLY$$Limit;
extern char Image$$ER_INIT_PRE_KERNEL_1$$Base;
extern char Image$$ER_INIT_PRE_KERNEL_1$$Limit;
extern char Image$$ER_INIT_PRE_KERNEL_2$$Base;
extern char Image$$ER_INIT_PRE_KERNEL_2$$Limit;
extern char Image$$ER_INIT_POST_KERNEL$$Base;
extern char Image$$ER_INIT_POST_KERNEL$$Limit;
extern char Image$$ER_INIT_APPLICATION$$Base;
extern char Image$$ER_INIT_APPLICATION$$Limit;

static const struct z_init_level z_init_levels[] = {
	{(const struct init_entry *)&Image$$ER_INIT_EARLY$$Base,
	 (const struct init_entry *)&Image$$ER_INIT_EARLY$$Limit},
	{(const struct init_entry *)&Image$$ER_INIT_PRE_KERNEL_1$$Base,
	 (const struct init_entry *)&Image$$ER_INIT_PRE_KERNEL_1$$Limit},
	{(const struct init_entry *)&Image$$ER_INIT_PRE_KERNEL_2$$Base,
	 (const struct init_entry *)&Image$$ER_INIT_PRE_KERNEL_2$$Limit},
	{(const struct init_entry *)&Image$$ER_INIT_POST_KERNEL$$Base,
	 (const struct init_entry *)&Image$$ER_INIT_POST_KERNEL$$Limit},
	{(const struct init_entry *)&Image$$ER_INIT_APPLICATION$$Base,
	 (const struct init_entry *)&Image$$ER_INIT_APPLICATION$$Limit},
};

#else /* GNU ld */

extern const struct init_entry __init_EARLY_start[];
extern const struct init_entry __init_EARLY_end[];
extern const struct init_entry __init_PRE_KERNEL_1_start[];
extern const struct init_entry __init_PRE_KERNEL_1_end[];
extern const struct init_entry __init_PRE_KERNEL_2_start[];
extern const struct init_entry __init_PRE_KERNEL_2_end[];
extern const struct init_entry __init_POST_KERNEL_start[];
extern const struct init_entry __init_POST_KERNEL_end[];
extern const struct init_entry __init_APPLICATION_start[];
extern const struct init_entry __init_APPLICATION_end[];

static const struct z_init_level z_init_levels[] = {
	{__init_EARLY_start, __init_EARLY_end},
	{__init_PRE_KERNEL_1_start, __init_PRE_KERNEL_1_end},
	{__init_PRE_KERNEL_2_start, __init_PRE_KERNEL_2_end},
	{__init_POST_KERNEL_start, __init_POST_KERNEL_end},
	{__init_APPLICATION_start, __init_APPLICATION_end},
};

#endif /* __ARMCOMPILER_VERSION */

/* Run every init entry of one level. */
void z_sys_init_run_level(uint32_t level)
{
	const struct init_entry *entry;

	if (level >= ARRAY_SIZE(z_init_levels)) {
		return;
	}

	for (entry = z_init_levels[level].start;
	     entry < z_init_levels[level].end; entry++) {
		if (entry->dev != NULL) {
			(void)do_device_init(entry->dev);
		} else if (entry->init_fn != NULL) {
			entry->init_fn();
		} else {
			/* Empty init entry (both fields NULL): nothing to run. */
		}
	}
}

/* Run all five init levels in order. */
void z_sys_init_run_all(void)
{
	uint32_t level;

	for (level = 0U; level < ARRAY_SIZE(z_init_levels); level++) {
		z_sys_init_run_level(level);
	}
}
