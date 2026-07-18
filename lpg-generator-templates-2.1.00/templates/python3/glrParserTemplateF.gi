--
-- Python 3 GLR parser driver template (GSS/SPPF runtime).
--
-- B E G I N N I N G   O F   T E M P L A T E   glrParserTemplateF
--
%options programming_Language=python3,margin=4,glr
%options table,error_maps,scopes
%options prefix=TK_
%options action-block=("*.py", "/.", "./")
%options ParseTable=ParseTable
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
            #
            # Rule $rule_number:  $rule_text
            #
            ./

    $DefaultAction
    /.$Header def Act$rule_number():./

    $BeginAction
    /.$DefaultAction
                   ##line $next_line "$input_file$"./

    $EndAction
    /.
             self.__rule_action[$rule_number] = Act$rule_number
    ./

    $BeginJava
    /.$DefaultAction
                    $symbol_declarations
                    ##line $next_line "$input_file$"./

    $EndJava /.$EndAction./

    $NoAction
    /.$Header
                     ./

    $BadAction
    /.$DefaultAction
                    raise ValueError("No action specified for rule " + str($rule_number))./

    $NullAction
    /.$DefaultAction
                    self.setResult(None)
                    ./

    $BeginActions
    /.
        def initRuleAction(self):
    ./

    $EndActions
    /../

    $entry_declarations
    /.
        def parse$entry_name(self, monitor: Monitor = None,
                             error_repair_count: int = 0):
            self.glrParser.setMonitor(monitor)
            try:
                return self.glrParser.parseEntry(
                    $sym_type.$entry_marker, error_repair_count)
            except BadParseException as e:
                self.prsStream.reset(e.error_token)
                diagnoseParser = DiagnoseParser(
                    self.prsStream, $action_type.prsTable)
                diagnoseParser.diagnoseEntry(
                    $sym_type.$entry_marker, e.error_token)
            return None
    ./

    $additional_interfaces /../
    $ast_class /.$ast_type./
    $super_class /.object./
    $unimplemented_symbols_warning /.False./

    --
    -- Old deprecated macros that should NEVER be used.
    --
    $setSym1 /. # macro setSym1 is deprecated. Use function setResult
                self.getParser().setSym1./
    $setResult /. # macro setResult is deprecated. Use function setResult
                 self.getParser().setSym1./
    $getSym /. # macro getSym is deprecated. Use function getRhsSym
              self.getParser().getSym./
    $getToken /. # macro getToken is deprecated. Use function getRhsTokenIndex
                self.getParser().getToken./
    $getIToken /. # macro getIToken is deprecated. Use function getRhsIToken
                 self.prsStream.getIToken./
    $getLeftSpan /. # macro getLeftSpan is deprecated. Use function getLeftSpan
                   self.getParser().getFirstToken./
    $getRightSpan /. # macro getRightSpan is deprecated. Use function getRightSpan
                    self.getParser().getLastToken./
%End

%Globals
    /.
from __future__ import annotations

from lpg2 import ArrayList, BadParseException, RuleAction, PrsStream, ParseTable, GLRParser, BacktrackingParser, IToken, ErrorToken, ILexStream, NullExportedSymbolsException, UnimplementedTerminalsException, UndefinedEofSymbolException, NotGLRParseTableException, BadParseSymFileException, IPrsStream, Monitor, DiagnoseParser, IAst, IAstVisitor, IAbstractArrayList, NullTerminalSymbolsException
from $prs_type import $prs_type
from $sym_type import $sym_type
    ./
%End

