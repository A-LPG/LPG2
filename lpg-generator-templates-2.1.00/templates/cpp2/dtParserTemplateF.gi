--
-- In a parser using this template, the following macro may be redefined:
--
--     $additional_interfaces
--     $ast_class
--
-- B E G I N N I N G   O F   T E M P L A T E   dtParserTemplateF
--
%Options programming_language=rt_cpp,margin=4
%Options table,error_maps,scopes
%Options prefix=TK_
%Options action-block=("*.h", "/.", "./")
%options action-block=("*.cpp", "/!", "!/")
%options ast-block=("/!", "!/")
%Options ParseTable=ParseTable

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
    /!$Header$case $rule_number: {
                   //#line $next_line "$input_file$"!/

    $EndAction
    /!            break;
                }!/

    $BeginJava
    /!$Header$case $rule_number: {
                    $symbol_declarations
                    //#line $next_line "$input_file$"!/

    $EndJava /!$EndAction!/

    $NoAction
    /!$Header$case $rule_number:
                    break;!/

    $BadAction
    /!$Header$case $rule_number:
                    throw  std::exception("No action specified for rule " + $rule_number);!/

    $NullAction
    /!$Header$case $rule_number:
                    setResult(nullptr);
                    break;!/

    $BeginActions
    /!
         #include "$action_type.h"
         void $action_type::ruleAction(int ruleNumber)
        {
            switch (ruleNumber)
            {!/

    $SplitActions
    /!
                    default:
                        ruleAction$rule_number(ruleNumber);
                        break;
                }
                return;
            }
        
             void ruleAction$rule_number(int ruleNumber)
            {
                switch (ruleNumber)
                {
                    //#line $next_line "$input_file$"!/

    $EndActions
    /!
                default:
                    break;
            }
            return;
        }!/


    $entry_declarations
    /.
         $ast_class * parse$entry_name()
        {
            return parse$entry_name(nullptr, 0);
        }
            
         $ast_class * parse$entry_name(Monitor* monitor)
        {
            return parse$entry_name(monitor, 0);
        }
            
         $ast_class * parse$entry_name(int error_repair_count)
        {
            return parse$entry_name(nullptr, error_repair_count);
        }
            
         void resetParse$entry_name()
        {
            dtParser->resetParserEntry($sym_type::$entry_marker);
        }
        
         $ast_class * parse$entry_name(Monitor* monitor, int error_repair_count)
        {
            dtParser->setMonitor(monitor);
            
            try
            {
                return ($ast_class *) dtParser->parseEntry($sym_type::$entry_marker);
            }
            catch (BadParseException& e)
            {
                prsStream->reset(e.error_token); // point to error token

                std::shared_ptr< DiagnoseParser> diagnoseParser = std::make_shared<DiagnoseParser>(prsStream, prsTable);
                diagnoseParser->diagnoseEntry($sym_type::$entry_marker, e.error_token);
            }

            return nullptr;
        }
    ./
        
    $additional_interfaces /../
    $ast_class /.$ast_type./
    $super_class /.Object./
    $unimplemented_symbols_warning /.false./
    
    $setSym1 /. // macro setSym1 is deprecated. Use function setResult
                getParser()->setSym1./
    $setResult /. // macro setResult is deprecated. Use function setResult
                 getParser()->setSym1./
    $getSym /. // macro getSym is deprecated. Use function getRhsSym
              getParser()->getSym./
    $getToken /. // macro getToken is deprecated. Use function getRhsTokenIndex
                getParser()->getToken./
    $getIToken /. // macro getIToken is deprecated. Use function getRhsIToken
                 prsStream->getIToken./
    $getLeftSpan /. // macro getLeftSpan is deprecated. Use function getLeftSpan
                   getParser()->getFirstToken./
    $getRightSpan /. // macro getRightSpan is deprecated. Use function getRightSpan
                    getParser()->getLastToken./
%End

%Globals
    /.
    #pragma once

#include <iostream>
#include "AstPoolHolder.h"
#include "DeterministicParser.h"
#include "diagnose.h"
#include "ErrorToken.h"
#include "Exception.h"
#include "IAbstractArrayList.h"
#include "IAst.h"
#include "IAstVisitor.h"
#include "ILexStream.h"
#include "$sym_type.h"
#include "$prs_type.h"
#include "Object.h"
#include "ParseTable.h"
#include "PrsStream.h"
#include "RuleAction.h"
#include "IcuUtil.h"
#include "Any.h"
    ./
%End

%Headers
    /.
     struct $action_type :public $super_class ,public RuleAction$additional_interfaces
    {
      
         PrsStream* prsStream = nullptr;
        ~$action_type (){
            delete prsStream;
            delete dtParser;
        }
         bool unimplementedSymbolsWarning = $unimplemented_symbols_warning;

        inline static ParseTable* prsTable = new $prs_type();
         ParseTable* getParseTable() { return prsTable; }

         DeterministicParser* dtParser = nullptr;
         DeterministicParser* getParser() { return dtParser; }

         void setResult(Object* object) { dtParser->setSym1(object); }
         Object* getRhsSym(int i) { return dtParser->getSym(i); }

         int getRhsTokenIndex(int i) { return dtParser->getToken(i); }
         IToken* getRhsIToken(int i) { return prsStream->getIToken(getRhsTokenIndex(i)); }
        
         int getRhsFirstTokenIndex(int i) { return dtParser->getFirstToken(i); }
         IToken* getRhsFirstIToken(int i) { return prsStream->getIToken(getRhsFirstTokenIndex(i)); }

         int getRhsLastTokenIndex(int i) { return dtParser->getLastToken(i); }
         IToken* getRhsLastIToken(int i) { return prsStream->getIToken(getRhsLastTokenIndex(i)); }

         int getLeftSpan() { return dtParser->getFirstToken(); }
         IToken* getLeftIToken()  { return prsStream->getIToken(getLeftSpan()); }

         int getRightSpan() { return dtParser->getLastToken(); }
         IToken* getRightIToken() { return prsStream->getIToken(getRightSpan()); }

         int getRhsErrorTokenIndex(int i)
        {
            int index = dtParser->getToken(i);
            IToken* err = prsStream->getIToken(index);
            return (dynamic_cast<ErrorToken*>(err) ? index : 0);
        }
         ErrorToken* getRhsErrorIToken(int i)
        {
            int index = dtParser->getToken(i);
            IToken* err = prsStream->getIToken(index);
            return (ErrorToken*) (dynamic_cast<ErrorToken*>(err) ? err : nullptr);
        }

         void reset(ILexStream* lexStream)
        {
            delete prsStream;
            prsStream = new PrsStream(lexStream);
            dtParser->reset(prsStream);

            try
            {
                prsStream->remapTerminalSymbols(orderedTerminalSymbols(), prsTable->getEoftSymbol());
            }
            catch(NullExportedSymbolsException& e) {
            }
            catch(NullTerminalSymbolsException& e) {
            }
            catch(UnimplementedTerminalsException& e)
            {
                if (unimplementedSymbolsWarning) {
                    auto unimplemented_symbols = e.getSymbols();
                    std::cout <<"The Lexer will not scan the following token(s):";
                    for (int i = 0; i < unimplemented_symbols.size(); i++)
                    {
                        auto id =  unimplemented_symbols[i];
                        std::wcout << L"    " << $sym_type::orderedTerminalSymbols[id];               
                    }
                    std::cout << std::endl;
                }
            }
            catch(UndefinedEofSymbolException& e)
            {
                std::string str = "The Lexer does not implement the Eof symbol ";
                str += IcuUtil::ws2s($sym_type::orderedTerminalSymbols[prsTable->getEoftSymbol()]);
                throw  UndefinedEofSymbolException(str);
            }
        }
        
         $action_type(ILexStream* lexStream = nullptr)
        {
            try
            {
                dtParser = new DeterministicParser(prsStream, prsTable,  this);
            }
            catch (NotDeterministicParseTableException& e)
            {
                throw  ( NotDeterministicParseTableException
                                    ("Regenerate $prs_type.java with -NOBACKTRACK option"));
            }
            catch (BadParseSymFileException& e)
            {
                throw  (BadParseSymFileException("Bad Parser Symbol File -- $sym_type.cpp. Regenerate $prs_type.cpp"));
            }
			if(lexStream){
				reset(lexStream);
			}
        }

        
         int numTokenKinds() { return $sym_type::numTokenKinds; }
         std::vector< std::wstring> orderedTerminalSymbols() { return $sym_type::orderedTerminalSymbols; }
         std::wstring getTokenKindName(int kind) { return $sym_type::orderedTerminalSymbols[kind]; }            
         int getEOFTokenKind() { return prsTable->getEoftSymbol(); }
         IPrsStream* getIPrsStream() { return prsStream; }

        /**
         * @deprecated replaced by {@link #getIPrsStream()}
         *
         */
         PrsStream* getPrsStream() { return prsStream; }

        /**
         * @deprecated replaced by {@link #getIPrsStream()}
         *
         */
         PrsStream * getParseStream() { return prsStream; }

         $ast_class * parser()
        {
            return parser(nullptr, 0);
        }
            
         $ast_class * parser(Monitor* monitor)
        {
            return parser(monitor, 0);
        }
            
         $ast_class * parser(int error_repair_count)
        {
            return parser(nullptr, error_repair_count);
        }
            
         $ast_class *parser(Monitor* monitor, int error_repair_count)
        {
            dtParser->setMonitor(monitor);

            try
            {
                return ($ast_class *) dtParser->parse();
            }
            catch (BadParseException& e)
            {
                prsStream->reset(e.error_token); // point to error token

                std::shared_ptr< DiagnoseParser> diagnoseParser = std::make_shared<DiagnoseParser>(prsStream, prsTable);
                diagnoseParser->diagnose(e.error_token);
            }

            return nullptr;
        }
        void ruleAction(int ruleNumber);
        //
        // Additional entry points, if any
        //
        $entry_declarations
    ./

%End

%Rules
    /!$BeginActions!/
%End

%Trailers
    /.
    };
    ./

     /!
        $EndActions
    !/
%End


--
-- E N D   O F   T E M P L A T E
--
