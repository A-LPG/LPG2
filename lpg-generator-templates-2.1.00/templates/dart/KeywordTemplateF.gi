--
-- An LPG Lexer Template Using lpg.jar
--
-- An instance of this template must have a %Export section and the export_terminals option
-- There must be only one non-terminal, the start symbol, for the keywords
-- The action for each keyword should be a call to %setResult(terminal_symbol)
--
-- Macro that may be redefined in an instance of this template
--
--     %eof_char
--
-- B E G I N N I N G   O F   T E M P L A T E   KeywordTemplateF (Similar to KeywordTemplateD)
--
%Options programming_Language=dart,margin=4
%Options table
%options action-block=("*.dart", "/.", "./")
%options ParseTable=ParseTable
%Options prefix=Char_
%Options single-productions



%Globals
    /.
    import '%prs_type.dart';
    import '%sym_type.dart';
    import '%exp_type.dart';
    
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
    -- Macro that may be respecified in an instance of this template
    --
    $eof_char /.%sym_type%.%prefix%EOF%suffix%./

    --
    -- Macros useful for specifying actions
    --
    $setResult /.keywordKind[%rule_number] = ./

    $Header
    /.
            //
            // Rule %rule_number:  %rule_text
            //
    ./

    $BeginAction /.%Header./

    $EndAction /../

    $BeginJava /.%BeginAction./

    $EndJava /.%EndAction./
%End


%Headers
    /.
    class %action_type extends %prs_type
    {
        late String inputChars;
        List<int> keywordKind  = List<int>.filled(%num_rules + 1,0);

        List<int>  getKeywordKinds()  { return keywordKind; }

        int lexer(int curtok, int lasttok)  
        {
            var current_kind = %action_type.getKind(inputChars.codeUnitAt(curtok)),
                act=0;

            for (act = tAction(%prs_type.START_STATE, current_kind);
                 act > %prs_type.NUM_RULES && act < %prs_type.ACCEPT_ACTION;
                 act = tAction(act, current_kind))
            {
                curtok++;
                current_kind = (curtok > lasttok
                                       ? %eof_char
                                       : %action_type.getKind(inputChars.codeUnitAt(curtok)));
            }

            if (act > %prs_type.ERROR_ACTION)
            {
                curtok++;
                act -= %prs_type.ERROR_ACTION;
            }

            return keywordKind[ (act == %prs_type.ERROR_ACTION  || curtok <= lasttok) ? 0 : act];
        }

        void setInputChars(String inputChars)  { this.inputChars = inputChars; }

    ./
%End

%Rules
    /.

        %action_type(String inputChars, int identifierKind)
        {
            this.inputChars = inputChars;
            keywordKind[0] = identifierKind;
    ./
%End

%Trailers
    /.
            for (var i = 0; i < keywordKind.length; i++)
            {
                if (keywordKind[i] == 0)
                    keywordKind[i] = identifierKind;
            }
        }
    }
    ./
%End

--
-- E N D   O F   T E M P L A T E
--
