--
-- An LPG Parser Template Using lpg.jar
--
-- Macros that may be redefined in an instance of this template
--
--     $additional_interfaces
--     $super_stream_class
--
-- B E G I N N I N G   O F   T E M P L A T E   dtUnifiedTemplateF
--
%Options programming_language=java
%Options table
%Options margin=4
%Options prefix=Char_
%Options action-block=("*.java", "/.", "./")
%Options ParseTable=lpg.runtime.ParseTable

--
-- The EOF and ERROR symbols are assigned a default here as a
-- convenience.
--
%EOF
    EOF
%End

%Define
    $Header
    /.
                //
                // Rule $rule_number:  $rule_text
                //
                ./

    $BeginAction
    /.$Header$case $rule_number: {./

    $EndAction
    /.          break;
                }./

    $BeginJava
    /.$BeginAction
                    $symbol_declarations./

    $EndJava /.$EndAction./

    $NoAction
    /.$Header$case $rule_number:
                    break;./

    $NullAction
    /.$Header$case $rule_number:
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

    $additional_interfaces /../
    $super_stream_class /.LpgLexStream./
%End

%Globals
    /.import lpg.runtime.*;
    ./
%End

%Headers
    /.
    public class $action_type extends $super_stream_class implements $sym_type, RuleAction$additional_interfaces
    {
        private static ParseTable prs = new $prs_type();
        private DeterministicParser dtParser;

        private void setResult(Object object) { dtParser.setSym1(object); }
        public DeterministicParser getParser() { return dtParser; }
        public Object getRhsSym(int i) { return dtParser.getSym(i); }
        public int getRhsTokenIndex(int i) { return dtParser.getToken(i); }
        public int getRhsFirstTokenIndex(int i) { return dtParser.getFirstToken(i); }
        public int getRhsLastTokenIndex(int i) { return dtParser.getLastToken(i); }

        public int getLeftSpan() { return dtParser.getFirstToken(); }
        public int getRightSpan() { return dtParser.getLastToken(); }

        public $action_type(String filename, int tab) throws java.io.IOException 
        {
            super(filename, tab);
        }

        public String[] orderedExportedSymbols() { return orderedTerminalSymbols; }
        public int getEOFTokenKind() { return $prs_type.EOFT_SYMBOL; }

        public ILexStream getILexStream() { return ($super_stream_class) this; }
        
        /**
         * @deprecated replaced by {@link #getILexStream()}
         */
        public ILexStream getLexStream() { return ($super_stream_class) this; }

        public $ast_type parser()
        {
            return parser(null, 0);
        }
            
        public $ast_type parser(Monitor monitor)
        {
            return parser(monitor, 0);
        }
            
        public $ast_type parser(int error_repair_count)
        {
            return parser(null, error_repair_count);
        }
            
        public $ast_type parser(Monitor monitor, int error_repair_count)
        {
            try
            {
                dtParser = new DeterministicParser(this, prs, this);
            }
            catch (NotDeterministicParseTableException e)
            {
                System.out.println("****Error: Regenerate $prs_type.java with -NOBACKTRACK option");
                System.exit(1);
            }
            catch (BadParseSymFileException e)
            {
                System.out.println("****Error: Bad Parser Symbol File -- $sym_type.java. Regenerate $prs_type.java");
                System.exit(1);
            }
            dtParser.setMonitor(monitor);

            try
            {
                return ($ast_type) dtParser.parse();
            }
            catch (BadParseException e)
            {
                reset(e.error_token); // point to error token

                System.out.print("Error detected on character " + e.error_token);
                if (e.error_token < getStreamLength())
                     System.out.print(" at line " + getLine(e.error_token) + ", column " + getColumn(e.error_token));
                else System.out.print(" at end of file ");
                System.out.println(" with kind " + getKind(e.error_token));
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
