%Options automatic_ast=nested,var=nt,visitor=default
%Options template=dtParserTemplateF.gi

%Terminals
    a b c EOF_TOKEN
%End

%Eof
    EOF_TOKEN
%End

%Recover
    Missing
%End

%Rules
    S$Root ::= a Missing
    Missing$Missing ::= b c
%End
