%Define
    $kw_lexer_class /.NoKWLexer./
    $_IDENTIFIER /.0./
%End
%Headers
    --
    -- Additional methods for the action class not provided in the template
    --
    /.
        pub struct NoKWLexer;

        impl NoKWLexer {
            pub fn new(_input_chars: Vec<char>, _identifier_kind: i32) -> Self {
                Self
            }

            pub fn get_keyword_kinds(&self) -> &[i32] {
                &[]
            }

            pub fn lexer(&self, _curtok: i32, _lasttok: i32) -> i32 {
                0
            }

            pub fn set_input_chars(&mut self, _input_chars: Vec<char>) {}

            pub fn get_kind(&self, _c: char) -> i32 {
                0
            }
        }
    ./
%End

%Import
    LexerBasicMapF.gi
%End
