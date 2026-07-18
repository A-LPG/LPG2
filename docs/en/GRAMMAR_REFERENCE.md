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

## GLR (Java / C++)

Use `-glr` with `glrParserTemplateF.gi` and the Java or C++ runtime `GLRParser`
when every legal parse must be retained. LPG emits the same multi-action
conflict encoding as backtracking plus `isGLR()`. AST alternatives for the same
grammar symbol and token-index span are canonicalized and linked through
`IAst.getNextAst()`, forming a packed local parse forest. Packing assumes
generated or otherwise side-effect-free AST-building actions; distinct non-AST
values are not merged.

With `error_repair_count>0`, a failed GLR drive falls back to
`BacktrackingParser.fuzzyParse*` (same `RecoveryParser` + `%Recover` prosthesis
as bt; single repaired tree, not a `nextAst` forest). If repair still fails,
the template runs `DiagnoseParser` and returns `null`. `error_repair_count==0`
skips repair. Cyclic/epsilon-loop grammars are rejected; non-cyclic nullable
rules are supported. Raw `GLRParser.parse*()` throws `BadParseException`;
generated `parser()` / `parseX()` catch it and return `null` (C++ nullptr).

Intentional GLR conflicts do not make `-glr -fail_on_conflicts` fail and do not
make `health.healthy` false; `conflict_count` still reports them. Coverage:
`glr_tables_golden_java`, `java_glr_ambiguous_e2e`,
`java_glr_entry_e2e`, `java_glr_rr_epsilon_e2e`,
`java_glr_correlation_e2e`, `java_glr_symbol_identity_e2e`,
`java_glr_cyclic_e2e`, `java_glr_non_ast_e2e`, `java_glr_recover_e2e`,
`cpp_glr_ambiguous_e2e`, and `cpp_glr_recover_e2e`.
All eight full backends also have `glr_*_smoke` generation coverage for table
and `nextAst` scaffolding output.

## Tutorial

- English walkthrough: [tutorial.md](tutorial.md)
- Commands only: [../../examples/calculator/README.md](../../examples/calculator/README.md)
  (`./scripts/run.sh java|typescript|cpp|rust`)

## Compatibility

Backend/runtime matrix: [../ECOSYSTEM.md](../ECOSYSTEM.md).
