--
-- An instance of my template must have a $Export section and the export_terminals option
--
-- Macros that may be redefined in an instance of my template
--
--     $eof_token
--     $additional_interfaces
--     $super_stream_class -- subclass com.ibm.lpg.LpgLexStream for GetKind
--     $prs_stream_class -- use /.PrsStream./ if not subclassing
--     $super_class
--
-- B E G I N N I N G   O F   T E M P L A T E   LexerTemplateF
--
%Options programming_language=go,margin=4
%Options table
%options action-block=("*.go", "/.", "./")
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
    -- Macros that are be needed in an instance of my template
    --
    $eof_token /.$_EOF_TOKEN./
    
    $additional_interfaces /../
    $super_stream_class /.$file_prefix$LpgLexStream./
    $prs_stream_class /.IPrsStream./
    $super_class /.interface{}./

    $prs_stream /. // macro prs_stream is deprecated. Use function GetPrsStream
                  my.GetPrsStream()./
    $setSym1 /. // macro setSym1 is deprecated. Use function SetResult
               my.lexParser.SetSym1./
    $setResult /. // macro setResult is deprecated. Use function SetResult
                 my.lexParser.SetSym1./
    $getSym /. // macro getSym is deprecated. Use function GetLastToken
              my.lexParser.GetSym./
    $getToken /. // macro getToken is deprecated. Use function GetToken
                my.lexParser.GetToken./
    $getLeftSpan /. // macro getLeftSpan is deprecated. Use function GetLeftSpan
                   my.lexParser.GetFirstToken./
    $getRightSpan /. // macro getRightSpan is deprecated. Use function GetRightSpan
                    my.lexParser.GetLastToken./

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
    /.            
                      break
                }./

    $BeginJava
    /.$BeginAction
                $symbol_declarations./

    $EndJava /.$EndAction./

    $NoAction
    /.$Header$case $rule_number:
                    break ./

    $BeginActions
    /.
        func (my *$action_type)  RuleAction(ruleNumber int){
            switch ruleNumber {./

    $SplitActions
    /.
                    default:
                        RuleAction$rule_number(ruleNumber)
                        break
                }
                return
            }

            func (my *$action_type)  RuleAction$rule_number(ruleNumber int)
            {
                switch ruleNumber
                {./

    $EndActions
    /.
                default:
                    break
            }
            return
        }./
%End

%Globals
    /.
import ."github.com/A-LPG/LPG-go-runtime/lpg2"
    ./
%End

%Headers
    /.
    type  $action_type struct{

        kwLexer  *$kw_lexer_class 
        PrintTokens  bool
        lexParser  *LexParser
        lexStream *$super_stream_class 
        prs     ParseTable
    }
    func New$action_type(filename string,  tab int ,input_chars []rune) *$action_type {
        my := new($action_type)
        my.prs =  New$prs_type()
        my.lexParser   =  NewLexParser()
        my.PrintTokens = false
        var e error
        my.lexStream, e =  New$super_stream_class(filename,input_chars, tab)
        if e != nil{
            return nil
        }
        my.lexParser.Reset( my.lexStream, my.prs,  my)
        my.ResetKeywordLexer()
        return my
    }
        func (my *$action_type)  GetParseTable()ParseTable {
         return my.prs 
        }

        func (my *$action_type)  GetParser() *LexParser{ 
            return my.lexParser 
        }

        func (my *$action_type)  GetToken(i int)int { 
        return my.lexParser.GetToken(i)
         }
        func (my *$action_type)  GetRhsFirstTokenIndex(i int)int {
         return my.lexParser.GetFirstTokenAt(i) 
         }
        func (my *$action_type)  GetRhsLastTokenIndex(i int) int{
         return my.lexParser.GetLastTokenAt(i) 
         }

        func (my *$action_type)  GetLeftSpan()int { 
        return my.lexParser.GetToken(1) 
        }
        func (my *$action_type)  GetRightSpan()int {
         return my.lexParser.GetLastToken() 
         }
  
        func (my *$action_type)  ResetKeywordLexer(){
            if my.kwLexer == nil {
                my.kwLexer = New$kw_lexer_class(my.lexStream.GetInputChars(), $_IDENTIFIER)
            }else {
                my.kwLexer.SetInputChars(my.lexStream.GetInputChars())
            }
        }
  
        func (my *$action_type)  Reset(filename string,tab int,input_chars []rune) error{
            var e error
            my.lexStream ,e = New$super_stream_class(filename,input_chars, tab)
            if e != nil{
                return e
            }
            my.lexParser.Reset(my.lexStream, my.prs, my)
            my.ResetKeywordLexer()
            return nil
        }
        

        func (my *$action_type)  GetILexStream()ILexStream { 
            return my.lexStream
        }



        func (my *$action_type)  InitializeLexer(prsStream $prs_stream_class, start_offSet int, end_offSet int) error{
            if my.lexStream.GetInputChars() == nil{
                return NewNullPointerException("LexStream was not initialized")
            }
            my.lexStream.SetPrsStream(prsStream)
            prsStream.MakeToken(start_offSet, end_offSet, 0) // Token list must start with a bad token
            return nil
        }

        func (my *$action_type)  AddEOF(prsStream $prs_stream_class, end_offSet int) {
            prsStream.MakeToken(end_offSet, end_offSet, $eof_token) // and end with the end of file token
            prsStream.SetStreamLength(prsStream.GetSize())
        }

        func (my *$action_type) LexerWithPosition(prsStream $prs_stream_class , start_offSet int , end_offSet int, monitor Monitor) error{
        
            if start_offSet <= 1{
                err := my.InitializeLexer(prsStream, 0, -1)
                if err != nil {
                    return err
                }
            }else {
                err := my.InitializeLexer(prsStream, start_offSet - 1, start_offSet - 1)
                if err != nil {
                    return err
                }
            }
            my.lexParser.ParseCharacters(start_offSet, end_offSet,monitor)
            var index int
            if end_offSet >= my.lexStream.GetStreamIndex(){
                index =my.lexStream.GetStreamIndex()
            }else{
                index = end_offSet + 1
            }
            my.AddEOF(prsStream,index)
            return nil
        }
        
        func (my *$action_type) Lexer(prsStream $prs_stream_class ,  monitor Monitor) error{
        
            err := my.InitializeLexer(prsStream, 0, -1)
            if err != nil {
                return err
            }
            my.lexParser.ParseCharactersWhitMonitor(monitor)
            my.AddEOF(prsStream, my.lexStream.GetStreamIndex())
            return nil
        }
    
        /**
         * If a parse stream was not passed to my Lexical analyser then we
         * simply report a lexical error. Otherwise, we produce a bad token.
         */
        func (my *$action_type)  ReportLexicalError(startLoc int, endLoc int) {
            var prs_stream = my.lexStream.GetIPrsStream()
            if prs_stream == nil{
                my.lexStream.ReportLexicalErrorPosition(startLoc, endLoc)
            }else {
                //
                // Remove any token that may have been processed that fall in the
                // range of the lexical error... then add one error token that spans
                // the error range.
                //
                var i int = prs_stream.GetSize() - 1
                for ; i > 0 ;i-- {
                    if prs_stream.GetStartOffset(i) >= startLoc {
                        prs_stream.RemoveLastToken()
                    } else {
                    break
                    }
                }
                prs_stream.MakeToken(startLoc, endLoc, 0) // add an error token to the prsStream
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
    ./
%End

--
-- E N D   O F   T E M P L A T E
--
