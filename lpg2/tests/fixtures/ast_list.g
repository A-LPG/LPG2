%Options automatic_ast=nested,var=nt,visitor=default
%Options template=dtParserTemplateF.gi

%Terminals
    a EOF_TOKEN
%End

%Eof
    EOF_TOKEN
%End

%Rules
    S$Root ::= AList
    AList$$a ::= a | AList a
%End
