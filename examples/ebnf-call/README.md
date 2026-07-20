# EBNF call example

Opt-in EBNF dogfood for `%Options ebnf`: optional args, comma-separated lists, and ISO `[SEMICOLON]`.

Calculator remains classic layered BNF for precedence teaching; this sample shows sugar.

**Reference:** [docs/en/GRAMMAR_REFERENCE.md](../../docs/en/GRAMMAR_REFERENCE.md) · [docs/GRAMMAR_REFERENCE.md](../../docs/GRAMMAR_REFERENCE.md)

## Run (Java)

```bash
export LPG_BIN=./lpg2/build/lpg-v2.3.0   # or your Release binary
git submodule update --init runtime/lpg-runtime
./examples/ebnf-call/scripts/run.sh
```

Accepts `f(x,1);` and `g()`; rejects a leading `COMMA`.
