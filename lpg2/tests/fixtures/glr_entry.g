%Options glr,automatic_ast=nested,var=nt,visitor=default

%Terminals
    A_TOKEN B_TOKEN
%End

%Eof
    EOF_TOKEN
%End

%Start
    A B
%End

%Rules
    A$A ::= A_TOKEN
    B$B ::= B_TOKEN
%End
