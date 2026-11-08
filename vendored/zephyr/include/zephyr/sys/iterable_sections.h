/*
 * Copyright (C) 2020, Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 *
 * Minimal portable closure of Zephyr's iterable sections helpers.
 * Derived from Zephyr v4.5.0: include/zephyr/sys/iterable_sections.h
 *
 * An "iterable section" gathers same-shaped objects from across translation
 * units into one linker section; the linker emits `_<secname>_list_start` and
 * `_<secname>_list_end` symbols around them so the code can walk the result as
 * a contiguous array. This is the mechanism behind both the init-entry tables
 * and the device-object registry.
 */

#ifndef ZEPHYR_INCLUDE_SYS_ITERABLE_SECTIONS_H_
#define ZEPHYR_INCLUDE_SYS_ITERABLE_SECTIONS_H_

#include <zephyr/sys/__assert.h>
#include <zephyr/sys/util.h>
#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Define one element of an iterable section. The section name is
 * ".<secname>.static.<postfix>"; the linker script/scatter file collects
 * everything matching ".<secname>.*" between the list_start/list_end symbols.
 */
#define TYPE_SECTION_ITERABLE(type, varname, secname, section_postfix)          \
	Z_DECL_ALIGN(type) varname                                              \
	__in_section(_##secname, static, _CONCAT(section_postfix, _)) __used   \
	__noasan

#define TYPE_SECTION_START(secname) _CONCAT(_##secname, _list_start)
#define TYPE_SECTION_END(secname) _CONCAT(_##secname, _list_end)

#define TYPE_SECTION_START_EXTERN(type, secname)                               \
	extern type TYPE_SECTION_START(secname)[]
#define TYPE_SECTION_END_EXTERN(type, secname)                                 \
	extern type TYPE_SECTION_END(secname)[]

/* Iterate a generic iterable section. */
#define TYPE_SECTION_FOREACH(type, secname, iterator)                          \
	TYPE_SECTION_START_EXTERN(type, secname);                              \
	TYPE_SECTION_END_EXTERN(type, secname);                                \
	for (type *iterator = TYPE_SECTION_START(secname);                     \
	     ({                                                                 \
		     __ASSERT(iterator <= TYPE_SECTION_END(secname),           \
			      "unexpected list end location");                \
		     iterator < TYPE_SECTION_END(secname);                     \
	     });                                                               \
	     iterator++)

/* Get an element by index; no bounds protection. */
#define TYPE_SECTION_GET(type, secname, i, dst)                                \
	do {                                                                   \
		TYPE_SECTION_START_EXTERN(type, secname);                      \
		*(dst) = &TYPE_SECTION_START(secname)[i];                      \
	} while (0)

/* Count the elements gathered in a section. */
#define TYPE_SECTION_COUNT(type, secname, dst)                                 \
	do {                                                                   \
		TYPE_SECTION_START_EXTERN(type, secname);                      \
		TYPE_SECTION_END_EXTERN(type, secname);                        \
		*(dst) = ((uintptr_t)TYPE_SECTION_END(secname) -               \
			  (uintptr_t)TYPE_SECTION_START(secname)) /            \
			 sizeof(type);                                         \
	} while (0)

/* --- struct-typed convenience wrappers --- */

#define STRUCT_SECTION_START(struct_type) TYPE_SECTION_START(struct_type)
#define STRUCT_SECTION_END(struct_type) TYPE_SECTION_END(struct_type)
#define STRUCT_SECTION_START_EXTERN(struct_type)                               \
	TYPE_SECTION_START_EXTERN(struct struct_type, struct_type)
#define STRUCT_SECTION_END_EXTERN(struct_type)                                 \
	TYPE_SECTION_END_EXTERN(struct struct_type, struct_type)

#define STRUCT_SECTION_ITERABLE_ALTERNATE(secname, struct_type, varname)       \
	TYPE_SECTION_ITERABLE(struct struct_type, varname, secname, varname)

#define STRUCT_SECTION_ITERABLE(struct_type, varname)                          \
	STRUCT_SECTION_ITERABLE_ALTERNATE(struct_type, struct_type, varname)

#define STRUCT_SECTION_ITERABLE_NAMED(struct_type, name, varname)              \
	TYPE_SECTION_ITERABLE(struct struct_type, varname, struct_type, name)

#define STRUCT_SECTION_ITERABLE_NAMED_ALTERNATE(struct_type, secname, name,    \
						varname)                       \
	TYPE_SECTION_ITERABLE(struct struct_type, varname, secname, name)

#define STRUCT_SECTION_FOREACH_ALTERNATE(secname, struct_type, iterator)       \
	TYPE_SECTION_FOREACH(struct struct_type, secname, iterator)

#define STRUCT_SECTION_FOREACH(struct_type, iterator)                          \
	STRUCT_SECTION_FOREACH_ALTERNATE(struct_type, struct_type, iterator)

#define STRUCT_SECTION_GET(struct_type, i, dst)                                \
	TYPE_SECTION_GET(struct struct_type, struct_type, i, dst)

#define STRUCT_SECTION_COUNT(struct_type, dst)                                 \
	TYPE_SECTION_COUNT(struct struct_type, struct_type, dst)

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_SYS_ITERABLE_SECTIONS_H_ */
