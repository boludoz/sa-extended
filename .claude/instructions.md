# Reversing the rest of GTA:SA

How to turn a function that still redirects into the original binary into real
C++. Follow this end to end; every step exists because skipping it has produced
wrong code before.

## The target

| | |
|---|---|
| Binary | `gta_sa.exe` **v1.0 US compact**, 5,189,632 bytes — addresses match nothing else |
| IDA | `C:\Program Files\IDA Professional 9.2\ida.exe` (`idat.exe` for headless) |
| Database | `C:\Users\franc\OneDrive\Documentos\IDA\gta_sa.i64` (18,354 named functions, symbols ported from the Android `libGTASA.so`) |
| Asm export | `C:\Users\franc\OneDrive\Documentos\IDA\gtaasm\` — 21,675 per-function `.asm` files plus `funcs.csv` (name, start, end) |
| Diaphora | `C:\Users\franc\Downloads\diaphora-3.2.1`, loaded by `plugins/diaphora_plugin.py` via `diaphora_plugin.cfg` |

Diaphora exports already on disk:

* `gta_sa.sqlite` — the v1.0 database, **partial** (1,597 functions, stops at `0x455560`), but demangled names.
* `gta_sa_v1.01.sqlite` — a full 15,277-function export **of v1.01**. Its addresses do
  not line up with v1.0. Use it as a diff source only, never for address lookups.

## What counts as a stub

A function whose body is nothing but a jump into the original code:

```cpp
// 0x461100
void CRoadBlocks::Init() {
    plugin::Call<0x461100>();
}
```

`plugin::Call`, `plugin::CallMethod` and `plugin::CallAndReturn` are all the same
thing. Their hook is registered with `{ .reversed = false }`.

Find them:

```bash
python tools/ida/list_stubs.py --top 30
python tools/ida/list_stubs.py --class CRoadBlocks
python tools/ida/list_stubs.py --csv stubs.csv
```

Prefer whole classes over scattered functions — one class shares one struct
layout, so the expensive part (working out field offsets) is paid once.

## Step 1 — dump the pseudocode

**Close the IDA GUI first, and always run against a copy.** `idat.exe` repacks
the `.i64` on exit, so pointing it at the real database while a GUI session is
open corrupts it. Symptom of an earlier bad exit: leftover `.id0`/`.id1`/`.nam`
files next to the `.i64`.

```bash
cp "C:/Users/franc/OneDrive/Documentos/IDA/gta_sa.i64" /tmp/work.i64

printf '0x461100\n0x460EC0\n' > /tmp/addrs.txt
RESA_ADDR_FILE=/tmp/addrs.txt RESA_OUT_DIR=/tmp/out \
  "C:/Program Files/IDA Professional 9.2/idat.exe" -A -L/tmp/ida.log \
  -S"C:\\Users\\franc\\OneDrive\\Documentos\\GitHub\\ReSA\\tools\\ida\\dump_pseudocode.py" /tmp/work.i64
```

Configuration goes through the **environment**, not through `-S` arguments: IDA
mangles Windows drive letters when it splits that string (`C:\x` arrives as
`C;C:\x`). Progress lands in the `-L` log as `[dump]` lines; stdout is swallowed.

Each address produces `<out>/0x461100.c` with the demangled name, the prototype,
the pseudocode and the callee list.

**If the dump comes back full of `__asm` blocks, do not try to read them.** It
means IDA guessed the signature wrong, and two fixes cover nearly every case —
both driven from the address list:

```
!0x6B0A90 | CAutomobile *__cdecl f(void *self, int model, unsigned char, bool)
0x4619C0  | void __cdecl f(CVector a1, CVector a2, unsigned char a3)
```

* A prototype after `|` replaces IDA's guess for the function being dumped. The
  telltale bad guess is `__usercall` with an `@<st0>` argument: it makes
  Hex-Rays abandon the x87 code entirely. `CreateRoadBlockBetween2Points` went
  from 60 `__asm` blocks to none this way.
* A leading `!` types a *callee* without dumping it. Constructors typed as
  returning `void *` leave every field access in the caller as a raw offset
  (`v41[1068] |= 0x40`); giving the constructor its real return type turns those
  back into `m_nVehFlags5`. Put `!` lines before the function that calls them.

## Step 2 — recover the data layout

Pseudocode leans on raw offsets (`*(v5 + 0x42A)`) and on structs the repo does
not declare. Resolve them before writing any C++:

```bash
RESA_SYMBOLS="CRoadBlocks::,ScriptParams,CRoadblock" RESA_OUT_FILE=/tmp/syms.txt \
  "C:/Program Files/IDA Professional 9.2/idat.exe" -A -L/tmp/ida.log \
  -S"C:\\Users\\franc\\OneDrive\\Documentos\\GitHub\\ReSA\\tools\\ida\\resolve_symbols.py" /tmp/work.i64
