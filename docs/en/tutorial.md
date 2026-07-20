# Tutorial: a calculator grammar from scratch

Build a minimal integer-expression parser with LPG2. Drivers exist for **Java / TypeScript / C++ / Rust**. Files live under [`examples/calculator/`](../../examples/calculator/).

Chinese edition: [../tutorial.md](../tutorial.md). Concepts: [CONCEPTS.md](CONCEPTS.md). Not running yet? [QUICKSTART.md](QUICKSTART.md).

**One-shot (recommended)** after `LPG_BIN` and the matching runtime submodule are ready:

```bash
# from repo root; Java is the default walkthrough (needs a JDK)
./examples/calculator/scripts/run.sh java
# or: typescript | cpp | rust | all
```

## 0. Goal and layout

| Path | Role |
|------|------|
| `examples/calculator/calculator.g` | Shared grammar |
| `examples/calculator/scripts/generate.sh` | Generate tables only |
| `examples/calculator/scripts/run.sh` | Generate + build + accept/reject |
| `examples/calculator/{java,typescript,cpp,rust}/` | Language drivers |

Success: accept `NUMBER + NUMBER * NUMBER`; reject a sequence that starts with `PLUS`.

## 1. Read `calculator.g`

Source: [`examples/calculator/calculator.g`](../../examples/calculator/calculator.g).

### Options

```text
%Options automatic_ast=nested,var=nt,visitor=default
%Options template=dtParserTemplateF.gi
%options verbose
%options package=Calculator
```

- `automatic_ast=nested` — nested AST nodes (class names from `$ClassName` on rules)
- `template=dtParserTemplateF.gi` — deterministic parser template (scripts also pass `-template=` per language)
- `package=Calculator` — package / namespace for generated code (language-dependent)

### Terminals and start/eof

```text
%Terminals
    NUMBER PLUS STAR LPAREN RPAREN
%End

%Eof
    EOF_TOKEN
%End

%Start
    Expr
%End
```

These names become `*sym.*` constants. Drivers must inject tokens with the **same numbers** (see each language `Main`).

### Layered rules (disambiguation)

```text
%Rules
    Expr$Expr ::= Expr PLUS Term | Term
    Term$Term ::= Term STAR Factor | Factor
    Factor$Factor ::= NUMBER | LPAREN Expr RPAREN
%End
```

- `Expr` handles `+`, `Term` handles `*`, `Factor` handles numbers and parentheses → `*` binds tighter than `+` with no shift/reduce conflict
- In `Expr$Expr`, the name after `$` is the automatic AST class name

### EBNF sugar (opt-in)

Calculator stays classic layered BNF so precedence teaching stays clear. For optional / list sugar, enable `%Options ebnf` (or `-ebnf`):

```text
%Options ebnf,automatic_ast=nested,var=nt,visitor=default
%Rules
    Call ::= ID LPAREN (Expr (COMMA Expr)*)? RPAREN
    Trailing ::= [SEMICOLON]    -- ISO optional
    Items ::= {Item}            -- ISO zero-or-more
%End
```

- Desugars to stable `__ebnf_*` auxiliaries before table generation; `*` / `{…}` get the same list AST shape as hand-written `List$$Elem`
- Quote or alias operator terminals (`'+'` / `PLUS`); bare `+` `*` `?` `( )` `[ ]` `{ }` are meta when `ebnf` is on
- Prefer explicit BNF when you need a custom list class name or non-list AST shape
- Runnable sample: [`examples/ebnf-call/`](../../examples/ebnf-call/) — see also [GRAMMAR_REFERENCE.md](../GRAMMAR_REFERENCE.md)

## 2. Analyze without writing

Confirm the grammar is clean (`LPG_BIN` = your binary):

```bash
"$LPG_BIN" -programming_language=java -table -nowrite -quiet \
  -template=lpg-generator-templates-2.1.00/templates/java/dtParserTemplateF.gi \
  -include-directory=lpg-generator-templates-2.1.00/include/java \
  examples/calculator/calculator.g
```

Or generate with the script (writes `out-*`):

```bash
./examples/calculator/scripts/generate.sh java
```

