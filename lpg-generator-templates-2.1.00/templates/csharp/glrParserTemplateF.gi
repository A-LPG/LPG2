--
-- C# GLR parser driver template (GSS/SPPF runtime).
--
-- B E G I N N I N G   O F   T E M P L A T E   glrParserTemplateF
--
%Options programming_language=csharp,margin=4,glr
%Options table,error_maps,scopes
%options prefix=TK_
%options action-block=("*.cs", "/.", "./")
%options ParseTable=LPG2.Runtime.ParseTable
%options nt-check

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
                    throw new System.Exception("No action specified for rule " + $rule_number);./

    $NullAction
    /.$Header$case $rule_number:
                    setResult(null);
                    break;./

    $BeginActions
    /.
        // Casting object to various generated AST types.
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
                {
                    //#line $next_line "$input_file$"./

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

        public $ast_class parse$entry_name(Monitor monitor, int error_repair_count)
        {
            glrParser.setMonitor(monitor);
            try
            {
                return ($ast_class) glrParser.parseEntry(
                    $sym_type.$entry_marker, error_repair_count);
            }
            catch (BadParseException e)
            {
                prsStream.reset(e.error_token);
                DiagnoseParser diagnoseParser =
                    new DiagnoseParser(prsStream, prsTable);
                diagnoseParser.diagnoseEntry(
                    $sym_type.$entry_marker, e.error_token);
            }
            return null;
        }
    ./

    $additional_interfaces /../
    $ast_class /.$ast_type./
    $super_class /.object./
    $unimplemented_symbols_warning /.false./

    --
    -- Old deprecated macros that should NEVER be used.
    --
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
    /.
    using LPG2.Runtime;
    using System;
    ./
%End

%Headers
    /.
    public class $action_type : $super_class, RuleAction$additional_interfaces
    {
        private PrsStream prsStream = null;
        private bool unimplementedSymbolsWarning = $unimplemented_symbols_warning;

        private static ParseTable prsTable = new $prs_type();
        public ParseTable getParseTable() { return prsTable; }

        private GLRParser glrParser = null;
        public GLRParser getParser() { return glrParser; }

        // During GLR-to-BT recovery, generated accessors must use BT stacks.
        private BacktrackingParser recoverParser = null;
        public void setRecoverParser(BacktrackingParser parser)
        {
            recoverParser = parser;
        }
        public BacktrackingParser getRecoverParser() { return recoverParser; }

        private void setResult(object value)
        {
            if (recoverParser != null) recoverParser.setSym1(value);
            else glrParser.setSym1(value);
        }
        public object getRhsSym(int i)
        {
            return recoverParser != null
                ? recoverParser.getSym(i)
                : glrParser.getSym(i);
        }

        public int getRhsTokenIndex(int i)
        {
            return recoverParser != null
                ? recoverParser.getToken(i)
                : glrParser.getToken(i);
        }
        public IToken getRhsIToken(int i)
        {
            return prsStream.getIToken(getRhsTokenIndex(i));
        }

        public int getRhsFirstTokenIndex(int i)
        {
            return recoverParser != null
                ? recoverParser.getFirstToken(i)
                : glrParser.getFirstToken(i);
        }
        public IToken getRhsFirstIToken(int i)
        {
            return prsStream.getIToken(getRhsFirstTokenIndex(i));
        }

        public int getRhsLastTokenIndex(int i)
        {
            return recoverParser != null
                ? recoverParser.getLastToken(i)
                : glrParser.getLastToken(i);
        }
        public IToken getRhsLastIToken(int i)
        {
            return prsStream.getIToken(getRhsLastTokenIndex(i));
        }

        public int getLeftSpan()
        {
            return recoverParser != null
                ? recoverParser.getFirstToken()
                : glrParser.getFirstToken();
        }
        public IToken getLeftIToken()
        {
            return prsStream.getIToken(getLeftSpan());
        }

        public int getRightSpan()
        {
            return recoverParser != null
                ? recoverParser.getLastToken()
                : glrParser.getLastToken();
        }
        public IToken getRightIToken()
        {
            return prsStream.getIToken(getRightSpan());
        }

        public int getRhsErrorTokenIndex(int i)
        {
            int index = getRhsTokenIndex(i);
            IToken err = prsStream.getIToken(index);
            return err is ErrorToken ? index : 0;
        }
        public ErrorToken getRhsErrorIToken(int i)
        {
            int index = getRhsTokenIndex(i);
            IToken err = prsStream.getIToken(index);
            return (ErrorToken) (err is ErrorToken ? err : null);
        }

        public void reset(ILexStream lexStream)
        {
            prsStream = new PrsStream(lexStream);
            glrParser.reset(prsStream);

            try
            {
                prsStream.remapTerminalSymbols(
                    orderedTerminalSymbols(), prsTable.getEoftSymbol());
            }
            catch (NullExportedSymbolsException e) {
            }
            catch (NullTerminalSymbolsException e) {
            }
            catch (UnimplementedTerminalsException e)
            {
                if (unimplementedSymbolsWarning) {
                    ArrayListHelper<int> unimplemented_symbols =
                        new ArrayListHelper<int>(e.getSymbols());
                    Console.Out.WriteLine(
                        "The Lexer will not scan the following token(s):");
                    for (int i = 0; i < unimplemented_symbols.Count; i++)
                    {
                        int id = unimplemented_symbols.get(i);
                        Console.Out.WriteLine(
                            "    " + $sym_type.orderedTerminalSymbols[id]);
                    }
                    Console.Out.WriteLine();
                }
            }
            catch (UndefinedEofSymbolException e)
            {
                throw new UndefinedEofSymbolException(
                    "The Lexer does not implement the Eof symbol " +
                    $sym_type.orderedTerminalSymbols[prsTable.getEoftSymbol()]);
            }
        }

        public $action_type()
        {
            try
            {
                glrParser = new GLRParser(
                    prsStream, prsTable, (RuleAction) this);
            }
            catch (NotGLRParseTableException e)
            {
                throw new NotGLRParseTableException(
                    "Regenerate $prs_type.cs with -GLR option");
            }
            catch (BadParseSymFileException e)
            {
                throw new BadParseSymFileException(
                    "Bad Parser Symbol File -- $sym_type.cs");
            }
        }

        public $action_type(ILexStream lexStream) : this()
        {
            reset(lexStream);
        }

        public int numTokenKinds() { return $sym_type.numTokenKinds; }
        public string[] orderedTerminalSymbols()
        {
            return $sym_type.orderedTerminalSymbols;
        }
        public string getTokenKindName(int kind)
        {
            return $sym_type.orderedTerminalSymbols[kind];
        }
        public int getEOFTokenKind() { return prsTable.getEoftSymbol(); }
        public IPrsStream getIPrsStream() { return prsStream; }
        public PrsStream getPrsStream() { return prsStream; }
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
            glrParser.setMonitor(monitor);
            try
            {
                return ($ast_class) glrParser.parse(error_repair_count);
            }
            catch (BadParseException e)
            {
                prsStream.reset(e.error_token);
                DiagnoseParser diagnoseParser =
                    new DiagnoseParser(prsStream, prsTable);
                diagnoseParser.diagnose(e.error_token);
            }
            return null;
        }

        //
        // Additional entry points, if any.
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
-- E N D   O F   T E M P L A T E   glrParserTemplateF
--
