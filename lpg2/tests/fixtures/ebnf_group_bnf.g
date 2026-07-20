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
    S ::= __ebnf_grp_r0_1 SEMI
    __ebnf_grp_r0_1 ::= a | b
%End
