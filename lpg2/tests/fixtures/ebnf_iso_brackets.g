%Options ebnf
%Options template=dtParserTemplateF.gi

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
    S ::= Opt Star Cs
    Opt ::= [a]
    Star ::= {b}
    Cs ::= {c}
%End
