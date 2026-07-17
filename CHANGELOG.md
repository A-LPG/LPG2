# Changelog

## Unreleased

### Adoption

- npm package `lpg2` (`npx lpg2`) downloads the platform binary from GitHub Releases.
- Browser playground (`playground/`) with Emscripten WASM build (`scripts/build-wasm.sh`, `wasm-playground.yml`).

## 2.3.0

### Generator

- Richer diagnostics: source line excerpt, caret span, and `= help:` hints for common errors/conflicts.
- Conflict messages include an example lookahead token.
- Stub backends (`c`, `ml`, `plx`, `plxasm`, `xml`) **removed** (no longer registered; migrate to a full backend).
- `python2` marked **deprecated** (no smoke/golden/CI; removal planned for 2.4). Prefer `python3`.
- Safer `Option::AllocateString` helpers (`memcpy` instead of `strcpy`/`strcat`).
- Shared `Action::GenerateTerminalGcDeleteReminder` to reduce duplicated AST-action scaffolding.
- Worklist-based alias nonterminal promotion in `grammar.cpp`.
- `%Recover` prosthetic AST factories for all eight AST backends (including optional `$allocation` blocks).

### Tests & CI

- Golden table coverage for all eight full backends (cpp/rust/java/go/python3/csharp/typescript/dart).
- Runtime CI requires nested **and** recover AST e2e per language job.
- Ubuntu CI enables `LPG2_WARNINGS_AS_ERRORS`; GCC/MSVC use full `-Werror`/`/WX`,
  while Clang currently promotes a narrower set (see `lpg2/CMakeLists.txt`).
- Optional performance baseline script: `lpg2/scripts/perf_baseline.sh`.
- Non-blocking CI job records generator timing on `jikespg.g`.

### Docs & community

- `CONTRIBUTING.md`, GitHub issue/PR templates, `docs/TODO_TRIAGE.md` (ecosystem backlog).
- English docs under `docs/en/`.
- Calculator tutorial + `examples/calculator/`.
- Ecosystem compatibility matrix: `docs/ECOSYSTEM.md` + `ecosystem/compat.json`.

## 2.2.03

Previous release baseline (binary packaging, runtime-integration CI, Rust/C++ AST tests).
