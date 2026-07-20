# LPG2 — AI / Agent Playbook

For **AI coding agents** (Cursor, Copilot, Claude Code, etc.). Human readers: start at [QUICKSTART.md](QUICKSTART.md). 中文: [../AI.md](../AI.md).

**After this page you can:** obtain the generator → write/edit `.g` → generate tables → link a runtime → verify. Do not treat the generator as a full lexer/parser framework.

## 0. Mental model

```
.g grammar  --(lpg-v2.3.0 -table)-->  tables + actions/AST  --(link runtime)-->  parse works
```

| Piece | Does | Does not |
|------|------|----------|
| **Generator** `lpg-v2.3.0` | Read grammar; emit tables + AST/actions | Provide a full production lexer; act as runtime |
| **Templates** `lpg-generator-templates-2.1.00/` | Shape generated code | — |
| **Runtime** `runtime/*` (submodules) | Table-driven parse, backtrack, recover | Read `.g` |
| **Your code** | Token stream, drive `parse`, consume AST | — |

## 1. Task routing

| User intent | Do this | Canonical docs |
|-------------|---------|----------------|
| Write/edit grammar, generate, integrate | Follow §2–§5 | [USER.md](USER.md), [GRAMMAR_REFERENCE.md](GRAMMAR_REFERENCE.md) |
| Smoke-test the toolchain | `./examples/calculator/scripts/run.sh <lang>` | [QUICKSTART.md](QUICKSTART.md) |
| Change generator C++ / bootstrap / new backend | Build `lpg2/`, run ctest | [DEVELOPER.md](DEVELOPER.md), `lpg2/BOOTSTRAP.md` |
| Runtime versions / release | Read compat | [ECOSYSTEM.md](../ECOSYSTEM.md), `ecosystem/compat.json` |

## 2. Obtain the generator

```bash
# A. Fastest
npx lpg2 --help
npx lpg2 -programming_language=java -table your.g

# Scaffold / Antlr import / smoke (wrapper subcommands)
npx lpg2 init ./my-parser --lang=java
npx lpg2 from-antlr Expr.g4 -o ./out-expr   # needs LPG2 checkout (antlr2lpg.py)
npx lpg2 test java

# B. Build from this repo
cd lpg2 && cmake -S . -B build && cmake --build build -j
export LPG_BIN="$PWD/build/lpg-v2.3.0"
# or: ./scripts/lpg2 …

# C. GitHub Release binary → export LPG_BIN=.../bin/lpg-v2.3.0
```

**Shortest generate** (CMake build or install layout auto-discovers templates; default `dtParserTemplateF.gi` when unset):

```bash
"$LPG_BIN" -programming_language=java -table -quiet -out_directory=./out grammar.g
# analysis only: add --dry-run (or -nowrite)
```

You can still pass `-template` / `-include-directory` explicitly. If you move the binary out of the install/`share/lpg2` layout, set `LPG2_RESOURCE_ROOT` / `LPG_TEMPLATE` / `LPG_INCLUDE`.

## 3. Standard workflow

```
- [ ] 1. Init the runtime submodule for the target language
- [ ] 2. Write/edit `.g` (skeleton below)
- [ ] 3. `-nowrite` to check conflicts/errors
- [ ] 4. `-table` into the project source tree
- [ ] 5. Link runtime; provide tokens + call parse
- [ ] 6. Run accept/reject or project tests
```

### Minimal `.g`

```text
%Options automatic_ast=nested,var=nt,visitor=default
%Options template=dtParserTemplateF.gi
%options package=MyLang

%Terminals
    ID NUMBER PLUS
%End

%Eof
    EOF_TOKEN
%End

%Start
    Expr
%End

%Rules
    Expr$Expr ::= Expr PLUS NUMBER
           | NUMBER
%End
```

Reference: `examples/calculator/calculator.g`.

### CLI essentials