%Headers
    /.
    class $action_type(RuleAction$additional_interfaces):

        def ruleAction(self, ruleNumber: int):
            act = self.__rule_action[ruleNumber]
            if act:
                act()

        prsTable: ParseTable = $prs_type()

        def getParseTable(self) -> ParseTable:
            return $action_type.prsTable

        def getParser(self) -> GLRParser:
            return self.glrParser

        def setRecoverParser(self, parser):
            self.recoverParser = parser

        def getRecoverParser(self):
            return self.recoverParser

        def setResult(self, value):
            if self.recoverParser is not None:
                self.recoverParser.setSym1(value)
            else:
                self.glrParser.setSym1(value)

        def getRhsSym(self, i: int):
            parser = (self.recoverParser if self.recoverParser is not None
                      else self.glrParser)
            return parser.getSym(i)

        def getRhsTokenIndex(self, i: int) -> int:
            parser = (self.recoverParser if self.recoverParser is not None
                      else self.glrParser)
            return parser.getToken(i)

        def getRhsIToken(self, i: int) -> IToken:
            return self.prsStream.getIToken(self.getRhsTokenIndex(i))

        def getRhsFirstTokenIndex(self, i: int) -> int:
            parser = (self.recoverParser if self.recoverParser is not None
                      else self.glrParser)
            return parser.getFirstToken(i)

        def getRhsFirstIToken(self, i: int) -> IToken:
            return self.prsStream.getIToken(self.getRhsFirstTokenIndex(i))

        def getRhsLastTokenIndex(self, i: int) -> int:
            parser = (self.recoverParser if self.recoverParser is not None
                      else self.glrParser)
            return parser.getLastToken(i)

        def getRhsLastIToken(self, i: int) -> IToken:
            return self.prsStream.getIToken(self.getRhsLastTokenIndex(i))

        def getLeftSpan(self) -> int:
            parser = (self.recoverParser if self.recoverParser is not None
                      else self.glrParser)
            return parser.getFirstToken()

        def getLeftIToken(self) -> IToken:
            return self.prsStream.getIToken(self.getLeftSpan())

        def getRightSpan(self) -> int:
            parser = (self.recoverParser if self.recoverParser is not None
                      else self.glrParser)
            return parser.getLastToken()

        def getRightIToken(self) -> IToken:
            return self.prsStream.getIToken(self.getRightSpan())

        def getRhsErrorTokenIndex(self, i: int) -> int:
            index = self.getRhsTokenIndex(i)
            err = self.prsStream.getIToken(index)
            return index if isinstance(err, ErrorToken) else 0

        def getRhsErrorIToken(self, i: int) -> ErrorToken:
            index = self.getRhsTokenIndex(i)
            err = self.prsStream.getIToken(index)
            return err if isinstance(err, ErrorToken) else None

        def reset(self, lexStream: ILexStream):
            self.prsStream.resetLexStream(lexStream)
            self.glrParser.reset(self.prsStream)

            try:
                self.prsStream.remapTerminalSymbols(
                    self.orderedTerminalSymbols(),
                    $action_type.prsTable.getEoftSymbol())
            except NullExportedSymbolsException:
                pass
            except NullTerminalSymbolsException:
                pass
            except UnimplementedTerminalsException as e:
                if self.unimplementedSymbolsWarning:
                    unimplemented_symbols = e.getSymbols()
                    print("The Lexer will not scan the following token(s):")
                    for i in range(unimplemented_symbols.size()):
                        symbol_id = unimplemented_symbols.get(i)
                        print("    " + str(
                            $sym_type.orderedTerminalSymbols[symbol_id]))
                    print("")
            except UndefinedEofSymbolException:
                raise UndefinedEofSymbolException(
                    "The Lexer does not implement the Eof symbol " +
                    $sym_type.orderedTerminalSymbols[
                        $action_type.prsTable.getEoftSymbol()])

        def __init__(self, lexStream: ILexStream = None):
            super().__init__()
            self.__rule_action = [None] * ($num_rules + 2)
            self.prsStream: PrsStream = PrsStream()
            self.glrParser: GLRParser = None
            self.recoverParser: BacktrackingParser = None
            self.unimplementedSymbolsWarning: bool = $unimplemented_symbols_warning
            self.initRuleAction()
            try:
                self.glrParser = GLRParser(
                    None, $action_type.prsTable, self)
            except NotGLRParseTableException:
                raise NotGLRParseTableException(
                    "Regenerate $prs_type.py with -GLR option")
            except BadParseSymFileException:
                raise BadParseSymFileException(
                    "Bad Parser Symbol File -- $sym_type.py")

            if lexStream is not None:
                self.reset(lexStream)

        def numTokenKinds(self) -> int:
            return $sym_type.numTokenKinds

        def orderedTerminalSymbols(self) -> list:
            return $sym_type.orderedTerminalSymbols

        def getTokenKindName(self, kind: int) -> str:
            return $sym_type.orderedTerminalSymbols[kind]

        def getEOFTokenKind(self) -> int:
            return $action_type.prsTable.getEoftSymbol()

        def getIPrsStream(self) -> IPrsStream:
            return self.prsStream

        def getPrsStream(self) -> PrsStream:
            return self.prsStream

        def getParseStream(self) -> PrsStream:
            return self.prsStream

        def parser(self, error_repair_count: int = 0,
                   monitor: Monitor = None):
            self.glrParser.setMonitor(monitor)
            try:
                return self.glrParser.parse(error_repair_count)
            except BadParseException as e:
                self.prsStream.reset(e.error_token)
                diagnoseParser = DiagnoseParser(
                    self.prsStream, $action_type.prsTable)
                diagnoseParser.diagnose(e.error_token)
            return None

        #
        # Additional entry points, if any.
        #
        $entry_declarations
    ./

%End

%Rules
    /.$BeginActions./
%End

%Trailers
    /.
        $EndActions

    ./
%End

--
-- E N D   O F   T E M P L A T E   glrParserTemplateF
--
