# Calculator example

Minimal integer expression grammar with **runnable** drivers for C++, Rust, Java, and TypeScript.

**Detailed walkthrough:** [docs/tutorial.md](../../docs/tutorial.md) (Chinese) · [docs/en/tutorial.md](../../docs/en/tutorial.md) (English)  
**5-minute path:** [docs/QUICKSTART.md](../../docs/QUICKSTART.md)

Each driver: generate tables → build → accept `NUMBER + NUMBER * NUMBER` → reject leading `PLUS`.

## Prerequisites

- Built generator (`lpg2/build/lpg-v2.3.0` or set `LPG_BIN`), or a Release binary
- Matching runtime submodule under `runtime/` (see [docs/ECOSYSTEM.md](../../docs/ECOSYSTEM.md))

## One-shot

```bash
# from repo root
export LPG_BIN=./lpg2/build/lpg-v2.3.0   # or your build / Release path
./examples/calculator/scripts/run.sh all          # cpp + rust + java + typescript
./examples/calculator/scripts/run.sh java         # fewest native deps (JDK)
./examples/calculator/scripts/run.sh cpp          # one language
```

Generate only:

```bash
./examples/calculator/scripts/generate.sh cpp|rust|java|typescript
```

## Layout

| Path | Role |
|------|------|
| `calculator.g` | Grammar (`automatic_ast=nested` + `dtParserTemplateF.gi`) |
| `scripts/generate.sh` | Table/AST generation |
| `scripts/run.sh` | Generate + build + accept/reject |
| `cpp/` | CMake driver linking `LPG-cpp-runtime` |
| `rust/` | Cargo tests linking `LPG-rust-runtime` |
| `java/` | `javac`/`java` driver with `lpg-runtime` |
| `typescript/` | `ts-node` driver with npm `lpg2ts` |

Drivers seed token kinds (no full lexer). That keeps the sample small while proving the generated parser + runtime path.

## Language-specific notes

- [java/README.md](java/README.md)
- [typescript/README.md](typescript/README.md)
- [cpp/README.md](cpp/README.md)
- [rust/README.md](rust/README.md)

## CI

The `calculator-examples` job in [`.github/workflows/ci.yml`](../../.github/workflows/ci.yml) runs all four languages on Ubuntu.
