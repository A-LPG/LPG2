--
-- An instance of my template must have a $Export section and the export_terminals option
--
-- Macros that may be redefined in an instance of my template
--
--     $eof_token
--     $additional_interfaces
--     $super_stream_class -- subclass lpg.runtime.LpgLexStream (or language runtime equivalent) for GetKind
--     $prs_stream_class -- use /.PrsStream./ if not subclassing
--     $super_class
--
-- B E G I N N I N G   O F   T E M P L A T E   LexerTemplateF
--
%Options programming_language=rust,margin=4
%Options table
%options action-block=("*.rs", "/.", "./")
%options ParseTable=ParseTable
%Options prefix=Char_

--
-- This template requires that the name of the EOF token be set
-- to EOF and that the prefix be "Char_" to be consistent with
-- KeywordTemplateD.
--
%Eof
    EOF
%End

--
-- This template also requires that the name of the parser EOF
-- Token to be exported be set to EOF_TOKEN
--
%Export
    EOF_TOKEN
%End

%Define
    --
    -- Macros that are be needed in an instance of my template
    --
    $eof_token /.$_EOF_TOKEN./
    
    $additional_interfaces /../
    $super_stream_class /.$file_prefix$LpgLexStream./
    $prs_stream_class /.PrsStreamRef./
    $super_class /../

    $prs_stream /. // macro prs_stream is deprecated. Use function get_prs_stream
                  self.get_prs_stream()./
    $setSym1 /. // macro setSym1 is deprecated. Use function set_result
               self.lex_parser.set_sym1./
    $setResult /. // macro setResult is deprecated. Use function set_result
                 self.lex_parser.set_sym1./
    $getSym /. // macro getSym is deprecated. Use function get_last_token
              self.lex_parser.get_sym./
    $getToken /. // macro getToken is deprecated. Use function get_token
                self.lex_parser.get_token./
    $getLeftSpan /. // macro getLeftSpan is deprecated. Use function get_left_span
                   self.lex_parser.get_first_token./
    $getRightSpan /. // macro getRightSpan is deprecated. Use function get_right_span
                    self.lex_parser.get_last_token./

    --
    -- Macros useful for specifying actions
    --
    $Header
    /.
                //
                // Rule $rule_number:  $rule_text
                //
                ./

    $DefaultAction
    /.$Header $rule_number => { ./

    $BeginAction /.$DefaultAction./

    $EndAction
    /.            },./

    $BeginJava
    /.$BeginAction
                $symbol_declarations./

    $EndJava /.$EndAction./

    $NoAction
    /.$Header $rule_number => {},./

    $BeginActions
    /.
        fn rule_action_impl(&mut self, rule_number: i32) {
            match rule_number {./

    $SplitActions
    /.
                _ => self.rule_action_$rule_number(rule_number),
            }
        }

        fn rule_action_$rule_number(&mut self, rule_number: i32) {
            match rule_number {./

    $EndActions
    /.
                _ => {}
            }
        }./
%End

%Globals
    /.
    ./

%End

