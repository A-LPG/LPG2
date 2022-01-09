--
-- In a parser using this template, the following macro may be redefined:
--
--     $additional_interfaces
--     $ast_class
--
-- B E G I N N I N G   O F   T E M P L A T E   btParserTemplateF
--
%Options programming_language=rt_cpp,margin=4,backtrack
%Options table,error_maps,scopes
%options prefix=TK_
%options action-block=("*.h", "/.", "./")
%options action-block=("*.cpp", "/!", "!/")
%options ast-block=("/!", "!/")
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
                    throw  std::runtime_error("No action specified for rule " + $rule_number);!/

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
            
         $ast_class * parse$entry_name(Monitor *monitor, int error_repair_count)
        {
            btParser->setMonitor(monitor);
            
            try
            {
                return ($ast_class *) btParser->fuzzyParseEntry($sym_type::$entry_marker, error_repair_count);
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

    --
    -- Macros that may be needed in a parser using this template
    --
    $additional_interfaces /../
    $ast_class /.$ast_type./
    $super_class /.Object./   
    $unimplemented_symbols_warning /.false./

    --
    -- Old deprecated macros that should NEVER be used.
    --
    $setSym1 /. // macro setSym1 is deprecated. Use function setResult
                getParser().setSym1./
    $setResult /. // macro setResult is deprecated. Use function setResult
                 getParser().setSym1./
    $getSym /. // macro getSym is deprecated. Use function getRhsSym
              getParser().getSym./
    $getToken /. // macro getToken is deprecated. Use function getRhsTokenIndex
                getParser().getToken./
    $getIToken /. // macro getIToken is deprecated. Use function getRhsIToken
                 prsStream->getIToken./
    $getLeftSpan /. // macro getLeftSpan is deprecated. Use function getLeftSpan
                   getParser().getFirstToken./
    $getRightSpan /. // macro getRightSpan is deprecated. Use function getRightSpan
                    getParser().getLastToken./
%End

%Globals
    /.
#pragma once

#include <iostream>
#include "lpg2/AstPoolHolder.h"
#include "lpg2/BacktrackingParser.h"
#include "lpg2/DeterministicParser.h"
#include "lpg2/Diagnose.h"
#include "lpg2/ErrorToken.h"
#include "lpg2/Exception.h"
#include "lpg2/IAbstractArrayList.h"
#include "lpg2/IAst.h"
#include "lpg2/IAstVisitor.h"
#include "lpg2/ILexStream.h"
#include "$sym_type.h"
#include "$prs_type.h"
#include "lpg2/Object.h"
#include "lpg2/ParseTable.h"
#include "lpg2/PrsStream.h"
#include "lpg2/RuleAction.h"
#include "lpg2/IcuUtil.h"
#include "lpg2/stringex.h"
#include "lpg2/Any.h"
    ./
%End

%Headers
    /.
     struct $action_type :public $super_class ,public RuleAction$additional_interfaces
    {
       
        PrsStream* prsStream = nullptr;
        ~$action_type (){
            delete prsStream;
            delete btParser;
        }
         bool unimplementedSymbolsWarning = $unimplemented_symbols_warning;

         inline static ParseTable* prsTable = new $prs_type();
         ParseTable* getParseTable() { return prsTable; }

         BacktrackingParser* btParser = nullptr;
         BacktrackingParser* getParser() { return btParser; }

         void setResult(Object* object) { btParser->setSym1(object); }
         Object* getRhsSym(int i) { return btParser->getSym(i); }

         int getRhsTokenIndex(int i) { return btParser->getToken(i); }
         IToken* getRhsIToken(int i) { return prsStream->getIToken(getRhsTokenIndex(i)); }
        
         int getRhsFirstTokenIndex(int i) { return btParser->getFirstToken(i); }
         IToken* getRhsFirstIToken(int i) { return prsStream->getIToken(getRhsFirstTokenIndex(i)); }

         int getRhsLastTokenIndex(int i) { return btParser->getLastToken(i); }
         IToken* getRhsLastIToken(int i) { return prsStream->getIToken(getRhsLastTokenIndex(i)); }

         int getLeftSpan() { return btParser->getFirstToken(); }
         IToken* getLeftIToken()  { return prsStream->getIToken(getLeftSpan()); }

         int getRightSpan() { return btParser->getLastToken(); }
         IToken* getRightIToken() { return prsStream->getIToken(getRightSpan()); }

         int getRhsErrorTokenIndex(int i)
        {
            int index = btParser->getToken(i);
            IToken* err = prsStream->getIToken(index);
            return ( dynamic_cast<ErrorToken*>(err) ? index : 0);
        }
         ErrorToken * getRhsErrorIToken(int i)
        {
            int index = btParser->getToken(i);
            IToken* err = prsStream->getIToken(index);
            return (ErrorToken*) ( dynamic_cast<ErrorToken*>(err) ? err : nullptr);
        }

         void reset(ILexStream* lexStream)
        {
            delete prsStream;
            prsStream = new PrsStream(lexStream);
            btParser->reset(prsStream);

            try
            {
                prsStream->remapTerminalSymbols(orderedTerminalSymbols(), prsTable->getEoftSymbol());
            }
            catch (NullExportedSymbolsException& e) {
            }
            catch (NullTerminalSymbolsException& e) {
            }
            catch (UnimplementedTerminalsException& e)
            {
                if (unimplementedSymbolsWarning) {
                   auto unimplemented_symbols = e.getSymbols();
                    std::cout << "The Lexer will not scan the following token(s):" << std::endl;
                    for (int i = 0; i < unimplemented_symbols.size(); i++)
                    {
                        auto id = unimplemented_symbols.at(i);
                        std::wcout <<L"    " << $sym_type::orderedTerminalSymbols[id] << std::endl;               
                    }
                   std::cout <<std::endl;  
                }
            }
            catch (UndefinedEofSymbolException& e)
            {
                std::stringex str= "The Lexer does not implement the Eof symbol ";
                str += IcuUtil::ws2s($sym_type::orderedTerminalSymbols[prsTable->getEoftSymbol()]);
                throw  UndefinedEofSymbolException(str);
            } 
        }
        
         $action_type(ILexStream* lexStream = nullptr)
        {
            try
            {
                btParser = new BacktrackingParser(prsStream, prsTable,  this);
            }
            catch (NotBacktrackParseTableException& e)
            {
                throw ( NotBacktrackParseTableException
                                    ("Regenerate $prs_type.java with -BACKTRACK option"));
            }
            catch (BadParseSymFileException& e)
            {
                throw ( BadParseSymFileException("Bad Parser Symbol File -- $sym_type::java"));
            }

            if(lexStream)
            {
                reset(lexStream);
            }
        }
        

        
         int numTokenKinds() { return $sym_type::numTokenKinds; }
         std::vector<std::wstring> orderedTerminalSymbols() { 
             return $sym_type::orderedTerminalSymbols; 
        }
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
         PrsStream* getParseStream() { return prsStream; }

         $ast_class* parser()
        {
            return parser(nullptr, 0);
        }
        
         $ast_class* parser(Monitor* monitor)
        {
            return parser(monitor, 0);
        }
        
         $ast_class * parser(int error_repair_count)
        {
            return parser(nullptr, error_repair_count);
        }

         $ast_class * parser(Monitor* monitor, int error_repair_count)
        {
            btParser->setMonitor(monitor);
            
            try
            {
                return ($ast_class *) btParser->fuzzyParse(error_repair_count);
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
