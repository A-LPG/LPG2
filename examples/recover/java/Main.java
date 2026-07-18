package Recover;

import lpg.runtime.*;

/** Demonstrates the typed prosthetic AST produced for malformed input. */
class StubLexStream extends LexStream {
    StubLexStream() {
        initialize(new char[] {'a', ' '}, "recover-example");
    }

    @Override
    public String[] orderedExportedSymbols() {
        return recoversym.orderedTerminalSymbols;
    }
}

public class Main {
    private static void seed(IPrsStream stream, int[] kinds) {
        stream.makeToken(0, 0, 0); // LPG streams reserve token zero.
        for (int i = 0; i < kinds.length; i++)
            stream.makeToken(i + 1, i + 1, kinds[i]);
        stream.setStreamLength(stream.getSize());
    }

    public static void main(String[] args) {
        StubLexStream lex = new StubLexStream();
        recover parser = new recover(lex);
        seed(parser.getIPrsStream(), new int[] {
            recoversym.TK_a,
            recoversym.TK_EOF_TOKEN,
        });

        // The input is "a", but S requires "a b c"; the ordinary parse rejects it.
        if (parser.parser() != null)
            throw new AssertionError("expected malformed input to be rejected");

        // Scope recovery represents a missing nonterminal with an ErrorToken, then
        // asks the generated %Recover factory for a type-correct prosthetic AST.
        IPrsStream stream = parser.getIPrsStream();
        IToken a = stream.getIToken(1);
        IToken eof = stream.getIToken(2);
        ParseTable table = parser.getParseTable();
        ProstheticAst[] factories = parser.getProstheticAst();
        recover.Missing missing = null;

        for (int kind = table.getNtOffset() + 1;
             kind <= table.getNtOffset() + table.getNumNonterminals();
             kind++) {
            int slot = table.getProsthesisIndex(kind);
            if (slot < 0 || slot >= factories.length || factories[slot] == null)
                continue;

            ErrorToken errorToken = new ErrorToken(
                a, a, eof, a.getStartOffset(), eof.getEndOffset(), kind);
            IAst prosthetic = factories[slot].create(errorToken);
            if (!(prosthetic instanceof recover.Missing))
                throw new AssertionError("expected a typed Missing prosthesis");
            missing = (recover.Missing) prosthetic;
            if (missing.getLeftIToken() != errorToken ||
                missing.getRightIToken() != errorToken)
                throw new AssertionError("Missing must wrap the recovery ErrorToken");
        }

        if (missing == null)
            throw new AssertionError("%Recover did not emit a Missing factory");

        System.out.println("bad input: a (expected a b c)");
        System.out.println("prosthetic AST: Missing");
        System.out.println("recover java: ok");
    }
}
