%options verbose
%options package=Calculator

%Terminals
    NUMBER
    PLUS
    STAR
    LPAREN
    RPAREN
    EOF_TOKEN
%End

%Eof
    EOF_TOKEN
%End

%Start
    Expr
%End

%Rules
    Expr ::= Expr PLUS Term
           | Term

    Term ::= Term STAR Factor
           | Factor

    Factor ::= NUMBER
             | LPAREN Expr RPAREN
%End
