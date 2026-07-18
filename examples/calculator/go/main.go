package main

import (
	"fmt"
	"os"

	. "github.com/A-LPG/LPG-go-runtime/lpg2"
)

type stubLex struct {
	*LexStream
}

func newStubLex() (*stubLex, error) {
	s := &stubLex{}
	ls, err := NewLexStreamExt(s, "calculator", []rune("1+2*3 "), 1, nil)
	if err != nil {
		return nil, err
	}
	s.LexStream = ls
	return s, nil
}

func (s *stubLex) OrderedExportedSymbols() []string {
	return calculatorsym.OrderedTerminalSymbols
}

func seed(stream IPrsStream, kinds []int) {
	stream.MakeToken(0, 0, 0)
	for i, k := range kinds {
		stream.MakeToken(i+1, i+1, k)
	}
	stream.SetStreamLength(stream.GetSize())
}

func main() {
	lex, err := newStubLex()
	if err != nil {
		os.Exit(10)
	}
	parser, err := Newcalculator(lex)
	if err != nil {
		os.Exit(11)
	}
	seed(parser.GetIPrsStream(), []int{
		calculatorsym.TK_NUMBER,
		calculatorsym.TK_PLUS,
		calculatorsym.TK_NUMBER,
		calculatorsym.TK_STAR,
		calculatorsym.TK_NUMBER,
		calculatorsym.TK_EOF_TOKEN,
	})
	root, err := parser.Parser()
	if err != nil || root == nil {
		fmt.Fprintln(os.Stderr, "expected successful parse of 1+2*3")
		os.Exit(2)
	}

	lex2, err := newStubLex()
	if err != nil {
		os.Exit(12)
	}
	parser2, err := Newcalculator(lex2)
	if err != nil {
		os.Exit(13)
	}
	seed(parser2.GetIPrsStream(), []int{
		calculatorsym.TK_PLUS,
		calculatorsym.TK_EOF_TOKEN,
	})
	root2, err2 := parser2.Parser()
	if err2 == nil && root2 != nil {
		fmt.Fprintln(os.Stderr, "expected reject of leading PLUS")
		os.Exit(3)
	}
	fmt.Println("calculator go: ok")
}
