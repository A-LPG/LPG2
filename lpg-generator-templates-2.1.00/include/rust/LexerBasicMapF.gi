
$Trailers 
/. 
    #[derive(Clone)]
    pub struct $super_stream_class {
        inner: LexStreamRef,
    }

    static $super_stream_class$_TOKEN_KIND: [i32; 128] = [
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 000    0x00
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 001    0x01
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 002    0x02
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 003    0x03
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 004    0x04
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 005    0x05
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 006    0x06
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 007    0x07
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 008    0x08
            $sym_type::$prefix$HT$suffix$,              // 009    0x09
            $sym_type::$prefix$LF$suffix$,              // 010    0x0A
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 011    0x0B
            $sym_type::$prefix$FF$suffix$,              // 012    0x0C
            $sym_type::$prefix$CR$suffix$,              // 013    0x0D
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 014    0x0E
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 015    0x0F
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 016    0x10
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 017    0x11
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 018    0x12
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 019    0x13
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 020    0x14
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 021    0x15
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 022    0x16
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 023    0x17
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 024    0x18
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 025    0x19
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 026    0x1A
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 027    0x1B
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 028    0x1C
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 029    0x1D
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 030    0x1E
            $sym_type::$prefix$CtlCharNotWS$suffix$,    // 031    0x1F
            $sym_type::$prefix$Space$suffix$,           // 032    0x20
            $sym_type::$prefix$Exclamation$suffix$,     // 033    0x21
            $sym_type::$prefix$DoubleQuote$suffix$,     // 034    0x22
            $sym_type::$prefix$Sharp$suffix$,           // 035    0x23
            $sym_type::$prefix$DollarSign$suffix$,      // 036    0x24
            $sym_type::$prefix$Percent$suffix$,         // 037    0x25
            $sym_type::$prefix$Ampersand$suffix$,       // 038    0x26
            $sym_type::$prefix$SingleQuote$suffix$,     // 039    0x27
            $sym_type::$prefix$LeftParen$suffix$,       // 040    0x28
            $sym_type::$prefix$RightParen$suffix$,      // 041    0x29
            $sym_type::$prefix$Star$suffix$,            // 042    0x2A
            $sym_type::$prefix$Plus$suffix$,            // 043    0x2B
            $sym_type::$prefix$Comma$suffix$,           // 044    0x2C
            $sym_type::$prefix$Minus$suffix$,           // 045    0x2D
            $sym_type::$prefix$Dot$suffix$,             // 046    0x2E
            $sym_type::$prefix$Slash$suffix$,           // 047    0x2F
            $sym_type::$prefix$0$suffix$,               // 048    0x30
            $sym_type::$prefix$1$suffix$,               // 049    0x31
            $sym_type::$prefix$2$suffix$,               // 050    0x32
            $sym_type::$prefix$3$suffix$,               // 051    0x33
            $sym_type::$prefix$4$suffix$,               // 052    0x34
            $sym_type::$prefix$5$suffix$,               // 053    0x35
            $sym_type::$prefix$6$suffix$,               // 054    0x36
            $sym_type::$prefix$7$suffix$,               // 055    0x37
            $sym_type::$prefix$8$suffix$,               // 056    0x38
            $sym_type::$prefix$9$suffix$,               // 057    0x39
            $sym_type::$prefix$Colon$suffix$,           // 058    0x3A
            $sym_type::$prefix$SemiColon$suffix$,       // 059    0x3B
            $sym_type::$prefix$LessThan$suffix$,        // 060    0x3C
            $sym_type::$prefix$Equal$suffix$,           // 061    0x3D
            $sym_type::$prefix$GreaterThan$suffix$,     // 062    0x3E
            $sym_type::$prefix$QuestionMark$suffix$,    // 063    0x3F
            $sym_type::$prefix$AtSign$suffix$,          // 064    0x40
            $sym_type::$prefix$A$suffix$,               // 065    0x41
            $sym_type::$prefix$B$suffix$,               // 066    0x42
            $sym_type::$prefix$C$suffix$,               // 067    0x43
            $sym_type::$prefix$D$suffix$,               // 068    0x44
            $sym_type::$prefix$E$suffix$,               // 069    0x45
            $sym_type::$prefix$F$suffix$,               // 070    0x46
            $sym_type::$prefix$G$suffix$,               // 071    0x47
            $sym_type::$prefix$H$suffix$,               // 072    0x48
            $sym_type::$prefix$I$suffix$,               // 073    0x49
            $sym_type::$prefix$J$suffix$,               // 074    0x4A
            $sym_type::$prefix$K$suffix$,               // 075    0x4B
            $sym_type::$prefix$L$suffix$,               // 076    0x4C
            $sym_type::$prefix$M$suffix$,               // 077    0x4D
            $sym_type::$prefix$N$suffix$,               // 078    0x4E
            $sym_type::$prefix$O$suffix$,               // 079    0x4F
            $sym_type::$prefix$P$suffix$,               // 080    0x50
            $sym_type::$prefix$Q$suffix$,               // 081    0x51
            $sym_type::$prefix$R$suffix$,               // 082    0x52
            $sym_type::$prefix$S$suffix$,               // 083    0x53
            $sym_type::$prefix$T$suffix$,               // 084    0x54
            $sym_type::$prefix$U$suffix$,               // 085    0x55
            $sym_type::$prefix$V$suffix$,               // 086    0x56
            $sym_type::$prefix$W$suffix$,               // 087    0x57
            $sym_type::$prefix$X$suffix$,               // 088    0x58
            $sym_type::$prefix$Y$suffix$,               // 089    0x59
            $sym_type::$prefix$Z$suffix$,               // 090    0x5A
            $sym_type::$prefix$LeftBracket$suffix$,     // 091    0x5B
            $sym_type::$prefix$BackSlash$suffix$,       // 092    0x5C
            $sym_type::$prefix$RightBracket$suffix$,    // 093    0x5D
            $sym_type::$prefix$Caret$suffix$,           // 094    0x5E
            $sym_type::$prefix$_$suffix$,               // 095    0x5F
            $sym_type::$prefix$BackQuote$suffix$,       // 096    0x60
            $sym_type::$prefix$a$suffix$,               // 097    0x61
            $sym_type::$prefix$b$suffix$,               // 098    0x62
            $sym_type::$prefix$c$suffix$,               // 099    0x63
            $sym_type::$prefix$d$suffix$,               // 100    0x64
            $sym_type::$prefix$e$suffix$,               // 101    0x65
            $sym_type::$prefix$f$suffix$,               // 102    0x66
            $sym_type::$prefix$g$suffix$,               // 103    0x67
            $sym_type::$prefix$h$suffix$,               // 104    0x68
            $sym_type::$prefix$i$suffix$,               // 105    0x69
            $sym_type::$prefix$j$suffix$,               // 106    0x6A
            $sym_type::$prefix$k$suffix$,               // 107    0x6B
            $sym_type::$prefix$l$suffix$,               // 108    0x6C
            $sym_type::$prefix$m$suffix$,               // 109    0x6D
            $sym_type::$prefix$n$suffix$,               // 110    0x6E
            $sym_type::$prefix$o$suffix$,               // 111    0x6F
            $sym_type::$prefix$p$suffix$,               // 112    0x70
            $sym_type::$prefix$q$suffix$,               // 113    0x71
            $sym_type::$prefix$r$suffix$,               // 114    0x72
            $sym_type::$prefix$s$suffix$,               // 115    0x73
            $sym_type::$prefix$t$suffix$,               // 116    0x74
            $sym_type::$prefix$u$suffix$,               // 117    0x75
            $sym_type::$prefix$v$suffix$,               // 118    0x76
            $sym_type::$prefix$w$suffix$,               // 119    0x77
            $sym_type::$prefix$x$suffix$,               // 120    0x78
            $sym_type::$prefix$y$suffix$,               // 121    0x79
            $sym_type::$prefix$z$suffix$,               // 122    0x7A
            $sym_type::$prefix$LeftBrace$suffix$,       // 123    0x7B
            $sym_type::$prefix$VerticalBar$suffix$,     // 124    0x7C
            $sym_type::$prefix$RightBrace$suffix$,      // 125    0x7D
            $sym_type::$prefix$Tilde$suffix$,           // 126    0x7E
            $sym_type::$prefix$AfterASCII$suffix$,      // 127    for all chars in range 128..65534
    ];

    impl $super_stream_class {
        pub fn new(
            file_name: String,
            input_chars: Option<Vec<char>>,
            tab: i32,
        ) -> Result<Self, LpgException> {
            let inner = LexStream::new(file_name, input_chars, tab, None).map_err(|e| {
                LpgException::NullPointer(NullPointerException::new(&e.to_string()))
            })?;
            Ok(Self { inner })
        }

        pub fn get_input_chars(&self) -> Vec<char> {
            self.inner.borrow().get_input_chars()
        }

        pub fn get_stream_index(&self) -> i32 {
            self.inner.borrow().get_stream_index()
        }

        pub fn get_i_lex_stream(&self) -> LexStreamRef {
            self.inner.clone()
        }

        pub fn get_i_prs_stream(&self) -> Option<PrsStreamRef> {
            self.inner.borrow().get_i_prs_stream()
        }

        pub fn report_lexical_error_position(&mut self, left_loc: i32, right_loc: i32) {
            self.inner
                .borrow_mut()
                .report_lexical_error_position(left_loc, right_loc);
        }
    }

    impl TokenStream for $super_stream_class {
        fn get_token_from_end_token(&mut self, end_token: i32) -> i32 {
            self.inner.borrow_mut().get_token_from_end_token(end_token)
        }

        fn get_token(&mut self) -> i32 {
            self.inner.borrow_mut().get_token()
        }

        fn get_kind(&self, i: i32) -> i32 {
            let c = if i >= self.inner.borrow().get_stream_length() {
                0xffff
            } else {
                self.inner.borrow().get_int_value(i)
            };
            if c < 128 {
                $super_stream_class$_TOKEN_KIND[c as usize]
            } else if c == 0xffff {
                $sym_type::$prefix$EOF$suffix$
            } else {
                $sym_type::$prefix$AfterASCII$suffix$
            }
        }

        fn get_next(&self, i: i32) -> i32 {
            self.inner.borrow().get_next(i)
        }

        fn get_previous(&self, i: i32) -> i32 {
            self.inner.borrow().get_previous(i)
        }

        fn get_name(&self, i: i32) -> String {
            self.inner.borrow().get_name(i)
        }

        fn peek(&self) -> i32 {
            self.inner.borrow().peek()
        }

        fn reset(&mut self) {
            self.inner.borrow_mut().reset()
        }

        fn reset_to(&mut self, i: i32) {
            self.inner.borrow_mut().reset_to(i)
        }

        fn bad_token(&self) -> i32 {
            self.inner.borrow().bad_token()
        }

        fn get_line(&self, i: i32) -> i32 {
            self.inner.borrow().get_line(i)
        }

        fn get_column(&self, i: i32) -> i32 {
            self.inner.borrow().get_column(i)
        }

        fn get_end_line(&self, i: i32) -> i32 {
            self.inner.borrow().get_end_line(i)
        }

        fn get_end_column(&self, i: i32) -> i32 {
            self.inner.borrow().get_end_column(i)
        }

        fn after_eol(&self, i: i32) -> bool {
            self.inner.borrow().after_eol(i)
        }

        fn get_file_name(&self) -> String {
            self.inner.borrow().get_file_name()
        }

        fn get_stream_length(&self) -> i32 {
            self.inner.borrow().get_stream_length()
        }

        fn get_first_real_token(&self, i: i32) -> i32 {
            self.inner.borrow().get_first_real_token(i)
        }

        fn get_last_real_token(&self, i: i32) -> i32 {
            self.inner.borrow().get_last_real_token(i)
        }

        fn report_error(
            &mut self,
            error_code: i32,
            left_token: i32,
            right_token: i32,
            error_info: &[String],
            error_token: i32,
        ) {
            self.inner.borrow_mut().report_error(
                error_code,
                left_token,
                right_token,
                error_info,
                error_token,
            )
        }
    }

    impl ILexStream for $super_stream_class {
        fn get_i_prs_stream(&self) -> Option<PrsStreamRef> {
            self.inner.borrow().get_i_prs_stream()
        }

        fn set_prs_stream(&mut self, stream: PrsStreamRef) {
            self.inner.borrow_mut().set_prs_stream(stream)
        }

        fn get_line_count(&self) -> i32 {
            self.inner.borrow().get_line_count()
        }

        fn get_stream_index(&self) -> i32 {
            self.inner.borrow().get_stream_index()
        }

        fn ordered_exported_symbols(&self) -> Option<Vec<String>> {
            Some(
                $sym_type::ORDERED_TERMINAL_SYMBOLS
                    .iter()
                    .map(|s| (*s).to_string())
                    .collect(),
            )
        }

        fn get_line_offset(&self, i: i32) -> i32 {
            self.inner.borrow().get_line_offset(i)
        }

        fn get_line_number_of_char_at(&self, i: i32) -> i32 {
            self.inner.borrow().get_line_number_of_char_at(i)
        }

        fn get_column_of_char_at(&self, i: i32) -> i32 {
            self.inner.borrow().get_column_of_char_at(i)
        }

        fn get_char_value(&self, i: i32) -> String {
            self.inner.borrow().get_char_value(i)
        }

        fn get_input_chars(&self) -> Vec<char> {
            self.inner.borrow().get_input_chars()
        }

        fn get_int_value(&self, i: i32) -> i32 {
            self.inner.borrow().get_int_value(i)
        }

        fn make_token(&mut self, start_loc: i32, end_loc: i32, kind: i32) {
            self.inner.borrow_mut().make_token(start_loc, end_loc, kind)
        }

        fn set_message_handler(&mut self, handler: Rc<RefCell<dyn IMessageHandler>>) {
            self.inner.borrow_mut().set_message_handler(handler)
        }

        fn get_message_handler(&self) -> Option<Rc<RefCell<dyn IMessageHandler>>> {
            self.inner.borrow().get_message_handler()
        }

        fn get_location(&self, left_loc: i32, right_loc: i32) -> [i32; 6] {
            self.inner.borrow().get_location(left_loc, right_loc)
        }

        fn report_lexical_error_position(&mut self, left_loc: i32, right_loc: i32) {
            self.inner
                .borrow_mut()
                .report_lexical_error_position(left_loc, right_loc)
        }

        fn report_lexical_error(
            &mut self,
            left_loc: i32,
            right_loc: i32,
            error_code: i32,
            error_left_loc: i32,
            error_right_loc: i32,
            error_info: &[String],
        ) {
            self.inner.borrow_mut().report_lexical_error(
                left_loc,
                right_loc,
                error_code,
                error_left_loc,
                error_right_loc,
                error_info,
            )
        }

        fn to_string_range(&self, start_offset: i32, end_offset: i32) -> String {
            self.inner.borrow().to_string_range(start_offset, end_offset)
        }
    }
    ./
