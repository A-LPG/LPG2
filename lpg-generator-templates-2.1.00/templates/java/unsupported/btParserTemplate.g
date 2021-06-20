--
-- B E G I N N I N G   O F   T E M P L A T E   btParserTemplate
--
-- In a parser using this template, the following macros may be redefined:
--
--     $import_classes
--     $action_class
--     $additional_interfaces
--     $ast_class
--
%Options programming_language=java,margin=4,backtrack
%Options table,error_maps,scopes
%options action-block=("*.java", "/.", "./")
%options ParseTable=lpg.runtime.ParseTable

%Notice /.$copyright./

%Define
        $Header
        /.
                //
                // Rule $rule_number:  $rule_text
                //./

        $DefaultAction
        /. $Header
                case $rule_number: ./

        $BeginAction
        /.$DefaultAction
                {./

        $EndAction
        /.          break;
                } ./

        $BeginJava
        /.$BeginAction
                    $symbol_declarations./

        $EndJava /.$EndAction./

        $NoAction
        /. $Header
                case $rule_number:
                    break; ./

        $NullAction
        /. $Header
                case $rule_number:
                    $setResult(null);
                    break;./

        $BeginActions
        /.
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
	        {./

        $EndActions
        /.
                default:
                    break;
            }
            return;
        }./

        --
        -- Macros that may be needed in a parser using this template
        --
        $copyright /../
        $import_classes /../
        $action_class /.$action_type./
        $additional_interfaces /../
        $ast_class /.$ast_type./
        $setSym1 /.getParser().setSym1./
        $setResult /.getParser().setSym1./
        $getSym /.getParser().getSym./
        $getToken /.getParser().getToken./
        $getIToken /.prsStream.getIToken./
        $getLeftSpan /.getParser().getFirstToken./
        $getRightSpan /.getParser().getLastToken./
        $prs_stream /.prsStream./

%End

%Headers
    /.
    $copyright
    $import_classes
    import lpg.runtime.*;

    public class $action_class implements RuleAction$additional_interfaces
    {
        private static ParseTable prs = new $prs_type();
        private PrsStream prsStream;
        private BacktrackingParser btParser;

        public BacktrackingParser getParser() { return btParser; }
        private void setResult(Object object) { btParser.setSym1(object); }
        public Object getRhsSym(int i) { return btParser.getSym(i); }

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
            return (err instanceof ErrorToken ? index : 0);
        }
        public ErrorToken getRhsErrorIToken(int i)
        {
            int index = btParser.getToken(i);
            IToken err = prsStream.getIToken(index);
            return (ErrorToken) (err instanceof ErrorToken ? err : null);
        }

        public $action_class(PrsStream prsStream)
        {
            this.prsStream = prsStream;

            try
            {
                prsStream.remapTerminalSymbols(orderedTerminalSymbols(), $prs_type.EOFT_SYMBOL);
            }
            catch(NullExportedSymbolsException e) {
            }
            catch(NullTerminalSymbolsException e) {
            }
            catch(UnimplementedTerminalsException e)
            {
                java.util.ArrayList unimplemented_symbols = e.getSymbols();
                System.out.println("The Lexer will not scan the following token(s):");
                for (int i = 0; i < unimplemented_symbols.size(); i++)
                {
                    Integer id = (Integer) unimplemented_symbols.get(i);
                    System.out.println("    " + $sym_type.orderedTerminalSymbols[id.intValue()]);               
                }
                System.out.println();                        
            }
            catch(UndefinedEofSymbolException e)
            {
                System.out.println("The Lexer does not implement the Eof symbol " +
                                   $sym_type.orderedTerminalSymbols[$prs_type.EOFT_SYMBOL]);
                System.exit(12);
            } 
        }

        public String[] orderedTerminalSymbols() { return $sym_type.orderedTerminalSymbols; }
        public String getTokenKindName(int kind) { return $sym_type.orderedTerminalSymbols[kind]; }
        public int getEOFTokenKind() { return $prs_type.EOFT_SYMBOL; }
        public PrsStream getParseStream() { return prsStream; }

        public $ast_class parser()
        {
            return parser(0);
        }
        
        public $ast_class parser(int error_repair_count)
        {
            try
            {
                btParser = new BacktrackingParser((TokenStream)prsStream, prs, (RuleAction)this);
            }
            catch (NotBacktrackParseTableException e)
            {
                System.out.println("****Error: Regenerate $prs_type.java with -BACKTRACK option");
                System.exit(1);
            }
            catch (BadParseSymFileException e)
            {
                System.out.println("****Error: Bad Parser Symbol File -- $sym_type.java. Regenerate $prs_type.java");
                System.exit(1);
            }

            try
            {
                return ($ast_class) btParser.parse(error_repair_count);
            }
            catch (BadParseException e)
            {
                prsStream.reset(e.error_token); // point to error token

                DiagnoseParser diagnoseParser = new DiagnoseParser(prsStream, prs);
                diagnoseParser.diagnose(e.error_token);
            }

            return null;
        }

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
