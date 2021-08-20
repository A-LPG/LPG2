--
-- In a parser using this template, the following macro may be redefined:
--
--     $additional_interfaces
--     $ast_class
--
-- B E G I N N I N G   O F   T E M P L A T E   btParserTemplateF
--
%options programming_Language=python3,margin=4,backtrack
%options table,error_maps,scopes
%options prefix=TK_
%options action-block=("*.py", "/.", "./")
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
            #
            # Rule $rule_number:  $rule_text
            #
            ./

    $DefaultAction
    /.$Header def Act$rule_number():./

    $BeginAction
    /.$DefaultAction 
                   ##line $next_line "$input_file$"./

    $EndAction
    /.
             self.__rule_action[$rule_number] =Act$rule_number
    ./

    $BeginJava
    /.$DefaultAction 
                    $symbol_declarations
                    ##line $next_line "$input_file$"./

    $EndJava /.$EndAction./

    $NoAction
    /.$Header
                     ./

    $BadAction
    /.$DefaultAction 
                    raise ValueError("No action specified for rule " + str($rule_number) )./

    $NullAction
    /.$DefaultAction 
                    self.setResult(None)
                    ./

    $BeginActions
    /.
        def initRuleAction(self) : 
    ./


    $EndActions
    /../

    $entry_declarations
    /.
       
        def parse$entry_name(self, monitor  = None, error_repair_count  = 0):

            self.btParser.setMonitor(monitor)
            try:
                return  self.btParser.fuzzyParseEntry($sym_type.$entry_marker, error_repair_count)
            except BadParseException as e:
                self.prsStream.reset(e.error_token) # point to error token
                diagnoseParser =  DiagnoseParser(self.prsStream, $action_type.prsTable)
                diagnoseParser.diagnoseEntry($sym_type.$entry_marker, e.error_token)
                
            return None
        
    ./

    --
    -- Macros that may be needed in a parser using self template
    --
    $additional_interfaces /../
    $ast_class /.$ast_type./
    $super_class /.object./   
    $unimplemented_symbols_warning /.False./

    --
    -- Old deprecated macros that should NEVER be used.
    --
    $setSym1 /. # macro setSym1 is deprecated. Use function setResult
                self.getParser().setSym1./
    $setResult /. # macro setResult is deprecated. Use function setResult
                 self.getParser().setSym1./
    $getSym /. # macro getSym is deprecated. Use function getRhsSym
              self.getParser().getSym./
    $getToken /. # macro getToken is deprecated. Use function getRhsTokenIndex
                self.getParser().getToken./
    $getIToken /. # macro getIToken is deprecated. Use function getRhsIToken
                 self.prsStream.getIToken./
    $getLeftSpan /. # macro getLeftSpan is deprecated. Use function getLeftSpan
                   self.getParser().getFirstToken./
    $getRightSpan /. # macro getRightSpan is deprecated. Use function getRightSpan
                    self.getParser().getLastToken./
%End

%Globals
    /.
from "lpg2ts" import BadParseException, RuleAction, PrsStream, ParseTable, BacktrackingParser, IToken, ErrorToken, ILexStream, NullExportedSymbolsException, 
UnimplementedTerminalsException, Lpg, UndefinedEofSymbolException, NotBacktrackParseTableException, BadParseSymFileException, 
IPrsStream, Monitor, DiagnoseParser, IAst, IAstVisitor, IAbstractArrayList, NotDeterministicParseTableException,
 DeterministicParser, NullTerminalSymbolsException 

from $prs_type import  $prs_type 
from $sym_type import  $sym_type 
    ./
%End

