--
-- An instance of this template must have a $Export section and the export_terminals option
--
-- Macros that may be redefined in an instance of this template
--
--     $eof_token
--     $additional_interfaces
--     $super_stream_class -- subclass com.ibm.lpg.Utf8LpgLexStream for getKind
--     $prs_stream_class -- use /.PrsStream./ if not subclassing
--
-- B E G I N N I N G   O F   T E M P L A T E   LexerTemplateD
--
%Options programming_language=java,margin=4
%Options table
%options action-block=("*.java", "/.", "./")
%options ParseTable=lpg.runtime.ParseTable
%Options prefix=Char_

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
    $super_stream_class /.$file_prefix$Utf8LpgLexStream./
    $prs_stream_class /.IPrsStream./
    $super_class /.any./

    --
    -- Macros useful for specifying actions
    --
    $Header
    /.
                //
                // Rule $rule_number:  $rule_text
                //
                ./

    $DefaultAction
    /.$Header$case $rule_number: { ./

    $BeginAction /.$DefaultAction./

    $EndAction
    /.          break;
                }./

    $BeginJava
    /.$BeginAction
                $symbol_declarations./

    $EndJava /.$EndAction./

    $NoAction
    /.$Header$case $rule_number:
                    break; ./

    $BeginActions
    /.
        public void ruleAction( ruleNumber : number )
        {
            switch(ruleNumber)
            {./

    $SplitActions
    /.
	            default:
	                ruleAction$rule_number(ruleNumber);
	                break;
	        }
	        return;
	    }
	
	    public void ruleAction$rule_number(ruleNumber : number )
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
    public class $action_type extends $super_class implements RuleAction$additional_interfaces
    {
        private $super_stream_class utf8LexStream;
        
        private static ParseTable prs = new $prs_type();
        public ParseTable getParseTable() { return prs; }

        private LexParser lexParser = new LexParser();
        public LexParser getParser() { return lexParser; }

        public number getToken(i : number) { return lexParser.getToken(i); }
        public number getRhsFirstTokenIndex(i : number) { return lexParser.getFirstToken(i); }
        public number getRhsLastTokenIndex(i : number) { return lexParser.getLastToken(i); }

        public number getLeftSpan() { return lexParser.getToken(1); }
        public number getRightSpan() { return lexParser.getLastToken(); }
  
        public void resetKeywordLexer()
        {
            if (kwLexer == null)
                  this.kwLexer = new $kw_lexer_class(utf8LexStream.getInputBytes(), $_IDENTIFIER);
            else this.kwLexer.setInputBytes(utf8LexStream.getInputBytes());
        }
  
        public void reset(string filename, number tab) 
        {
            utf8LexStream = new $super_stream_class(filename, tab);
            lexParser.reset((ILexStream) utf8LexStream, prs, (RuleAction) this);
            resetKeywordLexer();
        }

        public void reset(byte[] input_bytes, string filename)
        {
            reset(input_bytes, filename, 1);
        }
        
        public void reset(byte[] input_bytes, string filename, number tab)
        {
            utf8LexStream = new $super_stream_class(input_bytes, filename, tab);
            lexParser.reset((ILexStream) utf8LexStream, prs, (RuleAction) this);
            resetKeywordLexer();
        }
        
        public $action_type(string filename, number tab) 
        {
            reset(filename, tab);
        }

        public $action_type(byte[] input_bytes, string filename, number tab)
        {
            reset(input_bytes, filename, tab);
        }

        public $action_type(byte[] input_bytes, string filename)
        {
            reset(input_bytes, filename, 1);
        }

        public $action_type() {}

        public ILexStream getILexStream() { return utf8LexStream; }

        /**
         * @deprecated replaced by {@link #getILexStream()}
         */
        public ILexStream getLexStream() { return utf8LexStream; }

        private void initializeLexer($prs_stream_class this.prsStream, number start_offset, number end_offset)
        {
            if (utf8LexStream.getInputBytes() == null)
                throw new NullPointerException("LexStream was not initialized");
            utf8LexStream.setPrsStream(this.prsStream);
            this.prsStream.makeToken(start_offset, end_offset, 0); // Token list must start with a bad token
        }

        public void lexer($prs_stream_class this.prsStream)
        {
            lexer(null, this.prsStream);
        }
        
        public void lexer(Monitor monitor, $prs_stream_class this.prsStream)
        {
            if (utf8LexStream.getInputBytes() == null)
                throw new NullPointerException("Utf8LexStream was not initialized");

            utf8LexStream.setPrsStream(this.prsStream);

            this.prsStream.makeToken(0, 0, 0); // Token list must start with a bad token
                
            lexParser.parseCharacters(monitor);  // Lex the input characters
                
            i : number = utf8LexStream.getStreamIndex();
            this.prsStream.makeToken(i, i, $eof_token); // and end with the end of file token
            this.prsStream.setStreamLength(this.prsStream.getSize());
                
            return;
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
