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
-- B E G I N N I N G   O F   T E M P L A T E   LexerTemplateE
--
%Options programming_language=java,margin=8
%Options table
%options action-block=("*.java", "/.", "./")
%options headers=("*.java", "/:", ":/")
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
        $super_stream_class /.LpgLexStream./
        $prs_stream_class /.IPrsStream./

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

        $DefaultAllocation
        /:
                ruleAction[$rule_number] = new act$rule_number$();:/

        $NoAllocation
        /:
                ruleAction[$rule_number] = new NoAction();:/

        $NullAllocation
        /:
                ruleAction[$rule_number] = new NullAction();:/


        $Header
        /.
            //
            // Rule $rule_number:  $rule_text
            //./

        $DefaultAction
        /.$DefaultAllocation $Header
            final class act$rule_number extends Action./

        --
        -- This macro is used to initialize the ruleAction array
        -- to the null_action function.
        --
        $NullAction
        /. $NullAllocation
           $Header
            // final class NullAction extends Action
        ./

        --
        -- This macro is used to initialize the ruleAction array
        -- to the no_action function.
        --
        $NoAction
        /. $NoAllocation
           $Header
            // final class NullAction extends Action
        ./

        --
        -- This is the header for a ruleAction class
        --
        $BeginAction
        /.$DefaultAllocation $Header
            final class act$rule_number extends Action
            {
                public void action()
                {./

        $EndAction
        /.          return;
                }
            }./

        $BeginJava
        /.$BeginAction
                    $symbol_declarations./

        $EndJava /.$EndAction./

        $SplitActions /../

        --
        -- This macro is used to reset the parser's state stack
        -- after a Token has been reduced.
        --
        $ResetStackAction
        /.$BeginAction
                    lexParser.resetStateStack();
          $EndAction
        ./

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
            public ParseTable getParseTable() { return prs; }

            private LexParser lexParser = new LexParser(this, prs, this);
            public LexParser getParser() { return lexParser; }

            public int getToken(int i) { return lexParser.getToken(i); }
            public int getRhsFirstTokenIndex(int i) { return lexParser.getFirstToken(i); }
            public int getRhsLastTokenIndex(int i) { return lexParser.getLastToken(i); }

            public int getLeftSpan() { return lexParser.getToken(1); }
            public int getRightSpan() { return lexParser.getLastToken(); }

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
            public LexStream getLexStream() { return (LexStream) this; }

            private void initializeLexer($prs_stream_class prsStream)
            {
                if (getInputChars() == null)
                    throw new NullPointerException("LexStream was not initialized");
                setPrsStream(prsStream);
                prsStream.makeToken(0, -1, 0); // Token list must start with a bad token
            }

            private void addEOF($prs_stream_class prsStream)
            {
                int i = getStreamIndex();
                prsStream.makeToken(i, i, $eof_token); // and end with the end of file token
                prsStream.setStreamLength(prsStream.getSize());
            }

            public void lexer($prs_stream_class prsStream)
            {
                lexer(null, prsStream);
            }
        
            public void lexer(Monitor monitor, $prs_stream_class prsStream)
            {
                initializeLexer(prsStream);
                lexParser.parseCharacters(monitor);  // Lex the input characters
                addEOF(prsStream);
            }

            public void lexer($prs_stream_class prsStream, int start_offset, int end_offset)
            {
                lexer(null, prsStream, start_offset, end_offset);
            }
        
            public void lexer(Monitor monitor, $prs_stream_class prsStream, int start_offset, int end_offset)
            {
                initializeLexer(prsStream);
                lexParser.parseCharacters(monitor, start_offset, end_offset);
                addEOF(prsStream);
            }

            /**
             * If a parse stream was not passed to this Lexical analyser then we
             * simply report a lexical error. Otherwise, we produce a bad token.
             */
            public void reportLexicalError(int startLoc, int endLoc) {
                IPrsStream prs_stream = getPrsStream();
                if (prs_stream == null)
                    super.reportLexicalError(startLoc, endLoc);
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

            public final void ruleAction(int act)
            {
                ruleAction[act].action();
            }

            abstract class Action
            {
                public abstract void action();
            }

            final class NoAction extends Action
            {
                public void action() {}
            }

            final class NullAction extends Action
            {
                public void action() { lexParser.setSym1(0); }
            }

        ./

        /:

            //
            // Declare and initialize ruleAction array.
            //
            Action ruleAction[] = new Action[$NUM_RULES + 1];
            {
                ruleAction[0] = null;
        :/
%End

%Trailers
    /.
        }
    ./

    /:

                //
                // Make sure that all elements of ruleAction are properly initialized
                //
                for (int i = 0; i < ruleAction.length; i++)
                {
                    if (ruleAction[i] == null)
                         ruleAction[i] = new NoAction();
                }
            };
    :/
%End

--
-- E N D   O F   T E M P L A T E
--
