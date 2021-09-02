%Define
    $kw_lexer_class /.NoKWLexer./
    $_IDENTIFIER /.0./
%End
%Headers
    --
    -- Additional methods for the action class not provided in the template
    --
/.
type NoKWLexer struct{

}

func (my *$NoKWLexer)  GetKeywordKinds() []int { 
    return nil 
}

func (my *$NoKWLexer)  Lexer(curtok int, lasttok int) int{ 
    return 0
}

func (my *$NoKWLexer)  SetInputChars(inputChars []rune) { 
}

func (my *$NoKWLexer)  GetKind(c rune) int {
    return 0;
}
        
./
%End

%Import
    LexerBasicMapF.gi
%End
