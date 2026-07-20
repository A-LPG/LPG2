%Options ebnf,automatic_ast=nested,var=nt,visitor=default
%Options template=dtParserTemplateF.gi

%Terminals
    a EOF_TOKEN
%End

%Eof
    EOF_TOKEN
%End

%Start
    S
%End

%Rules
    S$Root ::= a*
%End
