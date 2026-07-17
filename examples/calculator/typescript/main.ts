import { LexStream, IPrsStream } from "lpg2ts";
import { calculator } from "./generated/calculator";
import { calculatorsym } from "./generated/calculatorsym";

class StubLexStream extends LexStream {
  constructor() {
    super("calculator", "1+2*3 ");
  }
  orderedExportedSymbols(): string[] {
    return calculatorsym.orderedTerminalSymbols;
  }
}

function seed(stream: IPrsStream, kinds: number[]): void {
  stream.makeToken(0, 0, 0);
  kinds.forEach((k, i) => stream.makeToken(i + 1, i + 1, k));
  stream.setStreamLength(stream.getSize());
}

function main(): void {
  const lex = new StubLexStream();
  const parser = new calculator(lex);
  seed(parser.getIPrsStream(), [
    calculatorsym.TK_NUMBER,
    calculatorsym.TK_PLUS,
    calculatorsym.TK_NUMBER,
    calculatorsym.TK_STAR,
    calculatorsym.TK_NUMBER,
    calculatorsym.TK_EOF_TOKEN,
  ]);
  const root = parser.parser();
  if (!root) {
    console.error("expected successful parse of 1+2*3");
    process.exit(2);
  }

  const lex2 = new StubLexStream();
  const parser2 = new calculator(lex2);
  seed(parser2.getIPrsStream(), [
    calculatorsym.TK_PLUS,
    calculatorsym.TK_EOF_TOKEN,
  ]);
  if (parser2.parser() != null) {
    console.error("expected reject of leading PLUS");
    process.exit(3);
  }
  console.log("calculator typescript: ok");
}

main();
