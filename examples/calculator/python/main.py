#!/usr/bin/env python3
"""Token-seeded accept/reject driver for the calculator grammar."""
import sys
from pathlib import Path

# Runtime + generated modules are injected onto sys.path by run.sh.
from lpg2 import LexStream
from calculator import calculator
from calculatorsym import calculatorsym as sym


class StubLexStream(LexStream):
    def __init__(self):
        super().__init__("calculator", "1+2*3 ")

    def orderedExportedSymbols(self):
        return sym.orderedTerminalSymbols


def seed(stream, kinds):
    stream.makeToken(0, 0, 0)
    for i, k in enumerate(kinds):
        stream.makeToken(i + 1, i + 1, k)
    stream.setStreamLength(stream.getSize())


def main():
    lex = StubLexStream()
    parser = calculator(lex)
    seed(
        parser.getIPrsStream(),
        [
            sym.TK_NUMBER,
            sym.TK_PLUS,
            sym.TK_NUMBER,
            sym.TK_STAR,
            sym.TK_NUMBER,
            sym.TK_EOF_TOKEN,
        ],
    )
    root = parser.parser()
    if root is None:
        print("expected successful parse of 1+2*3", file=sys.stderr)
        sys.exit(2)

    lex2 = StubLexStream()
    parser2 = calculator(lex2)
    seed(parser2.getIPrsStream(), [sym.TK_PLUS, sym.TK_EOF_TOKEN])
    if parser2.parser() is not None:
        print("expected reject of leading PLUS", file=sys.stderr)
        sys.exit(3)
    print("calculator python: ok")


if __name__ == "__main__":
    main()
