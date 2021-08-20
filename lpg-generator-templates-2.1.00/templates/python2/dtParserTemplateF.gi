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

    $BeginAction
    /.$Header$case $rule_number: 
                    ##line $next_line "$input_file$"./

    $EndAction
    /.            break
                ./

    $BeginJava
    /.$Header$case $rule_number: 
                    $symbol_declarations
                    ##line $next_line "$input_file$"./

    $EndJava /.$EndAction./

    $NoAction
    /.$Header$case $rule_number:
                    break./

    $BadAction
    /.$Header$case $rule_number:
                    raise Error("No action specified for rule " + $rule_number)./

    $NullAction
    /.$Header$case $rule_number:
                    self.setResult(None)
                    break./

    $BeginActions
    /.
          ruleAction(ruleNumber  ) : 
        
            switch (ruleNumber)
            
                ##line $next_line "$input_file$"./

    $SplitActions
    /.
	            default:
	                self.ruleAction$rule_number(ruleNumber)
	                break
	        
	        return
	    
	
	      ruleAction$rule_number(ruleNumber  ) : 
	    
	        switch (ruleNumber)
	        ./

    $EndActions
    /.
                default:
                    break
            
            return
        ./

    $entry_declarations
    /.
       
          resetParse$entry_name() : 
        
            self.dtParser.resetParserEntry($sym_type.$entry_marker)
        
        
          parse$entry_name(monitor  = None | None, error_repair_count = 0) : $ast_class | None
        
            if(monitor)
                self.dtParser.setMonitor(monitor)
            
            try:
            
                return <$ast_class> self.dtParser.parseEntry($sym_type.$entry_marker)
            
            catch (ex)
            
                if( ex instanceof BadParseException )
                  e = <BadParseException>(ex)
                  self.prsStream.reset(e.error_token) # point to error token

                  diagnoseParser =  DiagnoseParser(self.prsStream, $action_type.prsTable)
                  diagnoseParser.diagnoseEntry($sym_type.$entry_marker, e.error_token)
                
                else
                    raise ex
                
            

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
import BadParseException, RuleAction, PrsStream, ParseTable, BacktrackingParser, IToken, ErrorToken, ILexStream, NullExportedSymbolsException, 
UnimplementedTerminalsException, Lpg, UndefinedEofSymbolException, NotBacktrackParseTableException, BadParseSymFileException, 
IPrsStream, Monitor, DiagnoseParser, IAst, IAstVisitor, IAbstractArrayList, NotDeterministicParseTableException,
 DeterministicParser, NullTerminalSymbolsException  from "lpg2ts"
import  $prs_type  from ".\/$prs_type"
import  $sym_type  from ".\/$sym_type"
    ./
%End

