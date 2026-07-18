# `%Recover` cookbook (Java)

This example shows how LPG2 turns a recoverable nonterminal into a prosthetic
AST node. The grammar expects `a b c`; the driver supplies malformed input
containing only `a`.

`Missing` is a concrete generated AST type used by `Root`, so its `%Recover`
entry provides an allocation expression:

```text
%Recover
    Missing /. new Missing(error_token, error_token) ./
%End
```

The parser first diagnoses the missing `Missing` scope. The driver then follows
the same generated factory handoff used by runtime recovery: it maps the
nonterminal through `getProsthesisIndex`, passes an `ErrorToken` to the factory,
and verifies that the result is a type-correct `Missing` node whose source span
is the error token.

## Run

From the repository root:

```bash
git submodule update --init runtime/lpg-runtime
export LPG_BIN="$PWD/lpg2/build/lpg-v2.3.0" # or another lpg-v2.3.0 binary
./examples/recover/scripts/run.sh
```

The script generates the Java parser, compiles it with `runtime/lpg-runtime`,
and ends with:

```text
bad input: a (expected a b c)
prosthetic AST: Missing
recover java: ok
```

Generated files go to `out-java/` and compiled classes to `java/classes/`.
Neither directory is source-controlled.

## When the action can be omitted

If downstream code accepts a generic token-shaped placeholder, use:

```text
%Recover
    Missing
%End
```

LPG2 then allocates its default `AstToken(error_token)`. Use an explicit
allocation, as this cookbook does, when a parent field, rule action, visitor, or
other consumer requires a specific generated AST type.
