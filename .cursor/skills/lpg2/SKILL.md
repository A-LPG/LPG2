---
name: lpg2
description: >-
  Use the LPG2 lookahead parser generator: write/edit .g/.lpg grammars, run
  lpg-v2.3.0 or npx lpg2, generate parse tables/AST for java/cpp/rust/typescript/
  go/python3/csharp/dart, link language runtimes, and validate via
  examples/calculator. Use when the user mentions LPG, LPG2, .g grammars,
  lpg-v2, parse tables, automatic_ast, %Recover, or integrating an LPG runtime.
---

# LPG2 Agent Skill

Canonical playbook: [docs/AI.md](../../../docs/AI.md) (EN: [docs/en/AI.md](../../../docs/en/AI.md)). Read that file when you need detail beyond this skill.

## Mental model

```
.g → lpg-v2.3.0 -table → *prs*/*sym*/AST → link runtime/* → parse with your tokens
```

Generator ≠ runtime ≠ lexer. Calculator intentionally injects hand-built tokens.

## Quick path

1. **Obtain binary**: `npx lpg2`, or build `lpg2/` → `export LPG_BIN=…/lpg-v2.3.0`, or Release `bin/lpg-v2.3.0`.
2. **Init runtime** for the target language, e.g. `git submodule update --init runtime/lpg-runtime`.
3. **Generate** (source tree must pass templates):

```bash
TEMPLATES="$REPO/lpg-generator-templates-2.1.00"
"$LPG_BIN" -programming_language=java -table -quiet \
  -template="$TEMPLATES/templates/java/dtParserTemplateF.gi" \
  -include-directory="$TEMPLATES/include/java" \
  -out_directory=./out \
  grammar.g
```

Prefer mirroring `examples/calculator/scripts/generate.sh` over inventing flags.

4. **Smoke**: `./examples/calculator/scripts/run.sh java` (or `cpp|rust|typescript|go|python|csharp|dart|all`).
5. **Check before write**: `-nowrite`. **CI**: add `-fail_on_conflicts`. Exit **12** = error; conflict warnings alone still exit **0**.

## Language → runtime

| `-programming_language=` | Submodule |
|--------------------------|-----------|
| `java` | `runtime/lpg-runtime` |
| `cpp` / `c++` / `rt_cpp` | `runtime/LPG-cpp-runtime` |
| `typescript` | `runtime/LPG-typescript-runtime` |
| `python3` | `runtime/LPG-python-runtime` |
| `go` | `runtime/LPG-go-runtime` |
| `csharp` | `runtime/LPG-csharp-runtime` |
| `dart` | `runtime/LPG-Dart-runtime` |
| `rust` | `runtime/LPG-rust-runtime` |

Do **not** use removed values: `python2`, `c`, `ml`, `plx`, `plxasm`, `xml`.

## Grammar skeleton

```text
%Options automatic_ast=nested,var=nt,visitor=default
%Options template=dtParserTemplateF.gi
%Terminals … %End
%Eof EOF_TOKEN %End
%Start Expr %End
%Rules
    Expr$Expr ::= …
%End
```

Action blocks: `/.` … `./`. Reference: `examples/calculator/calculator.g`.

## Task routing

| Intent | Docs |
|--------|------|
| Author / integrate grammar | `docs/USER.md`, `docs/GRAMMAR_REFERENCE.md` |
| First-time smoke | `docs/QUICKSTART.md` |
| Change generator / bootstrap | `docs/DEVELOPER.md`, `lpg2/BOOTSTRAP.md` |
| Versions / release | `docs/ECOSYSTEM.md`, `ecosystem/compat.json` |

## Hard rules

- Empty `runtime/` → `git submodule update --init …`, not “missing code”.
- After `-table`, still need runtime + token stream + parse driver.
- Do not edit committed `lpg2/src/jikespg_*` without following `lpg2/BOOTSTRAP.md`.
- Prefer existing calculator scripts over one-off generate commands.
