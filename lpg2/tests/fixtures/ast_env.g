%Options automatic_ast=nested,var=nt,visitor=default
%Options template=dtParserTemplateF.gi

%Terminals
    a EOF_TOKEN
%End

%Eof
    EOF_TOKEN
%End

%Rules
    S$Root ::= Item
    /.
        // Marker action: presence of this block sets needs_environment.
        // Emitted into the AST module (not the rule switch), so keep it comment-only.
    ./
    Item$Item ::= a
%End
