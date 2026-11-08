/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Devicetree shim for the software-device route. The ported device model does
 * not consume devicetree; every devicetree node id is DT_INVALID_NODE and all
 * property accessors expand to their "absent" value. Drivers register via
 * DEVICE_DEFINE()/DEVICE_GET() instead of DEVICE_DT_*().
 */

#ifndef ZEPHYR_SHIM_DEVICETREE_H_
#define ZEPHYR_SHIM_DEVICETREE_H_

#define DT_INVALID_NODE 0

#define DT_NODE_EXISTS(node) 0
#define DT_NODE_HAS_STATUS(node, status) 0
#define DT_NODE_HAS_COMPAT_STATUS(node, compat, status) 0

#define DT_PROP(node, prop) 0
#define DT_PROP_OR(node, prop, dflt) (dflt)

#define DT_NODE_FULL_NAME(node) 0
#define DT_LABEL(node) 0

#define DT_DEP_ORD(node) 0
#define DT_DEP_ORD_STR_SORTABLE(node) 0
#define DT_NODE_HASH(node) 0

#define DT_HAS_COMPAT_STATUS_OKAY(compat) 0

#endif /* ZEPHYR_SHIM_DEVICETREE_H_ */
