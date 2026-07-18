%Options glr,automatic_ast=nested,var=nt,visitor=default

%Terminals
    NUMBER PLUS
%End

%Eof
    EOF_TOKEN
%End

%Start
    E
%End

%Rules
    E$E ::= E PLUS E
          | NUMBER
%End
