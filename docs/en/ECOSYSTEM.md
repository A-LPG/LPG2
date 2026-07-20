# LPG2 ecosystem compatibility and release

Machine-readable inventory: [`ecosystem/compat.json`](../../ecosystem/compat.json).

Generator **2.3.0** plus the pinned runtime / extension submodules form one verifiable ecosystem. When releasing or bumping submodules, update `compat.json` first, then this document.

Chinese version: [`../ECOSYSTEM.md`](../ECOSYSTEM.md).

## Supported backends

| Backend | CI (nested / recover) | GLR | Status |
|---------|------------------------|-----|--------|
| `cpp` / `c++` / `rt_cpp` | yes (plus glr e2e) | **driver** (`GLRParser`) | supported |
| `java` | yes (plus glr e2e) | **driver** (`GLRParser`) | supported |
| `python3` | yes (plus glr e2e) | **driver** (`GLRParser`) | supported |
| `csharp` | yes (plus glr e2e) | **driver** (`GLRParser`) | supported |
| `go` | yes (plus glr e2e) | **driver** (`GLRParser`) | supported |
| `typescript` | yes | **driver** (`GLRParser`; Playground browser demo) | supported |
| `dart` | yes (plus glr e2e) | **driver** (`GLRParser`) | supported |
| `rust` | yes (plus glr e2e) | **driver** (`GLRParser`) | supported |
| `cpp_legacy` | bootstrap | — | internal |

Features: nested AST, `%Recover` prosthetic AST, backtracking, eight-backend golden tables.
GLR: see `compat.json` → `features.glr` (eight-backend v2 GSS/SPPF drivers, `sppf: true`). Playground WASM can `-glr` generate; in-browser parse demo is TypeScript-only.

**Removed** (CLI rejects): `python2`, `c`, `ml`, `plx`, `plxasm`, `xml`. Use `python3` instead of `python2`.

## Runtime package coordinates

| Language | Submodule | Package | Version | Publish |
|----------|-----------|---------|---------|---------|
| C++ | `runtime/LPG-cpp-runtime` | CMake source | 1.0.0 | source |
| Java | `runtime/lpg-runtime` | `org.lpg2:lpg.runtime` | 1.9.0 | workflow |
| Rust | `runtime/LPG-rust-runtime` | crate `lpg2` | 1.0.0 | workflow |
| TypeScript | `runtime/LPG-typescript-runtime` | npm `lpg2ts` | 0.0.11 | workflow |
| C# | `runtime/LPG-csharp-runtime` | NuGet `LPG2.Runtime` | 1.0.2 | workflow |
| Python 3 | `runtime/LPG-python-runtime` | `lpg2-python3-runtime` | 0.0.1 | planned |
| Go | `runtime/LPG-go-runtime` | `github.com/A-LPG/LPG-go-runtime` | tags | tags |
| Dart | `runtime/LPG-Dart-runtime` | pub `lpg2` | 1.0.0 | manual |

Pinned SHAs live in `compat.json` → `pinned`.

## Toolchain

| Component | Version | Notes |
|-----------|---------|-------|
| Generator | 2.3.0 | GitHub Releases + CPack |
| Templates | `lpg-generator-templates-2.1.00` | shipped with install |
| VS Code extension | 0.0.20 | Marketplace / GitHub Release; EBNF TextMate + `-ebnf` setting |
| Language Server | 0.2.4 (submodule tip) | sugar-AST EBNF; optional via extension assemble script |

## Release checklist

1. Update `LPG2_VERSION` in `lpg2/CMakeLists.txt`
2. Update user-facing docs and `CHANGELOG.md`
3. `./lpg2/scripts/update_golden_tables.sh` if table format changed
4. Local / CI: smoke + eight-backend golden + nested/recover
5. Bump runtime / extension submodule pointers when needed
6. Update [`ecosystem/compat.json`](../../ecosystem/compat.json)
7. Sync this table
8. Tag `vX.Y.Z` → [`release.yml`](../../.github/workflows/release.yml)
9. Assemble VS Code VSIX and publish
10. Publish language packages by value (npm → NuGet → Cargo → Maven / PyPI / pub / Go tag)
11. Optional: `scripts/release-checklist.sh`

## Support policy

- **Supported:** the eight backends above; CI always runs nested + recover.
- **Removed:** `python2`, `c`, `ml`, `plx`, `plxasm`, `xml` (#13). Prefer `python3` / `java` / `cpp` / `rt_cpp`.
- **Incremental parsing (honest):** C++ runtime provides token-level incremental re-lex and statement-level re-parse — not tree-sitter-style subtree reuse.
- **Cross-backend AST shape:** nested/list fixture S-expr dump + `ast_shape_diff_*` ctests.
- **expected-tokens:** eight backends expose `expectedTerminalNames` (or equivalent) for editor completion.

## Related

- Publish secrets: [`../PUBLISH_SECRETS.md`](../PUBLISH_SECRETS.md)
- Triage (Chinese, with EN notes in §H): [`../TODO_TRIAGE.md`](../TODO_TRIAGE.md)
- Agent playbook: [`AI.md`](AI.md)
