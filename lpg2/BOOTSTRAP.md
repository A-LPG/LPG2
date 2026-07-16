# LPG2 Self-Hosting Bootstrap

LPG2 is implemented using its own grammar. The sources that ship in the
build are under `src/` and `include/`; regenerating them is a deliberate,
reviewed step.

## Source of truth for the compiler

| Path | Role |
|------|------|
| `grammar/jikespg.g` / `grammar/jikespg.lpg` | Self-hosting grammar (keep these in sync) |
| `src/jikespg_*.cpp`, `include/jikespg_*.h` | Generated parser wired into the CMake build |
| `grammar/.lpg/` | Optional staging output from a regenerate run |

## How to regenerate

From a built `lpg-v2.3.0` (or your current binary):

```bash
lpg-v2.3.0 -programming_language=cpp_legacy -table \
  -out_directory=grammar/.lpg \
  grammar/jikespg.g
```

Then compare conflict reports and generated tables before copying into `src/`
and `include/`.

## Why `grammar/.lpg/` is not auto-synced into `src/`

A regenerate into `grammar/.lpg/` has historically reported more shift/reduce
conflicts than the tables currently compiled from `src/` (for example 9 vs 2).
Blindly copying staging files into `src/` can change parser behavior and break
self-hosting.

**Policy:** keep `grammar/.lpg/` as a staging / review area. Only promote
generated files into `src/` after:

1. Diffing conflict reports (`*.l`) against the previous listing
2. Building lpg2 with the new tables
3. Re-running the bootstrap and runtime regression tests (`ctest` in this
   directory, plus downstream language runtime tests)

## Related files

- Runtime Rust crate: submodule `runtime/LPG-rust-runtime` (expects generated
  `*prs.rs` / `*sym.rs` implementing the `ParseTable` trait)
- Generator backends: `src/RustTable.cpp`, `src/RustAction.cpp`
