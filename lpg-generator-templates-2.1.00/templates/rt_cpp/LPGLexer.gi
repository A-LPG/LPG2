%options la=15
%options single-productions
%options template=LexerTemplateF.gi
%options filter=LPGKWLexer.gi

%Globals
    /.

    ./
%End

%Define
 
    $kw_lexer_class /.$LPGKWLexer./
    $_IDENTIFIER /.$_MACRO_NAME./
%End

%Include
   LexerBasicMapF.gi
   --Utf8LexerBasicMapF.gi
%End

%Export
    SINGLE_LINE_COMMENT
    
    MACRO_NAME
    SYMBOL
    BLOCK
    EQUIVALENCE
    PRIORITY_EQUIVALENCE
    ARROW
    PRIORITY_ARROW
    OR_MARKER
    EQUAL
    COMMA
    LEFT_PAREN
    RIGHT_PAREN
    LEFT_BRACKET
    RIGHT_BRACKET
    SHARP
    VBAR
%End

%Terminals
    CtlCharNotWS

    LF   CR   HT   FF

    a    b    c    d    e    f    g    h    i    j    k    l    m
    n    o    p    q    r    s    t    u    v    w    x    y    z
    _

    A    B    C    D    E    F    G    H    I    J    K    L    M
    N    O    P    Q    R    S    T    U    V    W    X    Y    Z

    0    1    2    3    4    5    6    7    8    9

    AfterASCII   ::= '\u0080..\ufffe'
    Space        ::= ' '
    LF           ::= NewLine
    CR           ::= Return
    HT           ::= HorizontalTab
    FF           ::= FormFeed
    DoubleQuote  ::= '"'
    SingleQuote  ::= "'"
    Percent      ::= '%'
    VerticalBar  ::= '|'
    Exclamation  ::= '!'
    AtSign       ::= '@'
    BackQuote    ::= '`'
    Tilde        ::= '~'
    Sharp        ::= '#'
    DollarSign   ::= '$'
    Ampersand    ::= '&'
    Caret        ::= '^'
    Colon        ::= ':'
    SemiColon    ::= ';'
    BackSlash    ::= '\'
    LeftBrace    ::= '{'
    RightBrace   ::= '}'
    LeftBracket  ::= '['
    RightBracket ::= ']'
    QuestionMark ::= '?'
    Comma        ::= ','
    Dot          ::= '.'
    LessThan     ::= '<'
    GreaterThan  ::= '>'
    Plus         ::= '+'
    Minus        ::= '-'
    Slash        ::= '/'
    Star         ::= '*'
    LeftParen    ::= '('
    RightParen   ::= ')'
    Equal        ::= '='
%End

%Start
    Token
%End

%Notice
/.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2007 IBM Corporation.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Eclipse Public License v1.0
// which accompanies this distribution, and is available at
// http://www.eclipse.org/legal/epl-v10.html
//
//Contributors:
//    Philippe Charles (pcharles@us.ibm.com) - initial API and implementation

////////////////////////////////////////////////////////////////////////////////
./
%End

%Rules
    Token ::= white /.$BeginJava skipToken(); $EndJava./
    Token ::= singleLineComment /.$BeginJava makeComment($_SINGLE_LINE_COMMENT); $EndJava./

    Token ::= OptionLines
    Token ::= MacroSymbol       /.$BeginJava checkForKeyWord();$EndJava./
    Token ::= Symbol            /.$BeginJava checkForKeyWord($_SYMBOL);$EndJava./
    Token ::= Block             /.$BeginJava makeToken($_BLOCK);$EndJava./
    Token ::= Equivalence       /.$BeginJava makeToken($_EQUIVALENCE);$EndJava./
    Token ::= Equivalence ?     /.$BeginJava makeToken($_PRIORITY_EQUIVALENCE);$EndJava./
    Token ::= '#'               /.$BeginJava makeToken($_SHARP);$EndJava./
    Token ::= Arrow             /.$BeginJava makeToken($_ARROW);$EndJava./
    Token ::= Arrow ?           /.$BeginJava makeToken($_PRIORITY_ARROW);$EndJava./
    Token ::= '|'               /.$BeginJava makeToken($_OR_MARKER);$EndJava./
    Token ::= '['               /.$BeginJava makeToken($_LEFT_BRACKET);$EndJava./
    Token ::= ']'               /.$BeginJava makeToken($_RIGHT_BRACKET);$EndJava./

    digit -> 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9

    aA -> a | A
    bB -> b | B
    cC -> c | C
    dD -> d | D
    eE -> e | E
    fF -> f | F
    gG -> g | G
    hH -> h | H
    iI -> i | I
    jJ -> j | J
    kK -> k | K
    lL -> l | L
    mM -> m | M
    nN -> n | N
    oO -> o | O
    pP -> p | P
    qQ -> q | Q
    rR -> r | R
    sS -> s | S
    tT -> t | T
    uU -> u | U
    vV -> v | V
    wW -> w | W
    xX -> x | X
    yY -> y | Y
    zZ -> z | Z

