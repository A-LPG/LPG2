--
-- An instance of this template must have a $Export section and the export_terminals option
--
-- Macros that may be redefined in an instance of this template
--
--     $eof_token
--     $additional_interfaces
--     $super_stream_class -- subclass com.ibm.lpg.Utf8LpgLexStream for getKind
--     $prs_stream_class -- use /.PrsStream./ if not subclassing
--
-- B E G I N N I N G   O F   T E M P L A T E   LexerTemplateD
--
%Options programming_language=rt_cpp,margin=4
%Options table
%options action-block=("*.h", "/.", "./")
%options ParseTable=lpg2/ParseTable
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
    $eof_token /.$_EOF_TOKEN./
    
    $additional_interfaces /../
    $super_stream_class /.$file_prefix$Utf8LpgLexStream./
    $prs_stream_class /.IPrsStream./
    $super_class /.Object./

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
    /.          break;
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
         void ruleAction( int ruleNumber)
        {
            switch(ruleNumber)
            {./

    $SplitActions
    /.
	            default:
	                ruleAction$rule_number(ruleNumber);
	                break;
	        }
	        return;
	    }
	
	     void ruleAction$rule_number(int ruleNumber)
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
	#pragma once  
    #include <iostream>
    #include "lpg2/IPrsStream.h"
    #include "lpg2/Object.h"
    #include "lpg2/ParseTable.h"
    #include "lpg2/RuleAction.h"
    #include "lpg2/stringex.h"
    #include "lpg2/Token.h"
    #include "$sym_type.h"
    #include "$prs_type.h"
    #include "$kw_lexer_class.h"
    #include "lpg2/LexParser.h"
    #include "lpg2/Utf8LpgLexStream.h"
    struct $action_type :public $super_class,  public RuleAction$additional_interfaces
    {
		struct  $super_stream_class;
         $super_stream_class *lexStream= nullptr;
        ~$action_type(){
            delete lexStream;
            delete lexParser;
        }
         inline  static ParseTable* prs = new $prs_type();
         ParseTable* getParseTable() { return prs; }

         LexParser* lexParser = new LexParser();
         LexParser* getParser() { return lexParser; }

         int getToken(int i) { return lexParser->getToken(i); }
         int getRhsFirstTokenIndex(int i) { return lexParser->getFirstToken(i); }
         int getRhsLastTokenIndex(int i) { return lexParser->getLastToken(i); }

         int getLeftSpan() { return lexParser->getToken(1); }
         int getRightSpan() { return lexParser->getLastToken(); }
  
         void resetKeywordLexer()
        {
            if (kwLexer == nullptr)
                  this->kwLexer = new $kw_lexer_class(lexStream->getInputBytes(), $_IDENTIFIER);
            else this->kwLexer->setInput(lexStream->getInputBytes());
        }
  
         void reset(const std::wstring&  filename, int tab) 
        {
			delete lexStream;
            lexStream = new $super_stream_class(filename, tab);
            lexParser->reset((ILexStream*) lexStream, prs,this);
            resetKeywordLexer();
        }

         void reset(shared_ptr_string input_bytes, const std::wstring& filename)
        {
            reset(input_bytes, filename, 1);
        }
        
         void reset(shared_ptr_string input_bytes, const std::wstring& filename, int tab)
        {
            lexStream = new $super_stream_class(input_bytes, filename, tab);
            lexParser->reset((ILexStream*) lexStream, prs,  this);
            resetKeywordLexer();
        }
        
         $action_type(const std::wstring& filename, int tab) 
        {
            reset(filename, tab);
        }

         $action_type(shared_ptr_string input_bytes, const std::wstring&  filename, int tab)
        {
            reset(input_bytes, filename, tab);
        }

         $action_type(shared_ptr_string input_bytes, const std::wstring&  filename)
        {
            reset(input_bytes, filename, 1);
        }

         $action_type() {}

         ILexStream* getILexStream() { return lexStream; }

        /**
         * @deprecated replaced by {@link #getILexStream()}
         */
         ILexStream* getLexStream() { return lexStream; }

         void initializeLexer($prs_stream_class * prsStream, int start_offset, int end_offset)
        {
            if (lexStream->getInputBytes().size() == 0)
                throw  std::runtime_error("LexStream was not initialized");
            lexStream->setPrsStream(prsStream);
            prsStream->makeToken(start_offset, end_offset, 0); // Token list must start with a bad token
        }

         void lexer($prs_stream_class* prsStream)
        {
            lexer(nullptr, prsStream);
        }
        
         void lexer(Monitor* monitor, $prs_stream_class* prsStream)
        {
            if (lexStream->getInputBytes().size() == 0)
                  throw  std::runtime_error("lexStream was not initialized");

            lexStream->setPrsStream(prsStream);

            prsStream->makeToken(0, 0, 0); // Token list must start with a bad token
                
            lexParser->parseCharacters(monitor);  // Lex the input characters
                
            int i = lexStream->getStreamIndex();
            prsStream->makeToken(i, i, $eof_token); // and end with the end of file token
            prsStream->setStreamLength(prsStream->getSize());
                
            return;
        }
    ./
%End

%Rules
    /.$BeginActions./
%End

%Trailers
    /.
        $EndActions
    };
    ./
%End

--
-- E N D   O F   T E M P L A T E
--
