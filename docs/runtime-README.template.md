# Runtime README template

Copy this structure into each language runtime repository README.

```markdown
# LPG-<lang>-runtime

<one-line role>

## Install / coordinates

- Package: <npm|nuget|maven|cargo|pypi|go|pub|cmake>
- Version: <x.y.z>
- Compatible LPG2 generator: >= 2.3.0 (see A-LPG/LPG2 `ecosystem/compat.json`)

## Minimum toolchain

- <language version / SDK>

## Build and test

```bash
<commands>
```

## Wiring generated files

1. Generate with LPG2 (`-programming_language=<lang> -table -template=…`)
2. Add generated `*prs*` / `*sym*` / parser sources to your project
3. Depend on this runtime as shown below

```bash
# example dependency snippet
```

## Features

| Feature | Status |
|---------|--------|
| Deterministic parser | yes/no |
| Backtracking | yes/no |
| Nested automatic AST | yes/no |
| `%Recover` prosthetic AST | yes/no |

## Publish status

- Channel: <url>
- Automation: manual | CI workflow | planned

## Links

- Generator: https://github.com/A-LPG/LPG2
- Ecosystem matrix: https://github.com/A-LPG/LPG2/blob/main/docs/ECOSYSTEM.md
```
