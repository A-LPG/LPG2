%Options automatic_ast=nested,var=nt,visitor=default
%Options template=dtParserTemplateF.gi
%Options package=Recover

%Terminals
    a b c
%End

%Eof
    EOF_TOKEN
%End

%Start
    S
%End

%Recover
    Missing /. new Missing(error_token, error_token) ./
%End

%Rules
    S$Root ::= a Missing
    Missing$Missing ::= b c
%End
