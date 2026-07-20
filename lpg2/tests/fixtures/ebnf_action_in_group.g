%Options ebnf
%Options template=dtParserTemplateF.gi

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
    S ::= (a /. /* action allowed in EBNF group */ ./ b)
%End
