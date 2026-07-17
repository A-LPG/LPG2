# Security Policy

## Supported versions

Security fixes are considered for:

- The latest **LPG2 generator** release on [GitHub Releases](https://github.com/A-LPG/LPG2/releases)
- Runtime libraries pinned in [`ecosystem/compat.json`](ecosystem/compat.json) for that generator version

Older generator/runtime combinations may not receive backports.

## Reporting a vulnerability

Please **do not** open a public GitHub issue for security-sensitive reports.

1. Email the maintainers via the contact listed on the [A-LPG organization](https://github.com/A-LPG) profile, **or**
2. Open a private security advisory on the affected repository (GitHub Security Advisories) if available.

Include:

- Affected component (generator / runtime language / VS Code extension / language server)
- Version or git commit
- Reproduction steps or proof-of-concept (non-destructive)
- Impact assessment (crash, code execution in generated parsers, supply-chain, etc.)

We aim to acknowledge reports within 14 days.

## Scope notes

- LPG2 generates parser tables and action stubs; **host-language action blocks are user code** and are outside the generator’s sandbox.
- Untrusted grammars should be treated like untrusted source: generate and compile in isolated environments.
- Please do not include exploit payloads that target third-party systems.
