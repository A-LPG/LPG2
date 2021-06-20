--
-- An LPG Parser Template Using lpg.jar
--
--
-- B E G I N N I N G   O F   T E M P L A T E   dtParserTemplateA
--
-- In a parser using this template, the following macros may be redefined:
--
--     $import_classes
--     $action_class
--     $additional_interfaces
--     $ast_class
--
%Options programming_language=java,margin=8,nobacktrack
%Options table,error_maps,scopes
%options action-block=("*.java", "/.", "./")
%options headers=("*.java", "/:", ":/")
%options ParseTable=lpg.runtime.ParseTable

%Notice /.$copyright./

%Define
        $copyright /../

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
        /. $NullAllocation $Header
            //
            // final class NullAction extends Action
            //
        ./

        --
        -- This macro is used to initialize the ruleAction array
        -- to the no_action function.
        --
        $NoAction
        /. $NoAllocation $Header
            //
            // final class NullAction extends Action
            //
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
        /.
                    return;
                }
            }./

        $BeginJava
        /.$BeginAction
                    $symbol_declarations./

        $EndJava /.$EndAction./

        $SplitActions /../

        --
        -- Macros that may be needed in a parser using this template
        --
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
        import java.lang.IndexOutOfBoundsException;
        import lpg.runtime.*;

        public class $action_class implements RuleAction$additional_interfaces
        {
            private static ParseTable prs = new $prs_type();
            private PrsStream prsStream;
            private DeterministicParser dtParser;

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
                try
                {
                    dtParser = new DeterministicParser(monitor, (TokenStream)prsStream, prs, (RuleAction)this);
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
                    return ($ast_class) dtParser.parse();
                }
                catch (BadParseException e)
                {
                    prsStream.reset(e.error_token); // point to error token

                    DiagnoseParser diagnoseParser = new DiagnoseParser((TokenStream)prsStream, prs);
                    diagnoseParser.diagnose(e.error_token);
                }

                return null;
            }

            public final void ruleAction(int rule_number)
            {
                ruleAction[rule_number].action();
            }

            abstract class Action
            {
                public abstract void action();
            }

            final class NoAction extends Action
            {
                public void action() {}
            }

            //
            // Action for a null rule
            //
            final class NullAction extends Action
            {
                public void action() { dtParser.setSym1(null); }
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
