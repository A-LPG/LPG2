%Options ebnf,automatic_ast=nested,var=nt,visitor=default
%Options template=dtParserTemplateF.gi
%Options package=EbnfCall

%Terminals
    ID
    NUMBER
    LPAREN
    RPAREN
    COMMA
    SEMICOLON
%End

%Eof
    EOF_TOKEN
%End

%Start
    Program
%End

%Rules
    Program$Program ::= Call*

    Call$Call ::= ID LPAREN Args? RPAREN [SEMICOLON]

    Args$Args ::= Expr (COMMA Expr)*

    Expr$Expr ::= ID | NUMBER
%End
