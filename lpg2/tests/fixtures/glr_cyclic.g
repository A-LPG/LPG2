%Options glr,ast_type=Object

%Terminals
    NUMBER
%End

%Eof
    EOF_TOKEN
%End

%Start
    S
%End

%Rules
    S ::= A NUMBER
        /. setResult(getRhsSym(2)); ./
    A ::= A
        /. setResult(null); ./
      | %Empty
        /. setResult(null); ./
%End