On `Shift/reduce conflict`, diagnostics usually include a source excerpt, caret `^`, `example lookahead: …`, and `= help:` hints. This sample should be conflict-free. Grammar/option errors exit **12**.

## 3. Generation and artifacts

```bash
./examples/calculator/scripts/generate.sh java|typescript|cpp|rust
```

Output dir: `examples/calculator/out-<lang>/`.

| Language | Script arg | Typical outputs |
|----------|------------|-----------------|
| Java | `java` | `calculator.java`, `calculatorprs.java`, `calculatorsym.java` |
| TypeScript | `typescript` | `calculator.ts`, `calculatorprs.ts`, `calculatorsym.ts` |
| C++ | `cpp` (uses `rt_cpp` internally) | `calculator*.h` / `*.cpp`, … |
| Rust | `rust` | `calculatorprs.rs`, `calculatorsym.rs`, … |

There is also a listing `*.l`. Failed runs do not overwrite good outputs.

## 4. Run one language (default: Java)

```bash
git submodule update --init runtime/lpg-runtime
export LPG_BIN=…   # Release binary or lpg2/build/lpg-v2.3.0
./examples/calculator/scripts/run.sh java
```

Other languages (see per-language READMEs):

| Language | Command | Submodule |
|----------|---------|-----------|
| TypeScript | `./examples/calculator/scripts/run.sh typescript` | `runtime/LPG-typescript-runtime` |
| C++ | `./examples/calculator/scripts/run.sh cpp` | `runtime/LPG-cpp-runtime` |
| Rust | `./examples/calculator/scripts/run.sh rust` | `runtime/LPG-rust-runtime` |

- [Java](../../examples/calculator/java/README.md)
- [TypeScript](../../examples/calculator/typescript/README.md)
- [C++](../../examples/calculator/cpp/README.md)
- [Rust](../../examples/calculator/rust/README.md)

## 5. What the driver does

Each language `Main` (or test) roughly:

1. Builds a list of **token kinds** (`NUMBER`, `PLUS`, `STAR`, …) — **no** full character lexer
2. Calls into the runtime with generated symbols + tables
3. Expects success on a valid sequence and failure when input starts with `PLUS`

This is intentional: prove tables + runtime. In a real project you write (or reuse) a lexer and feed tokens to the runtime.

## 6. Exercises

1. **Precedence declarations**: try `%Left PLUS` / `%Left STAR` (see [../GRAMMAR_REFERENCE.md](../GRAMMAR_REFERENCE.md)) versus layered nonterminals.
2. **See a conflict**: temporarily merge ambiguous `Expr`/`Term` rules; use `-nowrite` or drop `-quiet` and read the caret / `= help:` lines.
3. **Switch language**: run `run.sh typescript` on the same grammar and compare `out-*` suffixes.

Regenerate after grammar edits.

## 7. Troubleshooting

| Symptom | Likely cause | Fix |
|---------|--------------|-----|
| `Set LPG_BIN to lpg2 executable` | Generator not found | `export LPG_BIN=…` or build `lpg2/build` |
| Empty submodule dirs | Not initialized | `git submodule update --init runtime/…` |
| Missing template / include | Wrong cwd or path | Run scripts from repo root; keep `lpg-generator-templates-2.1.00/` |
| Exit code 12 | Grammar/option error (or conflicts with `-fail_on_conflicts`) | Read stderr; conflicts warn by default |
| Java `javac` fails | Missing runtime sources / old JDK | Check `runtime/lpg-runtime/src`; JDK 8+ |
| C++ link fails | Bad runtime path | Script sets `LPG2_CPP_RUNTIME_DIR`; init submodule |
| TS `npm` fails | Network or missing runtime | Init `LPG-typescript-runtime`, retry |

## 8. Next steps

- Integrate: [USER.md](USER.md)
- Directive cheat sheet: [../GRAMMAR_REFERENCE.md](../GRAMMAR_REFERENCE.md)
- More samples: `grammars-example/` / [LPG2-grammars-example](https://github.com/A-LPG/LPG2-grammars-example)
- Runtime versions: [../ECOSYSTEM.md](../ECOSYSTEM.md)
- Maintain the generator: [DEVELOPER.md](DEVELOPER.md)
