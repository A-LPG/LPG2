%Options glr,automatic_ast=nested,var=nt,visitor=default

%Eof
    EOF_TOKEN
%End

%Start
    S
%End

%Rules
    S$S ::= X
    X$X ::= A
          | B
    A$A ::= %Empty
    B$B ::= %Empty
%End
