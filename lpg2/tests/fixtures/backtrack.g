%Options backtrack

%Terminals
    a b EOF_TOKEN
%End

%Eof
    EOF_TOKEN
%End

%Rules
    S ::= a b
        | a
%End
