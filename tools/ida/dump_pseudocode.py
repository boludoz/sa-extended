"""
Headless Hex-Rays pseudocode dumper for the gta_sa.exe (v1.0 US) IDA database.

Run through idat.exe, never through the GUI:

    RESA_ADDR_FILE=addrs.txt RESA_OUT_DIR=out \
        idat.exe -A -S"tools/ida/dump_pseudocode.py" work.i64

Paths travel in the environment rather than in `-S` arguments: IDA mangles
Windows drive letters when it splits that string (a path starting with the
drive letter arrives corrupted, e.g. `C;C:` + the rest).

RESA_ADDR_FILE holds one address per line (`0x461100`, blank lines and `#`
comments ignored). For every address the script writes `$RESA_OUT_DIR/0x461100.c`
containing the demangled name, the prototype and the decompiled pseudocode.

A line may carry a prototype to force before decompiling:

    0x4629E0 | void __cdecl f()

Do this whenever IDA guessed `__usercall` with an `@<st0>` argument. That guess
makes Hex-Rays give up on the x87 code and emit raw `__asm` blocks instead of
float expressions; supplying the real signature fixes the whole function. Write
the prototype with `f` as the function name -- it is only a placeholder.

A line that is not a hex number is looked up as a function *name* instead:

    Interior_c::FurnishBedroom

That is how you dump from `libGTASA_without_source_lines.so.i64`, the ARM64
Android build: its addresses share nothing with the PC exe, but it kept the
original symbol names. Decompiling the same function from both and reading them
side by side is the most reliable check there is -- see `.claude/instructions.md`.

Prefixing a line with `!` types a function without dumping it:

    !0x6B0A90 | CAutomobile *__cdecl f(void *self, int model, unsigned char, bool)

Use that on callees whose return type IDA got wrong. A constructor typed as
returning `void *` leaves every field access in the caller as a raw offset
(`v41[1068]`), which is both unreadable and easy to mistranslate; giving the
callee its real return type turns those back into named members. Order matters
-- put the `!` lines before the functions that call them.

Always point idat.exe at a *copy* of the database: it repacks the .i64 on exit.
"""

import os
import re
import sys

import idaapi
import idautils
import idc
import ida_auto
import ida_hexrays
import ida_funcs
import ida_name


def resolve_name(query):
    """Find the function whose demangled name contains `query`. Returns an ea,
    or None. Used against the Android .so, whose addresses share nothing with
    the PC exe -- there, a name is the only handle you have."""
    matches = []
    for ea in idautils.Functions():
        name = ida_funcs.get_func_name(ea)
        demangled = idc.demangle_name(name, idc.get_inf_attr(idc.INF_SHORT_DN)) or name
        if query in demangled or query in name:
            matches.append((ea, demangled))
    if not matches:
        return None, "no function matching %r" % query

    if len(matches) > 1:
        exact = [m for m in matches if m[1].split("(")[0] == query]
        if exact:
            matches = exact

    if len(matches) > 1:
        # The .so carries thunks and ICF-merged aliases under the same demangled
        # name; the biggest body is the real implementation.
        names = {m[1].split("(")[0] for m in matches}
        if len(names) > 1:
            return None, "%r is ambiguous: %s" % (query, ", ".join(sorted(names)[:5]))
        matches.sort(key=lambda m: ida_funcs.get_func(m[0]).size(), reverse=True)

    return matches[0][0], matches[0][1]


def parse_entries(path):
    """Return [(target, prototype_or_None, dump)] from the list file.

    `target` is an int address, or a str name to look up."""
    entries = []
    with open(path, "r", encoding="utf-8") as fh:
        for line in fh:
            line = line.split("#", 1)[0].strip()
            if not line:
                continue
            dump = not line.startswith("!")
            target, _, proto = line.lstrip("!").partition("|")
            target = target.strip()
            try:
                target = int(target, 16)
            except ValueError:
                pass  # not hex, so treat it as a name
            entries.append((target, proto.strip() or None, dump))
    return entries


