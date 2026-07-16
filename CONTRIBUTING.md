# Contributing to LPG2

Thanks for contributing. This guide covers the generator repository ([LPG2](https://github.com/A-LPG/LPG2)).

## Prerequisites

- CMake ≥ 3.16
- A C++17 compiler (GCC, Clang, or MSVC)
- Optional: Rust toolchain (for Rust table / parser tests)
- Optional: Docker Ubuntu image (to match CI Linux)

## Build and test

```bash
git submodule update --init --recursive
cd lpg2
cmake -S . -B build -DLPG2_REQUIRE_RUST_TESTS=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
ctest --test-dir build -L smoke --output-on-failure   # fast path
```

Full C++/Rust parser–AST tests (as in CI `runtime-integration`):

```bash
cmake -S . -B build \
  -DLPG2_REQUIRE_RUST_PARSER_TESTS=ON \
  -DLPG2_REQUIRE_CPP_PARSER_TESTS=ON \
  -DLPG2_RUST_RUNTIME_DIR=/path/to/LPG-rust-runtime/lpg2
cmake --build build -j
ctest --test-dir build -R 'rust_|cpp_automatic' --output-on-failure
```

## Golden tables

Eight backends keep byte-stable `*prs.*` goldens under `lpg2/tests/golden/minimal/`.

```bash
LPG_BIN=./build/lpg-v2.3.0 ./scripts/update_golden_tables.sh
ctest --test-dir build -R '_golden$' --output-on-failure
```

If your change intentionally alters generated tables, update goldens in the same PR and explain why.

## Bootstrap (`jikespg_*`)

Do **not** copy `grammar/.lpg/` into `src/` casually. Follow [lpg2/BOOTSTRAP.md](lpg2/BOOTSTRAP.md): regenerate, compare conflicts, run `bootstrap_stage2`, then promote.

## Coding norms

- Prefer small, reviewable PRs; keep golden / smoke green.
- Avoid new `strcpy` / `sprintf`; use `memcpy` / `snprintf` / `std::string`.
- Stub backends (`c`, `ml`, `plx`, `plxasm`, `xml`) are deprecated — do not extend them.
- See [docs/TODO_TRIAGE.md](docs/TODO_TRIAGE.md) for curated `good-first-issue` work.

## Pull requests

1. Fork / branch from `main`.
2. Add or update tests (prefer `lpg2_add_generation_test` fixtures).
3. Ensure `ctest -L smoke` passes locally (Linux Docker if you changed include case or C++ runtime).
4. Fill the PR template; link related issues.

## Reporting bugs

Use the Bug report issue template. Include: OS, compiler, `lpg-v*` version,
minimal `.g` grammar, and full stderr (new diagnostics include source excerpts,
carets, and `= help:` hints).
