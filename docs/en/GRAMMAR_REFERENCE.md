# LPG2 Grammar Reference (summary)

Full Chinese edition: [../GRAMMAR_REFERENCE.md](../GRAMMAR_REFERENCE.md). This English page is the default summary for global readers.

**Beginners:** [CONCEPTS.md](CONCEPTS.md) → [tutorial.md](tutorial.md) → then this page / the Chinese full reference.

## Essentials

- Files: `.g` / `.lpg` / `.gi`; sections `%Name` … `%End`; action delimiters `/.` … `./`
- Core sections: `%Terminals`, `%Eof`, `%Start`, `%Rules`, `%Import`, `%DropActions`, `%Left`/`%Right`, `%Recover`
- AST: `%Options automatic_ast=nested,visitor=default` + `template=dtParserTemplateF.gi`
- Conflicts: warnings by default; use `-fail_on_conflicts` in CI (exit 12);
  GLR conflicts are handled and therefore exempt
- Backtrack: `-backtrack` + `btParserTemplateF.gi`
- Recover: `%Recover Sym /. expr ./` for prosthetic AST factories; covered by `*_automatic_ast_recover`
- EBNF sugar (opt-in): `%Options ebnf` / `-ebnf` enables postfix `?` `*` `+`, `(...)` groups,
  ISO `[X]`/`{X}`, group actions, and quantifier field macros (`X*$Foo` / `X$Foo*`);
  default remains classic BNF so bare `+`/`*` terminals stay valid. Desugars to `__ebnf_*`
  auxiliaries before table generation. See Chinese reference for expansion table and limits.
  Dogfood example: [`examples/ebnf-call/`](../../examples/ebnf-call/).
- CLI: `-programming_language=`, `-table`, `-out_directory=`, `-template=`, `-include-directory=`, `-help`
- Minimal example: [`examples/calculator/calculator.g`](../../examples/calculator/calculator.g)

## GLR (Java / C++ / TypeScript)

Use `-glr` with `glrParserTemplateF.gi` and the Java, C++, or TypeScript
runtime `GLRParser` when every legal parse must be retained. Templates live
under `templates/java/`, `templates/rt_cpp/`, and `templates/typescript/`.
LPG emits the same multi-action conflict encoding as backtracking plus
`isGLR()`. **GLR v2** uses a graph-structured stack (`GssNode`/`GssEdge`) with
prefix sharing and builds a shared packed parse forest (`SppfNode`). Compatible
clients still walk `IAst.getNextAst()` forests (same grammar symbol and
token-index span); true sharing is exposed via `GLRParser.getSppfRoot()` /
`getSppfSymbolCount()`. Packing assumes side-effect-free AST-building actions;
distinct non-AST values are not merged. Playground WASM can generate with
`-glr`; the in-browser forest demo is TypeScript only.

With `error_repair_count>0`, a failed Java/C++ GLR drive falls back to
`BacktrackingParser.fuzzyParse*` (same `RecoveryParser` + `%Recover` prosthesis
as bt; single repaired tree, not a `nextAst` forest). If repair still fails,
the template runs `DiagnoseParser` and returns `null`. `error_repair_count==0`
skips repair. Cyclic/epsilon-loop grammars are rejected; non-cyclic nullable
rules are supported. Raw `GLRParser.parse*()` throws `BadParseException`;
generated `parser()` / `parseX()` catch it and return `null` (C++ nullptr).

Intentional GLR conflicts do not make `-glr -fail_on_conflicts` fail and do not
make `health.healthy` false; `conflict_count` still reports them. Coverage:
`glr_tables_golden_java`, `java_glr_ambiguous_e2e` (Catalan + SPPF share),
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
