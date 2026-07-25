# ReSA

Fork of gta-reversed-modern: a reimplementation of GTA:SA, built as
`gta_reversed.asi` and injected into the original **v1.0 US compact** exe.

## Reversing work

Functions whose body is just `plugin::Call<0xADDR>(...)` are unimplemented —
they jump into the original binary. Turning those into C++ is the main task.

**Read [.claude/instructions.md](.claude/instructions.md) before touching one.**
It covers the IDA/Diaphora setup, the headless pseudocode pipeline in
`tools/ida/`, how to recover struct layouts, and the bookkeeping in
`docs/hooks.csv`.

Find work with `python tools/ida/list_stubs.py`.

## Build

```bash
cmake --build build --config Debug
```

Regenerate with `python setup.py` if CMake reports a stale cache directory.
Output: `bin/Debug/gta_reversed.asi`.

## Style

`docs/CodingGuidelines.MD` is not optional — 4-space indent, `rng::`/`rngv::`,
`m_` on class members, `StaticRef<T>(0xADDR)` for original globals, and a
`// 0xADDRESS` comment above every reversed function.
