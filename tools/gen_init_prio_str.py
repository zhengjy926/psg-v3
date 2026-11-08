#!/usr/bin/env python3
"""Regenerate vendored/zephyr/include/zephyr/sys/init_prio_str.h.

That header maps a compile-time priority (0..999) to a fixed-width 3-digit
section-name suffix, so armlink's lexicographic section ordering equals numeric
priority ordering. Run this only when the fixed width or range changes.
"""

import os

HEADER = os.path.join(
    os.path.dirname(__file__),
    "..",
    "vendored",
    "zephyr",
    "include",
    "zephyr",
    "sys",
    "init_prio_str.h",
)

WIDTH = 3
MAX = 10 ** WIDTH - 1


def main():
    lines = [
        "/*",
        " * Auto-generated fixed-width (3-digit) priority string table.",
        " * Regenerate with: tools/gen_init_prio_str.py",
        " * SPDX-License-Identifier: Apache-2.0",
        " */",
        "#ifndef ZEPHYR_INCLUDE_SYS_INIT_PRIO_STR_H_",
        "#define ZEPHYR_INCLUDE_SYS_INIT_PRIO_STR_H_",
        "",
        "#define Z_CAT(a, b) Z_CAT_I(a, b)",
        "#define Z_CAT_I(a, b) a ## b",
        "#define Z_PRIO_STR(prio) Z_CAT(Z_PRIO_STR_, prio)",
        "",
    ]
    for i in range(MAX + 1):
        lines.append(f'#define Z_PRIO_STR_{i} "{i:0{WIDTH}d}"')
    lines += [
        "",
        "#endif /* ZEPHYR_INCLUDE_SYS_INIT_PRIO_STR_H_ */",
    ]
    with open(HEADER, "w", encoding="ascii", newline="\n") as fh:
        fh.write("\n".join(lines) + "\n")
    print(f"wrote {HEADER} ({MAX + 1} entries)")


if __name__ == "__main__":
    main()