| Flag | Purpose |
|------|---------|
| `-programming_language=<lang>` | Target backend (§4) |
| `-table` | Emit tables |
| `-out_directory=<dir>` | Output for tables/actions/`.l` |
| `-nowrite` / `--dry-run` | Analyze only (aliases) |
| `-fail_on_conflicts` | Conflicts → exit 12 (**CI**) |
| `-ebnf` | Opt-in postfix EBNF sugar (`?` `*` `+`, groups, `[ ]`/`{ }`, group actions, quantifier field macros); default off |

Exit **0** = success (conflict warnings alone still 0 unless `-fail_on_conflicts`); **12** = grammar/option error. Failed runs do not overwrite prior outputs.

### Smoke test

```bash
git submodule update --init runtime/lpg-runtime
./examples/calculator/scripts/run.sh java   # or cpp|rust|typescript|go|python|csharp|dart|all
```

## 4. Language matrix

| CLI | Runtime submodule | Notes |
|-----|-------------------|-------|
| `java` | `runtime/lpg-runtime` | Good first choice; includes `GLRParser` (`-glr` + `glrParserTemplateF.gi`) |
| `cpp` / `c++` / `rt_cpp` | `runtime/LPG-cpp-runtime` | Aliases; includes `GLRParser` (`-glr` + `rt_cpp/glrParserTemplateF.gi`) |
| `typescript` | `runtime/LPG-typescript-runtime` | Includes `GLRParser` (`-glr` + `templates/typescript/glrParserTemplateF.gi`); Playground browser demo |
| `python3` | `runtime/LPG-python-runtime` | Not `python2` |
| `go` | `runtime/LPG-go-runtime` | |
| `csharp` | `runtime/LPG-csharp-runtime` | |
| `dart` | `runtime/LPG-Dart-runtime` | |
| `rust` | `runtime/LPG-rust-runtime` | nested + toplevel AST, recover, GLR v2 |

Removed: `c` / `ml` / `plx` / `plxasm` / `xml` / `python2`. Pins: `ecosystem/compat.json`.

## 5. Grammar pitfalls

