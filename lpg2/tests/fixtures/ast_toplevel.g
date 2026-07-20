%Options automatic_ast=toplevel,var=nt,visitor=default
%Options template=dtParserTemplateF.gi

%Terminals
    a EOF_TOKEN
%End

%Eof
    EOF_TOKEN
%End

%Rules
    S$Root ::= Item
    Item$Item ::= a
%End
