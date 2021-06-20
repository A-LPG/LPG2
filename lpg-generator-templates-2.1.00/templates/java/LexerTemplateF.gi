--
-- An instance of this template must have a $Export section and the export_terminals option
--
-- Macros that may be redefined in an instance of this template
--
--     $eof_token
--     $additional_interfaces
--     $super_stream_class -- subclass com.ibm.lpg.LpgLexStream for getKind
--     $prs_stream_class -- use /.PrsStream./ if not subclassing
--     $super_class
--
-- B E G I N N I N G   O F   T E M P L A T E   LexerTemplateF
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
    $super_stream_class /.$file_prefix$LpgLexStream./
    $prs_stream_class /.IPrsStream./
    $super_class /.Object./

    $prs_stream /. // macro prs_stream is deprecated. Use function getPrsStream
                  getPrsStream()./
    $setSym1 /. // macro setSym1 is deprecated. Use function setResult
               lexParser.setSym1./
    $setResult /. // macro setResult is deprecated. Use function setResult
                 lexParser.setSym1./
    $getSym /. // macro getSym is deprecated. Use function getLastToken
              lexParser.getSym./
    $getToken /. // macro getToken is deprecated. Use function getToken
                lexParser.getToken./
    $getLeftSpan /. // macro getLeftSpan is deprecated. Use function getLeftSpan
                   lexParser.getFirstToken./
    $getRightSpan /. // macro getRightSpan is deprecated. Use function getRightSpan
                    lexParser.getLastToken./

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
    /.            break;
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
        public void ruleAction(int ruleNumber)
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
    public class $action_type extends $super_class implements RuleAction$additional_interfaces
    {
        private $super_stream_class lexStream;
        
        private static ParseTable prs = new $prs_type();
        public ParseTable getParseTable() { return prs; }

        private LexParser lexParser = new LexParser();
        public LexParser getParser() { return lexParser; }

        public int getToken(int i) { return lexParser.getToken(i); }
        public int getRhsFirstTokenIndex(int i) { return lexParser.getFirstToken(i); }
        public int getRhsLastTokenIndex(int i) { return lexParser.getLastToken(i); }

        public int getLeftSpan() { return lexParser.getToken(1); }
        public int getRightSpan() { return lexParser.getLastToken(); }
  
        public void resetKeywordLexer()
        {
            if (kwLexer == null)
                  this.kwLexer = new $kw_lexer_class(lexStream.getInputChars(), $_IDENTIFIER);
            else this.kwLexer.setInputChars(lexStream.getInputChars());
        }
  
        public void reset(String filename, int tab) throws java.io.IOException
        {
            lexStream = new $super_stream_class(filename, tab);
            lexParser.reset((ILexStream) lexStream, prs, (RuleAction) this);
            resetKeywordLexer();
        }

        public void reset(char[] input_chars, String filename)
        {
            reset(input_chars, filename, 1);
        }
        
        public void reset(char[] input_chars, String filename, int tab)
        {
            lexStream = new $super_stream_class(input_chars, filename, tab);
            lexParser.reset((ILexStream) lexStream, prs, (RuleAction) this);
            resetKeywordLexer();
        }
        
        public $action_type(String filename, int tab) throws java.io.IOException 
        {
            reset(filename, tab);
        }

        public $action_type(char[] input_chars, String filename, int tab)
        {
            reset(input_chars, filename, tab);
        }

        public $action_type(char[] input_chars, String filename)
        {
            reset(input_chars, filename, 1);
        }

        public $action_type() {}

        public ILexStream getILexStream() { return lexStream; }

        /**
         * @deprecated replaced by {@link #getILexStream()}
         */
        public ILexStream getLexStream() { return lexStream; }

        private void initializeLexer($prs_stream_class prsStream, int start_offset, int end_offset)
        {
            if (lexStream.getInputChars() == null)
                throw new NullPointerException("LexStream was not initialized");
            lexStream.setPrsStream(prsStream);
            prsStream.makeToken(start_offset, end_offset, 0); // Token list must start with a bad token
        }

        private void addEOF($prs_stream_class prsStream, int end_offset)
        {
            prsStream.makeToken(end_offset, end_offset, $eof_token); // and end with the end of file token
            prsStream.setStreamLength(prsStream.getSize());
        }

        public void lexer($prs_stream_class prsStream)
        {
            lexer(null, prsStream);
        }
        
        public void lexer(Monitor monitor, $prs_stream_class prsStream)
        {
            initializeLexer(prsStream, 0, -1);
            lexParser.parseCharacters(monitor);  // Lex the input characters
            addEOF(prsStream, lexStream.getStreamIndex());
        }

        public void lexer($prs_stream_class prsStream, int start_offset, int end_offset)
        {
            lexer(null, prsStream, start_offset, end_offset);
        }
        
        public void lexer(Monitor monitor, $prs_stream_class prsStream, int start_offset, int end_offset)
        {
            if (start_offset <= 1)
                 initializeLexer(prsStream, 0, -1);
            else initializeLexer(prsStream, start_offset - 1, start_offset - 1);

            lexParser.parseCharacters(monitor, start_offset, end_offset);

            addEOF(prsStream, (end_offset >= lexStream.getStreamIndex() ? lexStream.getStreamIndex() : end_offset + 1));
        }
        
        public IPrsStream.Range incrementalLexer(char[] input_chars, int start_change_offset, int end_change_offset) {
            int offset_adjustment = input_chars.length - lexStream.getStreamLength();
//*System.out.println("The offset adjustment is " + offset_adjustment);
            if (start_change_offset <= 0 && start_change_offset < input_chars.length)
                throw new IndexOutOfBoundsException("The start offset " + start_change_offset +
                                                    " is out of bounds for range 0.." + (input_chars.length - 1));
            if (end_change_offset <= 0 && end_change_offset < input_chars.length)
                throw new IndexOutOfBoundsException("The end offset " + end_change_offset +
                                                    " is out of bounds for range 0.." + (input_chars.length - 1));
            
            //
            // Get the potential list of tokens to be rescanned
            //
            java.util.ArrayList<IToken> affected_tokens = lexStream.getIPrsStream().incrementalResetAtCharacterOffset(start_change_offset); 
            
            //
            // If the change occured between the first two affected tokens (or adjunct) and not immediately
            // on the characted after the first token (or adjunct), restart the scanning after the first
            // affected token. Otherwise, rescan the first token.
            //
            int affected_index = 0;
            int repair_offset = start_change_offset;
            if (affected_tokens.size() > 0) {
                if (affected_tokens.get(0).getEndOffset() + 1 < start_change_offset) {
                     repair_offset = affected_tokens.get(0).getEndOffset() + 1;
                     if (affected_tokens.get(0) instanceof Token)
                         lexStream.getIPrsStream().makeToken(affected_tokens.get(0), 0);
                    else lexStream.getIPrsStream().makeAdjunct(affected_tokens.get(0), 0);
                    affected_index++;                    
                }
                else repair_offset = affected_tokens.get(0).getStartOffset();
            } 

            lexStream.setInputChars(input_chars);
            lexStream.setStreamLength(input_chars.length);
            lexStream.computeLineOffsets(repair_offset);

            int first_new_token_index = lexStream.getIPrsStream().getTokens().size(),
                first_new_adjunct_index = lexStream.getIPrsStream().getAdjuncts().size();
            
            resetKeywordLexer();
            lexParser.resetTokenStream(repair_offset);
            int next_offset;
            do {
//*System.out.println("Scanning token starting at " + (lexStream.peek() - 1));            
                next_offset = lexParser.incrementalParseCharacters();
//*System.out.print("***Remaining string: \"");
//*for (int i = next_offset; i < input_chars.length; i++)
//*System.out.print(input_chars[i]);
//*System.out.println("\"");                    
                while (affected_index < affected_tokens.size() && 
                       affected_tokens.get(affected_index).getStartOffset() + offset_adjustment < next_offset)
//*{
//*System.out.println("---Skipping token " + affected_index + ": \"" + affected_tokens.get(affected_index).toString() +
//*"\" starting at adjusted offset " + (affected_tokens.get(affected_index).getStartOffset() + offset_adjustment));                           
                    affected_index++;
//*}
            } while(next_offset <= end_change_offset &&          // still in the damage region and ...
                    (affected_index < affected_tokens.size() &&  // not resynchronized with a token in the list of affected tokens
                     affected_tokens.get(affected_index).getStartOffset() + offset_adjustment != next_offset));

            //
            // If any new tokens were added, compute the first and the last one.
            //
            IToken first_new_token = null,
                   last_new_token = null;
            if (first_new_token_index < lexStream.getIPrsStream().getTokens().size()) {
                first_new_token = lexStream.getIPrsStream().getTokenAt(first_new_token_index);
                last_new_token = lexStream.getIPrsStream().getTokenAt(lexStream.getIPrsStream().getTokens().size() - 1);
            }
            //
            // If an adjunct was added prior to the first real token, chose it instead as the first token.
            // Similarly, if adjucts were added after the last token, chose the last adjunct added as the last token.
            //
            if (first_new_adjunct_index < lexStream.getIPrsStream().getAdjuncts().size()) {
                if (first_new_token == null ||
                    lexStream.getIPrsStream().getAdjunctAt(first_new_adjunct_index).getStartOffset() <
                    first_new_token.getStartOffset()) {
                    first_new_token = lexStream.getIPrsStream().getAdjunctAt(first_new_adjunct_index);
                }
                if (last_new_token == null ||
                    lexStream.getIPrsStream().getAdjunctAt(lexStream.getIPrsStream().getAdjuncts().size() - 1).getEndOffset() >
                    last_new_token.getEndOffset()) {
                    last_new_token = lexStream.getIPrsStream().getAdjunctAt(lexStream.getIPrsStream().getAdjuncts().size() - 1);
                }
            }
            
            //
            // For all remainng tokens (and adjuncts) in the list of affected tokens add them to the
            // list of tokens (and adjuncts).
            //
            for (int i = affected_index; i < affected_tokens.size(); i++) {
                if (affected_tokens.get(i) instanceof Token)
                     lexStream.getIPrsStream().makeToken(affected_tokens.get(i), offset_adjustment);
                else lexStream.getIPrsStream().makeAdjunct(affected_tokens.get(i), offset_adjustment);
//*System.out.println("+++Added affected token " + i + ": \"" + affected_tokens.get(i).toString() +
//*"\" starting at adjusted offset " + (affected_tokens.get(i).getStartOffset() + offset_adjustment));                           
            }
            
            return new IPrsStream.Range(lexStream.getIPrsStream(), first_new_token, last_new_token);
        }

        /**
         * If a parse stream was not passed to this Lexical analyser then we
         * simply report a lexical error. Otherwise, we produce a bad token.
         */
        public void reportLexicalError(int startLoc, int endLoc) {
            IPrsStream prs_stream = lexStream.getIPrsStream();
            if (prs_stream == null)
                lexStream.reportLexicalError(startLoc, endLoc);
            else {
                //
                // Remove any token that may have been processed that fall in the
                // range of the lexical error... then add one error token that spans
                // the error range.
                //
                for (int i = prs_stream.getSize() - 1; i > 0; i--) {
                    if (prs_stream.getStartOffset(i) >= startLoc)
                         prs_stream.removeLastToken();
                    else break;
                }
                prs_stream.makeToken(startLoc, endLoc, 0); // add an error token to the prsStream
            }        
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
