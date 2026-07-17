%Options automatic_ast=nested,variables=nt,visitor=default
%Options programming_language=typescript
%Options template=dtParserTemplateF.gi

%Terminals
    a PLUS EOF_TOKEN
%End

%Eof
    EOF_TOKEN
%End

%Rules
    E$Root ::= E PLUS T
             | T
    T$Term ::= a
%End
