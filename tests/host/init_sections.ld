/* SPDX-License-Identifier: Apache-2.0
 *
 * Host linker script fragment (GNU ld). Init entries are gathered into a
 * single .z_init section — their order is driven by the build-generated
 * init_order_host.inc table (consumed by lib/utils/init_runner.c), not by the
 * linker. The device / log_const / log_backend iterable sections keep their
 * list symbols. Inserted after .data via `INSERT AFTER`.
 */

SECTIONS
{
	.z_init : {
		KEEP(*(.z_init_*))
	}
	.z_device : {
		_device_list_start = .;
		KEEP(*(SORT(._device.static.*)))
		_device_list_end = .;
	}
	.z_log_const : {
		_log_const_list_start = .;
		KEEP(*(SORT(._log_const.static.*)))
		_log_const_list_end = .;
	}
	.z_log_backend : {
		_log_backend_list_start = .;
		KEEP(*(SORT(._log_backend.static.*)))
		_log_backend_list_end = .;
	}
}
INSERT AFTER .data;
