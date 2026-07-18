# LPG2

[![VS Code Marketplace version](https://vsmarketplacebadge.apphb.com/version-short/kuafuwang.lpg-vscode.svg)](https://marketplace.visualstudio.com/items?itemName=kuafuwang.lpg-vscode)

LPG2 (Lookahead Parser Generator v2) compiles `.g` / `.lpg` grammars into parse tables and semantic-action code for your target language.

**Current version: 2.3.0** (`lpg-v2.3.0`)

[中文文档](docs/README.zh.md) · [Documentation index](docs/README.md)

## Documentation

Docs are layered by audience. **New to LPG? Start with the Quick Start.**

| Document | Audience | Contents |
| --- | --- | --- |
| [**Quick Start (5 min)**](docs/en/QUICKSTART.md) | Beginners | Download/build generator → run calculator |
| [**Concepts**](docs/en/CONCEPTS.md) | Beginners | How generator / templates / runtime fit together |
| [**Tutorial**](docs/en/tutorial.md) | Beginners | Step-by-step calculator grammar |
| [**User guide**](docs/en/USER.md) | Grammar authors, integrators | Tooling, CLI, languages, runtimes, FAQ |
| [**Developer guide**](docs/en/DEVELOPER.md) | Generator maintainers | Build, layout, bootstrap, tests, backends |
| [**AI / Agent playbook**](docs/en/AI.md) | AI coding agents | Workflow, CLI, language matrix, anti-patterns ([AGENTS.md](AGENTS.md)) |
| [**Chinese docs**](docs/README.zh.md) | 中文读者 | 上手 / 概念 / 教程 / USER / AI 等 |
| [**Contributing**](CONTRIBUTING.md) | Contributors | Build, goldens, PR process |
| [**Doc index**](docs/README.md) | — | Pick a path by role |
| [**Ecosystem**](docs/ECOSYSTEM.md) | Integrators / release | Runtime versions, package coords, release checklist |

Topics:

- [Grammar reference](docs/en/GRAMMAR_REFERENCE.md) (EN summary) · [full Chinese](docs/GRAMMAR_REFERENCE.md)
- [Bootstrap policy](lpg2/BOOTSTRAP.md) — regenerating `jikespg_*` parsers
- Machine-readable matrix: [ecosystem/compat.json](ecosystem/compat.json)

## Repository layout

| Path | Role |
| --- | --- |
| [`lpg2/`](lpg2/) | Generator sources |
| [`runtime/`](runtime/) | Language runtimes (git submodules) |
| [`tool/`](tool/) | VS Code extension and language server (submodules) |
| [`grammars-example/`](grammars-example/) | Example grammars (submodule) |
| [`examples/calculator/`](examples/calculator/) | Runnable calculator starter |
| [`docs/`](docs/) | Documentation (`docs/en/` default; Chinese alongside) |

## Quick try

**Fastest: npm**

```bash
npx lpg2 --help
npx lpg2 -programming_language=java -table your.g
```

Browser playground (WASM, built by CI): [`playground/`](playground/).

The C++ runtime provides **token-level re-lex + statement-level re-parse** (not tree-sitter subtree reuse); see [USER.md](docs/en/USER.md) / [ECOSYSTEM.md](docs/ECOSYSTEM.md).

**Recommended: Release binary + calculator**

1. Download a platform archive from [GitHub Releases](https://github.com/A-LPG/LPG2/releases) and verify `SHA256SUMS`
2. Init the runtime submodule for your language (e.g. Java):

```bash
git submodule update --init runtime/lpg-runtime
export LPG_BIN=/path/to/bin/lpg-v2.3.0
./examples/calculator/scripts/run.sh java
```

Full steps: [docs/en/QUICKSTART.md](docs/en/QUICKSTART.md).

**Or build the generator from source:**

```bash
cd lpg2
cmake -S . -B build && cmake --build build -j
export LPG_BIN="$PWD/build/lpg-v2.3.0"
cd ..
./examples/calculator/scripts/run.sh java
```

Integration and CLI details: [user guide](docs/en/USER.md).

## Related projects

- [LPG-rust-runtime](https://github.com/A-LPG/LPG-rust-runtime) — Rust parse runtime
- [LPG2-grammars-example](https://github.com/A-LPG/LPG2-grammars-example) — grammar examples
- [LPG-VScode](https://github.com/A-LPG/LPG-VScode) — VS Code extension
