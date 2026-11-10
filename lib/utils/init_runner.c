/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Minimal runner for the init entries retained by this target.
 */

#include <device.h>
#include <init.h>

#if defined(__ARMCC_VERSION)
extern const struct init_entry init_EARLY_start[]
	__asm("Image$$ER_INIT_EARLY$$Base");
extern const struct init_entry init_EARLY_end[]
	__asm("Image$$ER_INIT_EARLY$$Limit");
extern const struct init_entry init_PRE_KERNEL_1_start[]
	__asm("Image$$ER_INIT_PRE_KERNEL_1$$Base");
extern const struct init_entry init_PRE_KERNEL_1_end[]
	__asm("Image$$ER_INIT_PRE_KERNEL_1$$Limit");
extern const struct init_entry init_PRE_KERNEL_2_start[]
	__asm("Image$$ER_INIT_PRE_KERNEL_2$$Base");
extern const struct init_entry init_PRE_KERNEL_2_end[]
	__asm("Image$$ER_INIT_PRE_KERNEL_2$$Limit");
extern const struct init_entry init_POST_KERNEL_P_1_start[]
	__asm("Image$$ER_INIT_POST_KERNEL_P_1$$Base");
extern const struct init_entry init_POST_KERNEL_P_1_end[]
	__asm("Image$$ER_INIT_POST_KERNEL_P_1$$Limit");
extern const struct init_entry init_POST_KERNEL_P_5_start[]
	__asm("Image$$ER_INIT_POST_KERNEL_P_5$$Base");
extern const struct init_entry init_POST_KERNEL_P_5_end[]
	__asm("Image$$ER_INIT_POST_KERNEL_P_5$$Limit");
extern const struct init_entry init_POST_KERNEL_P_10_start[]
	__asm("Image$$ER_INIT_POST_KERNEL_P_10$$Base");
extern const struct init_entry init_POST_KERNEL_P_10_end[]
	__asm("Image$$ER_INIT_POST_KERNEL_P_10$$Limit");
extern const struct init_entry init_APPLICATION_start[]
	__asm("Image$$ER_INIT_APPLICATION$$Base");
extern const struct init_entry init_APPLICATION_end[]
	__asm("Image$$ER_INIT_APPLICATION$$Limit");
#define INIT_LEVEL_START(level) init_##level##_start
#define INIT_LEVEL_END(level) init_##level##_end
#else
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
#define INIT_LEVEL_START(level) __init_##level##_start
#define INIT_LEVEL_END(level) __init_##level##_end
#endif

static void run_level(const struct init_entry *start,
		      const struct init_entry *end)
{
	for (const struct init_entry *entry = start; entry < end; entry++) {
		if (entry->init_fn != NULL) {
			(void)entry->init_fn();
		} else if (entry->dev != NULL) {
			(void)device_init(entry->dev);
		}
	}
}

void z_sys_init_run_all(void)
{
	run_level(INIT_LEVEL_START(EARLY), INIT_LEVEL_END(EARLY));
	run_level(INIT_LEVEL_START(PRE_KERNEL_1),
		  INIT_LEVEL_END(PRE_KERNEL_1));
	run_level(INIT_LEVEL_START(PRE_KERNEL_2),
		  INIT_LEVEL_END(PRE_KERNEL_2));
#if defined(__ARMCC_VERSION)
	run_level(init_POST_KERNEL_P_1_start, init_POST_KERNEL_P_1_end);
	run_level(init_POST_KERNEL_P_5_start, init_POST_KERNEL_P_5_end);
	run_level(init_POST_KERNEL_P_10_start, init_POST_KERNEL_P_10_end);
#else
	run_level(INIT_LEVEL_START(POST_KERNEL),
		  INIT_LEVEL_END(POST_KERNEL));
#endif
	run_level(INIT_LEVEL_START(APPLICATION),
		  INIT_LEVEL_END(APPLICATION));
}
