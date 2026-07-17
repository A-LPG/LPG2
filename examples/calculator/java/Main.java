package Calculator;

import lpg.runtime.*;

/** Token-seeded accept/reject driver for the calculator grammar. */
class StubLexStream extends LexStream {
    StubLexStream() {
        initialize(new char[]{'1', '+', '2', '*', '3', ' '}, "calculator");
    }

    @Override
    public String[] orderedExportedSymbols() {
        return calculatorsym.orderedTerminalSymbols;
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
        StubLexStream lex = new StubLexStream();
        calculator parser = new calculator(lex);
        seed(parser.getIPrsStream(), new int[] {
            calculatorsym.TK_NUMBER,
            calculatorsym.TK_PLUS,
            calculatorsym.TK_NUMBER,
            calculatorsym.TK_STAR,
            calculatorsym.TK_NUMBER,
            calculatorsym.TK_EOF_TOKEN,
        });
        calculator.Ast root = parser.parser();
        if (root == null) {
            System.err.println("expected successful parse of 1+2*3");
            System.exit(2);
        }

        StubLexStream lex2 = new StubLexStream();
        calculator parser2 = new calculator(lex2);
        seed(parser2.getIPrsStream(), new int[] {
            calculatorsym.TK_PLUS,
            calculatorsym.TK_EOF_TOKEN,
        });
        if (parser2.parser() != null) {
            System.err.println("expected reject of leading PLUS");
            System.exit(3);
        }
        System.out.println("calculator java: ok");
    }
}
