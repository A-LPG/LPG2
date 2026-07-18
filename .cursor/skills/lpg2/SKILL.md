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

## Ambiguity routing

Apply in order, rerunning `-nowrite -fail_on_conflicts` after each change.
With `-glr`, retained conflicts are handled rather than fatal, but remain in
diagnostics and `conflict_count`:

1. Operator precedence only → layer `Expr` / `Term` / `Factor`; if rules must stay flat, use `%Left` / `%Right`, then `%Priority` for explicit rule priority.
2. Fixed, bounded distinguishing prefix → use the smallest working `lalr=N`.
3. Contextual keyword versus identifier → declare keywords and enable `soft_keywords`.
4. Multiple paths must remain or lookahead is not usefully bounded → enable `backtrack`, use `btParserTemplateF.gi`, and verify runtime `BacktrackingParser` support.
5. Need all legal parse trees packed together → enable `-glr`, use `glrParserTemplateF.gi`, and Java or C++ runtime `GLRParser` (same-grammar-symbol/same-token-span `getNextAst()` forest; pure AST-building actions). v1: no error recovery/`DiagnoseParser`, no cyclic/epsilon-loop grammars; non-cyclic nullable rules work; other runtimes scaffolding only.
6. Otherwise inspect the listing state/lookahead and rewrite the minimal conflicting rules. Never accept conflicts just because they exit 0 by default.

For recovery, put only safely synthesizable nonterminals in `%Recover`; consumers must accept prosthetic AST placeholders. See [`examples/recover/`](../../../examples/recover/).

Structured diagnostics: `--diagnostics=json` / `-diagnostics=json` emits one JSON object on stdout (`schema_version`, `diagnostics[]`, `health`). Use with `-nowrite -quiet`; pair `-fail_on_conflicts` when conflicts must be errors. See `docs/AI.md` §5.3 / `docs/en/AI.md` §5.3.

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
