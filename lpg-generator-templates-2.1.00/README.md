# LPG generator templates (`lpg-generator-templates-2.1.00`)

Templates and include maps that shape code emitted by `lpg-v2.3.0` / `npx lpg2`.
Installed under `share/lpg2/lpg-generator-templates-2.1.00/` next to the binary.

## Layout

| Path | Role |
|------|------|
| `templates/<lang>/` | Parser / lexer / keyword driver templates (`*.gi`) |
| `include/<lang>/` | Character / keyword maps (`%Import` / `%Include`) |
| `docs/` | Historical notes (see legacy warning below) |
| `legacy/` | Removed language assets kept for reference only |

## Supported languages

| `-programming_language=` | Template dir | Notes |
|--------------------------|--------------|-------|
| `java` | `templates/java` | Full suite including GLR |
| `cpp` / `c++` / `rt_cpp` | `templates/rt_cpp` | Maps to `rt_cpp` |
| `csharp` | `templates/csharp` | Includes UTF-8 + unified |
| `typescript` | `templates/typescript` | |
| `python3` | `templates/python3` | |
| `go` | `templates/go` | |
| `dart` | `templates/dart` | |
| `rust` | `templates/rust` | |

Removed CLI values (`python2`, `c`, `ml`, …) are not supported. Former `python2`
templates/includes live under [`legacy/python2/`](legacy/python2/).

## Template selection

| Kind | Typical file | When |
|------|--------------|------|
| Deterministic | `dtParserTemplateF.gi` | Default LALR / calculator |
| Backtracking | `btParserTemplateF.gi` | `-backtrack` / soft keywords |
| GLR | `glrParserTemplateF.gi` | **Java only**; must pass explicitly |
| Lexer | `LexerTemplateF.gi` | Character scanner |
| Keyword | `KeywordTemplateF.gi` | Keyword filter |
| UTF-8 lexer | `Utf8LexerTemplateF.gi` | java / csharp / rt_cpp |
| Unified | `dtUnifiedTemplateF.gi` | java / csharp |

### GLR (Java)

`-glr` forces backtrack-style conflict tables and `isGLR()`, but **does not**
auto-select a template. Use:

```bash
-programming_language=java \
-template="$TEMPLATES/templates/java/glrParserTemplateF.gi" \
-include-directory="$TEMPLATES/include/java"
```

GLR: Java/C++/TypeScript/C# ship `glrParserTemplateF.gi` + runtime `GLRParser`
(GSS/SPPF). Other languages have table/AST `nextAst` scaffolding only
(see `docs/TODO_TRIAGE.md`).

## Include maps

Standard six-map suite (where present):

- `LexerBasicMapF.gi`, `LexerVeryBasicMapF.gi`, `LexerUnicodeMapF.gi`
- `KWLexerMapF.gi`, `KWLexerLowerCaseMapF.gi`, `KWLexerFoldedCaseMapF.gi`

`rt_cpp` also ships `Utf8LexerBasicMapF.gi`. Java UTF-8 grammars typically define
`$super_stream_class` / `getKind` in the grammar (no separate Java UTF-8 map yet).

## Unsupported / legacy

- `templates/java/unsupported/` and `include/java/unsupported/` — old A–E `.g`
  variants; **unmaintained**, do not use for new work.
- `docs/Ast.txt` — historical Java AST notes; prefer
  [docs/GRAMMAR_REFERENCE.md](../docs/GRAMMAR_REFERENCE.md) and
  [docs/AI.md](../docs/AI.md).

## Usage from a source checkout

```bash
TEMPLATES="$REPO/lpg-generator-templates-2.1.00"
"$LPG_BIN" -programming_language=java -table -quiet \
  -template="$TEMPLATES/templates/java/dtParserTemplateF.gi" \
  -include-directory="$TEMPLATES/include/java" \
  -out_directory=./out \
  grammar.g
```

See `examples/calculator/scripts/generate.sh` for the supported language matrix.
