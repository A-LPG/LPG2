%Options ebnf

%Terminals
    a b EOF_TOKEN
%End

%Eof
    EOF_TOKEN
%End

%Start
    S
%End

%Rules
    S ::= (a /. /* illegal action in group */ ./ b)
%End
