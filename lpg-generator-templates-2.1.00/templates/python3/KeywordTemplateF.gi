--
-- An LPG Lexer Template Using lpg.jar
--
-- An instance of this template must have a $Export section and the export_terminals option
-- There must be only one non-terminal, the start symbol, for the keywords
-- The action for each keyword should be a call to $setResult(terminal_symbol)
--
-- Macro that may be redefined in an instance of self template
--
--     $eof_char
--
-- B E G I N N I N G   O F   T E M P L A T E   KeywordTemplateF (Similar to KeywordTemplateD)
--
%options programming_Language=python3,margin=4
%options table
%options action-block=("*.py", "/.", "./")
%options ParseTable=ParseTable
%options prefix=Char_
%options single-productions



%Globals
/.
from $prs_type    import  $prs_type  
from $sym_type    import  $sym_type  
from $exp_type    import  $exp_type  
./
%End

--
-- This template requires that the name of the EOF token be set
-- to EOF and that the prefix be "Char_" to be consistent with
-- LexerTemplateD.
--
%Eof
    EOF
%End

%Define
    --
    -- Macro that may be respecified in an instance of self template
    --
    $eof_char /.$sym_type$.$prefix$EOF$suffix$./

    --
    -- Macros useful for specifying actions
    --
    $setResult /.self.keywordKind[$rule_number] = ./

    $Header
    /.
            #
            # Rule $rule_number:  $rule_text
            #
    ./

    $BeginAction /.$Header./

    $EndAction /../

    $BeginJava /.$BeginAction./

    $EndJava /.$EndAction./
%End


%Headers
    /.
    class $action_type($prs_type):
    
        def getKeywordKinds(self) ->list :  return self.keywordKind 

        def lexer(self,curtok : int, lasttok : int) -> int :
    
            current_kind = $action_type.getKind(self.inputChars.charCodeAt(curtok))
                    
            act = self.tAction(self.START_STATE, current_kind)
            while (act > self.NUM_RULES and act < self.ACCEPT_ACTION):
                curtok+=1
                current_kind = ( $eof_char if curtok > lasttok else 
                                $action_type.getKind(self.inputChars.charCodeAt(curtok)))
                act = self.tAction(act, current_kind)

            if (act > self.ERROR_ACTION):
            
                curtok+=1
                act -= self.ERROR_ACTION
            

            return self.keywordKind[act == self.ERROR_ACTION  or (0 if curtok <= lasttok  else  act) ]
    

        def setInputChars(self,inputChars : str ) :   self.inputChars = inputChars 

    ./
%End

%Rules
    /.

        def __init__(self, inputChars : str,  identifierKind : int):
        
            super().__init__()
            self.inputChars : str = None
            self.keywordKind  : list  =  [0]*($num_rules + 1)
            self.inputChars = inputChars
            self.keywordKind[0] = identifierKind
    ./
%End

%Trailers
    /.
            for i in range(0, len(self.keywordKind)):
                if (self.keywordKind[i] == 0):
                    self.keywordKind[i] = identifierKind
            
    ./
%End

--
-- E N D   O F   T E M P L A T E
--
