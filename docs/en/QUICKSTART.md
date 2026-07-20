# LPG2 Quick Start (about 5 minutes)

Goal: run the in-repo calculator example end-to-end so you see grammar → generated tables → runtime parse.

Deeper reading: [CONCEPTS.md](CONCEPTS.md) and [tutorial.md](tutorial.md). Chinese edition: [../QUICKSTART.md](../QUICKSTART.md).

## 1. Get the generator

Pick one.

**A. GitHub Release (recommended)**

1. Download a platform archive from [Releases](https://github.com/A-LPG/LPG2/releases) and verify `SHA256SUMS`
2. Note the path to `bin/lpg-v2.3.0`

```bash
export LPG_BIN=/path/to/bin/lpg-v2.3.0
```

**B. Build from source**

```bash
cd lpg2
cmake -S . -B build && cmake --build build -j
export LPG_BIN="$PWD/build/lpg-v2.3.0"
cd ..
```

## 2. Init runtime submodules

The sample links a language runtime. From the repo root:

```bash
# Java (fewest native deps — good first try)
git submodule update --init runtime/lpg-runtime

# or C++
git submodule update --init runtime/LPG-cpp-runtime

# or all eight backends (see examples/calculator/README.md)
git submodule update --init runtime/lpg-runtime runtime/LPG-cpp-runtime \
  runtime/LPG-rust-runtime runtime/LPG-typescript-runtime \
  runtime/LPG-go-runtime runtime/LPG-python-runtime \
  runtime/LPG-csharp-runtime runtime/LPG-Dart-runtime
```

Templates live in `lpg-generator-templates-2.1.00/` in the main tree (no submodule).

## 3. One command

From the repo root:

```bash
# Optional scaffold
./scripts/lpg2 init ./my-parser --lang=java
./scripts/lpg2 test --grammar=./my-parser/grammar.g

# Java (needs a JDK)
./examples/calculator/scripts/run.sh java

# any backend: cpp|rust|java|typescript|go|python|csharp|dart
./examples/calculator/scripts/run.sh go

# all eight (needs matching toolchains)
./examples/calculator/scripts/run.sh all

# Optional: opt-in EBNF showcase (Java)
./examples/ebnf-call/scripts/run.sh
```

If `LPG_BIN` is unset, the script looks for `lpg2/build/lpg-v*`.

## 4. Expected result

The driver will:

- **accept** a token sequence like `NUMBER + NUMBER * NUMBER`
- **reject** an illegal sequence that starts with `PLUS`

No errors and exit code 0 means success.

## 5. What you just ran

| Piece | Path / role |
|-------|-------------|
| Grammar | `examples/calculator/calculator.g` |
| Generated tables | under `examples/calculator/out-<lang>/` |
| Driver + runtime | `examples/calculator/<lang>/` linked to `runtime/` |

The generator does **not** emit a full lexer. This sample injects tokens by hand to prove the parse-table + runtime path.

## Next

1. [CONCEPTS.md](CONCEPTS.md) — mental model
2. [tutorial.md](tutorial.md) — walk the grammar and outputs
3. [USER.md](USER.md) — integrate into your project
