%Options glr,ast_type=Object

%Terminals
    NUMBER PLUS
%End

%Eof
    EOF_TOKEN
%End

%Start
    S
%End

%Rules
    S ::= E
        /. setResult(getRhsSym(1)); ./
    E ::= E PLUS E
        /. setResult(new Object[] { getRhsSym(1), getRhsSym(3) }); ./
      | NUMBER
        /. setResult(new Object()); ./
%End
