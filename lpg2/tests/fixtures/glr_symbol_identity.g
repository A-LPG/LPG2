%Options glr,automatic_ast=nested,var=nt,visitor=default

%Terminals
    TOKEN
%End

%Eof
    EOF_TOKEN
%End

%Start
    S
%End

%Rules
    S$S ::= X
    X$Left ::= A
    X$Right ::= B
    A$Shared ::= TOKEN
    B$Shared ::= TOKEN
%End
