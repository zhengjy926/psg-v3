/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Runner for the init entries retained by this build. Iterates the
 * build-generated table sorted by (init level, priority, sub-priority), so the
 * init order no longer depends on the linker sorting section names.
 *
 * Each build produces its own table: the armclang target uses init_order.inc,
 * the GNU host build uses init_order_host.inc. Both are emitted by
 * tools/generate_init_order.py and carry the init level ordinal plus the entry
 * pointer, so z_sys_init_run_level() can run a single level in isolation.
 */

#include <device.h>
#include <init.h>

struct z_init_entry {
	uint8_t level;
	const struct init_entry *entry;
};

#if defined(__ARMCC_VERSION)
#include "init_order.inc"
#else
#include "init_order_host.inc"
#endif

static void run_entry(const struct init_entry *entry)
{
	if (entry->init_fn != NULL) {
		(void)entry->init_fn();
	} else if (entry->dev != NULL) {
		(void)device_init(entry->dev);
	}
}

void z_sys_init_run_all(void)
{
	for (size_t i = 0; i < Z_INIT_ORDER_LEN; i++) {
		run_entry(z_init_order[i].entry);
	}
}

void z_sys_init_run_level(uint8_t level)
{
	for (size_t i = 0; i < Z_INIT_ORDER_LEN; i++) {
		if (z_init_order[i].level == level) {
			run_entry(z_init_order[i].entry);
		}
	}
}
