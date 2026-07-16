%Options automatic_ast=nested,var=nt,visitor=preorder
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
