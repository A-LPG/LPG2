# LPG2 Developer Guide

For **maintainers extending the generator**. Grammar authors should read [USER.md](USER.md).

Chinese edition: [../DEVELOPER.md](../DEVELOPER.md)

## Layout

```
LPG2/
├── lpg2/                 # generator sources + tests
├── runtime/              # language runtimes (submodules)
├── tool/                 # VS Code + language server
├── examples/             # runnable samples (calculator)
├── grammars-example/     # grammar corpus submodule (catalog + Java parse harness)
├── docs/                 # USER / DEVELOPER / tutorial
└── lpg-generator-templates-2.1.00/
```

### grammars-example parse harness

Ports from antlr/grammars-v4 live in the `grammars-example` submodule as two corpora: `bnf_example/` (classic BNF) and `ebnf_example/` (`%Options ebnf` pilot). Acceptance is lexer+parser+examples (Java), not calculator token seeding. Units also have a **quality** grade (`language_port` / `language_subset` / `token_stream_smoke` / `legacy`); required CI only gates `language_port` + `language_subset`.

```bash
bash grammars-example/bnf_example/harness/run-one.sh json
bash grammars-example/ebnf_example/harness/run-one.sh json
python3 grammars-example/bnf_example/tools/classify_quality.py
python3 grammars-example/bnf_example/tools/report.py
```

CI: `.github/workflows/grammars-example.yml` (`quality-gate` required — both `bnf_example` and `ebnf_example`; `smoke-optional` also covers both corpora with `continue-on-error`). See `grammars-example/bnf_example/CONTRIBUTING.md`.

## Build

```bash
cd lpg2
cmake -S . -B build -DLPG2_WARNINGS_AS_ERRORS=ON   # Ubuntu CI enables this
cmake --build build -j
ctest --test-dir build --output-on-failure
```

`LPG2_WARNINGS_AS_ERRORS` promotes `return-type` / `uninitialized` / `format`
(and on GCC also `sequence-point`, `unused-variable`, and `unused-but-set-variable`).
Clang currently elevates a narrower set; see `lpg2/CMakeLists.txt`.

Notable CMake options: `LPG2_REQUIRE_RUST_TESTS`, `LPG2_REQUIRE_RUST_PARSER_TESTS`, `LPG2_REQUIRE_CPP_PARSER_TESTS`, `LPG2_ENABLE_SANITIZERS`, plus per-language `LPG2_REQUIRE_*_PARSER_TESTS`.

## Architecture

| Area | Files |
|------|-------|
| CLI / options | `option*.cpp`, `control.cpp` |
| Grammar IR | `grammar.cpp`, `parser.cpp`, `jikespg_*` |
| Tables / actions | `*Table.cpp`, `*Action.cpp` |
| Conflicts / diagnostics | `resolve.cpp`, `diagnose.cpp`, `Option::Emit*` |

New languages need a `*Table` / `*Action` pair plus registration in `control.cpp` / `options.cpp`.

## Testing

- Smoke + **8-backend goldens** (`minimal_*_golden`)
- Feature / negative / CLI / bootstrap / sanitizers
- Runtime integration: C++ / Rust / Java / Python / Go / TypeScript / C# / Dart
  nested AST (`*_automatic_ast_nested`) **and** recover / prosthetic AST
  (`*_automatic_ast_recover`) — both are required in CI language jobs
- Soft perf thresholds: [../perf-baselines/THRESHOLDS.md](../perf-baselines/THRESHOLDS.md)
- Ecosystem matrix: [../ECOSYSTEM.md](../ECOSYSTEM.md) / [../../ecosystem/compat.json](../../ecosystem/compat.json)

Rust runtime is the git submodule `runtime/LPG-rust-runtime`:

```bash
git submodule update --init runtime/LPG-rust-runtime
# then -DLPG2_RUST_RUNTIME_DIR=$PWD/runtime/LPG-rust-runtime/lpg2
```

Update goldens: `./scripts/update_golden_tables.sh`
Perf baseline: `./scripts/perf_baseline.sh` (defaults to `lpg2/grammar/jikespg.g`)

## Known limits

- Stub backends `c` / `ml` / `plx` / `plxasm` / `xml` **removed** (issue #13); migrate to `java` / `cpp` / `rt_cpp` / …
- `cpp` / `c++` / `rt_cpp` are aliases of the modern C++ backend (`CppAction2` / `CppTable2`)
- `cpp_legacy` is self-host only (old `CppTable` + `*.cpp` tables); see [BOOTSTRAP.md](../../lpg2/BOOTSTRAP.md)
- Recover / prosthetic AST: **done for all shipped backends** (Java, C++, Rust, Go, C#, TypeScript, Dart, Python), including `$allocation`. `%Recover Sym /. expr ./` embeds `expr` in the factory (may reference `error_token`); without a block the generator emits a typed `Missing*(error_token, error_token)` prosthesis (`AstToken` fallback when `needs_environment`). `getProsthesisIndex` + `EmitProstheticAstFactories` + runtime `ProstheticAst`/`BacktrackingParser` close the loop, covered per backend by `*_automatic_ast_recover` e2e tests. Backends supply the optional accessors idiomatically (default interface methods, structural/optional interfaces, a Dart mixin, or Python duck typing), so grammars without `%Recover` keep the historical throw behavior.
- Rust automatic AST: `nested` and `toplevel` (satellite `.rs` under `<out>/ast/`, wired via `*_ast_includes.rs` `include!`; defaults to `<out>/ast` when `-ast_directory` is omitted), list, `parent_saved`, environment, interface/`dyn` RHS, default+preorder visitors — covered by `rust_automatic_ast_*_behavior` (incl. `rust_automatic_ast_toplevel_behavior`)
- Bootstrap promotion must follow [../../lpg2/BOOTSTRAP.md](../../lpg2/BOOTSTRAP.md)

## Contributing

See [../../CONTRIBUTING.md](../../CONTRIBUTING.md) and [../TODO_TRIAGE.md](../TODO_TRIAGE.md).