%Headers
    /.
    class $action_type ( $super_class , RuleAction$additional_interfaces):
    
        def ruleAction(self,ruleNumber ) :
            act = self.__rule_action[ruleNumber]
            if act:
                act() 

        prsTable  =  $prs_type()

        def getParseTable(self)  : return $action_type.prsTable 

        def getParser(self)  : return self.btParser 

        def setResult(self,object1) : self.btParser.setSym1(object1) 
        def getRhsSym(self, i ) : return self.btParser.getSym(i) 

        def getRhsTokenIndex(self, i ) : return self.btParser.getToken(i) 
        def getRhsIToken(self, i )  : return self.prsStream.getIToken(self.getRhsTokenIndex(i)) 
        
        def getRhsFirstTokenIndex(self, i )  : return self.btParser.getFirstToken(i) 
        def getRhsFirstIToken(self, i )  : return self.prsStream.getIToken(self.getRhsFirstTokenIndex(i)) 

        def getRhsLastTokenIndex(self, i ) : return self.btParser.getLastToken(i) 
        def getRhsLastIToken(self, i ) : return self.prsStream.getIToken(self.getRhsLastTokenIndex(i)) 

        def getLeftSpan(self)  : return self.btParser.getFirstToken() 
        def getLeftIToken(self)  : return self.prsStream.getIToken(self.getLeftSpan()) 

        def getRightSpan(self)  : return self.btParser.getLastToken() 
        def getRightIToken(self)  : return self.prsStream.getIToken(self.getRightSpan()) 

        def getRhsErrorTokenIndex(self, i )  :
        
            index = self.btParser.getToken(i)
            err = self.prsStream.getIToken(index)
            return ( index  if isinstance(err,ErrorToken)  else 0)
        
        def getRhsErrorIToken(self, i ) :
        
            index = self.btParser.getToken(i)
            err = self.prsStream.getIToken(index)
            return  (err if  isinstance(err,ErrorToken) else  None)
        

        def reset(self,lexStream ) : 
        
            self.prsStream.resetLexStream(lexStream)
            self.btParser.reset(self.prsStream)

            try: 
                self.prsStream.remapTerminalSymbols(self.orderedTerminalSymbols(), $action_type.prsTable.getEoftSymbol())
            except NullExportedSymbolsException as e :
                pass
            
            except UnimplementedTerminalsException as e:
                if (self.unimplementedSymbolsWarning): 
                    unimplemented_symbols = e.getSymbols()
                    print("The Lexer will not scan the following token(s):\n")
                    for i in range(unimplemented_symbols.size()):
                        id = unimplemented_symbols.get(i)
                        print("    " + str($sym_type.orderedTerminalSymbols[id]))               
                    
                    print("\n")
                
            
            except UndefinedEofSymbolException as e:
                raise  ( UndefinedEofSymbolException
                    ("The Lexer does not implement the Eof symbol " +
                    $sym_type.orderedTerminalSymbols[$action_type.prsTable.getEoftSymbol()]))
                

            
        
        
        def __init__(self,lexStream  = None):
        
            super().__init__()
            self.__rule_action = [None]* $num_rules
            self.prsStream   =  PrsStream()
            self.btParser  = None 

            unimplementedSymbolsWarning  = $unimplemented_symbols_warning

            try:
                self.btParser =  BacktrackingParser(None, $action_type.prsTable,  self) 
            except  NotBacktrackParseTableException as e:
                raise ( NotBacktrackParseTableException
                                    ("Regenerate $prs_type.ts with -BACKTRACK option"))
            except BadParseSymFileException as e:
                raise ( BadParseSymFileException("Bad Parser Symbol File -- $sym_type.ts"))
                

            
            if(lexStream):
              self.reset(lexStream)
            
        
        
       
        
        def numTokenKinds(self)  : return $sym_type.numTokenKinds 
        def orderedTerminalSymbols(self)   :  return $sym_type.orderedTerminalSymbols 
        def getTokenKindName(self,kind  )  : return $sym_type.orderedTerminalSymbols[kind] 
        def getEOFTokenKind(self) : return $action_type.prsTable.getEoftSymbol() 
        def getIPrsStream(self)   : return self.prsStream 



        def parser(self,error_repair_count  = 0 ,  monitor  = None) :  
        
            self.btParser.setMonitor(monitor)
            
            try:
                return  self.btParser.fuzzyParse(error_repair_count)
            except BadParseException as e :
                     
                self.prsStream.reset(e.error_token) # point to error token

                diagnoseParser =  DiagnoseParser(self.prsStream, $action_type.prsTable)
                diagnoseParser.diagnose(e.error_token)
                

            return None
        

        #
        # Additional entry points, if any
        #
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