%Headers
    /.
    class $action_type extends $super_class implements RuleAction$additional_interfaces
    
          prsStream   =  PrsStream()
        
          unimplementedSymbolsWarning = $unimplemented_symbols_warning

          static  prsTable  =  $prs_type()
          getParseTable()  return $action_type.prsTable 

          dtParser : DeterministicParser 
          getParser() : DeterministicParser return self.dtParser 

          setResult(object1 ) :void self.dtParser.setSym1(object1) 
          getRhsSym(self, i ) :   return self.dtParser.getSym(i) 

          getRhsTokenIndex(self, i )  : return self.dtParser.getToken(i) 
          getRhsIToken(self, i )  : return self.prsStream.getIToken(self.getRhsTokenIndex(i)) 
        
          getRhsFirstTokenIndex(self, i ) : return self.dtParser.getFirstToken(i) 
          getRhsFirstIToken(self, i )   : return self.prsStream.getIToken(self.getRhsFirstTokenIndex(i)) 

          getRhsLastTokenIndex(self, i ) : return self.dtParser.getLastToken(i) 
          getRhsLastIToken(self, i )   : return self.prsStream.getIToken(self.getRhsLastTokenIndex(i)) 

          getLeftSpan(self) : return self.dtParser.getFirstToken() 
          getLeftIToken(self)  : return self.prsStream.getIToken(self.getLeftSpan()) 

          getRightSpan(self)  : return self.dtParser.getLastToken() 
          getRightIToken(self)  : return self.prsStream.getIToken(self.getRightSpan()) 

          getRhsErrorTokenIndex(self, i )  :
        
            index = self.dtParser.getToken(i)
            err = self.prsStream.getIToken(index)
            return ( index  if isinstance(err,ErrorToken)  else 0)
        
          getRhsErrorIToken(self, i ) :
        
            index = self.dtParser.getToken(i)
            err = self.prsStream.getIToken(index)
            return  (err if  isinstance(err,ErrorToken) else  None)
        

          reset(lexStream ) : 
        
            self.prsStream.resetLexStream(lexStream)
            self.dtParser.reset(self.prsStream)

            try:
            
                self.prsStream.remapTerminalSymbols(self.orderedTerminalSymbols(), $action_type.prsTable.getEoftSymbol())
            
            catch(ex)
            
                if( ex  instanceof NullExportedSymbolsException) 
                
                else if(ex  instanceof NullTerminalSymbolsException) 
                
                else if(ex  instanceof UnimplementedTerminalsException)
                
                    if (self.unimplementedSymbolsWarning) 
                        e = <UnimplementedTerminalsException>(ex)
                        unimplemented_symbols = e.getSymbols()
                        print("The Lexer will not scan the following token(s):")
                        for (i  = 0 i < unimplemented_symbols.size() i++)
                        
                             id   = unimplemented_symbols.get(i)
                            print("    " + $sym_type.orderedTerminalSymbols[id])               
                        
                        print()
                    
                
                else if(ex  instanceof UndefinedEofSymbolException )
                
                    raise ( UndefinedEofSymbolException
                                        ("The Lexer does not implement the Eof symbol " +
                                        $sym_type.orderedTerminalSymbols[$action_type.prsTable.getEoftSymbol()]))
                
                else
                    raise ex
                
            


        
        
       def __init__(self,lexStream? )
        
            super()
          
            try:
            
                self.dtParser =  DeterministicParser(None, $action_type.prsTable,  self)
            
            catch (e)
            
                if( e instanceof NotDeterministicParseTableException)
                raise ( NotDeterministicParseTableException
                                    ("Regenerate $prs_type.ts with -NOBACKTRACK option"))
                else if( e instanceof BadParseSymFileException)
                 raise ( BadParseSymFileException("Bad Parser Symbol File -- $sym_type.ts. Regenerate $prs_type.ts"))
                
                else
                    raise e
                
            
            if(lexStream):
              self.reset(lexStream)
            
        

      

          numTokenKinds(self) : return $sym_type.numTokenKinds 
          orderedTerminalSymbols()   :  return $sym_type.orderedTerminalSymbols 
          getTokenKindName(kind  )  return $sym_type.orderedTerminalSymbols[kind]             
          getEOFTokenKind(self) : return $action_type.prsTable.getEoftSymbol() 
          getIPrsStream()   : return self.prsStream 

        /**
         * @deprecated replaced by @link #getIPrsStream()
         *
         */
          getPrsStream()  return self.prsStream 

        /**
         * @deprecated replaced by @link #getIPrsStream()
         *
         */
          getParseStream()  return self.prsStream 

         parser(error_repair_count  = 0 ,  monitor  = None) :  $ast_class | None
        
            self.dtParser.setMonitor(monitor)

            try:
            
                return <$ast_class> self.dtParser.parseEntry()
            
            catch ( ex)
            
                if( ex instanceof BadParseException )
                    e = <BadParseException>(ex)
                    self.prsStream.reset(e.error_token) # point to error token

                    diagnoseParser =  DiagnoseParser(self.prsStream, $action_type.prsTable)
                    diagnoseParser.diagnose(e.error_token)
                
                else
                    raise ex
                
            

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
