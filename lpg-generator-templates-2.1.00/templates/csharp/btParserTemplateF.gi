--
-- In a parser using this template, the following macro may be redefined:
--
--     $additional_interfaces
--     $ast_class
--
-- B E G I N N I N G   O F   T E M P L A T E   btParserTemplateF
--
%Options programming_language=csharp,margin=4,backtrack
%Options table,error_maps,scopes
%options prefix=TK_
%options action-block=("*.cs", "/.", "./")
%options ParseTable=LPG2.Runtime.ParseTable
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
                // Rule $rule_number:  $rule_text
                //
                ./

    $BeginAction
    /.$Header$case $rule_number: {
                   //#line $next_line "$input_file$"./

    $EndAction
    /.            break;
                }./

    $BeginJava
    /.$Header$case $rule_number: {
                    $symbol_declarations
                    //#line $next_line "$input_file$"./

    $EndJava /.$EndAction./

    $NoAction
    /.$Header$case $rule_number:
                    break;./

    $BadAction
    /.$Header$case $rule_number:
                    throw ("No action specified for rule " + $rule_number);./

    $NullAction
    /.$Header$case $rule_number:
                    setResult(null);
                    break;./

    $BeginActions
    /.
         // Casting object to various generic types
        public void ruleAction(int ruleNumber)
        {
            switch (ruleNumber)
            {./

    $SplitActions
    /.
                    default:
                        ruleAction$rule_number(ruleNumber);
                        break;
                }
                return;
            }
        
            public void ruleAction$rule_number(int ruleNumber)
            {
                switch (ruleNumber)
                {
                    //#line $next_line "$input_file$"./

    $EndActions
    /.
                default:
                    break;
            }
            return;
        }./

    $entry_declarations
    /.
        public $ast_class parse$entry_name()
        {
            return parse$entry_name(null, 0);
        }
            
        public $ast_class parse$entry_name(Monitor monitor)
        {
            return parse$entry_name(monitor, 0);
        }
            
        public $ast_class parse$entry_name(int error_repair_count)
        {
            return parse$entry_name(null, error_repair_count);
        }
            
        public $ast_class parse$entry_name(Monitor monitor, int error_repair_count)
        {
            btParser.setMonitor(monitor);
            
            try
            {
                return ($ast_class) btParser.fuzzyParseEntry($sym_type.$entry_marker, error_repair_count);
            }
            catch (BadParseException e)
            {
                prsStream.reset(e.error_token); // point to error token

                DiagnoseParser diagnoseParser = new DiagnoseParser(prsStream, prsTable);
                diagnoseParser.diagnoseEntry($sym_type.$entry_marker, e.error_token);
            }

            return null;
        }
    ./

    --
    -- Macros that may be needed in a parser using this template
    --
    $additional_interfaces /../
    $ast_class /.$ast_type./
    $super_class /.object./   
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
    using  LPG2.Runtime;
    using System;
    ./
%End

%Headers
    /.
    public class $action_type : $super_class , RuleAction$additional_interfaces
    {
        private PrsStream prsStream = null;
        
        private bool unimplementedSymbolsWarning = $unimplemented_symbols_warning;

        private static ParseTable prsTable = new $prs_type();
        public ParseTable getParseTable() { return prsTable; }

        private BacktrackingParser btParser = null;
        public BacktrackingParser getParser() { return btParser; }

        private void setResult(object _object) { btParser.setSym1(_object); }
        public object getRhsSym(int i) { return btParser.getSym(i); }

        public int getRhsTokenIndex(int i) { return btParser.getToken(i); }
        public IToken getRhsIToken(int i) { return prsStream.getIToken(getRhsTokenIndex(i)); }
        
        public int getRhsFirstTokenIndex(int i) { return btParser.getFirstToken(i); }
        public IToken getRhsFirstIToken(int i) { return prsStream.getIToken(getRhsFirstTokenIndex(i)); }

        public int getRhsLastTokenIndex(int i) { return btParser.getLastToken(i); }
        public IToken getRhsLastIToken(int i) { return prsStream.getIToken(getRhsLastTokenIndex(i)); }

        public int getLeftSpan() { return btParser.getFirstToken(); }
        public IToken getLeftIToken()  { return prsStream.getIToken(getLeftSpan()); }

        public int getRightSpan() { return btParser.getLastToken(); }
        public IToken getRightIToken() { return prsStream.getIToken(getRightSpan()); }

        public int getRhsErrorTokenIndex(int i)
        {
            int index = btParser.getToken(i);
            IToken err = prsStream.getIToken(index);
            return (err is ErrorToken ? index : 0);
        }
        public ErrorToken getRhsErrorIToken(int i)
        {
            int index = btParser.getToken(i);
            IToken err = prsStream.getIToken(index);
            return (ErrorToken) (err is ErrorToken ? err : null);
        }

        public void reset(ILexStream lexStream)
        {
            prsStream = new PrsStream(lexStream);
            btParser.reset(prsStream);

            try
            {
                prsStream.remapTerminalSymbols(orderedTerminalSymbols(), prsTable.getEoftSymbol());
            }
            catch (NullExportedSymbolsException e) {
            }
            catch (NullTerminalSymbolsException e) {
            }
            catch (UnimplementedTerminalsException e)
            {
                if (unimplementedSymbolsWarning) {
                    ArrayListHelper<int> unimplemented_symbols =  new ArrayListHelper<int>( e.getSymbols());
                    Console.Out.WriteLine("The Lexer will not scan the following token(s):");
                    for (int i = 0; i < unimplemented_symbols.Count; i++)
                    {
                        int  id = unimplemented_symbols.get(i);
                        Console.Out.WriteLine("    " + $sym_type.orderedTerminalSymbols[id]);               
                    }
                    Console.Out.WriteLine();
                }
            }
            catch (UndefinedEofSymbolException e)
            {
                throw (new UndefinedEofSymbolException
                                    ("The Lexer does not implement the Eof symbol " +
                                     $sym_type.orderedTerminalSymbols[prsTable.getEoftSymbol()]));
            } 
        }
        
        public $action_type()
        {
            try
            {
                btParser = new BacktrackingParser(prsStream, prsTable, (RuleAction) this);
            }
            catch (NotBacktrackParseTableException e)
            {
                throw (new NotBacktrackParseTableException
                                    ("Regenerate $prs_type.cs with -BACKTRACK option"));
            }
            catch (BadParseSymFileException e)
            {
                throw (new BadParseSymFileException("Bad Parser Symbol File -- $sym_type.cs"));
            }
        }
        
        public $action_type(ILexStream lexStream):this()
        {
            
            reset(lexStream);
        }
        
        public int numTokenKinds() { return $sym_type.numTokenKinds; }
        public string[] orderedTerminalSymbols() { return $sym_type.orderedTerminalSymbols; }
        public string getTokenKindName(int kind) { return $sym_type.orderedTerminalSymbols[kind]; }
        public int getEOFTokenKind() { return prsTable.getEoftSymbol(); }
        public IPrsStream getIPrsStream() { return prsStream; }

        /**
         * @deprecated replaced by {@link #getIPrsStream()}
         *
         */
        public PrsStream getPrsStream() { return prsStream; }

        /**
         * @deprecated replaced by {@link #getIPrsStream()}
         *
         */
        public PrsStream getParseStream() { return prsStream; }

        public $ast_class parser()
        {
            return parser(null, 0);
        }
        
        public $ast_class parser(Monitor monitor)
        {
            return parser(monitor, 0);
        }
        
        public $ast_class parser(int error_repair_count)
        {
            return parser(null, error_repair_count);
        }

        public $ast_class parser(Monitor monitor, int error_repair_count)
        {
            btParser.setMonitor(monitor);
            
            try
            {
                return ($ast_class) btParser.fuzzyParse(error_repair_count);
            }
            catch (BadParseException e)
            {
                prsStream.reset(e.error_token); // point to error token

                DiagnoseParser diagnoseParser = new DiagnoseParser(prsStream, prsTable);
                diagnoseParser.diagnose(e.error_token);
            }

            return null;
        }

        //
        // Additional entry points, if any
        //
        $entry_declarations
    ./

%End

%Rules
    /.$BeginActions./
%End

%Trailers
    /.
        $EndActions
    }
    ./
%End

--
-- E N D   O F   T E M P L A T E
--