- Blocks: `%Name` … `%End`; action delimiters `/.` … `./` must close
- `Nonterminal$ClassName` names AST classes when `automatic_ast=nested`
- With `-ebnf` / `%Options ebnf`, bare `+` `*` `?` `( )` `[ ]` `{ }` are meta; quote or alias operator terminals; see `examples/ebnf-call/`
- IDE: [LPG-language-server](https://github.com/A-LPG/LPG-language-server) + [lpg-vscode](https://github.com/A-LPG/LPG-VScode) parse EBNF as a **sugar AST** (highlight/outline/nav); they do not run the generator’s `EbnfExpander` / `__ebnf_*` conflict analysis
- Conflicts warn by default (exit 0); use the decision tree below
- **You usually write the lexer**; calculator injects hand-built tokens on purpose
- C++ incremental = token-prefix reuse + statement reparse, **not** tree-sitter subtree reuse

### 5.1 Ambiguity decision tree

Decide in this order; rerun `-nowrite -fail_on_conflicts` after each change.
GLR is the exception: `-glr -fail_on_conflicts` permits conflicts retained by
the GLR table while diagnostics still report their count:

1. **Only operator precedence/associativity?**
   - The AST should expose precedence levels → layer `Expr` / `Term` / `Factor` (clearest and the default choice).
   - The grammar should stay flat → use `%Left` / `%Right`; use `%Priority` when associativity declarations are insufficient and a rule priority must be explicit.
2. **Can a fixed number of following tokens distinguish the forms?** → set the smallest working `lalr=N`. This is for bounded, multi-token lookahead; do not keep increasing `N` to hide inherent ambiguity.
3. **May a keyword also be an identifier in selected contexts?** → declare the keywords and enable `soft_keywords`. This handles contextual keyword/identifier overlap, not general conflicts.
4. **Must the language retain multiple candidate paths, or is the common prefix not usefully bounded?** → enable `backtrack`, switch the template to `btParserTemplateF.gi`, and confirm the target runtime provides `BacktrackingParser`. Backtracking has runtime cost, so try rewriting, precedence, and bounded lookahead first.
5. **Need to keep all legal parse trees (ambiguity packing)?** → enable `-glr`, switch to `glrParserTemplateF.gi`, and use Java / C++ / TypeScript runtime `GLRParser` (v2: GSS + SPPF); alternatives for the same grammar symbol and token-index span are projected via `getNextAst()`; true sharing is on `getSppfRoot()`. Packing targets side-effect-free AST-building actions. On Java/C++, `parser(N)` with `N>0` falls back to BT `%Recover` prosthesis (single tree) after GLR failure; cyclic/epsilon-loop grammars are rejected (non-cyclic nullable rules work); other language runtimes still have `nextAst` scaffolding only. Playground WASM can generate; in-browser parse demo is TypeScript only.
6. **Conflict remains?** → inspect the listing state/lookahead and minimize the offending rules; do not ignore warnings merely because the default exit code is 0.

### 5.2 `%Recover`

Put only safely synthesizable nonterminals in `%Recover`. Recovery can produce prosthetic AST nodes, so consumers must tolerate placeholder nodes/tokens. See the runnable patterns in [`examples/recover/`](../../examples/recover/).

### 5.3 Structured diagnostics (`--diagnostics=json`)

Machine-readable entry point: `--diagnostics=json` (also `-diagnostics=json`). Human-readable diagnostics remain the default. In JSON mode:

- **stdout**: exactly one JSON object (optional trailing newline); no human prose and no generated source
- **stderr**: empty for normal runs (JSON mode suppresses most `***ERROR` banners)
- Prefer `-nowrite -quiet` for pre-release health checks; with `-fail_on_conflicts`, conflicts appear as errors in `diagnostics[]`

| Field | Meaning |
|-------|---------|
| `schema_version` | Currently `1` |
| `diagnostics[]` | `file` / `span{start,end: line,column,offset}` / `code` / `severity` / `message` / `help`; conflicts may add `conflict_kind`, `example_lookahead` |
| `health` | `available` / `healthy` / `conflict_count` / SR+RR counts / `backtrack` / `glr` / `soft_keywords` / `soft_conflicts` / `recover_symbols[]` / `programming_language` / `write_enabled` / `warning_summary` |

Common codes: `LPG0001` error, `LPG0002` warning, `LPG1001` unclosed action block, `LPG2001` shift/reduce, `LPG2002` reduce/reduce, `LPG2003` fail_on_conflicts.

## 6. Repo map

| Path | Role |
|------|------|
| `lpg2/` | Generator sources |
| `lpg-generator-templates-2.1.00/` | Templates (in main repo) |
| `runtime/*` | Runtimes (**git submodules**) |
| `examples/calculator/` | End-to-end starter |
| `grammars-example/` | More samples (submodule) |
| `docs/AI.md` | Chinese AI playbook (canonical for zh) |
| `.cursor/skills/lpg2/` | Cursor project skill |

## 7. Anti-patterns

1. Claiming parse works after generating `*prs*` without runtime + tokens
2. Expecting a full industrial lexer from the generator
3. Using removed language values
4. Moving the binary out of install/`share/lpg2` without `LPG2_RESOURCE_ROOT` / `LPG_TEMPLATE` (CMake builds usually auto-discover templates)
5. Treating conflict warnings as hard failure (unless `-fail_on_conflicts`)
6. Editing `src/jikespg_*` without `lpg2/BOOTSTRAP.md`
7. Empty `runtime/` without `git submodule update --init`

## 8. Progressive reading

[QUICKSTART](QUICKSTART.md) → [CONCEPTS](CONCEPTS.md) → [tutorial](tutorial.md) → [USER](USER.md) → [GRAMMAR_REFERENCE](GRAMMAR_REFERENCE.md) → [DEVELOPER](DEVELOPER.md) / [ECOSYSTEM](../ECOSYSTEM.md)
