%Options automatic_ast=nested,var=nt,visitor=default
%Options template=dtParserTemplateF.gi
%options verbose
%options package=Calculator

%Terminals
    NUMBER
    PLUS
    STAR
    LPAREN
    RPAREN
%End

%Eof
    EOF_TOKEN
%End

%Start
    Expr
%End

%Rules
    Expr$Expr ::= Expr PLUS Term
           | Term

    Term$Term ::= Term STAR Factor
           | Factor

    Factor$Factor ::= NUMBER
             | LPAREN Expr RPAREN
%End
