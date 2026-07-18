import 'dart:io';

import 'package:lpg2/lpg2.dart';

import '../generated/calculator.dart';
import '../generated/calculatorsym.dart';

class StubLexStream extends LexStream {
  StubLexStream() : super('calculator', '1+2*3 ');

  @override
  List<String> orderedExportedSymbols() {
    return calculatorsym.orderedTerminalSymbols;
  }
}

void seed(IPrsStream stream, List<int> kinds) {
  stream.makeToken(0, 0, 0);
  for (var i = 0; i < kinds.length; i++) {
    stream.makeToken(i + 1, i + 1, kinds[i]);
  }
  stream.setStreamLength(stream.getSize());
}

void main() {
  final lex = StubLexStream();
  final parser = calculator(lex);
  seed(parser.getIPrsStream(), [
    calculatorsym.TK_NUMBER,
    calculatorsym.TK_PLUS,
    calculatorsym.TK_NUMBER,
    calculatorsym.TK_STAR,
    calculatorsym.TK_NUMBER,
    calculatorsym.TK_EOF_TOKEN,
  ]);
  final root = parser.parser();
  if (root == null) {
    stderr.writeln('expected successful parse of 1+2*3');
    exit(2);
  }

  final lex2 = StubLexStream();
  final parser2 = calculator(lex2);
  seed(parser2.getIPrsStream(), [
    calculatorsym.TK_PLUS,
    calculatorsym.TK_EOF_TOKEN,
  ]);
  if (parser2.parser() != null) {
    stderr.writeln('expected reject of leading PLUS');
    exit(3);
  }
  print('calculator dart: ok');
}
