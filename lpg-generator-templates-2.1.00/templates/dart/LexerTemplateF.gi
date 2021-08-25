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
%Options programming_Language=dart,margin=4
%Options table
%options action-block=("*.dart", "/.", "./")
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
                  getPrsStream()./
    $setSym1 /. // macro setSym1 is deprecated. Use function setResult
                lexParser.setSym1./
    $setResult /. // macro setResult is deprecated. Use function setResult
                  lexParser.setSym1./
    $getSym /. // macro getSym is deprecated. Use function getLastToken
               lexParser.getSym./
    $getToken /. // macro getToken is deprecated. Use function getToken
                 lexParser.getToken./
    $getLeftSpan /. // macro getLeftSpan is deprecated. Use function getLeftSpan
                    lexParser.getFirstToken./
    $getRightSpan /. // macro getRightSpan is deprecated. Use function getRightSpan
                     lexParser.getLastToken./

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
        void ruleAction(int ruleNumber  )   
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

              ruleAction%rule_number(int ruleNumber  )   void
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
import 'package:lpg2/lpg2.dart';
import '%prs_type.dart';
import '%sym_type.dart';
import '%kw_lexer_class.dart';
import 'dart:io';
    ./
%End

%Headers
    /.
    class %action_type extends %super_class implements RuleAction%additional_interfaces
    {
        late %super_stream_class lexStream ;
        
        static  ParseTable prs = %prs_type();
        ParseTable  getParseTable() { return %action_type.prs; }

        LexParser  lexParser = LexParser();
        LexParser  getParser()  { return lexParser; }

        int getToken(int i)   { return lexParser.getToken(i); }
        int  getRhsFirstTokenIndex(int i) { return lexParser.getFirstToken(i); }
        int  getRhsLastTokenIndex(int i) { return lexParser.getLastToken(i); }

        int getLeftSpan()  { return lexParser.getToken(1); }
        int getRightSpan()   { return lexParser.getLastToken(); }
  
        void  resetKeywordLexer()   
        {
            if (null ==kwLexer)
                  kwLexer = %kw_lexer_class(lexStream.getInputChars(), %_IDENTIFIER);
             kwLexer!.setInputChars(lexStream.getInputChars());
        }
  
      
        
        void  reset(String filename , [int tab=4, String?input_chars])
        {
            lexStream = %super_stream_class(filename,input_chars, tab);
            lexParser.reset(lexStream, %action_type.prs,  this);
            resetKeywordLexer();
        }
        
       
        %action_type(String filename , [int tab=4, String?input_chars])
        {
            lexStream = %super_stream_class(filename,input_chars, tab);
            lexParser.reset(lexStream, %action_type.prs,  this);
            resetKeywordLexer();
        }

       

        ILexStream getILexStream(){ return  lexStream; }

        void initializeLexer(%prs_stream_class prsStream , int start_offset, int end_offset)    
        {
            if (lexStream.getInputChars() == null)
                throw NullPointerException("LexStream was not initialized");
            lexStream.setPrsStream(prsStream);
            prsStream.makeToken(start_offset, end_offset, 0); // Token list must start with a bad token
        }

        void  addEOF( %prs_stream_class prsStream , int end_offset)   
        {
            prsStream.makeToken(end_offset, end_offset, %eof_token); // and end with the end of file token
            prsStream.setStreamLength(prsStream.getSize());
        }

        void  lexerWithPosition(%prs_stream_class prsStream, int start_offset , int end_offset , [Monitor? monitor])   
        {
            if (start_offset <= 1)
                 initializeLexer(prsStream, 0, -1);
            else initializeLexer(prsStream, start_offset - 1, start_offset - 1);

            lexParser.parseCharacters(start_offset, end_offset,monitor);

            addEOF(prsStream, (end_offset >= lexStream.getStreamIndex() ? lexStream.getStreamIndex() : end_offset + 1));
        }

        void lexer(%prs_stream_class prsStream,  [Monitor? monitor])   
        {
           
            initializeLexer(prsStream, 0, -1);
            lexParser.parseCharactersWhitMonitor(monitor);
            addEOF(prsStream, lexStream.getStreamIndex());
        }
       

        /**
         * If a parse stream was not passed to this Lexical analyser then we
         * simply report a lexical error. Otherwise, we produce a bad token.
         */
        void  reportLexicalError(int startLoc, int endLoc)    {
            var prs_stream = lexStream.getIPrsStream();
            if (null ==prs_stream)
                lexStream.reportLexicalError(startLoc, endLoc);
            else {
                //
                // Remove any token that may have been processed that fall in the
                // range of the lexical error... then add one error token that spans
                // the error range.
                //
                for (var i = prs_stream.getSize() - 1; i > 0; i--) {
                    if (prs_stream.getStartOffset(i) >= startLoc)
                         prs_stream.removeLastToken();
                    else break;
                }
                prs_stream.makeToken(startLoc, endLoc, 0); // add an error token to the prsStream
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
