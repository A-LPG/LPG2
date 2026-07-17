--
-- In a parser using this template, the following macro may be redefined:
--
--     %additional_interfaces
--     %ast_class
--
-- B E G I N N I N G   O F   T E M P L A T E   btParserTemplateF
--
%Options programming_Language=dart,margin=4,backtrack
%Options table,error_maps,scopes
%options prefix=TK_
%options action-block=("*.dart", "/.", "./")
%options ParseTable=ParseTable
%options nt-check

--
-- This template requires that the name of the EOF token be set
-- to EOF_TOKEN to be consistent with LexerTemplateD and LexerTemplateE
--
%EOF
    EOF_TOKEN
%End

%ERROR
    ERROR_TOKEN
%End

%Define

    $Header
    /.
                //
                // Rule %rule_number:  %rule_text
                //
                ./

    $DefaultAction
    /.%Header _rule_action[%rule_number]=(){./

    $BeginAction
    /.%DefaultAction
                   //#line %next_line "%input_file%"./

    $EndAction
    /.            
                };./

    $BeginJava
    /.%Header%case %rule_number: {
                    %symbol_declarations
                    //#line %next_line "%input_file%"./

    $EndJava /.%EndAction./

    $NoAction
    /.%Header
                     ./

    $BadAction
    /.%DefaultAction
                    throw ("No action specified for rule " + %rule_number.toString());./

    $NullAction
    /.$DefaultAction 
                    setResult(null);
                    ./

    $BeginActions
    /.
        void initRuleAction(){
    ./


    $EndActions
    /.}
    ./

    $entry_declarations
    /.
       
        %ast_class?  parse%entry_name([Monitor? monitor, int error_repair_count= 0]) 
        {
            btParser.setMonitor(monitor);
            
            try
            {
                return btParser.fuzzyParseEntry(%sym_type.%entry_marker, error_repair_count) as %ast_class?;
            }
            on BadParseException catch(e)
            {
                prsStream.reset(e.error_token); // point to error token
                var diagnoseParser = DiagnoseParser(prsStream, %action_type.prsTable);
                diagnoseParser.diagnoseEntry(%sym_type.%entry_marker, e.error_token);
            }

            return null;
        }
    ./

    --
    -- Macros that may be needed in a parser using this template
    --
    $additional_interfaces /../
    $ast_class /.%ast_type./
    $super_class /.Object./   
    $unimplemented_symbols_warning /.false./

    --
    -- Old deprecated macros that should NEVER be used.
    --
    $setSym1 /. // macro setSym1 is deprecated. Use function setResult
                getParser().setSym1./
    $setResult /. // macro setResult is deprecated. Use function setResult
                 getParser().setSym1./
    $getSym /. // macro getSym is deprecated. Use function getRhsSym
              getParser().getSym./
    $getToken /. // macro getToken is deprecated. Use function getRhsTokenIndex
                getParser().getToken./
    $getIToken /. // macro getIToken is deprecated. Use function getRhsIToken
                 prsStream.getIToken./
    $getLeftSpan /. // macro getLeftSpan is deprecated. Use function getLeftSpan
                   getParser().getFirstToken./
    $getRightSpan /. // macro getRightSpan is deprecated. Use function getRightSpan
                    getParser().getLastToken./
%End

%Globals
    /.
    import 'package:lpg2/lpg2.dart';
    import 'dart:io';
    import '%prs_type.dart';
    import '%sym_type.dart';
    ./
%End

%Headers
    /.
    class %action_type extends %super_class with ProstheticAstProvider implements RuleAction%additional_interfaces
    {

        @override
        void ruleAction(int ruleNumber) {
               var act = _rule_action[ruleNumber];
                if(null != act){
                    act(); 
                }
        }

        PrsStream prsStream = PrsStream();
    
        bool unimplementedSymbolsWarning = %unimplemented_symbols_warning;

        static  ParseTable prsTable  = %prs_type();
        ParseTable getParseTable()  { return %action_type.prsTable; }

        late BacktrackingParser btParser;
        BacktrackingParser getParser(){ return btParser; }

        void setResult(Object? object1){ btParser.setSym1(object1); }
        Object? getRhsSym(int i){ return btParser.getSym(i); }

        int getRhsTokenIndex(int i)  { return btParser.getToken(i); }
        IToken getRhsIToken(int i)   { return prsStream.getIToken(getRhsTokenIndex(i)); }
    
        int getRhsFirstTokenIndex(int i)   { return btParser.getFirstToken(i); }
        IToken getRhsFirstIToken(int i)  { return prsStream.getIToken(getRhsFirstTokenIndex(i)); }

        int getRhsLastTokenIndex(int i)  { return btParser.getLastToken(i); }
        IToken getRhsLastIToken(int i)  { return prsStream.getIToken(getRhsLastTokenIndex(i)); }

        int getLeftSpan() { return btParser.getFirstToken(); }
        IToken getLeftIToken()   { return prsStream.getIToken(getLeftSpan()); }

        int getRightSpan() { return btParser.getLastToken(); }
        IToken getRightIToken() { return prsStream.getIToken(getRightSpan()); }

        int getRhsErrorTokenIndex(int i)  
        {
            var index = btParser.getToken(i);
            var err = prsStream.getIToken(index);
            return (err is ErrorToken ? index : 0);
        }
        ErrorToken? getRhsErrorIToken(int i)  
        {
            var index = btParser.getToken(i);
            var err = prsStream.getIToken(index);
            return err as ErrorToken?;
        }

        void  reset(ILexStream lexStream)   
        {
            prsStream.resetLexStream(lexStream);
            btParser.reset(prsStream);

            try
            {
                prsStream.remapTerminalSymbols(orderedTerminalSymbols(), %action_type.prsTable.getEoftSymbol());
            } 
            on NullExportedSymbolsException{}
            on UnimplementedTerminalsException catch (e)
            {
                if (unimplementedSymbolsWarning) {
                    var unimplemented_symbols = e.getSymbols();
                    stdout.writeln("The Lexer will not scan the following token(s):");
                    for (var i  = 0; i < unimplemented_symbols!.size(); i++)
                    {
                        int id = unimplemented_symbols.get(i);
                        stdout.writeln("    " + %sym_type.orderedTerminalSymbols[id]);               
                    }
                    stdout.writeln();
                }
            }
            on UndefinedEofSymbolException catch (e)
            {
                throw (UndefinedEofSymbolException
                    ("The Lexer does not implement the Eof symbol " +
                    %sym_type.orderedTerminalSymbols[%action_type.prsTable.getEoftSymbol()]));
            }

        }
        List<dynamic?> _rule_action = List.filled(%num_rules + 2, null);
        %action_type([ILexStream? lexStream])
        {
            initRuleAction();
            try
            {
                btParser = BacktrackingParser(null, %action_type.prsTable, this);
            }
            on  NotBacktrackParseTableException
            {
                throw (NotBacktrackParseTableException
                    ("Regenerate %prs_type.dart with -BACKTRACK option"));
            }
            on BadParseSymFileException
            {
                throw (BadParseSymFileException("Bad Parser Symbol File -- %sym_type.dart"));
            }

            if(lexStream != null){
              reset(lexStream);
            }
        }
        
       
        
        int numTokenKinds(){ return %sym_type.numTokenKinds; }
        List<String>  orderedTerminalSymbols()  { return %sym_type.orderedTerminalSymbols; }
        String  getTokenKindName(int kind)   { return %sym_type.orderedTerminalSymbols[kind]; }
        int  getEOFTokenKind() { return %action_type.prsTable.getEoftSymbol(); }
        IPrsStream getIPrsStream(){ return prsStream; }


        %ast_class? parser([int error_repair_count = 0 ,Monitor?  monitor])
        {
            btParser.setMonitor(monitor);
            
            try{
                return btParser.fuzzyParse(error_repair_count) as %ast_class?;
            }
            on BadParseException catch(e){
                prsStream.reset(e.error_token); // point to error token
                var diagnoseParser = DiagnoseParser(prsStream, %action_type.prsTable);
                diagnoseParser.diagnose(e.error_token);
            }
            return null;
        }

        //
        // Additional entry points, if any
        //
        %entry_declarations
    ./

%End

%Rules
    /.%BeginActions./
%End

%Trailers
    /.
        %EndActions
    }
    ./
%End

--
-- E N D   O F   T E M P L A T E
--
