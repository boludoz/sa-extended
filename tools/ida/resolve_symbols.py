"""
Resolve global symbols to addresses (and dump their struct layout) from the
gta_sa.exe (v1.0 US) IDA database.

    RESA_SYMBOLS="CRoadBlocks::aScriptRoadBlocks,ScriptParams" \
    RESA_OUT_FILE=symbols.txt \
        idat.exe -A -S"tools/ida/resolve_symbols.py" work.i64

RESA_SYMBOLS is a comma-separated list of demangled names. Each is matched
against every named location in the database, so a class-qualified prefix such
as `CRoadBlocks::` lists all statics of that class at once.

As with dump_pseudocode.py the configuration travels in the environment,
because IDA corrupts drive letters inside `-S` arguments.
"""

import os

import idaapi
import idautils
import idc
import ida_auto
import ida_bytes
import ida_name
import ida_typeinf


def collect(patterns):
    """Return [(ea, mangled, demangled)] for every name matching any pattern."""
    hits = []
    for ea, name in idautils.Names():
        demangled = idc.demangle_name(name, idc.get_inf_attr(idc.INF_SHORT_DN)) or name
        for pat in patterns:
            if pat in demangled or pat in name:
                hits.append((ea, name, demangled))
                break
    return sorted(hits)


def describe_type(ea):
    """Best-effort textual type of the item at `ea`."""
    tif = ida_typeinf.tinfo_t()
    if ida_nalt_get_tinfo(tif, ea):
        return tif._print()
    return idc.get_type(ea) or "<no type>"


def ida_nalt_get_tinfo(tif, ea):
    try:
        import ida_nalt
        return ida_nalt.get_tinfo(tif, ea)
    except Exception:
        return False


def read_value(ea, size):
    """Decode a 4- or 8-byte scalar. Mainly to put a number on the bare
    `flt_858C48` constants Hex-Rays leaves in x87 code."""
    import struct
    if size not in (4, 8):
        return None
    raw = ida_bytes.get_bytes(ea, size)
    if raw is None or len(raw) != size:
        return None
    if size == 4:
        f, i = struct.unpack("<f", raw)[0], struct.unpack("<i", raw)[0]
        return "float %g / int %d / hex 0x%08X" % (f, i, i & 0xFFFFFFFF)
    return "double %g" % struct.unpack("<d", raw)[0]


def dump_enum(name, out):
    """Print the members of a named enum. Worth asking for by name: enum
    constants are what turn `status == 9` into something readable."""
    tif = ida_typeinf.tinfo_t()
    if not tif.get_named_type(None, name):
        return False
    edt = ida_typeinf.enum_type_data_t()
    if not tif.get_enum_details(edt):
        return False
    out.append("enum %s  // size 0x%X" % (name, tif.get_size()))
    for m in edt:
        out.append("    %-44s = %d" % (m.name, m.value))
    out.append("")
    return True


def dump_struct(name, out):
    """Print the members of a named struct/udt if the database knows one."""
    tif = ida_typeinf.tinfo_t()
    if not tif.get_named_type(None, name):
        return False
    udt = ida_typeinf.udt_type_data_t()
    if not tif.get_udt_details(udt):
        return False
    out.append("struct %s  // size 0x%X" % (name, tif.get_size()))
    for m in udt:
        out.append("    +0x%-4X %-40s %s" % (m.offset // 8, m.type._print(), m.name))
    out.append("")
    return True


def main():
    patterns = [p.strip() for p in os.environ.get("RESA_SYMBOLS", "").split(",") if p.strip()]
    out_file = os.environ.get("RESA_OUT_FILE")
    if not patterns or not out_file:
        print("[sym] set RESA_SYMBOLS and RESA_OUT_FILE in the environment")
        return idc.qexit(2)

    ida_auto.auto_wait()

    # A pattern may be a bare address. IDA's auto-generated dummy names
    # (`flt_858C48`, `unk_A43438`) are absent from the name list, so the only
    # way to ask about one is by the address in its name.
    hits = []
    for pat in list(patterns):
        if pat.lower().startswith("0x"):
            ea = int(pat, 16)
            hits.append((ea, ida_name.get_ea_name(ea) or pat, pat))
            patterns.remove(pat)
    hits += collect(patterns)

    out = []
    for ea, mangled, demangled in sorted(hits):
        size = ida_bytes.get_item_size(ea) or 4
        out.append("0x%08X  size=0x%-6X  %s" % (ea, size, demangled))
        if mangled != demangled:
            out.append("            mangled: %s" % mangled)
        out.append("            type   : %s" % describe_type(ea))
        value = read_value(ea, size)
        if value is not None:
            out.append("            value  : %s" % value)
    out.append("")

    # Any pattern that names a type gets its layout or constants printed too.
    for pat in patterns:
        dump_struct(pat.strip(), out) or dump_enum(pat.strip(), out)

    text = "\n".join(out)
    with open(out_file, "w", encoding="utf-8", errors="replace") as fh:
        fh.write(text)
    print("[sym] wrote %d lines to %s" % (len(out), out_file))
    idc.qexit(0)


if __name__ == "__main__":
    main()
