%Options ebnf

%Terminals
    a b SEMI EOF_TOKEN
%End

%Eof
    EOF_TOKEN
%End

%Start
    S
%End

%Rules
    S ::= (a | b) SEMI
%End
