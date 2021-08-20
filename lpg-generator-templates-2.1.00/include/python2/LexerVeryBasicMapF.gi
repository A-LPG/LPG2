%Define
    $kw_lexer_class /.NoKWLexer./
    $_IDENTIFIER /.0./
%End
%Headers
    --
    -- Additional methods for the action class not provided in the template
    --
    /.
        class NoKWLexer(object):
        
            def getKeywordKinds(self) : return None 

            def lexer(self,curtok , lasttok ) : return 0 

            def setInputChars(self,inputChars ) : 
                pass  

            def getKind(self,c )  : return 0 

            def __init__(self,inputChars ,  identifierKind ) :
                super().__init__()
        
    ./
%End

%Import
    LexerBasicMapF.gi
%End
