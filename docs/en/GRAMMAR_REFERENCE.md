# LPG2 Grammar Reference (summary)

Chinese edition is authoritative: [../GRAMMAR_REFERENCE.md](../GRAMMAR_REFERENCE.md).

**Beginners:** [CONCEPTS.md](CONCEPTS.md) → [tutorial.md](tutorial.md) → then this page / the Chinese full reference.

## Essentials

- Files: `.g` / `.lpg` / `.gi`; sections `%Name` … `%End`; action delimiters `/.` … `./`
- Core sections: `%Terminals`, `%Eof`, `%Start`, `%Rules`, `%Import`, `%DropActions`, `%Left`/`%Right`, `%Recover`
- AST: `%Options automatic_ast=nested,visitor=default` + `template=dtParserTemplateF.gi`
- Conflicts: warnings by default; use `-fail_on_conflicts` in CI (exit 12);
  GLR conflicts are handled and therefore exempt
- Backtrack: `-backtrack` + `btParserTemplateF.gi`
- Recover: `%Recover Sym /. expr ./` for prosthetic AST factories; covered by `*_automatic_ast_recover`
- CLI: `-programming_language=`, `-table`, `-out_directory=`, `-template=`, `-include-directory=`, `-help`
- Minimal example: [`examples/calculator/calculator.g`](../../examples/calculator/calculator.g)

## GLR (Java)

Use `-glr` with `glrParserTemplateF.gi` and Java runtime `GLRParser` when every
legal parse must be retained. LPG emits the same multi-action conflict encoding
as backtracking plus `isGLR()`. AST alternatives for the same grammar symbol
and token-index span are canonicalized and linked through `IAst.getNextAst()`,
forming a packed local parse forest. Packing assumes generated or otherwise
side-effect-free AST-building actions; distinct non-AST values are not merged.

GLR v1's `glrParserTemplateF.gi` does not wire `DiagnoseParser` (the
deterministic/backtracking diagnostic repair helper). Generator `%Recover`
prosthetic AST factories may still be emitted, but the GLR driver itself does
not perform recover replay. Cyclic/epsilon-loop grammars are rejected, while
non-cyclic nullable rules are supported. Raw `GLRParser.parse*()` throws
`BadParseException`; generated `parser()` / `parseX()` methods catch it and
return `null`.

Intentional GLR conflicts do not make `-glr -fail_on_conflicts` fail and do not
make `health.healthy` false; `conflict_count` still reports them. Coverage:
`glr_tables_golden_java`, `java_glr_ambiguous_e2e`,
`java_glr_entry_e2e`, `java_glr_rr_epsilon_e2e`,
`java_glr_correlation_e2e`, and `java_glr_symbol_identity_e2e`.
All eight full backends also have `glr_*_smoke` generation coverage for table
and `nextAst` scaffolding output.

## Tutorial

- English walkthrough: [tutorial.md](tutorial.md)
- Commands only: [../../examples/calculator/README.md](../../examples/calculator/README.md)
  (`./scripts/run.sh java|typescript|cpp|rust`)

## Compatibility

Backend/runtime matrix: [../ECOSYSTEM.md](../ECOSYSTEM.md).
