# Changelog

## Unreleased

### GLR (Java / C++ / TypeScript)

- **GLR v2 (GSS/SPPF):** runtime `GssNode`/`GssEdge` prefix sharing and `SppfNode` shared packed parse forest; `getSppfRoot()` / `getSppfSymbolCount()`; `nextAst` remains the compatibility projection. Catalan e2e asserts SPPF symbol count ≪ derivation count (n=8).
- **TypeScript GLR driver:** `templates/typescript/glrParserTemplateF.gi`, `TypeScriptTable` `isGLR`, and `lpg2ts` `GLRParser` / GSS / SPPF.
- **Playground WASM GLR:** UI `-glr` generate for typescript/java/cpp; in-browser forest demo via `playground/glr-demo.bundle.js` (`./scripts/build-playground-glr-demo.sh`). Java generate-only in the browser (no JVM).
- GLR error repair: `parser(error_repair_count)` / `parseEntry(marker, N)` with `N>0` falls back to `BacktrackingParser.fuzzyParse*` after GLR failure so `%Recover` prosthetic AST replay matches bt; Diagnose remains the last resort. Forest packing stays on the error-free GLR path only. Tests: `java_glr_recover_e2e`, `cpp_glr_recover_e2e`.
- `-glr` generates multi-action conflict tables (same encoding as `-backtrack`) with `isGLR()` table flags (Java + C++ + TypeScript) and AST `nextAst` scaffolding across backends.
- GLR AST scaffolding stores the language-level `IAst` interface across Java, C++, C#, TypeScript, Dart, Go, Python, and Rust; generated compile checks no longer fail on base-to-derived assignments or recursive Go fields.
- Templates `glrParserTemplateF.gi` (Java, `rt_cpp`, TypeScript) plus runtime `GLRParser` drivers.
- Nested ambiguity retains exact, cycle-free Catalan derivations; correlated ambiguous stack slots, additional start entry points, reduce/reduce conflicts, and non-cyclic nullable rules are covered (Java e2e suite).
- Tests: `glr_tables_golden_java`, `java_glr_ambiguous_e2e`, `java_glr_correlation_e2e`, `java_glr_symbol_identity_e2e`, `java_glr_entry_e2e`, `java_glr_rr_epsilon_e2e`, `java_glr_cyclic_e2e`, `java_glr_non_ast_e2e`, `cpp_glr_ambiguous_e2e`. Other language runtimes still ship scaffolding only.
- Productization: `compat.json` / `ECOSYSTEM.md` `features.glr` (+ `sppf`); stronger `-glr` warning + diagnostics `glr_template_hint` when the active template is not `glrParserTemplateF.gi`.
- `-glr -fail_on_conflicts` treats retained conflicts as handled (`health.healthy=true`) while preserving conflict counts in diagnostics.
- Limits: no DiagnoseParser / GLR-side `%Recover` replay on the forest path; cyclic/ε-loop grammars rejected.
- Forest packing links alternatives without rewriting an incoming `nextAst` chain; accept packing keys on grammar symbol + token span.
- Fix Release-build empty `$entry_name` / `$entry_marker` expansion (`assert(InsertLocalMacro(...))` was stripped under `NDEBUG`).

### Diagnostics

- `--diagnostics=json` (also `-diagnostics=json`) emits one JSON object on stdout with structured diagnostics and a grammar `health` report; human-readable diagnostics remain the default.
- `-nowrite` now performs analysis without publishing generated or listing files. Its health report includes conflict totals, backtrack/glr/soft-keyword settings, recover symbols, target language, and warning counts.

### Grammars corpus

- `grammars-example`: parse-level ports of antlr/grammars-v4 (~390 units, tiers A–D) with Java harness (`harness/run-one.sh`), `catalog.json`, and CI workflow `grammars-example.yml`.
- Honest **quality** grading: `language_port` / `language_subset` / `token_stream_smoke` / `legacy` (`tools/classify_quality.py`, `QUALITY.md`). CI **quality-gate** requires port+subset; token-stream smoke is optional/`continue-on-error`.
- Upgraded mainstream subsets (JS/TS, Python3, Java, C, HTML/XML, Go, Rust, PHP, Kotlin, C#, …); legacy `leg`/`lpg`/`java` harnessed; unused mega-`examples/` quarantined where harness uses curated only.

### Adoption

- npm package `lpg2` (`npx lpg2`) downloads the platform binary from GitHub Releases.
- Browser playground (`playground/`) with Emscripten WASM build (`scripts/build-wasm.sh`, `wasm-playground.yml`).

### Incremental parsing

- C++ runtime incremental contract tests: `incremental_prs_stream` (token-stream damage reset + soft bench) and `cpp_automatic_ast_incremental` (reset + re-seed + re-parse).
- TypeScript runtime honest incremental API (`IncrementalParse`, damage-offset re-lex/reparse) plus playground demo; still not tree-sitter subtree reuse.
- Docs position the feature as token-level relex + statement-level reparse — not tree-sitter subtree reuse.

### Recover / prosthesis

- Default `%Recover` without `$allocation` emits a typed Missing*-style AST node (name + error-token span + visitor-distinguishable) across nested-AST backends; explicit `/. … ./` unchanged.

### Correctness & completion

- Cross-backend AST S-expression dumps on nested/list fixtures; `ast_shape_diff_nested` / `ast_shape_diff_list` assert identical shapes.
- `expectedTerminalNames` (language-idiomatic names) + unified `ParseIssue` (`code` / `span` / `expected[]` / `got`) on all eight runtimes; ctest `*_expected_tokens` for each.

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
