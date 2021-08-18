--
-- An LPG Lexer Template Using lpg.jar
--
-- An instance of this template must have a $Export section and the export_terminals option
-- There must be only one non-terminal, the start symbol, for the keywords
-- The action for each keyword should be a call to $setResult(terminal_symbol)
--
-- Macro that may be redefined in an instance of this template
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
    import { $prs_type } from ".\/$prs_type";
    import { $sym_type } from ".\/$sym_type";
    import { $exp_type } from ".\/$exp_type";
    ./
%End

--
-- This template requires that the name of the EOF token be set
-- to EOF and that the prefix be "Char_" to be consistent with
-- LexerTemplateD.
--
$Eof
    EOF
%End

%Define
    --
    -- Macro that may be respecified in an instance of this template
    --
    $eof_char /.$sym_type$.$prefix%EOF$suffix$./

    --
    -- Macros useful for specifying actions
    --
    $setResult /.self.keywordKind[$rule_number] = ./

    $Header
    /.
            //
            // Rule $rule_number:  $rule_text
            //
    ./

    $BeginAction /.$Header./

    $EndAction /../

    $BeginJava /.$BeginAction./

    $EndJava /.$EndAction./
%End


%Headers
    /.
    class $action_type extends $prs_type
    {
         inputChars : string;
           keywordKind  : number[] =  Array($num_rules + 1);

          getKeywordKinds() : number[] { return self.keywordKind; }

          lexer(curtok : number, lasttok : number) : number
        {
            let current_kind = $action_type.getKind(self.inputChars.charCodeAt(curtok)),
                act;

            for (act = self.tAction(self.START_STATE, current_kind);
                 act > self.NUM_RULES && act < self.ACCEPT_ACTION;
                 act = self.tAction(act, current_kind))
            {
                curtok++;
                current_kind = (curtok > lasttok
                                       ? $eof_char
                                       : $action_type.getKind(self.inputChars.charCodeAt(curtok)));
            }

            if (act > self.ERROR_ACTION)
            {
                curtok++;
                act -= self.ERROR_ACTION;
            }

            return self.keywordKind[act == self.ERROR_ACTION  || curtok <= lasttok ? 0 : act];
        }

         setInputChars(inputChars : string ) : void  { self.inputChars = inputChars; }

    ./
%End

%Rules
    /.

        def __init__(self, inputChars : string,  identifierKind : number)
        {
            super();
            self.inputChars = inputChars;
            self.keywordKind[0] = identifierKind;
    ./
%End

%Trailers
    /.
            for (let i : number = 0; i < self.keywordKind.length; i++)
            {
                if (self.keywordKind[i] == 0)
                    self.keywordKind[i] = identifierKind;
            }
        }
    }
    ./
%End

--
-- E N D   O F   T E M P L A T E
--