%End
%Headers

    --
    -- Additional methods for the action class not provided in the template
    --
    /.
    //
    // The Lexer contains an array of characters as the input stream to be parsed.
    // There are methods to retrieve and classify characters.
    // The lexparser "token" is implemented simply as the index of the next character in the array.
    // The Lexer extends the abstract class LpgLexStream with an implementation of the abstract
    // method get_kind.  The template defines the Lexer class and the lexer() method.
    // A driver creates the action class, "Lexer", passing an Option object to the constructor.
    //

        pub fn get_keyword_kinds(&self) -> &[i32] {
            self.kw_lexer.as_ref().unwrap().get_keyword_kinds()
        }

        pub fn make_token(&mut self, left_token: i32, right_token: i32, kind: i32) {
            self.lex_stream
                .make_token(left_token, right_token, kind);
        }

        pub fn make_token_with_kind(&mut self, kind: i32) {
            let start_offset = self.get_left_span();
            let end_offset = self.get_right_span();
            self.lex_stream
                .make_token(start_offset, end_offset, kind);
            if self.print_tokens {
                self.print_value(start_offset, end_offset);
            }
        }

        pub fn make_comment(&mut self, kind: i32) {
            let start_offset = self.get_left_span();
            let end_offset = self.get_right_span();
            if let Some(prs_stream) = self.lex_stream.get_i_prs_stream() {
                prs_stream
                    .borrow_mut()
                    .make_adjunct(start_offset, end_offset, kind);
            }
        }

        pub fn skip_token(&mut self) {
            if self.print_tokens {
                self.print_value(self.get_left_span(), self.get_right_span());
            }
        }

        pub fn check_for_key_word(&mut self) {
            let start_offset = self.get_left_span();
            let end_offset = self.get_right_span();
            let kw_kind = self
                .kw_lexer
                .as_ref()
                .unwrap()
                .lexer(start_offset, end_offset);
            self.lex_stream
                .make_token(start_offset, end_offset, kw_kind);
            if self.print_tokens {
                self.print_value(start_offset, end_offset);
            }
        }

        //
        // This flavor of check_for_key_word is necessary when the default kind
        // (which is returned when the keyword filter doesn't match) is something
        // other than _IDENTIFIER.
        //

        pub fn check_for_key_word_with_kind(&mut self, default_kind: i32) {
            let start_offset = self.get_left_span();
            let end_offset = self.get_right_span();
            let mut kw_kind = self
                .kw_lexer
                .as_ref()
                .unwrap()
                .lexer(start_offset, end_offset);
            if kw_kind == $_IDENTIFIER {
                kw_kind = default_kind;
            }
            self.lex_stream
                .make_token(start_offset, end_offset, kw_kind);
            if self.print_tokens {
                self.print_value(start_offset, end_offset);
            }
        }

        pub fn print_value(&self, start_offset: i32, end_offset: i32) {
            let chars = self.lex_stream.get_input_chars();
            let s: String = chars[start_offset as usize..=end_offset as usize]
                .iter()
                .collect();
            print!("{s}");
        }
    ./
%End
