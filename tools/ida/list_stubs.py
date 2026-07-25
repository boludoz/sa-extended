"""
Inventory every function that still redirects into the original binary.

    python tools/ida/list_stubs.py [--csv out.csv] [--class CRoadBlocks] [--top 30]

Two things are counted, and they do not fully overlap:

  * `plugin::Call<0x...>` / `CallMethod` / `CallAndReturn` bodies in source/ --
    the actual hex redirects, i.e. functions with no C++ implementation at all.
  * rows in docs/hooks.csv with `reversed=0` -- the project's own bookkeeping,
    which also covers functions whose body exists but was never verified.

Without --csv it prints a per-class summary, biggest first: that is the list to
pick the next batch of work from.
"""

import argparse
import csv
import os
import re
import sys
from collections import Counter, defaultdict

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SOURCE_DIR = os.path.join(ROOT, "source")
HOOKS_CSV = os.path.join(ROOT, "docs", "hooks.csv")

# `plugin::Call<0x461100>(...)`, `plugin::CallAndReturn<bool, 0x460EE0, ...>(...)`,
# `plugin::CallMethod<0x123456, CFoo*>(...)` -- the address is the first hex literal.
CALL_RE = re.compile(r"plugin::Call[A-Za-z]*<\s*(?:[^<>]*?,\s*)?(0x[0-9A-Fa-f]+)")

# The enclosing function signature, taken from the nearest preceding line that
# looks like a definition. Good enough to label a stub; not a C++ parser.
SIGNATURE_RE = re.compile(r"^[A-Za-z_].*\b(\w+)::(~?\w+)\s*\(")


def scan_sources():
    """Yield (address, class_name, func_name, relpath, line_no) per redirect."""
    for dirpath, _, filenames in os.walk(SOURCE_DIR):
        for fn in filenames:
            if not fn.endswith((".cpp", ".hpp", ".h")):
                continue
            path = os.path.join(dirpath, fn)
            rel = os.path.relpath(path, ROOT).replace("\\", "/")
            with open(path, "r", encoding="utf-8", errors="replace") as fh:
                lines = fh.readlines()
            for i, line in enumerate(lines):
                m = CALL_RE.search(line)
                if not m:
                    continue
                cls, func = "?", "?"
                for j in range(i, max(-1, i - 40), -1):
                    sm = SIGNATURE_RE.match(lines[j])
                    if sm:
                        cls, func = sm.group(1), sm.group(2)
                        break
                yield int(m.group(1), 16), cls, func, rel, i + 1


def read_hooks():
    """Rows of docs/hooks.csv that are still flagged unreversed."""
    if not os.path.exists(HOOKS_CSV):
        return []
    with open(HOOKS_CSV, "r", encoding="utf-8", newline="") as fh:
        return [r for r in csv.DictReader(fh) if r.get("reversed") == "0"]


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--csv", help="write the full stub list here instead of a summary")
    ap.add_argument("--class", dest="cls", help="only report this class")
    ap.add_argument("--top", type=int, default=30, help="how many classes to show (default 30)")
    args = ap.parse_args()

    stubs = sorted(scan_sources())
    if args.cls:
        stubs = [s for s in stubs if s[1] == args.cls]

    unreversed = read_hooks()
    unrev_by_class = Counter(r["class"] for r in unreversed)

    if args.csv:
        with open(args.csv, "w", encoding="utf-8", newline="") as fh:
            w = csv.writer(fh)
            w.writerow(["address", "class", "function", "file", "line"])
            for addr, cls, func, rel, ln in stubs:
                w.writerow(["0x%06X" % addr, cls, func, rel, ln])
        print("wrote %d hex redirects to %s" % (len(stubs), args.csv))
        return

    by_class = defaultdict(list)
    for addr, cls, func, rel, ln in stubs:
        by_class[cls].append((addr, func, rel, ln))

    print("hex redirects in source/ : %d across %d classes" % (len(stubs), len(by_class)))
    print("hooks.csv reversed=0     : %d\n" % len(unreversed))
    print("%-34s %8s %10s" % ("class", "redirects", "unreversed"))
    print("-" * 54)
    for cls, entries in sorted(by_class.items(), key=lambda kv: -len(kv[1]))[: args.top]:
        print("%-34s %8d %10d" % (cls, len(entries), unrev_by_class.get(cls, 0)))

    if args.cls:
        print("\n%s:" % args.cls)
        for addr, func, rel, ln in by_class.get(args.cls, []):
            print("  0x%06X  %-40s %s:%d" % (addr, func, rel, ln))


if __name__ == "__main__":
    sys.exit(main())
