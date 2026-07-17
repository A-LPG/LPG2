# LPG2 Grammar Reference (summary)

Chinese edition is authoritative: [../GRAMMAR_REFERENCE.md](../GRAMMAR_REFERENCE.md).

**Beginners:** [CONCEPTS.md](CONCEPTS.md) → [tutorial.md](tutorial.md) → then this page / the Chinese full reference.

## Essentials

- Files: `.g` / `.lpg` / `.gi`; sections `%Name` … `%End`; action delimiters `/.` … `./`
- Core sections: `%Terminals`, `%Eof`, `%Start`, `%Rules`, `%Import`, `%DropActions`, `%Left`/`%Right`, `%Recover`
- AST: `%Options automatic_ast=nested,visitor=default` + `template=dtParserTemplateF.gi`
- Conflicts: warnings by default; use `-fail_on_conflicts` in CI (exit 12)
- Recover: `%Recover Sym /. expr ./` for prosthetic AST factories; covered by `*_automatic_ast_recover`
- CLI: `-programming_language=`, `-table`, `-out_directory=`, `-template=`, `-include-directory=`, `-help`
- Minimal example: [`examples/calculator/calculator.g`](../../examples/calculator/calculator.g)

## Tutorial

- English walkthrough: [tutorial.md](tutorial.md)
- Commands only: [../../examples/calculator/README.md](../../examples/calculator/README.md)
  (`./scripts/run.sh java|typescript|cpp|rust`)

## Compatibility

Backend/runtime matrix: [../ECOSYSTEM.md](../ECOSYSTEM.md).