```

It prints the address, size and type of every matching symbol, and the full
member layout of any name that is also a struct. A class-qualified prefix like
`CRoadBlocks::` lists all statics of that class at once.

An entry may also be a bare address (`RESA_SYMBOLS="0x859CE8,0x858C48"`), which
additionally prints the value there. Use it on the `flt_859CE8` constants
Hex-Rays leaves unresolved in float comparisons — those names are IDA
auto-generated and cannot be looked up any other way. Guessing them is how a
`< -90.0f` bound silently becomes something else.

To map a bare offset onto an existing repo field, dump the owning struct
(`RESA_SYMBOLS=CVehicle`) and match by offset — e.g. `[esi+0x42A] & 0x10` is byte
2 of the flags at `0x428`, bit 4, which is `vehicleFlags.bCreateRoadBlockPeds`.

## Step 3 — read the asm when the pseudocode is vague

Hex-Rays flattens the interesting parts: `__shifted` pointers, unrolled search
loops, whether a store is `1` or a flag. The per-function asm export settles it
and costs nothing:

```
C:\Users\franc\OneDrive\Documentos\IDA\gtaasm\ida_asm_exports\0x460df0__ZN11CRoadBlocks23Register....asm
```

Many of these already carry hand-written comments from earlier sessions. Read
them; they encode decisions someone already made.

Do this **whenever a write target, a loop bound, or a return value is not
obvious from the pseudocode**. Guessing here is how silent behaviour changes get
introduced.

## Step 3b — cross-check against the Android build

`C:\Users\franc\OneDrive\Documentos\IDA\libGTASA_without_source_lines.so.i64` is
the ARM64 Android build of the same game, and it carries **real field and static
names** the PC exe lost: `CRoadBlocks::InOrOut`, `NumRoadBlocks`,
`CScriptRoadBlock::bSafeToCreate`, `bGangRoadBlock`. Decompile the same function
there and read the two side by side.

Do this for any function you had to fight with. It catches things the x86 dump
hides, and it is how three separate mistakes in `GenerateRoadBlocks` were found
after it already compiled:

* **Whole blocks going missing.** The x86 pseudocode ran 530 lines with a
  `goto LABEL_21` jumping over the tail. An entire script-roadblock section, and
  an `else` that resets `bGenerateDynamicRoadBlocks`, sat past where the reading
  stopped. ARM64 decompiles to structured code with no gotos, so the tail is
  simply there.
* **x87 flag tests read wrong.** `test ah, 44h` / `jp` is MSVC's **float
  equality** idiom -- 0x44 is C3|C2, not C3|C0. Misreading it turned
  `width == otherWidth` into `width <= otherWidth`. ARM emits a plain `==`.
* **Boundary conditions.** `fcomp` + `test ah, 5` rejects on *equal* as well as
  greater, so the bound is `>= 90.0f`, not `> 90.0f`. ARM shows the operator
  outright.

The two builds are not identical, so treat a difference as a question, not as
proof: confirm it in the x86 disassembly before changing code. Every one of the
three above was verified against the PC asm first.

**Keep x86 as the primary source for logic.** The ARM compiler auto-vectorises
tight loops, and Hex-Rays renders that as pages of `int64x2_t` NEON intrinsics --
`Interior_c::ResetTiles` is 200 lines of unreadable vector shuffling on ARM and a
plain nested loop on x86. Reach for ARM to answer a specific question, not to
read the function.

What ARM is unbeatable for:

* **Names.** Field, static, enum and parameter names. Take these without
  hesitation -- no amount of staring at x86 recovers them. `Interior_c` had a
  header full of `field_412`/`gap436` at offsets that did not even align; ARM
  gave `m_tiles[30][30]`, `m_gotoPts`, `m_exitPts`, `m_distSq`, and the corrected
  layout then passed `VALIDATE_SIZE` on the first try.
* **Struct layout.** Offsets differ (64-bit pointers), but field *order* and
  *names* transfer directly. Map ARM offset -> x86 offset by tracking pointer
  widths; if the total then matches `VALIDATE_SIZE`, the mapping is right.
* **Confirming a signature** when IDA guessed a parameter away.

Names are not always there to be had. `eTileStatus` came back as
`ITS_STATE_0..9` in both builds -- placeholders someone else had already given
up on. Do not invent meanings to fill that gap; leave the constants numbered and
say so in a comment.

## Step 4 — write it in project style

Read `docs/CodingGuidelines.MD` first. The points that come up constantly:

* `rng::`/`rngv::` over hand-rolled loops; range-`for` over index loops.
* `m_` on class members, no prefix on struct members, `s_` on file-local globals.
* Original globals become `static inline auto& s_Foo = StaticRef<T>(0xADDR);`.
* Keep the `// 0xADDRESS` comment above each function.
* Fix the declared signature when the disassembly disagrees with it — stubs were
  often typed by guesswork. Check for callers first.
* Name parameters. `a1, a2, a3` in an implemented function is unfinished work.

Translate *semantics*, not instructions. The compiler's unrolled 8-way search in
`RegisterScriptRoadBlock` is one `rng::find_if`. But keep observable behaviour
identical, including the order of side effects and early returns.

## Step 5 — flip the flags and build

1. Drop `{ .reversed = false }` from that function's `RH_ScopedInstall`.
2. Set the `reversed` column to `1` in `docs/hooks.csv` for each function done.
3. Build:

```bash
cmake --build build --config Debug
```

If CMake complains the cache came from a different directory, regenerate with
`python setup.py` — the `build/` tree is not portable across repo paths or CMake
versions.

Output is `bin/Debug/gta_reversed.asi`. Test by injecting into the game; a clean
compile proves nothing about behaviour.

## About "a standalone executable"

The README's goal is a standalone exe. Nothing here gets you there on its own,
and it is worth being blunt about why:

* ~930 functions are still unreversed (`docs/hooks.csv`), plus ~1,000 hex
  redirect sites in `source/`.
* Every `StaticRef<T>(0xADDR)` still reads the original binary's memory. Even a
  fully reversed codebase stays a DLL injected into `gta_sa.exe` until all game
  state is owned by this project rather than referenced by address.
* There is no standalone entry point, no asset pipeline independent of the
  original exe, and RenderWare is still called through the original binary.

So: reversing functions is necessary but nowhere near sufficient. Treat the exe
as the long-term direction, and measure progress in classes completed.
