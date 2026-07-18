# Calculator example

Minimal integer expression grammar with **runnable** drivers for all eight backends:
C++, Rust, Java, TypeScript, Go, Python 3, C#, and Dart.

**Detailed walkthrough:** [docs/en/tutorial.md](../../docs/en/tutorial.md) (English) · [docs/tutorial.md](../../docs/tutorial.md) (中文)  
**5-minute path:** [docs/en/QUICKSTART.md](../../docs/en/QUICKSTART.md)

Each driver: generate tables → build → accept `NUMBER + NUMBER * NUMBER` → reject leading `PLUS`.

## Prerequisites

- Built generator (`lpg2/build/lpg-v2.3.0` or set `LPG_BIN`), or a Release binary
- Matching runtime submodule under `runtime/` (see [docs/ECOSYSTEM.md](../../docs/ECOSYSTEM.md))

## One-shot

```bash
# from repo root
export LPG_BIN=./lpg2/build/lpg-v2.3.0   # or your build / Release path
./examples/calculator/scripts/run.sh all   # all 8 backends
./examples/calculator/scripts/run.sh java  # fewest native deps (JDK)
./examples/calculator/scripts/run.sh go    # one language
```

Generate only:

```bash
./examples/calculator/scripts/generate.sh cpp|rust|java|typescript|go|python3|csharp|dart
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
| `go/` | Go driver with `LPG-go-runtime` |
| `python/` | Python 3 driver with `LPG-python-runtime` |
| `csharp/` | .NET driver with `LPG2.Runtime` |
| `dart/` | Dart driver with pub `lpg2` path dep |

Drivers seed token kinds (no full lexer). That keeps the sample small while proving the generated parser + runtime path.

## Language-specific notes

- [java/README.md](java/README.md)
- [typescript/README.md](typescript/README.md)
- [cpp/README.md](cpp/README.md)
- [rust/README.md](rust/README.md)
- [go/README.md](go/README.md)
- [python/README.md](python/README.md)
- [csharp/README.md](csharp/README.md)
- [dart/README.md](dart/README.md)

## CI

The `calculator-examples` job in [`.github/workflows/ci.yml`](../../.github/workflows/ci.yml) runs all eight languages on Ubuntu.
