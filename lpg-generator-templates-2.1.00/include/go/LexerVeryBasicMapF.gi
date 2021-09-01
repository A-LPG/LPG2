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

func (self *$NoKWLexer)  GetKeywordKinds() []int { 
    return nil 
}

func (self *$NoKWLexer)  lexer(curtok int, lasttok int) int{ 
    return 0
}

func (self *$NoKWLexer)  setInputChars(inputChars *string) { 
}

func (self *$NoKWLexer)  GetKind(c int) int {
    return 0;
}
        
./
%End

%Import
    LexerBasicMapF.gi
%End
