--
-- An LPG Parser Template Using lpg.jar
--
-- An instance of this template must have a $Export section and the export_terminals option
--
-- Macros that may be redefined in an instance of this template
--
--     $eof_token
--     $additional_interfaces
--     $super_stream_class -- subclass com.ibm.lpg.LpgLexStream for getKind
--     $prs_stream_class -- use /.PrsStream./ if not subclassing
--
-- B E G I N N I N G   O F   T E M P L A T E   dtUnifiedTemplate
--
%Options programming_language=java,margin=4
%Options table,error_maps,scopes
%options action-block=("*.java", "/.", "./")
%options ParseTable=lpg.runtime.ParseTable
%Options prefix=Char_
-- %Options export_terminals=(*exp.java,"TK_","")

--
-- This template requires that the name of the EOF token be set
-- to EOF and that the prefix be "Char_" to be consistent with
-- KeywordTemplateD.
--
%Eof
    EOF
%End

--
-- This template also requires that the name of the parser EOF
-- Token to be exported be set to EOF_TOKEN
--
%Export
    EOF_TOKEN
%End

%Define
    --
    -- Macros that are be needed in an instance of this template
    --
    $eof_token /.$_EOF_TOKEN./
    
    $additional_interfaces /../
    $super_stream_class /.LpgLexStream./
    $prs_stream_class /.PrsStream./

    $prs_stream /. // macro prs_stream is deprecated. Use function getPrsStream
                  getPrsStream()./
    $setSym1 /. // macro setSym1 is deprecated. Use function setResult
               dtParser.setSym1./
    $setResult /. // macro setResult is deprecated. Use function setResult
                 dtParser.setSym1./
    $getSym /. // macro getSym is deprecated. Use function getLastToken
              dtParser.getSym./
    $getToken /. // macro getToken is deprecated. Use function getToken
                dtParser.getToken./
    $getLeftSpan /. // macro getLeftSpan is deprecated. Use function getLeftSpan
                   dtParser.getFirstToken./
    $getRightSpan /. // macro getRightSpan is deprecated. Use function getRightSpan
                    dtParser.getLastToken./

    $Header
    /.
                //
                // Rule $rule_number:  $rule_text
                //./

    $BeginAction
    /. $Header
                case $rule_number: {./

    $EndAction
    /.          break;
                }./

    $BeginJava
    /.$BeginAction
                    $symbol_declarations./

    $EndJava /.$EndAction./

    $NoAction
    /. $Header
                case $rule_number:
                    break;./

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

%End

%Globals
    /.import lpg.runtime.*;
    ./
%End

%Headers
    /.
    public class $action_type extends $super_stream_class implements $exp_type, $sym_type, RuleAction$additional_interfaces
    {
        private static ParseTable prs = new $prs_type();
        private DeterministicParser dtParser;
        private $prs_stream_class prsStream;

        public $prs_stream_class getPrsStream() { return prsStream; }
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

        public $action_type(char[] input_chars, String filename, int tab)
        {
            super(input_chars, filename, tab);
        }

        public $action_type(char[] input_chars, String filename)
        {
            this(input_chars, filename, 1);
        }

        public $action_type() {}

        public String[] orderedExportedSymbols() { return $exp_type.orderedTerminalSymbols; }
        public int getEOFTokenKind() { return $prs_type.EOFT_SYMBOL; }
        public $super_stream_class getLexStream() { return ($super_stream_class) this; }

        public boolean lexer($prs_stream_class prsStream)
        {
            return lexer(null, prsStream);
        }
        
        public boolean lexer(Monitor monitor, $prs_stream_class prsStream)
        {
            boolean result = false;
            if (getInputChars() == null)
                throw new NullPointerException("LexStream was not initialized");

            this.prsStream = prsStream;

            prsStream.makeToken(0, 0, 0); // Token list must start with a bad token
                
            result = parser(monitor);  // Lex the input characters
                
            int i = getStreamIndex();
            prsStream.makeToken(i, i, $eof_token); // and end with the end of file token
            prsStream.setStreamLength(prsStream.getSize());
                
            return result;
        }
        public boolean parser()
        {
            return parser(null, 0);
        }
            
        public boolean parser(Monitor monitor)
        {
            return parser(monitor, 0);
        }
            
        public boolean parser(int error_repair_count)
        {
            return parser(null, error_repair_count);
        }
            
        public boolean parser(Monitor monitor, int error_repair_count)
        {
            try
            {
                dtParser = new DeterministicParser(monitor, (TokenStream)this, prs, (RuleAction)this);
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

            try
            {
                dtParser.parse();
                return true;
            }
            catch (BadParseException e)
            {
                reset(e.error_token); // point to error token

                DiagnoseParser diagnoseParser = new DiagnoseParser(this, prs);
                diagnoseParser.diagnose(e.error_token);

            }

            return false;
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
