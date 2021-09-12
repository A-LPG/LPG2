--
-- In a parser using my template, the following macro may be redefined:
--
--     $additional_interfaces
--     $ast_class
--
-- B E G I N N I N G   O F   T E M P L A T E   btParserTemplateF
--
%Options programming_language=go,margin=4,backtrack
%Options table,error_maps,scopes
%options prefix=TK_
%options action-block=("*.go", "/.", "./")
%options ParseTable=ParseTable
%options nt-check

--
-- This template requires that the name of the EOF token be set
-- to EOF_TOKEN to be consistent with LexerTemplateD and LexerTemplateE
--
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
                    throw new Error("No action specified for rule " + $rule_number)./

    $NullAction
    /.$Header$case $rule_number:
                    my.SetResult(nil)
                    break./

    $BeginActions
    /.
       
        func (my *$action_type)  RuleAction(ruleNumber int){
            switch ruleNumber{./


    $EndActions
    /.
                default:
                    break
            }
            return
        }./

    $entry_declarations
    /.
        
    func (my *$action_type)  Parse$entry_name(error_repair_count int,monitor Monitor) (interface{}, error){
        my.btParser.SetMonitor(monitor)
        
        var ast,ex = my.btParser.FuzzyParseEntry($sym_type.$entry_marker, error_repair_count)
        if nil != ex  {
            var e,ok= ex.(*BadParseException)
            if ok{
                my.prsStream.ResetTo(e.ErrorToken) // point to error token

                var diagnoseParser = NewDiagnoseParser(my.prsStream, my.prsTable, 0,0,nil)
                diagnoseParser.DiagnoseEntry($sym_type.$entry_marker, e.ErrorToken)
            }
            return nil,ex
        }
        return ast,nil
    }
