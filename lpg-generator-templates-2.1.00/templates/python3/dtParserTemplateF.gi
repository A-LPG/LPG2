--
-- In a parser using this template, the following macro may be redefined:
--
--     $additional_interfaces
--     $ast_class
--
-- B E G I N N I N G   O F   T E M P L A T E   dtParserTemplateF
--
%options programming_Language=python3,margin=4
%options table,error_maps,scopes
%options prefix=TK_
%options action-block=("*.py", "/.", "./")
%options ParseTable=ParseTable

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
             self.__rule_action[$rule_number]= Act$rule_number
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
       
        def resetParse$entry_name(self) : 
        
            self.dtParser.resetParserEntry($sym_type.$entry_marker)
        
        
        def parse$entry_name(self,monitor: Monitor = None , error_repair_count: int = 0):
        
            self.dtParser.setMonitor(monitor)
            try:
                return self.dtParser.parseEntry($sym_type.$entry_marker)
            except BadParseException as e:
           
                self.prsStream.reset(e.error_token) # point to error token
                diagnoseParser= DiagnoseParser(self.prsStream, $action_type.prsTable)
                diagnoseParser.diagnoseEntry($sym_type.$entry_marker, e.error_token)
            
            return None
        
    ./
        
    $additional_interfaces /../
    $ast_class /.$ast_type./
    $super_class /.object./
    $unimplemented_symbols_warning /.False./
    
    $setSym1 /. # macro setSym1 is deprecated. Use function setResult
                self.getParser().setSym1./
    $setResult /. # macro setResult is deprecated. Use function setResult
                 self.dtParsergetParser().setSym1./
    $getSym /. # macro getSym is deprecated. Use function getRhsSym
              self.dtParsergetParser().getSym./
    $getToken /. # macro getToken is deprecated. Use function getRhsTokenIndex
                self.dtParsergetParser().getToken./
    $getIToken /. # macro getIToken is deprecated. Use function getRhsIToken
                 self.prsStream.getIToken./
    $getLeftSpan /. # macro getLeftSpan is deprecated. Use function getLeftSpan
                   self.dtParsergetParser().getFirstToken./
    $getRightSpan /. # macro getRightSpan is deprecated. Use function getRightSpan
                   self.dtParsergetParser().getLastToken./
%End

%Globals
    /.
from lpg2 import ArrayList, BadParseException, RuleAction, PrsStream, ParseTable, BacktrackingParser, IToken, ErrorToken, ILexStream, NullExportedSymbolsException, UnimplementedTerminalsException,  UndefinedEofSymbolException, NotBacktrackParseTableException, BadParseSymFileException, IPrsStream, Monitor, DiagnoseParser, IAst, IAstVisitor, IAbstractArrayList, NotDeterministicParseTableException,DeterministicParser, NullTerminalSymbolsException 
from $prs_type import  $prs_type 
from $sym_type import  $sym_type 
    ./
%End

%Headers
    /.
    class $action_type(RuleAction$additional_interfaces):
    
        def ruleAction(self, ruleNumber: int) :
            act = self.__rule_action[ruleNumber]
            if act:
                act() 
        
        prsTable  : ParseTable=  $prs_type()

        def getParseTable(sel) ->ParseTable :
            return $action_type.prsTable 

        def getParser(self) -> DeterministicParser:
            return self.dtParser 

        def setResult(self, object1 ) :
            self.dtParser.setSym1(object1) 

        def getRhsSym(self, i: int) :
            return self.dtParser.getSym(i) 

        def getRhsTokenIndex(self, i: int) -> int :  
            return self.dtParser.getToken(i) 

        def getRhsIToken(self, i: int) -> IToken :
            return self.prsStream.getIToken(self.getRhsTokenIndex(i)) 
        
        def getRhsFirstTokenIndex(self, i: int) -> int:
            return self.dtParser.getFirstToken(i) 

        def getRhsFirstIToken(self, i: int)  -> IToken : 
            return self.prsStream.getIToken(self.getRhsFirstTokenIndex(i)) 

        def getRhsLastTokenIndex(self, i: int) -> int:
            return self.dtParser.getLastToken(i) 

        def getRhsLastIToken(self, i: int) -> IToken :
            return self.prsStream.getIToken(self.getRhsLastTokenIndex(i)) 

        def getLeftSpan(self) -> int:
            return self.dtParser.getFirstToken() 

        def getLeftIToken(self) -> IToken :
            return self.prsStream.getIToken(self.getLeftSpan()) 

        def getRightSpan(self) -> int :
            return self.dtParser.getLastToken() 

        def getRightIToken(self) -> IToken :
            return self.prsStream.getIToken(self.getRightSpan()) 

        def getRhsErrorTokenIndex(self, i: int) -> int :
        
            index = self.dtParser.getToken(i)
            err = self.prsStream.getIToken(index)
            return index  if isinstance(err,ErrorToken)  else 0
        
        def getRhsErrorIToken(self, i: int) -> ErrorToken:
        
            index = self.dtParser.getToken(i)
            err = self.prsStream.getIToken(index)
            return  err if  isinstance(err,ErrorToken) else  None
        

        def reset(self, lexStream : ILexStream) : 
        
            self.prsStream.resetLexStream(lexStream)
            self.dtParser.reset(self.prsStream)

            try:
                self.prsStream.remapTerminalSymbols(self.orderedTerminalSymbols(), $action_type.prsTable.getEoftSymbol())    
            except NullExportedSymbolsException as e :
                pass
            except NullTerminalSymbolsException as e :
                pass
            except UnimplementedTerminalsException as e :
                if self.unimplementedSymbolsWarning:

                    unimplemented_symbols = e.getSymbols()
                    print("The Lexer will not scan the following token(s):")
                    for i in range(unimplemented_symbols.size()):
                        id: int = unimplemented_symbols.get(i)
                        print("    " + str($sym_type.orderedTerminalSymbols[id]) )      

            except UndefinedEofSymbolException as e :           
                raise ( UndefinedEofSymbolException
                                    ("The Lexer does not implement the Eof symbol " +
                                    $sym_type.orderedTerminalSymbols[$action_type.prsTable.getEoftSymbol()]))
        def __init__(self, lexStream: ILexStream = None):
        
            super().__init__()
            self.__rule_action = [None]* ($num_rules + 2)
            self.prsStream  : PrsStream =  PrsStream()
            self.btParser : DeterministicParser = None 
            self.unimplementedSymbolsWarning : bool = $unimplemented_symbols_warning
            self.initRuleAction()
            try:
                self.dtParser =  DeterministicParser(None, $action_type.prsTable,  self)
            except NotDeterministicParseTableException as e:
                raise NotDeterministicParseTableException("Regenerate $prs_type.py with -NOBACKTRACK option")

            except BadParseSymFileException as e:        
                raise BadParseSymFileException("Bad Parser Symbol File -- $sym_type.py. Regenerate $prs_type.py")

            if lexStream is not None:
               self.reset(lexStream)
            
        
        def numTokenKinds(self) -> int:
            return $sym_type.numTokenKinds 

        def orderedTerminalSymbols(self) ->list :
            return $sym_type.orderedTerminalSymbols 

        def getTokenKindName(self, kind: int ) -> str:
            return $sym_type.orderedTerminalSymbols[kind]   

        def getEOFTokenKind(self) -> int:
            return $action_type.prsTable.getEoftSymbol() 

        def getIPrsStream(self) -> IPrsStream:
            return self.prsStream 

        def parser(self, error_repair_count: int = 0 , monitor: Monitor = None):
        
            self.dtParser.setMonitor(monitor)

            try:
                return self.dtParser.parseEntry()
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
