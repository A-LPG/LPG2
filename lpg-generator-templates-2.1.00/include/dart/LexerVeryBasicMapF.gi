%Define
    $kw_lexer_class /.NoKWLexer./
    $_IDENTIFIER /.0./
%End
%Headers
    --
    -- Additional methods for the action class not provided in the template
    --
    /.
        class NoKWLexer
        {
            List<int> getKeywordKinds(){ return []; }

            int lexer(int curtok ,int lasttok ){ return 0; }

            void setInputChars(String inputChars){ }

            int getKind(int c){ return 0; }

            NoKWLexer(String inputChars, int  identifierKind) { }
        }
    ./
%End

%Import
    LexerBasicMapF.gi
%End
