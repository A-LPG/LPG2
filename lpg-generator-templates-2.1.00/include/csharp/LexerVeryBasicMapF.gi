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
            public int[] getKeywordKinds() { return null; }

            public int lexer(int curtok, int lasttok) { return 0; }

            public void setInputChars(char[] inputChars) { }

             int getKind(int c) { return 0; }

            public NoKWLexer(char[] inputChars, int identifierKind) { }
        }
    ./
%End

%Import
    LexerBasicMapF.gi
%End
