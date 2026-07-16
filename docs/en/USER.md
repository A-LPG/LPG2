# LPG2 User Guide

For **authors of `.g` / `.lpg` grammars** and people integrating generated parsers. To change the generator itself, see [DEVELOPER.md](DEVELOPER.md).

Chinese edition: [../USER.md](../USER.md)

## What is LPG2?

LPG2 (Lookahead Parser Generator v2) reads `.g` / `.lpg` grammars and emits parse tables plus semantic-action stubs for the chosen language. It supports LALR parsing, backtracking disambiguation, grammar import, and automatic AST generation.

**Current version: 2.3.0** (`lpg-v2.3.0`).

## Obtaining the generator

1. **GitHub Releases** — download Linux / macOS / Windows archives and verify `SHA256SUMS`.
2. **VS Code extension** — [lpg-vscode](https://marketplace.visualstudio.com/items?itemName=kuafuwang.lpg-vscode).
3. **From source**:

```bash
cd lpg2
cmake -S . -B build && cmake --build build -j
cmake --install build --prefix ./install
./install/bin/lpg-v2.3.0 --help
```

## Basic workflow

```bash
lpg-v2.3.0 -programming_language=cpp -table \
  -out_directory=./out \
  path/to/grammar.g
```

| Flag | Meaning |
|------|---------|
| `-programming_language=` | Target language (see table below) |
| `-table` | Emit parse tables |
| `-out_directory=` | Output directory |
| `-quiet` | Less console noise |
| `-fail_on_conflicts` | Treat shift/reduce conflicts as errors (exit 12) |

Help / version exit 0. Grammar or option errors exit **12** and print diagnostics on stderr. Failed runs do not leave half-written outputs.

### Diagnostics

Errors and warnings look like:

```text
path/grammar.g:10:13:10:13:...: Error: Block not properly terminated
  |     S ::= a /.
  |             ^
  = help: Close the action block with the matching end marker ...
```

## Supported languages

| Language | Value | Status |
|----------|-------|--------|
| C++ | `cpp` / `rt_cpp` | Full; `rt_cpp` links `LPG-cpp-runtime` |
| Java / C# / Go / Python 3 / TypeScript / Dart | `java` … `dart` | Full generation; CI emphasizes smoke + goldens |
| Rust | `rust` | Tables + parsers; automatic AST covers `nested` (incl. `get_children` without `parent_saved`), list, `parent_saved`, `needs_environment`, interface/`dyn` RHS recovery, default/preorder visitors (`rust_automatic_ast_*_behavior`). Complex grammars still warrant small-step validation; not full Java/C++ AST parity (no `toplevel`/GLR claim) |
| C / ML / Plx / Plxasm / Xml | `c` … | **Deprecated stubs** — warning at run time; prefer a full backend |

## Runtimes

Generated tables need a language runtime under `runtime/` (git submodules). Rust lives in the sibling [LPG-rust-runtime](https://github.com/A-LPG/LPG-rust-runtime) repo.

```bash
git clone --recursive https://github.com/A-LPG/LPG2.git
```

## Examples and tutorial

- Hands-on calculator (C++ & Rust): [../tutorial.md](../tutorial.md)
- Runnable sample: [../../examples/calculator/](../../examples/calculator/)

## FAQ

**Q: I see `Shift/reduce conflict … (example lookahead: X)`. What now?**
A: Add `%Left` / `%Right` / `%Priority`, rewrite the ambiguous rules, or pass `-fail_on_conflicts` in CI so conflicts fail the build.

**Q: `Block not properly terminated`**
A: Action blocks must be closed (default markers `/.` … `./`).

**Q: Stub backend deprecated warning**
A: Switch to `cpp`, `java`, `rust`, etc. Stub languages may be removed in a later release.

**Q: Tables changed after a generator upgrade**
A: Compare against `lpg2/tests/golden/minimal/<lang>/` or re-run your project’s golden update script.
