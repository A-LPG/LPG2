%Terminals
    a PLUS EOF_TOKEN
%End

%Eof
    EOF_TOKEN
%End

%Rules
    E ::= E PLUS E
        | a
%End
