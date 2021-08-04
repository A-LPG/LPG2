%Define
    $kw_lexer_class /.NoKWLexer./
    $_IDENTIFIER /.0./
%End
%Headers
    --
    -- Additional methods for the action class not provided in the template
    --
    /.
         public class NoKWLexer
        {
            public  getKeywordKinds() :number[]{ return null; }

            public  lexer(curtok : number, lasttok : number): number { return 0; }

            public  setInputChars(inputChars : string) : void{ }

            public  getKind(c : number) : number{ return 0; }

            public NoKWLexer(inputChars : string,  identifierKind : number) { }
        }
    ./
%End

%Import
    LexerBasicMapF.gi
%End
