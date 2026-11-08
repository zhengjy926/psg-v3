/* SPDX-License-Identifier: Apache-2.0
 *
 * Host linker script fragment (GNU ld). Mirrors the armlink scatter file's
 * init-section ordering for the Linux/GCC host build: each init level's
 * `.z_init_<LEVEL>_P_*` sections are gathered (SORT by the zero-padded
 * priority) between `__init_<LEVEL>_start/end` symbols, and the device /
 * log_const / log_backend iterable sections get their `_*_list_start/end`
 * symbols. Inserted after .rodata via `INSERT AFTER`.
 */

SECTIONS
{
	.z_init_early : {
		__init_EARLY_start = .;
		KEEP(*(SORT(.z_init_EARLY_P_*)))
		__init_EARLY_end = .;
	}
	.z_init_pre_kernel_1 : {
		__init_PRE_KERNEL_1_start = .;
		KEEP(*(SORT(.z_init_PRE_KERNEL_1_P_*)))
		__init_PRE_KERNEL_1_end = .;
	}
	.z_init_pre_kernel_2 : {
		__init_PRE_KERNEL_2_start = .;
		KEEP(*(SORT(.z_init_PRE_KERNEL_2_P_*)))
		__init_PRE_KERNEL_2_end = .;
	}
	.z_init_post_kernel : {
		__init_POST_KERNEL_start = .;
		KEEP(*(SORT(.z_init_POST_KERNEL_P_*)))
		__init_POST_KERNEL_end = .;
	}
	.z_init_application : {
		__init_APPLICATION_start = .;
		KEEP(*(SORT(.z_init_APPLICATION_P_*)))
		__init_APPLICATION_end = .;
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
