--
-- An instance of this template must have a $Export section and the export_terminals option
--
-- Macros that may be redefined in an instance of this template
--
--     $eof_token
--     $additional_interfaces
--     $super_stream_class -- subclass LpgLexStream for getKind
--     $prs_stream_class -- use /.PrsStream./ if not subclassing
--     $super_class
--
-- B E G I N N I N G   O F   T E M P L A T E   LexerTemplateF
--
%options programming_Language=python3,margin=4
%options table
%options action-block=("*.py", "/.", "./")
%options ParseTable=ParseTable
%options prefix=Char_

--
-- This template requires that the name of the EOF token be set
-- to EOF and that the prefix be "Char_" to be consistent with
-- KeywordTemplateD.
--
$Eof
    EOF
%End

--
-- This template also requires that the name of the parser EOF
-- Token to be exported be set to EOF_TOKEN
--
$Export
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
    $super_class /.object./

    $prs_stream /. // macro prs_stream is deprecated. Use function getPrsStream
                  self.getPrsStream()./
    $setSym1 /. // macro setSym1 is deprecated. Use function setResult
                self.self.lexParser.setSym1./
    $setResult /. // macro setResult is deprecated. Use function setResult
                  self.self.lexParser.setSym1./
    $getSym /. // macro getSym is deprecated. Use function getLastToken
               self.self.lexParser.getSym./
    $getToken /. // macro getToken is deprecated. Use function getToken
                 self.self.lexParser.getToken./
    $getLeftSpan /. // macro getLeftSpan is deprecated. Use function getLeftSpan
                    self.self.lexParser.getFirstToken./
    $getRightSpan /. // macro getRightSpan is deprecated. Use function getRightSpan
                     self.self.lexParser.getLastToken./

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
          ruleAction(ruleNumber : number ) : void
        {
            switch(ruleNumber)
            {./

    $SplitActions
    /.
                    default:
                        self.ruleAction$rule_number(ruleNumber);
                        break;
                }
                return;
            }

              ruleAction$rule_number(ruleNumber : number ) : void
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
    /.
  import { RuleAction, ParseTable, LexParser, ILexStream, IPrsStream, Monitor,LpgLexStream} from "lpg2ts";
  import { $prs_type } from ".\/$prs_type";
  import { $sym_type } from ".\/$sym_type";
  import { $kw_lexer_class } from ".\/$kw_lexer_class";
    ./
%End

%Headers
    /.
    class $action_type extends $super_class implements RuleAction$additional_interfaces
    {
         lexStream: $super_stream_class ;
        
         static  prs : ParseTable =  $prs_type();
          getParseTable() : ParseTable{ return $action_type.prs; }

          lexParser  : LexParser=  LexParser();
          getParser()  : LexParser{ return self.lexParser; }

          getToken(i : number)  : number{ return self.lexParser.getToken(i); }
          getRhsFirstTokenIndex(i : number) : number{ return self.lexParser.getFirstToken(i); }
          getRhsLastTokenIndex(i : number)  : number{ return self.lexParser.getLastToken(i); }

         getLeftSpan() : number{ return self.lexParser.getToken(1); }
         getRightSpan() : number { return self.lexParser.getLastToken(); }
  
          resetKeywordLexer() : void
        {
            if (!self.kwLexer)
                  self.kwLexer =  $kw_lexer_class(self.lexStream.getInputChars(), $_IDENTIFIER);
             self.kwLexer.setInputChars(self.lexStream.getInputChars());
        }
  
      
        
          reset( filename : string,  tab : number = 4, input_chars? : string) : void
        {
            self.lexStream =  $super_stream_class(filename,input_chars, tab);
            self.lexParser.reset(<ILexStream>  self.lexStream, $action_type.prs, <RuleAction> this);
            self.resetKeywordLexer();
        }
        
       

        def __init__(self, filename : string,  tab : number =  4 ,input_chars? : string)
        {
            super();
            self.lexStream =  $super_stream_class(filename,input_chars, tab);
            self.lexParser.reset(<ILexStream>  self.lexStream, $action_type.prs, <RuleAction> this);
            self.resetKeywordLexer();
        }

       

          getILexStream()  : ILexStream{ return  self.lexStream; }

        /**
         * @deprecated replaced by {@link #getILexStream()}
         */
          getLexStream()  : ILexStream{ return  self.lexStream; }

         initializeLexer(prsStream : $prs_stream_class ,  start_offset : number, end_offset : number) : void 
        {
            if (self.lexStream.getInputChars() == null)
                raise  ReferenceError("LexStream was not initialized");
            self.lexStream.setPrsStream(prsStream);
            prsStream.makeToken(start_offset, end_offset, 0); // Token list must start with a bad token
        }

          addEOF(prsStream : $prs_stream_class, end_offset : number ) : void
        {
            prsStream.makeToken(end_offset, end_offset, $eof_token); // and end with the end of file token
            prsStream.setStreamLength(prsStream.getSize());
        }

          lexerWithPosition(prsStream: $prs_stream_class , start_offset : number , end_offset : number, monitor? : Monitor) : void
        {
            if (start_offset <= 1)
                 self.initializeLexer(prsStream, 0, -1);
            else self.initializeLexer(prsStream, start_offset - 1, start_offset - 1);

            self.lexParser.parseCharacters(start_offset, end_offset,monitor);

            self.addEOF(prsStream, (end_offset >= self.lexStream.getStreamIndex() ? self.lexStream.getStreamIndex() : end_offset + 1));
        }

          lexer(prsStream: $prs_stream_class ,  monitor? : Monitor) : void
        {
           
            self.initializeLexer(prsStream, 0, -1);
            self.lexParser.parseCharactersWhitMonitor(monitor);
            self.addEOF(prsStream, self.lexStream.getStreamIndex());
        }
       

        /**
         * If a parse stream was not passed to this Lexical analyser then we
         * simply report a lexical error. Otherwise, we produce a bad token.
         */
          reportLexicalError( startLoc : number,  endLoc : number) : void {
            let prs_stream = self.lexStream.getIPrsStream();
            if (!prs_stream)
                self.lexStream.reportLexicalError(startLoc, endLoc);
            else {
                //
                // Remove any token that may have been processed that fall in the
                // range of the lexical error... then add one error token that spans
                // the error range.
                //
                for (let i : number = prs_stream.getSize() - 1; i > 0; i--) {
                    if (prs_stream.getStartOffset(i) >= startLoc)
                         prs_stream.removeLastToken();
                    else break;
                }
                prs_stream.makeToken(startLoc, endLoc, 0); // add an error token to the self.prsStream
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
