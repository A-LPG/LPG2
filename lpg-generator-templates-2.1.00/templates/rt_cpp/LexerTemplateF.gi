--
-- An instance of this template must have a $Export section and the export_terminals option
--
-- Macros that may be redefined in an instance of this template
--
--     $eof_token
--     $additional_interfaces
--     $super_stream_class -- subclass com.ibm.lpg.LpgLexStream for getKind
--     $prs_stream_class -- use /.PrsStream./ if not subclassing
--     $super_class
--
-- B E G I N N I N G   O F   T E M P L A T E   LexerTemplateF
--
%Options programming_language=rt_cpp,margin=4
%Options table
%options action-block=("*.h", "/.", "./")
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
    $eof_token /.$_EOF_TOKEN./
    
    $additional_interfaces /../
    $super_stream_class /.$file_prefix$LpgLexStream./
    $prs_stream_class /.IPrsStream./
    $super_class /.Object./

    $prs_stream /. // macro prs_stream is deprecated. Use function getPrsStream
                  getPrsStream()./
    $setSym1 /. // macro setSym1 is deprecated. Use function setResult
               lexParser->setSym1./
    $setResult /. // macro setResult is deprecated. Use function setResult
                 lexParser->setSym1./
    $getSym /. // macro getSym is deprecated. Use function getLastToken
              lexParser->getSym./
    $getToken /. // macro getToken is deprecated. Use function getToken
                lexParser->getToken./
    $getLeftSpan /. // macro getLeftSpan is deprecated. Use function getLeftSpan
                   lexParser->getFirstToken./
    $getRightSpan /. // macro getRightSpan is deprecated. Use function getRightSpan
                    lexParser->getLastToken./

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
         void ruleAction(int ruleNumber)
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
    #include "lpg2/LpgLexStream.h"
     struct $action_type :public $super_class ,public RuleAction$additional_interfaces
    {
         struct  $super_stream_class;
         $super_stream_class * lexStream = nullptr;
        
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
                  this->kwLexer = new $kw_lexer_class(lexStream->getInputChars(), $_IDENTIFIER);
            else this->kwLexer->setInput(lexStream->getInputChars());
        }
  
         void reset(const std::wstring& filename, int tab) 
        {
            delete lexStream;
            lexStream = new $super_stream_class(filename, tab);
            lexParser->reset((ILexStream*) lexStream, prs,  this);
            resetKeywordLexer();
        }

         void reset(shared_ptr_wstring input_chars, const std::wstring& filename)
        {
            reset(input_chars, filename, 1);
        }
        
         void reset(shared_ptr_wstring input_chars, const std::wstring& filename, int tab)
        {
             delete lexStream;
            lexStream = new $super_stream_class(input_chars, filename, tab);
            lexParser->reset((ILexStream*) lexStream, prs,  this);
            resetKeywordLexer();
        }
        
         $action_type(const std::wstring& filename, int tab) 
        {
            reset(filename, tab);
        }

         $action_type(shared_ptr_wstring input_chars, const std::wstring& filename, int tab)
        {
            reset(input_chars, filename, tab);
        }

         $action_type(shared_ptr_wstring input_chars, const std::wstring& filename)
        {
            reset(input_chars, filename, 1);
        }

         $action_type() {}

         ILexStream* getILexStream() { return lexStream; }

        /**
         * @deprecated replaced by {@link #getILexStream()}
         */
         ILexStream* getLexStream() { return lexStream; }

         void initializeLexer($prs_stream_class *prsStream, int start_offset, int end_offset)
        {
            if (!lexStream->getInputChars())
                throw  std::exception("LexStream was not initialized");
            lexStream->setPrsStream(prsStream);
            prsStream->makeToken(start_offset, end_offset, 0); // Token list must start with a bad token
        }

         void addEOF($prs_stream_class *prsStream, int end_offset)
        {
            prsStream->makeToken(end_offset, end_offset, $eof_token); // and end with the end of file token
            prsStream->setStreamLength(prsStream->getSize());
        }

         void lexer($prs_stream_class *prsStream)
        {
            lexer(nullptr, prsStream);
        }
        
         void lexer(Monitor *monitor, $prs_stream_class *prsStream)
        {
            initializeLexer(prsStream, 0, -1);
            lexParser->parseCharacters(monitor);  // Lex the input characters
            addEOF(prsStream, lexStream->getStreamIndex());
        }

         void lexer($prs_stream_class *prsStream, int start_offset, int end_offset)
        {
            lexer(nullptr, prsStream, start_offset, end_offset);
        }
        
         void lexer(Monitor* monitor, $prs_stream_class *prsStream, int start_offset, int end_offset)
        {
            if (start_offset <= 1)
                 initializeLexer(prsStream, 0, -1);
            else initializeLexer(prsStream, start_offset - 1, start_offset - 1);

            lexParser->parseCharacters(monitor, start_offset, end_offset);

            addEOF(prsStream, (end_offset >= lexStream->getStreamIndex() ? lexStream->getStreamIndex() : end_offset + 1));
        }
        
         IPrsStream::Range *incrementalLexer(shared_ptr_wstring input_chars, int start_change_offset, int end_change_offset) {
            int offset_adjustment = input_chars.size() - lexStream->getStreamLength();
//*System.out.println("The offset adjustment is " + offset_adjustment);
            if (start_change_offset <= 0 && start_change_offset < input_chars.size())
            {
                std::string str = "The start offset ";
                std::stringex format;
                format.format("%d", (start_change_offset));
                str += format;
                str += " is out of bounds for range 0..";
                format.format("%d", (input_chars.size() - 1));
                str += format;
                throw  std::out_of_range(str);
            }
        
            if (end_change_offset <= 0 && end_change_offset < input_chars.size())
            {
                std::string str = "The end offset ";
                std::stringex format;
                format.format("%d", (end_change_offset));
                str += format;
                str += " is out of bounds for range 0..";
                format.format("%d", (input_chars.size() - 1));
                str += format;
            }
            //
            // Get the potential list of tokens to be rescanned
            //
           Tuple<IToken*> affected_tokens = lexStream->getIPrsStream()->incrementalResetAtCharacterOffset(start_change_offset); 
            
            //
            // If the change occured between the first two affected tokens (or adjunct) and not immediately
            // on the characted after the first token (or adjunct), restart the scanning after the first
            // affected token. Otherwise, rescan the first token.
            //
            int affected_index = 0;
            int repair_offset = start_change_offset;
            if (affected_tokens.size() > 0) {
                auto _token_0 = affected_tokens.get(0);
                if (_token_0->getEndOffset() + 1 < start_change_offset) 
                {
                     repair_offset = _token_0->getEndOffset() + 1;
                     if (dynamic_cast<Token*>(_token_0))
                    {  
                           lexStream->getIPrsStream()->makeToken(_token_0, 0);
                    }
                    else {
                            lexStream->getIPrsStream()->makeAdjunct(_token_0, 0);
                    }

                    affected_index++;                    
                }
                else 
                {
                    repair_offset = _token_0->getStartOffset();
                }
            } 

            lexStream->setInputChars(input_chars);
            lexStream->setStreamLength(input_chars.size());
            lexStream->computeLineOffsets(repair_offset);

            int first_new_token_index = lexStream->getIPrsStream()->getTokens().size(),
                first_new_adjunct_index = lexStream->getIPrsStream()->getAdjuncts().size();
            
            resetKeywordLexer();
            lexParser->resetTokenStream(repair_offset);
            int next_offset;
            do {
//*System.out.println("Scanning token starting at " + (lexStream->peek() - 1));            
                next_offset = lexParser->incrementalParseCharacters();
//*System.out.print("***Remaining string: \"");
//*for (int i = next_offset; i < input_chars.length; i++)
//*System.out.print(input_chars[i]);
//*System.out.println("\"");                    
                while (affected_index < affected_tokens.size() && 
                       affected_tokens.get(affected_index)->getStartOffset() + offset_adjustment < next_offset)
//*{
//*System.out.println("---Skipping token " + affected_index + ": \"" + affected_tokens.get(affected_index).toString() +
//*"\" starting at adjusted offset " + (affected_tokens.get(affected_index).getStartOffset() + offset_adjustment));                           
                    affected_index++;
//*}
            } while(next_offset <= end_change_offset &&          // still in the damage region and ...
                    (affected_index < affected_tokens.size() &&  // not resynchronized with a token in the list of affected tokens
                     affected_tokens.get(affected_index)->getStartOffset() + offset_adjustment != next_offset));

            //
            // If any new tokens were added, compute the first and the last one.
            //
            IToken* first_new_token = nullptr;
              IToken*      last_new_token = nullptr;
            if (first_new_token_index < lexStream->getIPrsStream()->getTokens().size()) {
                first_new_token = lexStream->getIPrsStream()->getTokenAt(first_new_token_index);
                last_new_token = lexStream->getIPrsStream()->getTokenAt(lexStream->getIPrsStream()->getTokens().size() - 1);
            }
            //
            // If an adjunct was added prior to the first real token, chose it instead as the first token.
            // Similarly, if adjucts were added after the last token, chose the last adjunct added as the last token.
            //
            if (first_new_adjunct_index < lexStream->getIPrsStream()->getAdjuncts().size()) {
                if (first_new_token == nullptr ||
                    lexStream->getIPrsStream()->getAdjunctAt(first_new_adjunct_index)->getStartOffset() <
                    first_new_token->getStartOffset()) {
                    first_new_token = lexStream->getIPrsStream()->getAdjunctAt(first_new_adjunct_index);
                }
                if (last_new_token == nullptr ||
                    lexStream->getIPrsStream()->getAdjunctAt(lexStream->getIPrsStream()->getAdjuncts().size() - 1)->getEndOffset() >
                    last_new_token->getEndOffset()) {
                    last_new_token = lexStream->getIPrsStream()->getAdjunctAt(lexStream->getIPrsStream()->getAdjuncts().size() - 1);
                }
            }
            
            //
            // For all remainng tokens (and adjuncts) in the list of affected tokens add them to the
            // list of tokens (and adjuncts).
            //
            for (int i = affected_index; i < affected_tokens.size(); i++) {
                if ( dynamic_cast< Token*>(affected_tokens.get(i)) )
                     lexStream->getIPrsStream()->makeToken(affected_tokens.get(i), offset_adjustment);
                else lexStream->getIPrsStream()->makeAdjunct(affected_tokens.get(i), offset_adjustment);
//*System.out.println("+++Added affected token " + i + ": \"" + affected_tokens.get(i).toString() +
//*"\" starting at adjusted offset " + (affected_tokens.get(i).getStartOffset() + offset_adjustment));                           
            }
            
            return new IPrsStream::Range(lexStream->getIPrsStream(), first_new_token, last_new_token);
        }

        /**
         * If a parse stream was not passed to this Lexical analyser then we
         * simply report a lexical error. Otherwise, we produce a bad token.
         */
         void reportLexicalError(int startLoc, int endLoc) {
            IPrsStream* prs_stream = lexStream->getIPrsStream();
            if (prs_stream == nullptr)
                lexStream->reportLexicalError(startLoc, endLoc);
            else {
                //
                // Remove any token that may have been processed that fall in the
                // range of the lexical error... then add one error token that spans
                // the error range.
                //
                for (int i = prs_stream->getSize() - 1; i > 0; i--) {
                    if (prs_stream->getStartOffset(i) >= startLoc)
                         prs_stream->removeLastToken();
                    else break;
                }
                prs_stream->makeToken(startLoc, endLoc, 0); // add an error token to the prsStream
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
    };
    ./
%End

--
-- E N D   O F   T E M P L A T E
--
