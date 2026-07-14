--
-- An LPG Lexer Template Using lpg.jar
--
-- An instance of my template must have a $Export section and the export_terminals option
-- There must be only one non-terminal, the start symbol, for the keywords
-- The action for each keyword should be a call to $setResult(terminal_symbol)
--
-- Macro that may be redefined in an instance of my template
--
--     $eof_char
--
-- B E G I N N I N G   O F   T E M P L A T E   KeywordTemplateF (Similar to KeywordTemplateD)
--
%Options programming_language=rust,margin=4
%Options table
%options action-block=("*.rs", "/.", "./")
%options ParseTable=ParseTable
%Options prefix=Char_
%Options single-productions

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
    -- Macro that may be respecified in an instance of my template
    --
    $eof_char /.$sym_type::$prefix$EOF$suffix$./

    --
    -- Macros useful for specifying actions
    --
    $setResult /.my.keyword_kind[$rule_number as usize] = ./

    $Header
    /.
            //
            // Rule $rule_number:  $rule_text
            //
            ./

    $BeginAction /.$Header./

    $EndAction /. ;./

    $BeginJava /.$BeginAction./

    $EndJava /.$EndAction./
%End

%Globals
    /.
    ./
%End

%Headers
    /.
    pub struct $action_type {
        prs: $prs_type,
        input_chars: Vec<char>,
        keyword_kind: Vec<i32>,
    }

    impl $action_type {
        pub fn get_keyword_kinds(&self) -> &[i32] {
            &self.keyword_kind
        }

        pub fn lexer(&self, mut curtok: i32, lasttok: i32) -> i32 {
            let mut current_kind = self.get_kind(self.input_chars[curtok as usize]);
            let mut act = self.prs.t_action($prs_type$_START_STATE, current_kind);
            while act > $prs_type$_NUM_RULES && act < $prs_type$_ACCEPT_ACTION {
                curtok += 1;
                current_kind = if curtok > lasttok {
                    $eof_char
                } else {
                    self.get_kind(self.input_chars[curtok as usize])
                };
                act = self.prs.t_action(act, current_kind);
            }

            if act > $prs_type$_ERROR_ACTION {
                curtok += 1;
                act -= $prs_type$_ERROR_ACTION;
            }

            if act == $prs_type$_ERROR_ACTION || curtok <= lasttok {
                self.keyword_kind[0]
            } else {
                self.keyword_kind[act as usize]
            }
        }

        pub fn set_input_chars(&mut self, input_chars: Vec<char>) {
            self.input_chars = input_chars;
        }
    ./
%End

%Rules
    /.

        pub fn new(input_chars: Vec<char>, identifier_kind: i32) -> Self {
            let mut keyword_kind = vec![0; ($num_rules + 1) as usize];
            keyword_kind[0] = identifier_kind;
            let mut my = Self {
                prs: $prs_type::new(),
                input_chars,
                keyword_kind,
            };
    ./
%End

%Trailers
    /.
            for i in 0..my.keyword_kind.len() {
                if my.keyword_kind[i] == 0 {
                    my.keyword_kind[i] = identifier_kind;
                }
            }
            my
        }
    }
    ./
%End

--
-- E N D   O F   T E M P L A T E
--
