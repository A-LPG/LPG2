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
        
            def getKeywordKinds(self) ->list: return None 

            def lexer(self,curtok : int, lasttok : int)-> int : return 0 

            def setInputChars(self,inputChars : str) : 
                pass  

            def getKind(self,c : int) -> int : return 0 

            def __init__(self,inputChars : str,  identifierKind : int) :
                super().__init__()
        
    ./
%End

%Import
    LexerBasicMapF.gi
%End
