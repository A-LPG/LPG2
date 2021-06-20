--
-- In a parser using this template, the following macro may be redefined:
--
--     $additional_interfaces
--     $ast_class
--
-- B E G I N N I N G   O F   T E M P L A T E   dtParserTemplateF
--
%Options programming_language=java,margin=4
%Options table,error_maps,scopes
%Options prefix=TK_
%Options action-block=("*.java", "/.", "./")
%Options ParseTable=lpg.runtime.ParseTable

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
                    throw new Error("No action specified for rule " + $rule_number);./

    $NullAction
    /.$Header$case $rule_number:
                    setResult(null);
                    break;./

    $BeginActions
    /.
        public void ruleAction(int ruleNumber)
        {
            switch (ruleNumber)
            {
                //#line $next_line "$input_file$"./

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
	        {./

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
            
        public void resetParse$entry_name()
        {
            dtParser.resetParserEntry($sym_type.$entry_marker);
        }
        
        public $ast_class parse$entry_name(Monitor monitor, int error_repair_count)
        {
            dtParser.setMonitor(monitor);
            
            try
            {
                return ($ast_class) dtParser.parseEntry($sym_type.$entry_marker);
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
        
    $additional_interfaces /../
    $ast_class /.$ast_type./
    $super_class /.Object./
    $unimplemented_symbols_warning /.false./
    
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
    /.import lpg.runtime.*;
    ./
%End

%Headers
    /.
    public class $action_type extends $super_class implements RuleAction$additional_interfaces
    {
        private PrsStream prsStream = null;
        
        private boolean unimplementedSymbolsWarning = $unimplemented_symbols_warning;

        private static ParseTable prsTable = new $prs_type();
        public ParseTable getParseTable() { return prsTable; }

        private DeterministicParser dtParser = null;
        public DeterministicParser getParser() { return dtParser; }

        private void setResult(Object object) { dtParser.setSym1(object); }
        public Object getRhsSym(int i) { return dtParser.getSym(i); }

        public int getRhsTokenIndex(int i) { return dtParser.getToken(i); }
        public IToken getRhsIToken(int i) { return prsStream.getIToken(getRhsTokenIndex(i)); }
        
        public int getRhsFirstTokenIndex(int i) { return dtParser.getFirstToken(i); }
        public IToken getRhsFirstIToken(int i) { return prsStream.getIToken(getRhsFirstTokenIndex(i)); }

        public int getRhsLastTokenIndex(int i) { return dtParser.getLastToken(i); }
        public IToken getRhsLastIToken(int i) { return prsStream.getIToken(getRhsLastTokenIndex(i)); }

        public int getLeftSpan() { return dtParser.getFirstToken(); }
        public IToken getLeftIToken()  { return prsStream.getIToken(getLeftSpan()); }

        public int getRightSpan() { return dtParser.getLastToken(); }
        public IToken getRightIToken() { return prsStream.getIToken(getRightSpan()); }

        public int getRhsErrorTokenIndex(int i)
        {
            int index = dtParser.getToken(i);
            IToken err = prsStream.getIToken(index);
            return (err instanceof ErrorToken ? index : 0);
        }
        public ErrorToken getRhsErrorIToken(int i)
        {
            int index = dtParser.getToken(i);
            IToken err = prsStream.getIToken(index);
            return (ErrorToken) (err instanceof ErrorToken ? err : null);
        }

        public void reset(ILexStream lexStream)
        {
            prsStream = new PrsStream(lexStream);
            dtParser.reset(prsStream);

            try
            {
                prsStream.remapTerminalSymbols(orderedTerminalSymbols(), prsTable.getEoftSymbol());
            }
            catch(NullExportedSymbolsException e) {
            }
            catch(NullTerminalSymbolsException e) {
            }
            catch(UnimplementedTerminalsException e)
            {
                if (unimplementedSymbolsWarning) {
                    java.util.ArrayList unimplemented_symbols = e.getSymbols();
                    System.out.println("The Lexer will not scan the following token(s):");
                    for (int i = 0; i < unimplemented_symbols.size(); i++)
                    {
                        Integer id = (Integer) unimplemented_symbols.get(i);
                        System.out.println("    " + $sym_type.orderedTerminalSymbols[id.intValue()]);               
                    }
                    System.out.println();
                }
            }
            catch(UndefinedEofSymbolException e)
            {
                throw new Error(new UndefinedEofSymbolException
                                    ("The Lexer does not implement the Eof symbol " +
                                     $sym_type.orderedTerminalSymbols[prsTable.getEoftSymbol()]));
            }
        }
        
        public $action_type()
        {
            try
            {
                dtParser = new DeterministicParser(prsStream, prsTable, (RuleAction) this);
            }
            catch (NotDeterministicParseTableException e)
            {
                throw new Error(new NotDeterministicParseTableException
                                    ("Regenerate $prs_type.java with -NOBACKTRACK option"));
            }
            catch (BadParseSymFileException e)
            {
                throw new Error(new BadParseSymFileException("Bad Parser Symbol File -- $sym_type.java. Regenerate $prs_type.java"));
            }
        }

        public $action_type(ILexStream lexStream)
        {
            this();
            reset(lexStream);
        }

        public int numTokenKinds() { return $sym_type.numTokenKinds; }
        public String[] orderedTerminalSymbols() { return $sym_type.orderedTerminalSymbols; }
        public String getTokenKindName(int kind) { return $sym_type.orderedTerminalSymbols[kind]; }            
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
            dtParser.setMonitor(monitor);

            try
            {
                return ($ast_class) dtParser.parse();
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