def apply_prototype(ea, proto):
    """Override IDA's guessed signature; returns False if the type won't parse."""
    if not idc.SetType(ea, proto if proto.endswith(";") else proto + ";"):
        return False
    ida_hexrays.mark_cfunc_dirty(ea)
    return True


def dump_one(ea, out_dir, proto=None, label=None):
    """Decompile the function containing `ea`; return (ok, message)."""
    func = ida_funcs.get_func(ea)
    if func is None:
        return False, "no function at this address"

    if proto and not apply_prototype(func.start_ea, proto):
        return False, "could not apply prototype %r" % proto

    name = ida_name.get_ea_name(func.start_ea)
    demangled = idc.demangle_name(name, idc.get_inf_attr(idc.INF_SHORT_DN)) or name

    try:
        cfunc = ida_hexrays.decompile(func.start_ea)
    except ida_hexrays.DecompilationFailure as exc:
        return False, "decompilation failed: %s" % exc
    if cfunc is None:
        return False, "decompiler returned nothing"

    lines = [
        "// address : 0x%X" % func.start_ea,
        "// end     : 0x%X" % func.end_ea,
        "// mangled : %s" % name,
        "// name    : %s" % demangled,
        "// proto   : %s" % cfunc.print_dcl(),
        "",
        str(cfunc),
        "",
    ]

    # Callees are the cheapest hint about which helpers already exist.
    callees = sorted({
        idc.demangle_name(ida_name.get_ea_name(t), idc.get_inf_attr(idc.INF_SHORT_DN))
        or ida_name.get_ea_name(t)
        for head in idautils.Heads(func.start_ea, func.end_ea)
        for t in idautils.CodeRefsFrom(head, False)
        if ida_funcs.get_func(t) and ida_funcs.get_func(t).start_ea != func.start_ea
    })
    if callees:
        lines.append("/* calls:")
        lines.extend("     %s" % c for c in callees)
        lines.append("*/")

    stem = label or ("0x%X" % ea)
    out_path = os.path.join(out_dir, "%s.c" % re.sub(r"[^A-Za-z0-9_.-]", "_", stem))
    with open(out_path, "w", encoding="utf-8", errors="replace") as fh:
        fh.write("\n".join(lines))
    return True, out_path


def main():
    addr_file = os.environ.get("RESA_ADDR_FILE")
    out_dir = os.environ.get("RESA_OUT_DIR")
    if not addr_file or not out_dir:
        print("[dump] set RESA_ADDR_FILE and RESA_OUT_DIR in the environment")
        return idc.qexit(2)

    os.makedirs(out_dir, exist_ok=True)

    ida_auto.auto_wait()
    if not ida_hexrays.init_hexrays_plugin():
        print("[dump] FATAL: Hex-Rays decompiler is not available")
        return idc.qexit(3)

    entries = parse_entries(addr_file)
    print("[dump] %d entries requested" % len(entries))

    failures = 0
    dumped = 0
    for target, proto, dump in entries:
        label = None
        if isinstance(target, str):
            ea, label = resolve_name(target)
            if ea is None:
                failures += 1
                print("[dump] %s FAIL %s" % (target, label))
                continue
            label = target
        else:
            ea = target

        if not dump:
            ok = apply_prototype(ea, proto) if proto else False
            print("[dump] 0x%X type %s" % (ea, "applied" if ok else "FAILED"))
            failures += not ok
            continue
        dumped += 1
        ok, msg = dump_one(ea, out_dir, proto, label)
        if not ok:
            failures += 1
            print("[dump] 0x%X FAIL %s" % (ea, msg))
        else:
            print("[dump] 0x%X ok %s" % (ea, label or ""))

    print("[dump] done: %d dumped, %d failed" % (dumped, failures))
    idc.qexit(0)


if __name__ == "__main__":
    main()
