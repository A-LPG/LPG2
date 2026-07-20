%Options ebnf

%Terminals
    a EOF_TOKEN
%End

%Eof
    EOF_TOKEN
%End

%Start
    S
%End

%Recover
    __ebnf_opt_r0_1
%End

%Rules
    S ::= a?
%End
