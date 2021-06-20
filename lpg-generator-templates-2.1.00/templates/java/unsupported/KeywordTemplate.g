--
-- An LPG Lexer Template Using lpg.jar
--
-- An instance of this template must have a $Export section and the export_terminals option
-- There must be only one non-terminal, the start symbol, for the keywords
-- The action for each keyword should be a call to $setResult(terminal_symbol)
--
-- Macro that must be defined in an instance of this template:
--
--     $eof_char
--
-- Macros that may be redefined in an instance of this template
--
--     $import_classes
--     $action_class
--     $token_kind_map
--
-- B E G I N N I N G   O F   T E M P L A T E   KeywordTemplate
--
%Options programming_language=java,margin=8
%Options table
%options action-block=("*.java", "/.", "./")
%options ParseTable=lpg.runtime.ParseTable

%Notice /.$copyright./

%Define

        $copyright /../
        $import_classes /../
        $action_class /.$action_type./
        $ast_class /.$ast_type./

--
-- Macros that may be needed in an instance of this template
--
        $setResult /.keywordKind[$rule_number] = ./
--
-- Macros useful for specifying actions
--
        $Header
        /.
                //
                // Rule $rule_number:  $rule_text
                //./

        $BeginAction /.$Header./

        $EndAction /../
%End

%Headers
        /.
        $copyright
        $import_classes
        import lpg.runtime.*;

        public class $action_class extends $prs_type implements $exp_type
        {
            private char[] inputChars;
            private final int keywordKind[] = new int[$num_rules + 1];

            public int[] getKeywordKinds() { return keywordKind; }

            public int lexer(int curtok, int lasttok)
            {
                int current_kind = $token_kind_map.getKind(inputChars[curtok]),
                    act;

                for (act = tAction(START_STATE, current_kind);
                     act > NUM_RULES && act < ACCEPT_ACTION;
                     act = tAction(act, current_kind))
                {
                    curtok++;
                    current_kind =
                        (curtok > lasttok
                                ? $token_kind_map.EOF
                                : $token_kind_map.getKind(inputChars[curtok]));
                }

                if (act > ERROR_ACTION)
                {
                    curtok++;
                    act -= ERROR_ACTION;
                }

                return keywordKind[act == ERROR_ACTION  || curtok <= lasttok ? 0 : act];
            }

            public $action_class(char[] inputChars, int identifierKind)
            {
                this.inputChars = inputChars;
                keywordKind[0] = identifierKind;

        ./
%End

%Trailers
        /.

                for (int i = 0; i < keywordKind.length; i++)
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
