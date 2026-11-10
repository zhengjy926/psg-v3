#!/usr/bin/env python3
"""Verify that a linked firmware image orders its init-entry sections by
(init level, priority, sub-priority).

Parses the armlink `--map` output produced by tools/build_target.ps1 and checks
that every `.z_init_<LEVEL>_P_<prio>_SUB_<sub>_` section appears at a strictly
non-decreasing address in strictly non-decreasing (level, prio, sub) order.
This is the build-time proof of acceptance criterion #1 (the scatter sorts the
init sections by level+priority).

Usage: python tools/verify_init_order.py <path-to-map.txt>
"""

import re
import sys

LEVEL_ORD = {
    "EARLY": 0,
    "PRE_KERNEL_1": 1,
    "PRE_KERNEL_2": 2,
    "POST_KERNEL": 3,
    "APPLICATION": 4,
}

SECTION_RE = re.compile(
    r"^\s*0x([0-9a-fA-F]+)\s+.*\.z_init_([A-Z0-9_]+)_P_(\d+)_SUB_(\d+)_\s"
)


def parse_map(path):
    entries = []
    with open(path, "r", encoding="ascii", errors="ignore") as fh:
        for line in fh:
            m = SECTION_RE.match(line)
            if not m:
                continue
            addr = int(m.group(1), 16)
            level = m.group(2)
            prio = int(m.group(3))
            sub = int(m.group(4))
            if level not in LEVEL_ORD:
                print(f"unknown init level: {level}", file=sys.stderr)
                return 1
            entries.append((addr, LEVEL_ORD[level], prio, sub, line.strip()))
    return entries


def main():
    if len(sys.argv) != 2:
        print(__doc__, file=sys.stderr)
        return 2
    entries = parse_map(sys.argv[1])
    if entries == 1:
        return 1
    if not entries:
        print("ERROR: no .z_init_* sections found in map", file=sys.stderr)
        return 1

    prev = None
    for addr, level, prio, sub, raw in entries:
        key = (level, prio, sub)
        if prev is not None:
            if addr <= prev[0]:
                print(f"ERROR: address not increasing:\n  {prev[2]}\n  {raw}",
                      file=sys.stderr)
                return 1
            if key < prev[1]:
                print(f"ERROR: init order violated (level/prio/sub):\n"
                      f"  {prev[2]}\n  {raw}", file=sys.stderr)
                return 1
        prev = (addr, key, raw)

    print(f"OK: {len(entries)} init entries in level+priority order")
    for _, _, _, _, raw in entries:
        print(f"  {raw}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
