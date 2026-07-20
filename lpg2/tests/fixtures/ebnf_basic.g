%Options ebnf
%Options template=dtParserTemplateF.gi

%Terminals
    a b c COMMA LPAREN RPAREN PLUS STAR QUESTION EOF_TOKEN
    PLUS ::= '+'
    STAR ::= '*'
    QUESTION ::= '?'
%End

%Eof
    EOF_TOKEN
%End

%Start
    S
%End

%Rules
    S ::= a b? c* Call Quoted
    Call ::= LPAREN (a (COMMA a)*)? RPAREN
    Quoted ::= PLUS STAR QUESTION
%End
