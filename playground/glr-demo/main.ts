import { LexStream, IPrsStream, GLRParser, IAst } from "lpg2ts";
import { glr_expr } from "./glr_expr";
import { glr_exprsym } from "./glr_exprsym";

class StubLex extends LexStream {
  constructor() {
    super("glr-playground", "n+n+n+n+n+n+n+n ");
  }
  orderedExportedSymbols(): string[] {
    return glr_exprsym.orderedTerminalSymbols as unknown as string[];
  }
}

function seed(stream: IPrsStream, kinds: number[]) {
  stream.makeToken(0, 0, 0);
  for (let i = 0; i < kinds.length; i++) stream.makeToken(i + 1, i + 1, kinds[i]);
  stream.setStreamLength(stream.getSize());
}

function countAlts(n: IAst | null): number {
  let c = 0;
  for (let p = n; p != null; p = p.getNextAst()) c++;
  return c;
}

function expressionKinds(operands: number): number[] {
  const kinds: number[] = [];
  for (let n = 0; n < operands; n++) {
    kinds.push(glr_exprsym.TK_NUMBER);
    if (n + 1 < operands) kinds.push(glr_exprsym.TK_PLUS);
  }
  kinds.push(glr_exprsym.TK_EOF_TOKEN);
  return kinds;
}

export type GlrDemoResult = {
  operands: number;
  rootAlts: number;
  sppfSymbols: number;
  ok: boolean;
  error?: string;
};

export function runGlrDemo(operands: number): GlrDemoResult {
  try {
    const lex = new StubLex();
    const parser = new glr_expr(lex as any);
    seed(parser.getIPrsStream(), expressionKinds(operands));
    const root = parser.parser();
    if (!root) return { operands, rootAlts: 0, sppfSymbols: 0, ok: false, error: "null root" };
    const glr = parser.getParser() as GLRParser;
    return {
      operands,
      rootAlts: countAlts(root),
      sppfSymbols: glr.getSppfSymbolCount(),
      ok: true,
    };
  } catch (e: any) {
    return { operands, rootAlts: 0, sppfSymbols: 0, ok: false, error: String(e && e.message ? e.message : e) };
  }
}

export function runGlrDemoSuite(): GlrDemoResult[] {
  return [1, 2, 3, 4, 5, 6, 7, 8].map(runGlrDemo);
}
