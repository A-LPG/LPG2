package EbnfCall;

import lpg.runtime.*;

/** Token-seeded accept/reject driver for the ebnf-call grammar. */
class StubLexStream extends LexStream {
    StubLexStream() {
        initialize(new char[]{'f', '(', 'x', ',', '1', ')', ';', ' '}, "ebnf-call");
    }

    @Override
    public String[] orderedExportedSymbols() {
        return callsym.orderedTerminalSymbols;
    }
}

public class Main {
    static void seed(IPrsStream stream, int[] kinds) {
        stream.makeToken(0, 0, 0);
        for (int i = 0; i < kinds.length; i++)
            stream.makeToken(i + 1, i + 1, kinds[i]);
        stream.setStreamLength(stream.getSize());
    }

    public static void main(String[] args) {
        // Accept: f(x,1);
        StubLexStream lex = new StubLexStream();
        call parser = new call(lex);
        seed(parser.getIPrsStream(), new int[] {
            callsym.TK_ID,
            callsym.TK_LPAREN,
            callsym.TK_ID,
            callsym.TK_COMMA,
            callsym.TK_NUMBER,
            callsym.TK_RPAREN,
            callsym.TK_SEMICOLON,
            callsym.TK_EOF_TOKEN,
        });
        call.Ast root = parser.parser();
        if (root == null) {
            System.err.println("expected successful parse of f(x,1);");
            System.exit(2);
        }

        // Accept: g()  (no args, optional semicolon omitted)
        StubLexStream lex2 = new StubLexStream();
        call parser2 = new call(lex2);
        seed(parser2.getIPrsStream(), new int[] {
            callsym.TK_ID,
            callsym.TK_LPAREN,
            callsym.TK_RPAREN,
            callsym.TK_EOF_TOKEN,
        });
        if (parser2.parser() == null) {
            System.err.println("expected successful parse of g()");
            System.exit(3);
        }

        // Reject: leading COMMA
        StubLexStream lex3 = new StubLexStream();
        call parser3 = new call(lex3);
        seed(parser3.getIPrsStream(), new int[] {
            callsym.TK_COMMA,
            callsym.TK_EOF_TOKEN,
        });
        if (parser3.parser() != null) {
            System.err.println("expected reject of leading COMMA");
            System.exit(4);
        }
        System.out.println("ebnf-call java: ok");
    }
}
