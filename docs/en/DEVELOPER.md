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
├── docs/                 # USER / DEVELOPER / tutorial
└── lpg-generator-templates-2.1.00/
```

## Build

```bash
cd lpg2
cmake -S . -B build -DLPG2_WARNINGS_AS_ERRORS=ON   # Ubuntu CI enables this
cmake --build build -j
ctest --test-dir build --output-on-failure
```

`LPG2_WARNINGS_AS_ERRORS` uses full `-Werror`/`/WX` with GCC/MSVC. Clang
currently promotes only `return-type` and `uninitialized` while legacy
override/unused warnings are being cleaned up.

Notable CMake options: `LPG2_REQUIRE_RUST_TESTS`, `LPG2_REQUIRE_RUST_PARSER_TESTS`, `LPG2_REQUIRE_CPP_PARSER_TESTS`, `LPG2_ENABLE_SANITIZERS`.

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
- Runtime integration job clones C++ + Rust runtimes

Update goldens: `./scripts/update_golden_tables.sh`
Perf baseline: `./scripts/perf_baseline.sh` (defaults to `grammars-example/jdt.core/java.g`)

## Known limits

- Stub backends deprecated (`c` / `ml` / `plx` / `plxasm` / `xml`)
- Rust automatic AST: `nested` (children without `parent_saved`), list, `parent_saved`, environment, interface/`dyn` RHS, default+preorder visitors are covered by behavior tests; still not full Java/C++ AST parity (no `toplevel`/GLR claim)
- Bootstrap promotion must follow [../../lpg2/BOOTSTRAP.md](../../lpg2/BOOTSTRAP.md)

## Contributing

See [../../CONTRIBUTING.md](../../CONTRIBUTING.md) and [../TODO_TRIAGE.md](../TODO_TRIAGE.md).
