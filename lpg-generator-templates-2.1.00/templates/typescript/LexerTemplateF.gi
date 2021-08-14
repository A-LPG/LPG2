--
-- An instance of this template must have a %Export section and the export_terminals option
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
%Options programming_Language=typescript,margin=4
%Options table
%options action-block=("*.ts", "/.", "./")
%options ParseTable=ParseTable
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
    $eof_token /.%_EOF_TOKEN./
    
    $additional_interfaces /../
    $super_stream_class /.%file_prefix%LpgLexStream./
    $prs_stream_class /.IPrsStream./
    $super_class /.Object./

    $prs_stream /. // macro prs_stream is deprecated. Use function getPrsStream
                  this.getPrsStream()./
    $setSym1 /. // macro setSym1 is deprecated. Use function setResult
                this.this.lexParser.setSym1./
    $setResult /. // macro setResult is deprecated. Use function setResult
                  this.this.lexParser.setSym1./
    $getSym /. // macro getSym is deprecated. Use function getLastToken
               this.this.lexParser.getSym./
    $getToken /. // macro getToken is deprecated. Use function getToken
                 this.this.lexParser.getToken./
    $getLeftSpan /. // macro getLeftSpan is deprecated. Use function getLeftSpan
                    this.this.lexParser.getFirstToken./
    $getRightSpan /. // macro getRightSpan is deprecated. Use function getRightSpan
                     this.this.lexParser.getLastToken./

    --
    -- Macros useful for specifying actions
    --
    $Header
    /.
                //
                // Rule %rule_number:  %rule_text
                //
                ./

    $DefaultAction
    /.%Header%case %rule_number: { ./

    $BeginAction /.%DefaultAction./

    $EndAction
    /.            break;
                }./

    $BeginJava
    /.%BeginAction
                %symbol_declarations./

    $EndJava /.%EndAction./

    $NoAction
    /.%Header%case %rule_number:
                    break; ./

    $BeginActions
    /.
        public  ruleAction(ruleNumber : number ) : void
        {
            switch(ruleNumber)
            {./

    $SplitActions
    /.
                    default:
                        this.ruleAction%rule_number(ruleNumber);
                        break;
                }
                return;
            }

            public  ruleAction%rule_number(ruleNumber : number ) : void
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
  import { %prs_type } from ".\/%prs_type";
  import { %sym_type } from ".\/%sym_type";
  import { %kw_lexer_class } from ".\/%kw_lexer_class";
    ./
%End

%Headers
    /.
    export class %action_type extends %super_class implements RuleAction%additional_interfaces
    {
        private lexStream: %super_stream_class ;
        
        private static  prs : ParseTable = new %prs_type();
        public  getParseTable() : ParseTable{ return %action_type.prs; }

        private  lexParser  : LexParser= new LexParser();
        public  getParser()  : LexParser{ return this.lexParser; }

        public  getToken(i : number)  : number{ return this.lexParser.getToken(i); }
        public  getRhsFirstTokenIndex(i : number) : number{ return this.lexParser.getFirstToken(i); }
        public  getRhsLastTokenIndex(i : number)  : number{ return this.lexParser.getLastToken(i); }

        public getLeftSpan() : number{ return this.lexParser.getToken(1); }
        public getRightSpan() : number { return this.lexParser.getLastToken(); }
  
        public  resetKeywordLexer() : void
        {
            if (!this.kwLexer)
                  this.kwLexer = new %kw_lexer_class(this.lexStream.getInputChars(), %_IDENTIFIER);
             this.kwLexer.setInputChars(this.lexStream.getInputChars());
        }
  
      
        
        public  reset( filename : string,  tab : number = 4, input_chars? : string) : void
        {
            this.lexStream = new %super_stream_class(filename,input_chars, tab);
            this.lexParser.reset(<ILexStream>  this.lexStream, %action_type.prs, <RuleAction> this);
            this.resetKeywordLexer();
        }
        
       

        constructor( filename : string,  tab : number =  4 ,input_chars? : string)
        {
            super();
            this.lexStream = new %super_stream_class(filename,input_chars, tab);
            this.lexParser.reset(<ILexStream>  this.lexStream, %action_type.prs, <RuleAction> this);
            this.resetKeywordLexer();
        }

       

        public  getILexStream()  : ILexStream{ return  this.lexStream; }

        /**
         * @deprecated replaced by {@link #getILexStream()}
         */
        public  getLexStream()  : ILexStream{ return  this.lexStream; }

        private initializeLexer(prsStream : %prs_stream_class ,  start_offset : number, end_offset : number) : void 
        {
            if (this.lexStream.getInputChars() == null)
                throw new ReferenceError("LexStream was not initialized");
            this.lexStream.setPrsStream(prsStream);
            prsStream.makeToken(start_offset, end_offset, 0); // Token list must start with a bad token
        }

        private  addEOF(prsStream : %prs_stream_class, end_offset : number ) : void
        {
            prsStream.makeToken(end_offset, end_offset, %eof_token); // and end with the end of file token
            prsStream.setStreamLength(prsStream.getSize());
        }

        public  lexerWithPosition(prsStream: %prs_stream_class , start_offset : number , end_offset : number, monitor? : Monitor) : void
        {
            if (start_offset <= 1)
                 this.initializeLexer(prsStream, 0, -1);
            else this.initializeLexer(prsStream, start_offset - 1, start_offset - 1);

            this.lexParser.parseCharacters(start_offset, end_offset,monitor);

            this.addEOF(prsStream, (end_offset >= this.lexStream.getStreamIndex() ? this.lexStream.getStreamIndex() : end_offset + 1));
        }

        public  lexer(prsStream: %prs_stream_class ,  monitor? : Monitor) : void
        {
           
            this.initializeLexer(prsStream, 0, -1);
            this.lexParser.parseCharactersWhitMonitor(monitor);
            this.addEOF(prsStream, this.lexStream.getStreamIndex());
        }
       

        /**
         * If a parse stream was not passed to this Lexical analyser then we
         * simply report a lexical error. Otherwise, we produce a bad token.
         */
        public  reportLexicalError( startLoc : number,  endLoc : number) : void {
            let prs_stream = this.lexStream.getIPrsStream();
            if (!prs_stream)
                this.lexStream.reportLexicalError(startLoc, endLoc);
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
                prs_stream.makeToken(startLoc, endLoc, 0); // add an error token to the this.prsStream
            }        
        }
    ./
%End

%Rules
    /.%BeginActions./
%End

%Trailers
    /.
        %EndActions
    }
    ./
%End

--
-- E N D   O F   T E M P L A T E
--