%Headers
    /.
    struct $action_type$RuleProxy {
        owner: *mut $action_type,
    }

    impl RuleAction for $action_type$RuleProxy {
        fn rule_action(&mut self, rule_number: i32) {
            unsafe {
                (*self.owner).rule_action_impl(rule_number);
            }
        }
    }

    pub struct $action_type {
        kw_lexer: Option<$kw_lexer_class>,
        print_tokens: bool,
        lex_parser: LexParser<$super_stream_class, $prs_type, $action_type$RuleProxy>,
        lex_stream: $super_stream_class,
        prs: $prs_type,
    }

    impl $action_type {
        pub fn new(
            filename: String,
            tab: i32,
            input_chars: Option<Vec<char>>,
        ) -> Result<Box<Self>, LpgException> {
            let lex_stream = $super_stream_class::new(filename, input_chars, tab)?;
            let prs = $prs_type;
            let mut boxed = Box::new(Self {
                kw_lexer: None,
                print_tokens: false,
                lex_parser: LexParser::new(),
                lex_stream: lex_stream.clone(),
                prs,
            });
            let owner = boxed.as_mut() as *mut Self;
            boxed.lex_parser.reset(
                lex_stream,
                boxed.prs.clone(),
                $action_type$RuleProxy { owner },
            );
            boxed.reset_keyword_lexer();
            Ok(boxed)
        }

        pub fn get_parse_table(&self) -> &$prs_type {
            &self.prs
        }

        pub fn get_parser(&mut self) -> &mut LexParser<$super_stream_class, $prs_type, $action_type$RuleProxy> {
            &mut self.lex_parser
        }

        pub fn get_token(&self, i: i32) -> i32 {
            self.lex_parser.get_token(i)
        }

        pub fn get_rhs_first_token_index(&self, i: i32) -> i32 {
            self.lex_parser.get_first_token_at(i)
        }

        pub fn get_rhs_last_token_index(&self, i: i32) -> i32 {
            self.lex_parser.get_last_token_at(i)
        }

        pub fn get_left_span(&self) -> i32 {
            self.lex_parser.get_token(1)
        }

        pub fn get_right_span(&self) -> i32 {
            self.lex_parser.get_last_token()
        }

        pub fn reset_keyword_lexer(&mut self) {
            if self.kw_lexer.is_none() {
                self.kw_lexer = Some($kw_lexer_class::new(
                    self.lex_stream.get_input_chars(),
                    $_IDENTIFIER,
                ));
            } else {
                self.kw_lexer
                    .as_mut()
                    .unwrap()
                    .set_input_chars(self.lex_stream.get_input_chars());
            }
        }

        pub fn reset(
            &mut self,
            filename: String,
            tab: i32,
            input_chars: Option<Vec<char>>,
        ) -> Result<(), LpgException> {
            self.lex_stream = $super_stream_class::new(filename, input_chars, tab)?;
            let owner = self as *mut Self;
            self.lex_parser.reset(
                self.lex_stream.clone(),
                self.prs.clone(),
                $action_type$RuleProxy { owner },
            );
            self.reset_keyword_lexer();
            Ok(())
        }

        pub fn get_i_lex_stream(&self) -> LexStreamRef {
            self.lex_stream.get_i_lex_stream()
        }

        fn initialize_lexer(
            &mut self,
            prs_stream: $prs_stream_class,
            start_offset: i32,
            end_offset: i32,
        ) -> Result<(), LpgException> {
            if self.lex_stream.get_input_chars().is_empty() {
                return Err(LpgException::NullPointer(NullPointerException::new(
                    "LexStream was not initialized",
                )));
            }
            let lex_ref = self.get_i_lex_stream();
            lex_ref.borrow_mut().set_prs_stream(prs_stream.clone());
            prs_stream.borrow_mut().set_lex_stream(lex_ref);
            prs_stream
                .borrow_mut()
                .make_token(start_offset, end_offset, 0);
            Ok(())
        }

        fn add_eof(&self, prs_stream: $prs_stream_class, end_offset: i32) {
            prs_stream
                .borrow_mut()
                .make_token(end_offset, end_offset, $eof_token);
            let size = prs_stream.borrow().get_size();
            prs_stream.borrow_mut().set_stream_length(size);
        }

        pub fn lexer_with_position(
            &mut self,
            prs_stream: $prs_stream_class,
            start_offset: i32,
            end_offset: i32,
            monitor: Option<&dyn Monitor>,
        ) -> Result<(), LpgException> {
            if start_offset <= 1 {
                self.initialize_lexer(prs_stream.clone(), 0, -1)?;
            } else {
                self.initialize_lexer(prs_stream.clone(), start_offset - 1, start_offset - 1)?;
            }
            self.lex_parser
                .parse_characters(start_offset, end_offset, monitor);
            let index = if end_offset >= self.lex_stream.get_stream_index() {
                self.lex_stream.get_stream_index()
            } else {
                end_offset + 1
            };
            self.add_eof(prs_stream, index);
            Ok(())
        }

        pub fn lexer(
            &mut self,
            prs_stream: $prs_stream_class,
            monitor: Option<&dyn Monitor>,
        ) -> Result<(), LpgException> {
            self.initialize_lexer(prs_stream.clone(), 0, -1)?;
            self.lex_parser.parse_characters_with_monitor(monitor);
            self.add_eof(prs_stream, self.lex_stream.get_stream_index());
            Ok(())
        }

        /// If a parse stream was not passed to the lexical analyser then we
        /// simply report a lexical error. Otherwise, we produce a bad token.
        pub fn report_lexical_error(&mut self, start_loc: i32, end_loc: i32) {
            if let Some(prs_stream) = self.lex_stream.get_i_prs_stream() {
                let mut i = prs_stream.borrow().get_size() - 1;
                while i > 0 {
                    if prs_stream.borrow().get_start_offset(i) >= start_loc {
                        prs_stream.borrow_mut().remove_last_token();
                    } else {
                        break;
                    }
                    i -= 1;
                }
                prs_stream
                    .borrow_mut()
                    .make_token(start_loc, end_loc, 0);
            } else {
                self.lex_stream
                    .report_lexical_error_position(start_loc, end_loc);
            }
        }
    ./
%End

%Rules
    /.$BeginActions./
%End

%Trailers
    /.
        $EndActions
    }
    ./
%End

--
-- E N D   O F   T E M P L A T E
--
