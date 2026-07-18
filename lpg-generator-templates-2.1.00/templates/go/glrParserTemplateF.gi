--
-- Go GLR parser template. GLRParser executes all conflict alternatives,
-- packs compatible ASTs through nextAst, and exposes the shared SPPF.
--
-- B E G I N N I N G   O F   T E M P L A T E   glrParserTemplateF
--
%Options programming_language=go,margin=4,glr
%Options table,error_maps,scopes
%options prefix=TK_
%options action-block=("*.go", "/.", "./")
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
                //
                // Rule $rule_number:  $rule_text
                //
                ./

    $BeginAction
    /.$Header$case $rule_number: {
                   //#line $next_line "$input_file$"./

    $EndAction
    /.                break
                }./

    $BeginJava
    /.$Header$case $rule_number: {
                    $symbol_declarations
                    //#line $next_line "$input_file$"./

    $EndJava /.$EndAction./

    $NoAction
    /.$Header$case $rule_number:
                    break./

    $BadAction
    /.$Header$case $rule_number:
                    panic("No action specified for rule $rule_number")./

    $NullAction
    /.$Header$case $rule_number:
                    my.SetResult(nil)
                    break./

    $BeginActions
    /.

        func (my *$action_type) RuleAction(ruleNumber int) {
            switch ruleNumber {./

    $EndActions
    /.
                default:
                    break
            }
        }./

    $entry_declarations
    /.

    func (my *$action_type) Parse$entry_name(error_repair_count int, monitor Monitor) (interface{}, error) {
        my.glrParser.SetMonitor(monitor)
        ast, ex := my.glrParser.ParseEntry($sym_type.$entry_marker, error_repair_count)
        if ex != nil {
            if e, ok := ex.(*BadParseException); ok {
                my.prsStream.ResetTo(e.ErrorToken)
                diagnoseParser := NewDiagnoseParser(my.prsStream, my.prsTable, 0, 0, nil)
                diagnoseParser.DiagnoseEntry($sym_type.$entry_marker, e.ErrorToken)
            }
            return nil, ex
        }
        return ast, nil
    }
./

    $additional_interfaces /../
    $ast_class /.$ast_type./
    $unimplemented_symbols_warning /.false./

    $setSym1 /. // macro setSym1 is deprecated. Use function SetResult
                my.GetParser().SetSym1./
    $setResult /. // macro SetResult is deprecated. Use function SetResult
                 my.GetParser().SetSym1./
    $getSym /. // macro getSym is deprecated. Use function GetRhsSym
              my.GetParser().GetSym./
    $getToken /. // macro getToken is deprecated. Use function GetRhsTokenIndex
                my.GetParser().GetToken./
    $getIToken /. // macro getIToken is deprecated. Use function GetRhsIToken
                 my.prsStream.GetIToken./
    $getLeftSpan /. // macro getLeftSpan is deprecated. Use function GetLeftSpan
                   my.GetParser().GetFirstToken./
    $getRightSpan /. // macro getRightSpan is deprecated. Use function GetRightSpan
                    my.GetParser().GetLastToken./
%End

%Globals
    /.
import (
    . "github.com/A-LPG/LPG-go-runtime/lpg2"
)
    ./
%End

%Headers
    /.
    type $action_type struct {
        prsStream *PrsStream
        glrParser *GLRParser
        recoverParser *BacktrackingParser
        unimplementedSymbolsWarning bool
        prsTable ParseTable
    }

    func New$action_type(lexStream ILexStream) (*$action_type, error) {
        my := new($action_type)
        my.prsTable = &$prs_type{}
        my.prsStream = NewPrsStream(lexStream)
        my.unimplementedSymbolsWarning = $unimplemented_symbols_warning

        parser, e := NewGLRParser(nil, my.prsTable, my, nil)
        if e != nil {
            if _, ok := e.(*NotGLRParseTableException); ok {
                return nil, NewNotGLRParseTableException(
                    "Regenerate %prs_type.go with -GLR option")
            }
            if _, ok := e.(*BadParseSymFileException); ok {
                return nil, NewBadParseSymFileException(
                    "Bad Parser Symbol File -- %sym_type.go")
            }
            return nil, e
        }
        my.glrParser = parser
        if lexStream != nil {
            if e := my.Reset(lexStream); e != nil {
                return nil, e
            }
        }
        return my, nil
    }

    func (my *$action_type) GetParseTable() ParseTable {
        return my.prsTable
    }

    func (my *$action_type) GetParser() *GLRParser {
        return my.glrParser
    }

    // SetRecoverParser lets GLRParser route semantic actions to the
    // BacktrackingParser while performing error-repair fallback.
    func (my *$action_type) SetRecoverParser(parser *BacktrackingParser) {
        my.recoverParser = parser
    }

    func (my *$action_type) GetRecoverParser() *BacktrackingParser {
        return my.recoverParser
    }

    func (my *$action_type) SetResult(object interface{}) {
        if my.recoverParser != nil {
            my.recoverParser.SetSym1(object)
        } else {
            my.glrParser.SetSym1(object)
        }
    }

    func (my *$action_type) GetRhsSym(i int) interface{} {
        if my.recoverParser != nil {
            return my.recoverParser.GetSym(i)
        }
        return my.glrParser.GetSym(i)
    }

    func (my *$action_type) GetRhsTokenIndex(i int) int {
        if my.recoverParser != nil {
            return my.recoverParser.GetToken(i)
        }
        return my.glrParser.GetToken(i)
    }

    func (my *$action_type) GetRhsIToken(i int) IToken {
        return my.prsStream.GetIToken(my.GetRhsTokenIndex(i))
    }

    func (my *$action_type) GetRhsFirstTokenIndex(i int) int {
        if my.recoverParser != nil {
            return my.recoverParser.GetFirstTokenAt(i)
        }
        return my.glrParser.GetFirstTokenAt(i)
    }

    func (my *$action_type) GetRhsFirstIToken(i int) IToken {
        return my.prsStream.GetIToken(my.GetRhsFirstTokenIndex(i))
    }

    func (my *$action_type) GetRhsLastTokenIndex(i int) int {
        if my.recoverParser != nil {
            return my.recoverParser.GetLastTokenAt(i)
        }
        return my.glrParser.GetLastTokenAt(i)
    }

    func (my *$action_type) GetRhsLastIToken(i int) IToken {
        return my.prsStream.GetIToken(my.GetRhsLastTokenIndex(i))
    }

    func (my *$action_type) GetLeftSpan() int {
        if my.recoverParser != nil {
            return my.recoverParser.GetFirstToken()
        }
        return my.glrParser.GetFirstToken()
    }

    func (my *$action_type) GetLeftIToken() IToken {
        return my.prsStream.GetIToken(my.GetLeftSpan())
    }

    func (my *$action_type) GetRightSpan() int {
        if my.recoverParser != nil {
            return my.recoverParser.GetLastToken()
        }
        return my.glrParser.GetLastToken()
    }

    func (my *$action_type) GetRightIToken() IToken {
        return my.prsStream.GetIToken(my.GetRightSpan())
    }

    func (my *$action_type) GetRhsErrorTokenIndex(i int) int {
        index := my.GetRhsTokenIndex(i)
        if _, ok := my.prsStream.GetIToken(index).(*ErrorToken); ok {
            return index
        }
        return 0
    }

    func (my *$action_type) GetRhsErrorIToken(i int) *ErrorToken {
        token, _ := my.prsStream.GetIToken(
            my.GetRhsTokenIndex(i)).(*ErrorToken)
        return token
    }

    func (my *$action_type) Reset(lexStream ILexStream) error {
        my.prsStream = NewPrsStream(lexStream)
        if err := my.glrParser.Reset(my.prsStream, nil, nil, nil); err != nil {
            return err
        }
        ex := my.prsStream.RemapTerminalSymbols(
            my.OrderedTerminalSymbols(), my.prsTable.GetEoftSymbol())
        if ex == nil {
            return nil
        }
        if _, ok := ex.(*NullExportedSymbolsException); ok {
            return ex
        }
        if _, ok := ex.(*NullTerminalSymbolsException); ok {
            return ex
        }
        if e, ok := ex.(*UnimplementedTerminalsException); ok {
            if my.unimplementedSymbolsWarning {
                println("The Lexer will not scan the following token(s):")
                symbols := e.GetSymbols()
                for i := 0; i < symbols.Size(); i++ {
                    println("    " + $sym_type.OrderedTerminalSymbols[
                        symbols.Get(i)])
                }
                println()
            }
            return ex
        }
        if _, ok := ex.(*UndefinedEofSymbolException); ok {
            return NewUndefinedEofSymbolException(
                "The Lexer does not implement the Eof symbol " +
                $sym_type.OrderedTerminalSymbols[
                    my.prsTable.GetEoftSymbol()])
        }
        return ex
    }

    func (my *$action_type) NumTokenKinds() int {
        return $sym_type.NumTokenKinds
    }

    func (my *$action_type) OrderedTerminalSymbols() []string {
        return $sym_type.OrderedTerminalSymbols
    }

    func (my *$action_type) GetTokenKindName(kind int) string {
        return $sym_type.OrderedTerminalSymbols[kind]
    }

    func (my *$action_type) GetEOFTokenKind() int {
        return my.prsTable.GetEoftSymbol()
    }

    func (my *$action_type) GetIPrsStream() IPrsStream {
        return my.prsStream
    }

    func (my *$action_type) Parser() (interface{}, error) {
        return my.ParserWithMonitor(0, nil)
    }

    func (my *$action_type) ParserWithMonitor(
        error_repair_count int, monitor Monitor) (interface{}, error) {
        my.glrParser.SetMonitor(monitor)
        ast, ex := my.glrParser.Parse(error_repair_count)
        if ex == nil {
            return ast, nil
        }
        if e, ok := ex.(*BadParseException); ok {
            my.prsStream.ResetTo(e.ErrorToken)
            diagnoseParser := NewDiagnoseParser(
                my.prsStream, my.prsTable, 0, 0, nil)
            diagnoseParser.Diagnose(e.ErrorToken)
        }
        return nil, ex
    }

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
-- E N D   O F   T E M P L A T E
--
