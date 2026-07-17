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

Full C++/Rust/Java parser–AST tests:

```bash
# Rust/C++/Java runtimes ship as git submodules under runtime/ (recursive clone).
cmake -S . -B build \
  -DLPG2_REQUIRE_RUST_PARSER_TESTS=ON \
  -DLPG2_REQUIRE_CPP_PARSER_TESTS=ON \
  -DLPG2_REQUIRE_JAVA_PARSER_TESTS=ON \
  -DLPG2_RUST_RUNTIME_DIR=$PWD/../runtime/LPG-rust-runtime/lpg2 \
  -DLPG2_CPP_RUNTIME_DIR=$PWD/../runtime/LPG-cpp-runtime \
  -DLPG2_JAVA_RUNTIME_DIR=$PWD/../runtime/lpg-runtime
cmake --build build -j
ctest --test-dir build -R 'rust_|cpp_automatic|java_automatic' --output-on-failure
```

Language runtimes ship as git submodules under `runtime/` (see
[docs/DEVELOPER.md](docs/DEVELOPER.md)). Compatibility matrix:
[docs/ECOSYSTEM.md](docs/ECOSYSTEM.md) / [ecosystem/compat.json](ecosystem/compat.json).

For a one-command toolchain check and recommended CMake flags:

```bash
./scripts/dev-bootstrap.sh
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
- Stub backends (`c` / `ml` / `plx` / `plxasm` / `xml`) were **removed**; do not reintroduce them.
- See [docs/TODO_TRIAGE.md](docs/TODO_TRIAGE.md) for curated `good-first-issue` work.

## VS Code / Language Server checklist (2.3.0+)

After generator or CLI changes that affect the extension:

1. Build `lpg-v2.3.0` and run `./tool/LPG-VScode/scripts/assemble-release.sh`.
2. Generate a grammar with `-table -programming_language=rust` (and `java` / `rt_cpp`) from the Command Palette; confirm tables land in the expected out directory.
3. Force a failing generate (bad `.g` or `-fail_on_conflicts` on a conflict grammar); confirm the extension surfaces `showErrorMessage` and does not treat exit 12 as success.
4. Confirm `programming_language` settings no longer list removed stubs (`c`/`ml`/`plx`/`plxasm`/`xml`).
5. Optional: point LSP at the new binary and hover/completion still work on a small `.g`.

## Pull requests

1. Fork / branch from `main`.
2. Add or update tests (prefer `lpg2_add_generation_test` fixtures).
3. Ensure `ctest -L smoke` passes locally (Linux Docker if you changed include case or C++ runtime).
4. Fill the PR template; link related issues.

## Reporting bugs

Use the Bug report issue template. Include: OS, compiler, `lpg-v*` version,
minimal `.g` grammar, and full stderr (new diagnostics include source excerpts,
carets, and `= help:` hints).
