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
| Runtime versions / release | Read compat | [ECOSYSTEM.md](ECOSYSTEM.md), `ecosystem/compat.json` |

## 2. Obtain the generator

```bash
# A. Fastest
npx lpg2 --help
npx lpg2 -programming_language=java -table your.g

# B. Build from this repo
cd lpg2 && cmake -S . -B build && cmake --build build -j
export LPG_BIN="$PWD/build/lpg-v2.3.0"

# C. GitHub Release binary → export LPG_BIN=.../bin/lpg-v2.3.0
```

In a source tree, **always** pass templates (see calculator scripts):

```bash
TEMPLATES="$REPO/lpg-generator-templates-2.1.00"
"$LPG_BIN" -programming_language=java -table -quiet \
  -template="$TEMPLATES/templates/java/dtParserTemplateF.gi" \
  -include-directory="$TEMPLATES/include/java" \
  -out_directory=./out \
  grammar.g
```

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
| `-nowrite` | Analyze only |
| `-fail_on_conflicts` | Conflicts → exit 12 (**CI**) |

Exit **0** = success (conflict warnings alone still 0 unless `-fail_on_conflicts`); **12** = grammar/option error. Failed runs do not overwrite prior outputs.

### Smoke test

```bash
git submodule update --init runtime/lpg-runtime
./examples/calculator/scripts/run.sh java   # or cpp|rust|typescript|go|python|csharp|dart|all
```

## 4. Language matrix

| CLI | Runtime submodule | Notes |
|-----|-------------------|-------|
| `java` | `runtime/lpg-runtime` | Good first choice |
| `cpp` / `c++` / `rt_cpp` | `runtime/LPG-cpp-runtime` | Equivalent |
| `typescript` | `runtime/LPG-typescript-runtime` | |
| `python3` | `runtime/LPG-python-runtime` | Not `python2` |
| `go` | `runtime/LPG-go-runtime` | |
| `csharp` | `runtime/LPG-csharp-runtime` | |
| `dart` | `runtime/LPG-Dart-runtime` | |
| `rust` | `runtime/LPG-rust-runtime` | nested AST + recover; not full toplevel/GLR parity |

Removed: `c` / `ml` / `plx` / `plxasm` / `xml` / `python2`. Pins: `ecosystem/compat.json`.

## 5. Grammar pitfalls

- Blocks: `%Name` … `%End`; action delimiters `/.` … `./` must close
- `Nonterminal$ClassName` names AST classes when `automatic_ast=nested`
- Conflicts warn by default (exit 0); use `%Left`/`%Right`/`%Priority` or Expr/Term/Factor layering
- `%Recover` prosthetic AST supported on all eight AST backends
- **You usually write the lexer**; calculator injects hand-built tokens on purpose
- C++ incremental = token-prefix reuse + statement reparse, **not** tree-sitter subtree reuse

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
4. Forgetting `-template` / `-include-directory` in a source checkout
5. Treating conflict warnings as hard failure (unless `-fail_on_conflicts`)
6. Editing `src/jikespg_*` without `lpg2/BOOTSTRAP.md`
7. Empty `runtime/` without `git submodule update --init`

## 8. Progressive reading

[QUICKSTART](QUICKSTART.md) → [CONCEPTS](CONCEPTS.md) → [tutorial](tutorial.md) → [USER](USER.md) → [GRAMMAR_REFERENCE](GRAMMAR_REFERENCE.md) → [DEVELOPER](DEVELOPER.md) / [ECOSYSTEM](ECOSYSTEM.md)
