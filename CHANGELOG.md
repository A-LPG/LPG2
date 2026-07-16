# Changelog

## 2.3.0

### Generator

- Richer diagnostics: source line excerpt, caret span, and `= help:` hints for common errors/conflicts.
- Conflict messages include an example lookahead token.
- Stub backends (`c`, `ml`, `plx`, `plxasm`, `xml`) emit a deprecation warning.
- Safer `Option::AllocateString` helpers (`memcpy` instead of `strcpy`/`strcat`).
- Shared `Action::GenerateTerminalGcDeleteReminder` to reduce duplicated AST-action scaffolding.
- Worklist-based alias nonterminal promotion in `grammar.cpp`.

### Tests & CI

- Golden table coverage for all eight full backends (cpp/rust/java/go/python3/csharp/typescript/dart).
- Ubuntu CI enables `LPG2_WARNINGS_AS_ERRORS`; GCC/MSVC use full `-Werror`/`/WX`,
  while Clang currently promotes `return-type` and `uninitialized`.
- Optional performance baseline script: `lpg2/scripts/perf_baseline.sh`.
- Non-blocking CI job records generator timing on `jdt.core` when the submodule is present.

### Docs & community

- `CONTRIBUTING.md`, GitHub issue/PR templates, `docs/TODO_TRIAGE.md`.
- English docs under `docs/en/`.
- Calculator tutorial + `examples/calculator/`.

## 2.2.03

Previous release baseline (binary packaging, runtime-integration CI, Rust/C++ AST tests).
