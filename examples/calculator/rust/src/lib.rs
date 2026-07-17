//! Calculator quickstart driver (token-seeded; no full lexer).
#![allow(
    dead_code,
    non_camel_case_types,
    non_snake_case,
    non_upper_case_globals,
    private_interfaces,
    unused_imports,
    unused_mut,
    unused_variables
)]

use lpg2::prelude::*;
use std::any::Any;
use std::cell::RefCell;
use std::rc::Rc;

include!("../../out-rust/calculatorsym.rs");
include!("../../out-rust/calculatorprs.rs");
include!("../../out-rust/calculator.rs");

fn parser_with(kinds: &[i32]) -> Box<calculator> {
    let parser = calculator::new(None).expect("construct");
    let stream = parser.get_i_prs_stream();
    {
        let mut stream = stream.borrow_mut();
        stream.make_token(0, 0, 0);
        for (index, kind) in kinds.iter().copied().enumerate() {
            stream.make_token(index as i32, index as i32, kind);
        }
        let size = stream.get_size();
        stream.set_stream_length(size);
    }
    parser
}

#[test]
fn accepts_one_plus_two_times_three() {
    let mut parser = parser_with(&[
        calculatorsym::TK_NUMBER,
        calculatorsym::TK_PLUS,
        calculatorsym::TK_NUMBER,
        calculatorsym::TK_STAR,
        calculatorsym::TK_NUMBER,
        calculatorsym::TK_EOF_TOKEN,
    ]);
    let result = parser.parser().expect("parse should succeed");
    assert!(result.is_some());
}

#[test]
fn rejects_leading_plus() {
    let outcome = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        let mut parser = parser_with(&[calculatorsym::TK_PLUS, calculatorsym::TK_EOF_TOKEN]);
        parser.parser()
    }));
    match outcome {
        Ok(Ok(Some(_))) => panic!("leading PLUS must not yield a successful AST"),
        _ => {}
    }
}
