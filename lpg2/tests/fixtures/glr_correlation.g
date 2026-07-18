%Options glr,automatic_ast=nested,var=nt,visitor=default

%Terminals
    NUMBER PLUS COMMA
%End

%Eof
    EOF_TOKEN
%End

%Start
    S
%End

%Rules
    S$S ::= E COMMA E
    E$E ::= E PLUS E
          | NUMBER
%End
