%Options ebnf,automatic_ast=nested,var=nt,visitor=default
%Options template=dtParserTemplateF.gi

%Terminals
    a b c EOF_TOKEN
%End

%Eof
    EOF_TOKEN
%End

%Start
    S
%End

%Rules
    S$Root ::= a*$As b?$Opt c$Cs*
%End