./

    --
    -- Macros that may be needed in a parser using my template
    --
    $additional_interfaces /../
    $ast_class /.$ast_type./
   
    $unimplemented_symbols_warning /.false./

    --
    -- Old deprecated macros that should NEVER be used.
    --
    $setSym1 /. // macro setSym1 is deprecated. Use function SetResult
                my.GetParser().SetSym1./
    $setResult /. // macro SetResult is deprecated. Use function SetResult
                 my.GetParser().SetSym1./
    $getSym /. // macro getSym is deprecated. Use function GetRhsSym
              my.GetParser().GetSym./
    $getToken /. // macro getToken is deprecated. Use function GetRhsTokenIndex
                my.GetParser().GetToken./
    $getIToken /. // macro getIToken is deprecated. Use function GetRhsIToken
                 my.my.prsStream.GetIToken./
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
    type  $action_type  struct{
        prsStream   *PrsStream
        btParser *BacktrackingParser
        unimplementedSymbolsWarning  bool
        prsTable  ParseTable
    }
    func New$action_type(lexStream ILexStream) (*$action_type,error){
        my := new($action_type)
        my.prsTable = &$prs_type{}
        my.prsStream =  NewPrsStream(lexStream)
        my.unimplementedSymbolsWarning  = $unimplemented_symbols_warning
        var e error
        my.btParser,e =  NewBacktrackingParser(nil, my.prsTable,  my,nil) 
        if e == nil{
            if lexStream != nil{
                err := my.Reset(lexStream)
                if err != nil {
                    return nil, err
                }
            }
            return  my,nil
        }
        var _,ok = e.(*NotBacktrackParseTableException)
        if ok{
            e = NewNotBacktrackParseTableException("Regenerate %prs_type.go with -BACKTRACK option")
            return  nil,e
        }
        _,ok = e.(*BadParseSymFileException)
        if ok{
            e= NewBadParseSymFileException("Bad Parser Symbol File -- %sym_type.go")
            return  nil,e
        }
        return nil,e
    }
        

    func (my *$action_type)  GetParseTable() ParseTable{ 
        return my.prsTable 
    }

    func (my *$action_type)  GetParser() *BacktrackingParser{
        return my.btParser 
    }

    func (my *$action_type)  SetResult( object interface{}) {
        my.btParser.SetSym1(object)
    }
    func (my *$action_type) GetRhsSym(i int) interface{} { 
        return my.btParser.GetSym(i) 
    }

    func (my *$action_type)  GetRhsTokenIndex(i int)int {
        return my.btParser.GetToken(i)
        }
    func (my *$action_type)  GetRhsIToken(i int) IToken{
        return my.prsStream.GetIToken(my.GetRhsTokenIndex(i)) 
    }

    func (my *$action_type)  GetRhsFirstTokenIndex(i int)int { 
    return my.btParser.GetFirstTokenAt(i)
        }
    func (my *$action_type)  GetRhsFirstIToken(i int) IToken{
        return my.prsStream.GetIToken(my.GetRhsFirstTokenIndex(i)) 
        }

    func (my *$action_type)  GetRhsLastTokenIndex(i int)int {
        return my.btParser.GetLastTokenAt(i) 
        }
    func (my *$action_type)  GetRhsLastIToken(i int) IToken{ 
    return my.prsStream.GetIToken(my.GetRhsLastTokenIndex(i)) 
    }

    func (my *$action_type)  GetLeftSpan() int{ 
        return my.btParser.GetFirstToken() 
    }
    func (my *$action_type)  GetLeftIToken() IToken {
        return my.prsStream.GetIToken(my.GetLeftSpan())
    }

    func (my *$action_type)  GetRightSpan() int{
        return my.btParser.GetLastToken() 
    }
    func (my *$action_type)  GetRightIToken()IToken {
        return my.prsStream.GetIToken(my.GetRightSpan())
    }

    func (my *$action_type)  GetRhsErrorTokenIndex(i int)int{
        var index = my.btParser.GetToken(i)
        var err = my.prsStream.GetIToken(index)
        var _,ok = err.(*ErrorToken)
        if ok {
            return index
        }else{
            return 0
        }
    }
    func (my *$action_type)  GetRhsErrorIToken(i int) *ErrorToken{
        var index = my.btParser.GetToken(i)
        var err = my.prsStream.GetIToken(index)
        var token,_ = err.(*ErrorToken)
        return token
    }

    func (my *$action_type)  Reset(lexStream ILexStream ) error{
        my.prsStream = NewPrsStream(lexStream)
        err := my.btParser.Reset(my.prsStream,nil,nil,nil)
        if err != nil {
            return err
        }
        var ex = my.prsStream.RemapTerminalSymbols(my.OrderedTerminalSymbols(), my.prsTable.GetEoftSymbol())
        if ex == nil{
            return nil
        }
        var _,ok = ex.(*NullExportedSymbolsException)
        if ok {
            return ex
        }
        _,ok = ex.(*NullTerminalSymbolsException)
        if ok {
            return ex
        }
        var e *UnimplementedTerminalsException
        e,ok = ex.(*UnimplementedTerminalsException)
        if ok {
            if my.unimplementedSymbolsWarning {
                var unimplemented_symbols = e.GetSymbols()
                println("The Lexer will not scan the following token(s):")
                var i int = 0
                for ; i < unimplemented_symbols.Size() ;i++{
                    var id = unimplemented_symbols.Get(i)
                    println("    " + $sym_type.OrderedTerminalSymbols[id])
                }
                println()
            }
            return  ex
        }
        _,ok = ex.(*UndefinedEofSymbolException)
        if ok {
            return NewUndefinedEofSymbolException("The Lexer does not implement the Eof symbol " +
            $sym_type.OrderedTerminalSymbols[my.prsTable.GetEoftSymbol()])
        }
        return ex
    }


    func (my *$action_type)  NumTokenKinds()int {
            return $sym_type.NumTokenKinds 
    }
    func (my *$action_type)  OrderedTerminalSymbols()[]string {
        return $sym_type.OrderedTerminalSymbols
    }
    func (my *$action_type)  GetTokenKindName(kind int) string{
            return $sym_type.OrderedTerminalSymbols[kind] 
    }
    func (my *$action_type)  GetEOFTokenKind() int{
        return my.prsTable.GetEoftSymbol()
        }
    func (my *$action_type)  GetIPrsStream() IPrsStream{
        return my.prsStream
        }
    func (my *$action_type) Parser() (interface{}, error) {
        return my.ParserWithMonitor(0,nil)
    }
    func (my *$action_type) ParserWithMonitor(error_repair_count int ,  monitor Monitor) (interface{}, error){

        my.btParser.SetMonitor(monitor)
        
        var ast,ex= my.btParser.FuzzyParseEntry(0, error_repair_count)
        if ex == nil{
            return ast,ex
        }
        var e,ok= ex.(*BadParseException)
        if ok{
            my.prsStream.ResetTo(e.ErrorToken) // point to error token

            var diagnoseParser = NewDiagnoseParser(my.prsStream, my.prsTable,0,0,nil)
            diagnoseParser.Diagnose(e.ErrorToken)
        }
        return ast,ex
    }
    //
    // Additional entry points, if any
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
    ./
%End

--
-- E N D   O F   T E M P L A T E
--
