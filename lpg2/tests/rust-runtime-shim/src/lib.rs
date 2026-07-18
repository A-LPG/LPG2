//! Minimal, dependency-free LPG runtime ABI used by generator table tests.
//!
//! This is deliberately not a user runtime. It keeps CI self-contained while
//! checking that generated tables implement the public ParseTable contract and
//! can drive deterministic parsing.

pub mod traits {
    pub trait ParseTable: Send + Sync {
        fn base_check(&self, index: i32) -> i32;
        fn rhs(&self, index: i32) -> i32;
        fn base_action(&self, index: i32) -> i32;
        fn lhs(&self, index: i32) -> i32;
        fn term_check(&self, index: i32) -> i32;
        fn term_action(&self, index: i32) -> i32;
        fn asb(&self, index: i32) -> i32;
        fn asr(&self, index: i32) -> i32;
        fn nasb(&self, index: i32) -> i32;
        fn nasr(&self, index: i32) -> i32;
        fn terminal_index(&self, index: i32) -> i32;
        fn nonterminal_index(&self, index: i32) -> i32;
        fn scope_prefix(&self, index: i32) -> i32;
        fn scope_suffix(&self, index: i32) -> i32;
        fn scope_lhs(&self, index: i32) -> i32;
        fn scope_la(&self, index: i32) -> i32;
        fn scope_state_set(&self, index: i32) -> i32;
        fn scope_rhs(&self, index: i32) -> i32;
        fn scope_state(&self, index: i32) -> i32;
        fn in_symb(&self, index: i32) -> i32;
        fn name(&self, index: i32) -> String;
        fn original_state(&self, state: i32) -> i32;
        fn asi(&self, state: i32) -> i32;
        fn nasi(&self, state: i32) -> i32;
        fn in_symbol(&self, state: i32) -> i32;
        fn nt_action(&self, state: i32, sym: i32) -> i32;
        fn t_action(&self, act: i32, sym: i32) -> i32;
        fn look_ahead(&self, act: i32, sym: i32) -> i32;
        fn get_error_symbol(&self) -> i32;
        fn get_scope_ubound(&self) -> i32;
        fn get_scope_size(&self) -> i32;
        fn get_max_name_length(&self) -> i32;
        fn get_num_states(&self) -> i32;
        fn get_nt_offset(&self) -> i32;
        fn get_la_state_offset(&self) -> i32;
        fn get_max_la(&self) -> i32;
        fn get_num_rules(&self) -> i32;
        fn get_num_nonterminals(&self) -> i32;
        fn get_num_symbols(&self) -> i32;
        fn get_start_state(&self) -> i32;
        fn get_start_symbol(&self) -> i32;
        fn get_eoft_symbol(&self) -> i32;
        fn get_eolt_symbol(&self) -> i32;
        fn get_accept_action(&self) -> i32;
        fn get_error_action(&self) -> i32;
        fn is_nullable(&self, symbol: i32) -> bool;
        fn is_valid_for_parser(&self) -> bool;
        fn get_backtrack(&self) -> bool;
        fn is_glr(&self) -> bool {
            false
        }
    }

    pub trait TokenStream {
        fn get_token_from_end_token(&mut self, end_token: i32) -> i32;
        fn get_token(&mut self) -> i32;
        fn get_kind(&self, i: i32) -> i32;
        fn get_next(&self, i: i32) -> i32;
        fn get_previous(&self, i: i32) -> i32;
        fn get_name(&self, i: i32) -> String;
        fn peek(&self) -> i32;
        fn reset(&mut self);
        fn reset_to(&mut self, i: i32);
        fn bad_token(&self) -> i32;
        fn get_line(&self, i: i32) -> i32;
        fn get_column(&self, i: i32) -> i32;
        fn get_end_line(&self, i: i32) -> i32;
        fn get_end_column(&self, i: i32) -> i32;
        fn after_eol(&self, i: i32) -> bool;
        fn get_file_name(&self) -> String;
        fn get_stream_length(&self) -> i32;
        fn get_first_real_token(&self, i: i32) -> i32;
        fn get_last_real_token(&self, i: i32) -> i32;
        fn report_error(
            &mut self,
            error_code: i32,
            left_token: i32,
            right_token: i32,
            error_info: &[String],
            error_token: i32,
        );
    }

    pub trait RuleAction {
        fn rule_action(&mut self, rule_number: i32);
    }
}

pub mod parser {
    use crate::traits::{ParseTable, RuleAction, TokenStream};

    #[derive(Debug, Clone, Copy, PartialEq, Eq)]
    pub enum ParseError {
        InvalidTable,
        BacktrackingTable,
        InvalidInput,
        InvalidReduction,
        StepLimit,
    }

    pub struct DeterministicParser<TS, PT, RA>
    where
        TS: TokenStream,
        PT: ParseTable + Clone,
        RA: RuleAction,
    {
        stream: TS,
        table: PT,
        actions: RA,
    }

    impl<TS, PT, RA> DeterministicParser<TS, PT, RA>
    where
        TS: TokenStream,
        PT: ParseTable + Clone,
        RA: RuleAction,
    {
        pub fn new(
            stream: TS,
            table: PT,
            actions: RA,
            _monitor: Option<()>,
        ) -> Result<Self, ParseError> {
            if !table.is_valid_for_parser() {
                return Err(ParseError::InvalidTable);
            }
            if table.get_backtrack() {
                return Err(ParseError::BacktrackingTable);
            }
            Ok(Self {
                stream,
                table,
                actions,
            })
        }

        fn resolve_lookahead(&self, mut action: i32, mut token: i32) -> i32 {
            while action > self.table.get_la_state_offset() {
                action = self.table.look_ahead(
                    action - self.table.get_la_state_offset(),
                    self.stream.get_kind(token),
                );
                token = self.stream.get_next(token);
            }
            action
        }

        fn reduce(
            &mut self,
            states: &mut Vec<i32>,
            mut action: i32,
        ) -> Result<i32, ParseError> {
            while action <= self.table.get_num_rules() {
                let pop_count = self.table.rhs(action) - 1;
                if pop_count < 0 || pop_count as usize >= states.len() {
                    return Err(ParseError::InvalidReduction);
                }
                states.truncate(states.len() - pop_count as usize);
                self.actions.rule_action(action);
                action = self
                    .table
                    .nt_action(*states.last().unwrap(), self.table.lhs(action));
            }
            Ok(action)
        }

        pub fn parse_entry(&mut self, _marker_kind: i32) -> Result<(), ParseError> {
            self.stream.reset();
            let mut token = self.stream.get_token();
            let mut kind = self.stream.get_kind(token);
            let mut action = self.table.get_start_state();
            let mut states = Vec::with_capacity(64);

            for _ in 0..100_000 {
                states.push(action);
                action = self.table.t_action(action, kind);
                if action > self.table.get_la_state_offset() {
                    action = self.resolve_lookahead(action, self.stream.peek());
                }

                if action <= self.table.get_num_rules() {
                    states.pop();
                    action = self.reduce(&mut states, action)?;
                } else if action > self.table.get_error_action() {
                    token = self.stream.get_token();
                    kind = self.stream.get_kind(token);
                    action -= self.table.get_error_action();
                    action = self.reduce(&mut states, action)?;
                } else if action < self.table.get_accept_action() {
                    token = self.stream.get_token();
                    kind = self.stream.get_kind(token);
                } else if action == self.table.get_accept_action() {
                    return Ok(());
                } else {
                    return Err(ParseError::InvalidInput);
                }
            }
            Err(ParseError::StepLimit)
        }
    }
}
