# Calculator example

Minimal integer expression grammar used by [docs/tutorial.md](../../docs/tutorial.md).

## Generate tables

```bash
# from repo root, after building lpg2
./examples/calculator/scripts/generate.sh cpp
./examples/calculator/scripts/generate.sh rust
```

Outputs land in `out-cpp/` and `out-rust/` (gitignored).

## Layout

| Path | Role |
|------|------|
| `calculator.g` | Grammar |
| `scripts/generate.sh` | One-shot generation |
| `cpp/` | Notes for wiring `LPG-cpp-runtime` |
| `rust/` | Notes for wiring `LPG-rust-runtime` |

This sample lives in the main LPG2 tree (not the `grammars-example` submodule) so it stays reviewable with generator changes.
