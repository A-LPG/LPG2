%Options ebnf

%Terminals
    a b c EOF_TOKEN
%End

%Eof
    EOF_TOKEN
%End

%Start
    S
%End

%Rules
    S ::= Opt Star Plus
    Opt ::= b?
    Star ::= c*
    Plus ::= a+
%End
