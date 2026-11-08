/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * LLEXT symbol shim. LLEXT is out of scope for the bare-metal port; the export
 * macros expand to nothing.
 */

#ifndef ZEPHYR_SHIM_LLEXT_SYMBOL_H_
#define ZEPHYR_SHIM_LLEXT_SYMBOL_H_

#define EXPORT_SYMBOL(x)
#define EXPORT_SYMBOL_GROUP(...)
#define EXPORT_GROUP_SYMBOL(...)

#endif /* ZEPHYR_SHIM_LLEXT_SYMBOL_H_ */
