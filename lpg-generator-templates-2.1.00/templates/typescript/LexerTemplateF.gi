--
-- An instance of this template must have a %Export section and the export_terminals option
--
-- Macros that may be redefined in an instance of this template
--
--     %eof_token
--     %additional_interfaces
--     %super_stream_class -- subclass com.ibm.lpg.LpgLexStream for getKind
--     %prs_stream_class -- use /.PrsStream./ if not subclassing
--     %super_class
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
    $super_class /.any./

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
        public void ruleAction(ruleNumber : number )
        {
            switch(ruleNumber)
            {./

    $SplitActions
    /.
                    default:
                        ruleAction%rule_number(ruleNumber);
                        break;
                }
                return;
            }

            public void ruleAction%rule_number(ruleNumber : number )
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
    
    ./
%End

%Headers
    /.
    public class %action_type extends %super_class implements RuleAction%additional_interfaces
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
            if (kwLexer == null)
                  this.kwLexer = new %kw_lexer_class(lexStream.getInputChars(), %_IDENTIFIER);
            else this.kwLexer.setInputChars(lexStream.getInputChars());
        }
  
      
        
        public void reset( filename :string, number tab=1,input_chars? : string)
        {
            lexStream = new %super_stream_class(input_chars, filename, tab);
            this.lexParser.reset((ILexStream) lexStream, prs, (RuleAction) this);
            resetKeywordLexer();
        }
        
       

        constructor( string filename,  tab : number =  1 ,input_chars? : string)
        {
            reset(filename,tab,input_chars);
        }

       

        public  getILexStream()  : ILexStream{ return lexStream; }

        /**
         * @deprecated replaced by {@link #getILexStream()}
         */
        public  getLexStream()  : ILexStream{ return lexStream; }

        private void initializeLexer(%prs_stream_class this.prsStream, number start_offset, number end_offset)
        {
            if (lexStream.getInputChars() == null)
                throw new NullPointerException("LexStream was not initialized");
            lexStream.setPrsStream(this.prsStream);
            this.prsStream.makeToken(start_offset, end_offset, 0); // Token list must start with a bad token
        }

        private void addEOF(prsStream : %prs_stream_class, end_offset : number )
        {
            this.prsStream.makeToken(end_offset, end_offset, %eof_token); // and end with the end of file token
            this.prsStream.setStreamLength(this.prsStream.getSize());
        }

        public void lexer(prsStream: %prs_stream_class , start_offset : number = 0, end_offset : number = -1, monitor : Monitor = null)
        {
            if (start_offset <= 1)
                 initializeLexer(this.prsStream, 0, -1);
            else initializeLexer(this.prsStream, start_offset - 1, start_offset - 1);

            this.lexParser.parseCharacters(monitor, start_offset, end_offset);

            addEOF(this.prsStream, (end_offset >= lexStream.getStreamIndex() ? lexStream.getStreamIndex() : end_offset + 1));
        }
        
       

        /**
         * If a parse stream was not passed to this Lexical analyser then we
         * simply report a lexical error. Otherwise, we produce a bad token.
         */
        public void reportLexicalError( startLoc : number,  endLoc : number) {
            let prs_stream = lexStream.getIPrsStream();
            if (prs_stream == null)
                lexStream.reportLexicalError(startLoc, endLoc);
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