--  lower ::= a | b | c | d | e | f | g | h | i | j | k | l | m | n | o | p | q | r | s | t | u | v | w | x | y | z
--  upper ::= A | B | C | D | E | F | G | H | I | J | K | L | M | N | O | P | Q | R | S | T | U | V | W | X | Y | Z

    letter -> AfterASCII | '_' | aA | bB | cC | dD | eE | fF | gG | hH | iI | jJ | kK | lL | mM | nN | oO | pP | qQ | rR | sS | tT | uU | vV | wW | xX | yY | zZ

    anyNonWhiteChar -> letter | digit | special

    special -> specialNoDotOrSlash | '.' | '/'

    --    special -> '+' | '-' | '(' | ')' | '"' | '!' | '@' | '`' | '~' | '.' | '/' |
    --               '%' | '&' | '^' | ':' | ';' | "'" | '\' | '|' | '{' | '}' |
    --               '[' | ']' | '?' | ',' | '<' | '>' | '=' | '#' | '*' | '$'

    specialNoExclamationDotColonDollar -> '+' | '-' | '(' | ')' | '"' | '@' | '`' | '~' | '/' |
                                          '%' | '&' | '^' | ';' | "'" | '\' | '|' | '{' | '}' |
                                          '[' | ']' | '?' | ',' | '<' | '>' | '=' | '#' | '*'

    specialNoColonDollar -> '+' | '-' | '(' | ')' | '"' | '!' | '@' | '`' | '~' | '.' | '/' |
                            '%' | '&' | '^' | ';' | "'" | '\' | '|' | '{' | '}' |
                            '[' | ']' | '?' | ',' | '<' | '>' | '=' | '#' | '*'

    specialNoEqualDollar -> '+' | '-' | '(' | ')' | '"' | '!' | '@' | '`' | '~' | '.' | '/' |
                            '%' | '&' | '^' | ':' | ';' | "'" | '\' | '|' | '{' | '}' |
                            '[' | ']' | '?' | ',' | '<' | '>' | '#' | '*'

    specialNoQuestionDollar -> '+' | '-' | '(' | ')' | '"' | '!' | '@' | '`' | '~' | '.' | '/' |
                               '%' | '&' | '^' | ':' | ';' | "'" | '\' | '|' | '{' | '}' |
                               '[' | ']' | ',' | '<' | '>' | '=' | '#' | '*'

    specialNoMinusRightAngleDollar -> '+' | '(' | ')' | '!' | '@' | '`' | '~' | '.' | '/' |
                                      '%' | '&' | '^' | ':' | ';' | '"' | '\' | '|' | '{' | '}' |
                                      '[' | ']' | '?' | ',' | '<' | "'" | '=' | '#' | '*' | '$'

    specialNoDollar -> '+' | '-' | '(' | ')' | '"' | '!' | '@' | '`' | '~' | '.' | '/' |
                       '%' | '&' | '^' | ':' | ';' | "'" | '\' | '|' | '{' | '}' |
                       '[' | ']' | '?' | ',' | '<' | '>' | '=' | '#' | '*'

    specialNoDollarBracketSharp -> '+' | '-' | '(' | ')' | '"' | '!' | '@' | '`' | '~' | '.' | '/' |
                       '%' | '&' | '^' | ':' | ';' | "'" | '\' | '|' | '{' | '}' |
                       '?' | ',' | '<' | '>' | '=' | '*'

    specialNoDotOrSlash -> '+' | '-' | '(' | ')' | '"' | '!' | '@' | '`' | '~' |
                           '%' | '&' | '^' | ':' | ';' | "'" | '\' | '|' | '{' | '}' |
                           '[' | ']' | '?' | ',' | '<' | '>' | '=' | '#' | '*' | '$'

    specialNoColonOrSlash -> '+' | '-' | '(' | ')' | '"' | '!' | '@' | '`' | '~' | '.' |
                             '%' | '&' | '^' | ';' | "'" | '\' | '|' | '{' | '}' |
                             '[' | ']' | '?' | ',' | '<' | '>' | '=' | '#' | '*' | '$'

    specialNoExclamationOrSlash -> '+' | '-' | '(' | ')' | '"' | '@' | '`' | '~' | '.' |
                                   '%' | '&' | '^' | ':' | ';' | "'" | '\' | '|' | '{' | '}' |
                                   '[' | ']' | '?' | ',' | '<' | '>' | '=' | '#' | '*' | '$'

    specialNoDoubleQuote -> '+' | '-' | '(' | ')' | '!' | '@' | '`' | '~' | '.' | '/' |
                            '%' | '&' | '^' | ':' | ';' | "'" | '\' | '|' | '{' | '}' |
                            '[' | ']' | '?' | ',' | '<' | '>' | '=' | '#' | '*' | '$'

    specialNoSingleQuote -> '+' | '-' | '(' | ')' | '!' | '@' | '`' | '~' | '.' | '/' |
                            '%' | '&' | '^' | ':' | ';' | '"' | '\' | '|' | '{' | '}' |
                            '[' | ']' | '?' | ',' | '<' | '>' | '=' | '#' | '*' | '$'

    specialNoRightAngle -> '+' | '-' | '(' | ')' | '!' | '@' | '`' | '~' | '.' | '/' |
                           '%' | '&' | '^' | ':' | ';' | '"' | '\' | '|' | '{' | '}' |
                           '[' | ']' | '?' | ',' | '<' | "'" | '=' | '#' | '*' | '$'

    specialNoMinusSingleQuoteDoublequoteLeftAngleCommaLparenRparen -> '+' | '!' | '@' | '`' | '~' | '.' | '/' |
                                                                      '%' | '&' | '^' | ':' | ';' | '\' | '|' | '{' | '}' |
                                                                      '[' | ']' | '?' | '>' | '=' | '#' | '*' | '$'

    specialNoColonMinusSingleQuoteDoublequoteLeftAngleLRBracketOrSlashDollarPercent -> '+' | '(' | ')' | '!' | '@' | '`' | '~' | '.' |
                                                                              '&' | '^' | ';' | '\' | '{' | '}' |
                                                                              '?' | ',' | '>' | '=' | '*'

    Eol -> LF | CR

    whiteChar -> Space | Eol | HT | FF

    white ::= whiteChar
            | white whiteChar

    notEOL -> letter | digit | special | Space | HT | FF

    notEOLOrQuote -> letter | digit | specialNoSingleQuote | Space | HT | FF

    notEOLOrDoubleQuote -> letter | digit | specialNoDoubleQuote | Space | HT | FF

    notEOLOrRightAngle -> letter | digit | specialNoRightAngle | Space | HT | FF

    notEOLOrQuotes ::= %Empty
                     | notEOLOrQuotes notEOLOrQuote

    notEOLOrDoubleQuotes ::= %Empty
                           | notEOLOrDoubleQuotes notEOLOrDoubleQuote

    notEOLOrRightAngles ::= notEOLOrRightAngle
                          | notEOLOrRightAngles notEOLOrRightAngle

    singleLineComment ::= '-' '-'
                        | singleLineComment notEOL

    Equivalence ::= ':' ':' '='
    Arrow       ::= '-' '>'

    Exclamations ::= '!'
                   | Exclamations '!'

    InsideExclamationBlockChar -> letter | whiteChar | digit | specialNoExclamationOrSlash

    InsideExclamationBlock ::= %Empty
                             | InsideExclamationBlock InsideExclamationBlockChar
                             | InsideExclamationBlock Exclamations InsideExclamationBlockChar
                             | InsideExclamationBlock '/'

    Dots ::= '.'
           | Dots '.'

    InsideDotBlockChar -> letter | whiteChar | digit | specialNoDotOrSlash

    InsideDotBlock ::= %Empty
                     | InsideDotBlock InsideDotBlockChar
                     | InsideDotBlock Dots InsideDotBlockChar
                     | InsideDotBlock '/'

    Colons ::= ':'
             | Colons ':'

    InsideColonBlockChar -> letter | whiteChar | digit | specialNoColonOrSlash

    InsideColonBlock ::= %Empty
                       | InsideColonBlock InsideColonBlockChar
                       | InsideColonBlock Colons InsideColonBlockChar
                       | InsideColonBlock '/'

    Block ::= '/' '.' InsideDotBlock Dots '/'
            | '/' ':' InsideColonBlock Colons '/'
            | '/' '!' InsideExclamationBlock Exclamations '/'

    Symbol -> delimitedSymbol
            | specialSymbol
            | normalSymbol

    delimitedSymbol ::= "'" notEOLOrQuotes "'"
                      | '"' notEOLOrDoubleQuotes '"'
                      | '<' letter notEOLOrRightAngles '>'

    MacroSymbol ::= '$'
                  | MacroSymbol letter
                  | MacroSymbol digit

    anyNonWhiteNoColonMinusSingleQuoteDoublequoteLeftAngleLRBracketOrSlashDollarPercent -> letter | digit | specialNoColonMinusSingleQuoteDoublequoteLeftAngleLRBracketOrSlashDollarPercent
    normalSymbol ::= anyNonWhiteNoColonMinusSingleQuoteDoublequoteLeftAngleLRBracketOrSlashDollarPercent
                   | normalSymbol anyNonWhiteNoDollarBracketSharp

    --
    -- Below, we write special rules to recognize initial 
    -- prefixes of these special metasymbols as valid symbols.
    --
    --    BLOCK            /.  ...
    --    EQUIVALENCE      ::=[?]
    --    ARROW            ->[?]
    --    COMMENT          -- ...
    --    OR_MARKER        |
    --    OPTIONS_KEY      %options
    --    bracketed symbol < ... >
    --

    letterNoOo -> AfterASCII | '_' | aA | bB | cC | dD | eE | fF | gG | hH | iI | jJ | kK | lL | mM | nN | pP | qQ | rR | sS | tT | uU | vV | wW | xX | yY | zZ
    letterNoPp -> AfterASCII | '_' | aA | bB | cC | dD | eE | fF | gG | hH | iI | jJ | kK | lL | mM | nN | oO | qQ | rR | sS | tT | uU | vV | wW | xX | yY | zZ
    letterNoTt -> AfterASCII | '_' | aA | bB | cC | dD | eE | fF | gG | hH | iI | jJ | kK | lL | mM | nN | oO | pP | qQ | rR | sS | uU | vV | wW | xX | yY | zZ
    letterNoIi -> AfterASCII | '_' | aA | bB | cC | dD | eE | fF | gG | hH | jJ | kK | lL | mM | nN | oO | pP | qQ | rR | sS | tT | uU | vV | wW | xX | yY | zZ
    letterNoNn -> AfterASCII | '_' | aA | bB | cC | dD | eE | fF | gG | hH | iI | jJ | kK | lL | mM | oO | pP | qQ | rR | sS | tT | uU | vV | wW | xX | yY | zZ
    letterNoSs -> AfterASCII | '_' | aA | bB | cC | dD | eE | fF | gG | hH | iI | jJ | kK | lL | mM | nN | oO | pP | qQ | rR | tT | uU | vV | wW | xX | yY | zZ

    anyNonWhiteNoLetterDollar -> digit | specialNoDollar
    anyNonWhiteNoExclamationDotColonDollar -> letter | digit | specialNoExclamationDotColonDollar
    anyNonWhiteNoColonDollar -> letter | digit | specialNoColonDollar
    anyNonWhiteNoEqualDollar -> letter | digit | specialNoEqualDollar
    anyNonWhiteNoQuestionDollar -> letter | digit | specialNoQuestionDollar
    anyNonWhiteNoMinusRightAngleDollar -> letter | digit | specialNoMinusRightAngleDollar
    anyNonWhiteNoDollar -> letter | digit | specialNoDollar
    anyNonWhiteNoDollarBracketSharp -> letter | digit | specialNoDollarBracketSharp

    anyNonWhiteNoOoDollar -> letterNoOo | digit | specialNoDollar
    anyNonWhiteNoPpDollar -> letterNoPp | digit | specialNoDollar
    anyNonWhiteNoTtDollar -> letterNoTt | digit | specialNoDollar
    anyNonWhiteNoIiDollar -> letterNoIi | digit | specialNoDollar
    anyNonWhiteNoNnDollar -> letterNoNn | digit | specialNoDollar
    anyNonWhiteNoSsDollar -> letterNoSs | digit | specialNoDollar

    specialSymbol -> simpleSpecialSymbol
                   | complexSpecialSymbol

    simpleSpecialSymbol ::= '<'
                          | '/'
                          | ':'
                          | ':' ':'
                          | '-'
                          | '%'
                          | '%' oO
                          | '%' oO pP
                          | '%' oO pP tT
                          | '%' oO pP tT iI
                          | '%' oO pP tT iI oO
                          | '%' oO pP tT iI oO nN

    complexSpecialSymbol ::= '<' anyNonWhiteNoLetterDollar
                           | '/' anyNonWhiteNoExclamationDotColonDollar
                           | ':' anyNonWhiteNoColonDollar
                           | ':' ':' anyNonWhiteNoEqualDollar
                           | ':' ':' '=' anyNonWhiteNoQuestionDollar
                           | ':' ':' '=' '?' anyNonWhiteNoDollar
                           | '-' anyNonWhiteNoMinusRightAngleDollar
                           | '-' '>' anyNonWhiteNoQuestionDollar
                           | '-' '>' '?' anyNonWhiteNoDollar
                           | '|' anyNonWhiteNoDollar
                           | '%' anyNonWhiteNoOoDollar
                           | '%' oO anyNonWhiteNoPpDollar
                           | '%' oO pP anyNonWhiteNoTtDollar
                           | '%' oO pP tT anyNonWhiteNoIiDollar
                           | '%' oO pP tT iI anyNonWhiteNoOoDollar
                           | '%' oO pP tT iI oO anyNonWhiteNoNnDollar
                           | '%' oO pP tT iI oO nN anyNonWhiteNoSsDollar
                           | '%' oO pP tT iI oO nN sS anyNonWhiteNoDollar
                           | complexSpecialSymbol anyNonWhiteNoDollar

    number ::= digit
             | number digit

   --
   -- The following rules are used for processing options.
   --
   OptionLines ::= OptionLineList
          /.$BeginJava
                      // What ever needs to happen after the options have been 
                      // scanned must happen here.
            $EndJava
          ./

   OptionLineList ::= OptionLine
                    | OptionLineList OptionLine
   OptionLine ::= options Eol
                | OptionsHeader Eol
                | OptionsHeader optionList Eol
                | OptionsHeader OptionComment Eol
                | OptionsHeader optionList optionWhiteChar OptionComment Eol

   OptionsHeader ::= options optionWhiteChar optionWhite
   
   options ::= '%' oO pP tT iI oO nN sS
          /.$BeginJava
                      makeToken(getLeftSpan(), getRightSpan(), $_OPTIONS_KEY);
            $EndJava
          ./

   OptionComment ::= singleLineComment /.$BeginJava makeComment($_SINGLE_LINE_COMMENT); $EndJava./
   
   _opt -> %Empty
         | '_'
         | '-'

   no ::= nN oO

   none ::= nN oO nN eE

   anyNoMinusSingleQuoteDoublequoteLeftAngleCommaLparenRparen -> letter
                                                               | digit
                                                               | specialNoMinusSingleQuoteDoublequoteLeftAngleCommaLparenRparen

   optionSymbol ::= anyNoMinusSingleQuoteDoublequoteLeftAngleCommaLparenRparen
                  | '-' anyNoMinusSingleQuoteDoublequoteLeftAngleCommaLparenRparen
                  | optionSymbol '-' anyNoMinusSingleQuoteDoublequoteLeftAngleCommaLparenRparen
                  | optionSymbol anyNoMinusSingleQuoteDoublequoteLeftAngleCommaLparenRparen

   Value ::= delimitedSymbol
           | '-'
           | optionSymbol
           | optionSymbol '-'

   optionWhiteChar -> Space | HT | FF
   optionWhite ::= %Empty
                 | optionWhite optionWhiteChar

   optionList ::= option
                | optionList separator option
   separator ::= ','$comma /.$BeginJava  makeToken(getLeftSpan(), getRightSpan(), $_COMMA); $EndJava./
   --
   -- action_block
   -- ast_directory
   -- ast_type
   -- automatic_ast
   -- attributes
   --
   option ::= action_block$ab optionWhite '='$eq optionWhite '('$lp optionWhite filename$fn optionWhite ','$comma1 optionWhite block_begin$bb optionWhite ','$comma2 optionWhite block_end$be optionWhite ')'$rp optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($ab), getRhsLastTokenIndex($ab), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($lp), getRhsLastTokenIndex($lp), $_LEFT_PAREN);
                      makeToken(getRhsFirstTokenIndex($fn), getRhsLastTokenIndex($fn), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($comma1), getRhsLastTokenIndex($comma1), $_COMMA);
                      makeToken(getRhsFirstTokenIndex($bb), getRhsLastTokenIndex($bb), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($comma2), getRhsLastTokenIndex($comma2), $_COMMA);
                      makeToken(getRhsFirstTokenIndex($be), getRhsLastTokenIndex($be), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($rp), getRhsLastTokenIndex($rp), $_RIGHT_PAREN);
            $EndJava
          ./
   action_block ::= aA cC tT iI oO nN _opt bB lL oO cC kK
                  | aA bB
   filename -> Value
   block_begin -> Value
   block_end -> Value

   option ::= ast_directory$ad optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($ad), getRhsLastTokenIndex($ad), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   ast_directory ::= aA sS tT _opt dD iI rR eE cC tT oO rR yY
                   | aA dD 

   option ::= ast_type$at optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($at), getRhsLastTokenIndex($at), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   ast_type ::= aA sS tT _opt tT yY pP eE
              | aA tT 

   option ::= attributes$a optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($a), getRhsLastTokenIndex($a), $_SYMBOL); $EndJava./
            | no attributes$a optionWhite  /.$BeginJava  makeToken(getRhsFirstTokenIndex($a), getRhsLastTokenIndex($a), $_SYMBOL); $EndJava./
   attributes ::= aA tT tT rR iI bB uU tT eE sS

   option ::= automatic_ast$a optionWhite  /.$BeginJava  makeToken(getRhsFirstTokenIndex($a), getRhsLastTokenIndex($a), $_SYMBOL); $EndJava./
            | no automatic_ast$a optionWhite  /.$BeginJava  makeToken(getRhsFirstTokenIndex($a), getRhsLastTokenIndex($a), $_SYMBOL); $EndJava./
   option ::= automatic_ast$aa optionWhite '='$eq optionWhite automatic_ast_value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($aa), getRhsLastTokenIndex($aa), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   automatic_ast ::= aA uU tT oO mM aA tT iI cC _opt aA sS tT
                   | aA aA
   automatic_ast_value ::= none
                         | nN eE sS tT eE dD
                         | tT oO pP _opt lL eE vV eE lL

   --
   -- backtrack
   -- byte
   --
   option ::= backtrack$b optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($b), getRhsLastTokenIndex($b), $_SYMBOL); $EndJava./
            | no backtrack$b optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($b), getRhsLastTokenIndex($b), $_SYMBOL); $EndJava./
   backtrack ::= bB aA cC kK tT rR aA cC kK

   option ::= byte$b optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($b), getRhsLastTokenIndex($b), $_SYMBOL); $EndJava./
            | no byte$b optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($b), getRhsLastTokenIndex($b), $_SYMBOL); $EndJava./
   byte ::= bB yY tT eE
   

   --
   -- conflicts
   --
   option ::= conflicts$c optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($c), getRhsLastTokenIndex($c), $_SYMBOL); $EndJava./
            | no conflicts$c optionWhite  /.$BeginJava  makeToken(getRhsFirstTokenIndex($c), getRhsLastTokenIndex($c), $_SYMBOL); $EndJava./
   conflicts ::= cC oO nN fF lL iI cC tT sS

   --
   -- dat_directory
   -- dat_file
   -- dcl_file
   -- def_file
   -- debug
   --
   option ::= dat_directory$dd optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($dd), getRhsLastTokenIndex($dd), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   dat_directory ::= dD aA tT _opt dD iI rR eE cC tT oO rR yY 
                   | dD dD

   option ::= dat_file$df optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($df), getRhsLastTokenIndex($df), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   dat_file ::= dD aA tT _opt fF iI lL eE

   option ::= dcl_file$df optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($df), getRhsLastTokenIndex($df), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   dcl_file ::= dD cC lL _opt fF iI lL eE

   option ::= def_file$df optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($df), getRhsLastTokenIndex($df), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   def_file ::= dD eE fF _opt fF iI lL eE

   option ::= debug$d optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($d), getRhsLastTokenIndex($d), $_SYMBOL); $EndJava./
            | no debug$d optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($d), getRhsLastTokenIndex($d), $_SYMBOL); $EndJava./
   debug ::= dD eE bB uU gG

   --
   -- edit
   -- error_maps
   -- escape
   -- export_terminals
   -- extends_parsetable
   --
   option ::= edit$e optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($e), getRhsLastTokenIndex($e), $_SYMBOL); $EndJava./
            | no edit$e optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($e), getRhsLastTokenIndex($e), $_SYMBOL); $EndJava./
   edit ::= eE dD iI tT

   option ::= error_maps$e optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($e), getRhsLastTokenIndex($e), $_SYMBOL); $EndJava./
            | no error_maps$e optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($e), getRhsLastTokenIndex($e), $_SYMBOL); $EndJava./
   error_maps ::= eE rR rR oO rR _opt mM aA pP sS
                | eE mM

   option ::= escape$e optionWhite '='$eq optionWhite anyNonWhiteChar$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($e), getRhsLastTokenIndex($e), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   escape ::= eE sS cC aA pP eE

   option ::= export_terminals$et optionWhite '='$eq optionWhite filename$fn optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($et), getRhsLastTokenIndex($et), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($fn), getRhsLastTokenIndex($fn), $_SYMBOL);
            $EndJava
          ./
   option ::= export_terminals$et optionWhite '='$eq optionWhite '('$lp optionWhite filename$fn optionWhite ')'$rp optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($et), getRhsLastTokenIndex($et), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($lp), getRhsLastTokenIndex($lp), $_LEFT_PAREN);
                      makeToken(getRhsFirstTokenIndex($fn), getRhsLastTokenIndex($fn), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($rp), getRhsLastTokenIndex($rp), $_RIGHT_PAREN);
            $EndJava
          ./
   option ::= export_terminals$et optionWhite '='$eq optionWhite '('$lp optionWhite filename$fn optionWhite ','$comma optionWhite export_prefix$ep optionWhite ')'$rp optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($et), getRhsLastTokenIndex($et), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($lp), getRhsLastTokenIndex($lp), $_LEFT_PAREN);
                      makeToken(getRhsFirstTokenIndex($fn), getRhsLastTokenIndex($fn), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($comma), getRhsLastTokenIndex($comma), $_COMMA);
                      makeToken(getRhsFirstTokenIndex($ep), getRhsLastTokenIndex($ep), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($rp), getRhsLastTokenIndex($rp), $_RIGHT_PAREN);
            $EndJava
          ./
   option ::= export_terminals$et optionWhite '='$eq optionWhite '('$lp optionWhite filename$fn optionWhite ','$comma1 optionWhite export_prefix$ep optionWhite ','$comma2 optionWhite export_suffix$es optionWhite ')'$rp optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($et), getRhsLastTokenIndex($et), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($lp), getRhsLastTokenIndex($lp), $_LEFT_PAREN);
                      makeToken(getRhsFirstTokenIndex($fn), getRhsLastTokenIndex($fn), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($comma1), getRhsLastTokenIndex($comma1), $_COMMA);
                      makeToken(getRhsFirstTokenIndex($ep), getRhsLastTokenIndex($ep), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($comma2), getRhsLastTokenIndex($comma2), $_COMMA);
                      makeToken(getRhsFirstTokenIndex($es), getRhsLastTokenIndex($es), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($rp), getRhsLastTokenIndex($rp), $_RIGHT_PAREN);
            $EndJava
          ./
   export_terminals ::= eE xX pP oO rR tT _opt tT eE rR mM iI nN aA lL sS
                      | eE tT 
   export_prefix -> Value
   export_suffix -> Value

   option ::= extends_parsetable$e optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($e), getRhsLastTokenIndex($e), $_SYMBOL); $EndJava./
            | no extends_parsetable$e optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($e), getRhsLastTokenIndex($e), $_SYMBOL); $EndJava./
   option ::= extends_parsetable$ep optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($ep), getRhsLastTokenIndex($ep), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   extends_parsetable ::= eE xX tT eE nN dD sS _opt pP aA rR sS eE tT aA bB lL eE
                        | eE pP

   --
   -- factory
   -- file_prefix
   -- filter
   -- first
   -- follow
   --
   option ::= factory$f optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($f), getRhsLastTokenIndex($f), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   factory ::= fF aA cC tT oO rR yY

   option ::= file_prefix$fp optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($fp), getRhsLastTokenIndex($fp), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   file_prefix ::= fF iI lL eE _opt pP rR eE fF iI xX
                 | fF pP

   option ::= filter$f optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($f), getRhsLastTokenIndex($f), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   filter ::= fF iI lL tT eE rR

   option ::= first$f optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($f), getRhsLastTokenIndex($f), $_SYMBOL); $EndJava./
            | no first$f optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($f), getRhsLastTokenIndex($f), $_SYMBOL); $EndJava./
   first ::= fF iI rR sS tT

   option ::= follow$f optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($f), getRhsLastTokenIndex($f), $_SYMBOL); $EndJava./
            | no follow$f optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($f), getRhsLastTokenIndex($f), $_SYMBOL); $EndJava./
   follow ::= fF oO lL lL oO wW

   --
   -- goto_default
   --
   option ::= goto_default$g optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($g), getRhsLastTokenIndex($g), $_SYMBOL); $EndJava./
            | no goto_default$g optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($g), getRhsLastTokenIndex($g), $_SYMBOL); $EndJava./
   goto_default ::= gG oO tT oO _opt dD eE fF aA uU lL tT
                  | gG dD

   --
   -- Headers
   --
   option ::= headers$h optionWhite '='$eq optionWhite '('$lp optionWhite filename$fn optionWhite ','$comma1 optionWhite block_begin$bb optionWhite ','$comma2 optionWhite block_end$be optionWhite ')'$rp optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($h), getRhsLastTokenIndex($h), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($lp), getRhsLastTokenIndex($lp), $_LEFT_PAREN);
                      makeToken(getRhsFirstTokenIndex($fn), getRhsLastTokenIndex($fn), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($comma1), getRhsLastTokenIndex($comma1), $_COMMA);
                      makeToken(getRhsFirstTokenIndex($bb), getRhsLastTokenIndex($bb), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($comma2), getRhsLastTokenIndex($comma2), $_COMMA);
                      makeToken(getRhsFirstTokenIndex($be), getRhsLastTokenIndex($be), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($rp), getRhsLastTokenIndex($rp), $_RIGHT_PAREN);
            $EndJava
          ./
   headers ::= hH eE aA dD eE rR sS

   --
   -- imp_file
   -- import_terminals
   -- include_directory/include_directories
   --
   option ::= imp_file$if optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($if), getRhsLastTokenIndex($if), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   imp_file ::= iI mM pP _opt fF iI lL eE
              | iI fF 

   option ::= import_terminals$it optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($it), getRhsLastTokenIndex($it), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   import_terminals ::= iI mM pP oO rR tT _opt tT eE rR mM iI nN aA lL sS
                      | iI tT

   option ::= include_directory$id optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($id), getRhsLastTokenIndex($id), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   include_directory ::= iI nN cC lL uU dD eE _opt dD iI rR eE cC tT oO rR yY
                       | iI nN cC lL uU dD eE _opt dD iI rR eE cC tT oO rR iI eE sS 
                       | iI dD

   --
   -- lalr_level
   -- list
   --
   option ::= lalr_level$l optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($l), getRhsLastTokenIndex($l), $_SYMBOL); $EndJava./
            | no lalr_level$l optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($l), getRhsLastTokenIndex($l), $_SYMBOL); $EndJava./
   option ::= lalr_level$l optionWhite '='$eq optionWhite number$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($l), getRhsLastTokenIndex($l), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   lalr_level ::= lL aA lL rR _opt lL eE vV eE lL
                | lL aA lL rR
                | lL aA
                | lL lL

   option ::= list$l optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($l), getRhsLastTokenIndex($l), $_SYMBOL); $EndJava./
            | no list$l optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($l), getRhsLastTokenIndex($l), $_SYMBOL); $EndJava./
   list ::= lL iI sS tT 

   --
   -- margin
   -- max_cases
   --
   option ::= margin$m optionWhite '='$eq optionWhite number$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($m), getRhsLastTokenIndex($m), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   margin ::= mM aA rR gG iI nN

   option ::= max_cases$mc optionWhite '='$eq optionWhite number$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($mc), getRhsLastTokenIndex($mc), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   max_cases ::= mM aA xX _opt cC aA sS eE sS
               | mM cC

   --
   -- names
   -- nt_check
   --
   option ::= names$n optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($n), getRhsLastTokenIndex($n), $_SYMBOL); $EndJava./
            | no names$n optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($n), getRhsLastTokenIndex($n), $_SYMBOL); $EndJava./
   option ::= names$n optionWhite '='$eq optionWhite names_value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($n), getRhsLastTokenIndex($n), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   names ::= nN aA mM eE sS

   names_value ::= oO pP tT iI mM iI zZ eE dD
                 | mM aA xX iI mM uU mM
                 | mM iI nN iI mM uU mM
   

   option ::= nt_check$n optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($n), getRhsLastTokenIndex($n), $_SYMBOL); $EndJava./
            | no nt_check$n optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($n), getRhsLastTokenIndex($n), $_SYMBOL); $EndJava./
   nt_check ::= nN tT _opt cC hH eE cC kK
              | nN cC

   --
   -- or_marker
   -- out_directory
   --
   option ::= or_marker$om optionWhite '='$eq optionWhite anyNonWhiteChar$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($om), getRhsLastTokenIndex($om), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   or_marker ::= oO rR _opt mM aA rR kK eE rR
               | oO mM 

   option ::= out_directory$dd optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($dd), getRhsLastTokenIndex($dd), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   out_directory ::= oO uU tT _opt dD iI rR eE cC tT oO rR yY 
                   | oO dD

   --
   -- package
   -- parent_saved
   -- parsetable_interfaces
   -- prefix
   -- priority
   -- programming_language
   -- prs_file
   --
   option ::= parent_saved$ps optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($ps), getRhsLastTokenIndex($ps), $_SYMBOL); $EndJava ./
            | no parent_saved$ps optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($ps), getRhsLastTokenIndex($ps), $_SYMBOL); $EndJava ./
   parent_saved ::= pP aA rR eE nN tT _opt sS aA vV eE dD
                  | pP sS

   option ::= package$p optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($p), getRhsLastTokenIndex($p), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   package ::= pP aA cC kK aA gG eE

   option ::= parsetable_interfaces$pi optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($pi), getRhsLastTokenIndex($pi), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   parsetable_interfaces ::= pP aA rR sS eE tT aA bB lL eE _opt iI nN tT eE rR fF aA cC eE sS
                           | pP aA rR sS eE tT aA bB lL eE
                           | pP iI

   option ::= prefix$p optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($p), getRhsLastTokenIndex($p), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   prefix ::= pP rR eE fF iI xX

   option ::= priority$p optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($p), getRhsLastTokenIndex($p), $_SYMBOL); $EndJava./
            | no priority$p optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($p), getRhsLastTokenIndex($p), $_SYMBOL); $EndJava./
   priority ::= pP rR iI oO rR iI tT yY

   option ::= programming_language$pl optionWhite '='$eq optionWhite programming_language_value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($pl), getRhsLastTokenIndex($pl), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   programming_language ::= pP rR oO gG rR aA mM mM iI nN gG _opt lL aA nN gG uU aA gG eE
                          | pP lL
   programming_language_value ::= none
                                | xX mM lL
                                | cC
                                | cC pP pP
                                | jJ aA vV aA
                                | pP lL xX
                                | pP lL xX aA sS mM
                                | mM lL
   option ::= prs_file$pf optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($pf), getRhsLastTokenIndex($pf), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   prs_file ::= pP rR sS _opt fF iI lL eE
              | pP fF
   

   --
   -- quiet
   --
   option ::= quiet$q optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($q), getRhsLastTokenIndex($q), $_SYMBOL); $EndJava./
            | no quiet$q optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($q), getRhsLastTokenIndex($q), $_SYMBOL); $EndJava./
   quiet ::= qQ uU iI eE tT

   --
   -- read_reduce
   -- remap_terminals
   --
   option ::= read_reduce$r optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($r), getRhsLastTokenIndex($r), $_SYMBOL); $EndJava./
            | no read_reduce$r optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($r), getRhsLastTokenIndex($r), $_SYMBOL); $EndJava./
   read_reduce ::= rR eE aA dD _opt rR eE dD uU cC eE
                 | rR rR

   option ::= remap_terminals$r optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($r), getRhsLastTokenIndex($r), $_SYMBOL); $EndJava./
            | no remap_terminals$r optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($r), getRhsLastTokenIndex($r), $_SYMBOL); $EndJava./
   remap_terminals ::= rR eE mM aA pP _opt tT eE rR mM iI nN aA lL sS
                     | rR tT

   --
   -- scopes
   -- serialize
   -- shift_default
   -- single_productions
   -- slr
   -- soft_keywords
   -- states
   -- suffix
   -- sym_file
   --
   option ::= scopes$s optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($s), getRhsLastTokenIndex($s), $_SYMBOL); $EndJava ./
            | no scopes$s optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($s), getRhsLastTokenIndex($s), $_SYMBOL); $EndJava ./
   scopes ::= sS cC oO pP eE sS

   option ::= serialize$s optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($s), getRhsLastTokenIndex($s), $_SYMBOL); $EndJava ./
            | no serialize$s optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($s), getRhsLastTokenIndex($s), $_SYMBOL); $EndJava ./
   serialize ::= sS eE rR iI aA lL iI zZ eE

   option ::= shift_default$s optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($s), getRhsLastTokenIndex($s), $_SYMBOL); $EndJava ./
            | no shift_default$s optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($s), getRhsLastTokenIndex($s), $_SYMBOL); $EndJava ./
   shift_default ::= sS hH iI fF tT _opt dD eE fF aA uU lL tT
                   | sS dD

   option ::= single_productions$s optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($s), getRhsLastTokenIndex($s), $_SYMBOL); $EndJava ./
            | no single_productions$s optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($s), getRhsLastTokenIndex($s), $_SYMBOL); $EndJava ./
   single_productions ::= sS iI nN gG lL eE _opt pP rR oO dD uU cC tT iI oO nN sS
                        | sS pP

   option ::= slr$s optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($s), getRhsLastTokenIndex($s), $_SYMBOL); $EndJava ./
            | no slr$s optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($s), getRhsLastTokenIndex($s), $_SYMBOL); $EndJava ./
   slr ::= sS lL rR

   option ::= soft_keywords$s optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($s), getRhsLastTokenIndex($s), $_SYMBOL); $EndJava ./
            | no soft_keywords$s optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($s), getRhsLastTokenIndex($s), $_SYMBOL); $EndJava ./
   soft_keywords ::= sS oO fF tT _opt kK eE yY wW oO rR dD sS 
                   | sS oO fF tT
                   | sS kK

   option ::= states$s optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($s), getRhsLastTokenIndex($s), $_SYMBOL); $EndJava ./
            | no states$s optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($s), getRhsLastTokenIndex($s), $_SYMBOL); $EndJava ./
   states ::= sS tT aA tT eE sS

   option ::= suffix$s optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($s), getRhsLastTokenIndex($s), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   suffix ::= sS uU fF fF iI xX 

   option ::= sym_file$sf optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($sf), getRhsLastTokenIndex($sf), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   sym_file ::= sS yY mM _opt fF iI lL eE
              | sS fF 

   --
   -- tab_file
   -- table
   -- template
   -- trace
   --
   option ::= tab_file$tf optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($tf), getRhsLastTokenIndex($tf), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   tab_file ::= tT aA bB _opt fF iI lL eE
              | tT fF

   option ::= template$t optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($t), getRhsLastTokenIndex($t), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   template ::= tT eE mM pP lL aA tT eE
   

   option ::= trailers$t optionWhite '='$eq optionWhite '('$lp optionWhite filename$fn optionWhite ','$comma1 optionWhite block_begin$bb optionWhite ','$comma2 optionWhite block_end$be optionWhite ')'$rp optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($t), getRhsLastTokenIndex($t), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($lp), getRhsLastTokenIndex($lp), $_LEFT_PAREN);
                      makeToken(getRhsFirstTokenIndex($fn), getRhsLastTokenIndex($fn), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($comma1), getRhsLastTokenIndex($comma1), $_COMMA);
                      makeToken(getRhsFirstTokenIndex($bb), getRhsLastTokenIndex($bb), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($comma2), getRhsLastTokenIndex($comma2), $_COMMA);
                      makeToken(getRhsFirstTokenIndex($be), getRhsLastTokenIndex($be), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($rp), getRhsLastTokenIndex($rp), $_RIGHT_PAREN);
            $EndJava
          ./
   trailers ::= tT rR aA iI lL eE rR sS

   option ::= table$t optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($t), getRhsLastTokenIndex($t), $_SYMBOL); $EndJava ./
            | no table$t optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($t), getRhsLastTokenIndex($t), $_SYMBOL); $EndJava ./
   option ::= table$t optionWhite '='$eq optionWhite programming_language_value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($t), getRhsLastTokenIndex($t), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   table ::= tT aA bB lL eE 

   option ::= trace$t optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($t), getRhsLastTokenIndex($t), $_SYMBOL); $EndJava ./
            | no trace$t optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($t), getRhsLastTokenIndex($t), $_SYMBOL); $EndJava ./
   option ::= trace$t optionWhite '='$eq optionWhite trace_value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($t), getRhsLastTokenIndex($t), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   trace ::= tT rR aA cC eE

   trace_value ::= none
                 | cC oO nN fF lL iI cC tT sS
                 | fF uU lL lL

   --
   -- variables
   -- verbose
   -- visitor
   -- visitor_type
   --
   option ::= variables$v optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($v), getRhsLastTokenIndex($v), $_SYMBOL); $EndJava ./
            | no variables$v optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($v), getRhsLastTokenIndex($v), $_SYMBOL); $EndJava ./
   option ::= variables$v optionWhite '='$eq optionWhite variables_value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($v), getRhsLastTokenIndex($v), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   variables ::= vV aA rR iI aA bB lL eE sS
   variables_value ::= none
                     | bB oO tT hH
                     | tT eE rR mM iI nN aA lL sS
                     | nN oO nN _opt tT eE rR mM iI nN aA lL sS
                     | nN tT

   option ::= verbose$v optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($v), getRhsLastTokenIndex($v), $_SYMBOL); $EndJava ./
            | no verbose$v optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($v), getRhsLastTokenIndex($v), $_SYMBOL); $EndJava ./
   verbose ::= vV eE rR bB oO sS eE

   option ::= visitor$v optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($v), getRhsLastTokenIndex($v), $_SYMBOL); $EndJava ./
            | no visitor$v optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($v), getRhsLastTokenIndex($v), $_SYMBOL); $EndJava ./
   option ::= visitor$v optionWhite '='$eq optionWhite visitor_value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($v), getRhsLastTokenIndex($v), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   visitor ::= vV iI sS iI tT oO rR
   visitor_value ::= none
                   | dD eE fF aA uU lL tT
                   | pP rR eE oO rR dD eE rR

   option ::= visitor_type$vt optionWhite '='$eq optionWhite Value$val optionWhite
          /.$BeginJava
                      makeToken(getRhsFirstTokenIndex($vt), getRhsLastTokenIndex($vt), $_SYMBOL);
                      makeToken(getRhsFirstTokenIndex($eq), getRhsLastTokenIndex($eq), $_EQUAL);
                      makeToken(getRhsFirstTokenIndex($val), getRhsLastTokenIndex($val), $_SYMBOL);
            $EndJava
          ./
   visitor_type ::= vV iI sS iI tT oO rR _opt tT yY pP eE
                  | vV tT

   --
   -- warnings
   --
   option ::= warnings$w optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($w), getRhsLastTokenIndex($w), $_SYMBOL); $EndJava ./
            | no warnings$w optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($w), getRhsLastTokenIndex($w), $_SYMBOL); $EndJava ./
   warnings ::= wW aA rR nN iI nN gG sS

   --
   -- xref
   --
   option ::= xreference$x optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($x), getRhsLastTokenIndex($x), $_SYMBOL); $EndJava ./
            | no xreference$x optionWhite /.$BeginJava  makeToken(getRhsFirstTokenIndex($x), getRhsLastTokenIndex($x), $_SYMBOL); $EndJava ./
   xreference ::= xX rR eE fF
                | xX rR eE fF eE rR eE nN cC eE
%End
