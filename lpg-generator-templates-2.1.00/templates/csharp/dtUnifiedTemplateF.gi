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
%Options programming_language=csharp
%Options table
%Options margin=4
%Options prefix=Char_
%Options action-block=("*.cs", "/.", "./")
%Options ParseTable=LPG2.Runtime.ParseTable

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
    /.
      using LPG2.Runtime;
      using System;
    ./
%End

%Headers
    /.
    public class $action_type : $super_stream_class , $sym_type, RuleAction$additional_interfaces
    {
        private static ParseTable prs = new $prs_type();
        private DeterministicParser dtParser;

        private void setResult(object _object) { dtParser.setSym1(_object); }
        public DeterministicParser getParser() { return dtParser; }
        public object getRhsSym(int i) { return dtParser.getSym(i); }
        public int getRhsTokenIndex(int i) { return dtParser.getToken(i); }
        public int getRhsFirstTokenIndex(int i) { return dtParser.getFirstToken(i); }
        public int getRhsLastTokenIndex(int i) { return dtParser.getLastToken(i); }

        public int getLeftSpan() { return dtParser.getFirstToken(); }
        public int getRightSpan() { return dtParser.getLastToken(); }

        public $action_type(string filename, int tab) :  base(filename, tab)
        {
           
        }

        public override string[] orderedExportedSymbols() { return orderedTerminalSymbols; }
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
                 Console.Out.WriteLine("****Error: Regenerate $prs_type.cs with -NOBACKTRACK option");
                 return null;
            }
            catch (BadParseSymFileException e)
            {
                 Console.Out.WriteLine("****Error: Bad Parser Symbol File -- $sym_type.cs. Regenerate $prs_type.cs");
                 return null;
            }
            dtParser.setMonitor(monitor);

            try
            {
                return ($ast_type) dtParser.parse();
            }
            catch (BadParseException e)
            {
                reset(e.error_token); // point to error token

                 Console.Out.Write("Error detected on character " + e.error_token);
                if (e.error_token < getStreamLength())
                      Console.Out.Write(" at line " + getLine(e.error_token) + ", column " + getColumn(e.error_token));
                else  Console.Out.Write(" at end of file ");
                 Console.Out.WriteLine(" with kind " + getKind(e.error_token));
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
