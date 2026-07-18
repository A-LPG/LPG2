using System;
using LPG2.Runtime;

namespace Calculator
{
    class StubLexStream : LexStream
    {
        public StubLexStream()
            : base(new char[] { '1', '+', '2', '*', '3', ' ' }, "calculator")
        {
        }

        public override string[] orderedExportedSymbols()
        {
            return calculatorsym.orderedTerminalSymbols;
        }
    }

    public class Program
    {
        static void Seed(IPrsStream stream, int[] kinds)
        {
            stream.makeToken(0, 0, 0);
            for (int i = 0; i < kinds.Length; i++)
                stream.makeToken(i + 1, i + 1, kinds[i]);
            stream.setStreamLength(stream.getSize());
        }

        public static int Main(string[] args)
        {
            StubLexStream lex = new StubLexStream();
            calculator parser = new calculator(lex);
            Seed(parser.getIPrsStream(), new int[] {
                calculatorsym.TK_NUMBER,
                calculatorsym.TK_PLUS,
                calculatorsym.TK_NUMBER,
                calculatorsym.TK_STAR,
                calculatorsym.TK_NUMBER,
                calculatorsym.TK_EOF_TOKEN,
            });
            calculator.Ast root = parser.parser();
            if (root == null)
            {
                Console.Error.WriteLine("expected successful parse of 1+2*3");
                return 2;
            }

            StubLexStream lex2 = new StubLexStream();
            calculator parser2 = new calculator(lex2);
            Seed(parser2.getIPrsStream(), new int[] {
                calculatorsym.TK_PLUS,
                calculatorsym.TK_EOF_TOKEN,
            });
            if (parser2.parser() != null)
            {
                Console.Error.WriteLine("expected reject of leading PLUS");
                return 3;
            }
            Console.WriteLine("calculator csharp: ok");
            return 0;
        }
    }
}
