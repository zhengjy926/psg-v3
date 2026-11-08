/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Firmware entry point (target). Reached after z_sys_init_run_all() has run
 * every init level; here the main loop advances the application. The watchdog
 * feed lands at the end of the loop in a later ticket.
 */

#include "app/app.h"

int main(void)
{
	for (;;) {
		app_step();
	}
}
