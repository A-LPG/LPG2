--
-- An LPG Lexer Template Using lpg.jar
--
-- An instance of this template must have a $Export section and the export_terminals option
--
-- Macro that must be defined in an instance of this template:
--
--     $eof_token
--
-- Macros that may be redefined in an instance of this template
--
--     $import_classes
--     $action_class
--     $additional_interfaces
--     $super_stream_class -- subclass com.ibm.lpg.LpgLexStream for getKind
--     $prs_stream_class -- use /.PrsStream./ if not subclassing
--     $prs_stream       -- use /.prsStream./
--
-- B E G I N N I N G   O F   T E M P L A T E   LexerTemplateC
--
%Options programming_language=java,margin=8
%Options table
%options action-block=("*.java", "/.", "./")
%options headers=("*.java", "/:", ":/")
%options ParseTable=lpg.runtime.ParseTable

%Notice /.$copyright./

%Define

        $copyright /../

        --
        -- Macros that may be needed in an instance of this template
        --
        $import_classes /../
        $action_class /.$action_type./
        $additional_interfaces /../
        $super_stream_class /.LpgLexStream./
        $prs_stream_class /.PrsStream./
        $prs_stream /.prsStream./
        
        $setSym1 /.lexParser.setSym1./
        $setResult /.lexParser.setSym1./
        $getSym /.lexParser.getSym./
        $getToken /.lexParser.getToken./
        $getLeftSpan /.lexParser.getFirstToken./
        $getRightSpan /.lexParser.getLastToken./

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

        $SplitActions /../

        $EndAction
        /.          return;
                }
            }./

        $BeginJava
        /.$BeginAction
                    $symbol_declarations./

        $EndJava /.$EndAction./

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

%Headers
        /.
        $copyright
        $import_classes
        import lpg.runtime.*;

        public class $action_class extends $super_stream_class implements $exp_type, $sym_type, RuleAction$additional_interfaces
        {
            private static ParseTable prs = new $prs_type();
            private $prs_stream_class $prs_stream;
            private LexParser lexParser = new LexParser(this, prs, this);

            public $prs_stream_class getPrsStream() { return $prs_stream; }
            public int getToken(int i)      { return lexParser.getToken(i); }
            public int getFirstToken(int i) { return lexParser.getFirstToken(i); }
            public int getLastToken(int i)  { return lexParser.getLastToken(i); }

            public int getLeftSpan() { return lexParser.getToken(1); }
            public int getRightSpan() { return lexParser.getLastToken(); }

            public $action_class(String filename, int tab) throws java.io.IOException 
            {
                super(filename, tab);
            }

            public $action_class(char[] input_chars, String filename, int tab)
            {
                super(input_chars, filename, tab);
            }

            public $action_class(char[] input_chars, String filename)
            {
                this(input_chars, filename, 1);
            }

            public $action_class() {}

            public String[] orderedExportedSymbols() { return $exp_type.orderedTerminalSymbols; }
            public LexStream getLexStream() { return (LexStream) this; }

            public void lexer()
            {
                lexer(null);
            }
        
            public void lexer(Monitor monitor)
            {
                if (getInputChars() == null)
                    throw new NullPointerException("LexStream was not initialized");

                this.$prs_stream = prsStream;

                prsStream.makeToken(0, 0, 0); // Token list must start with a bad token
                
                lexParser.parseCharacters(monitor); // Lex the input characters
                
                int i = getStreamIndex();
                prsStream.makeToken(i, i, $eof_token); // and end with the end of file token
                prsStream.setStreamLength(prsStream.getSize());
                
                return;
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
