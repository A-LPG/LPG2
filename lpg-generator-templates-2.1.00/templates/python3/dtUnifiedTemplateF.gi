--
-- An LPG Parser Template Using lpg.jar
--
-- Macros that may be redefined in an instance of self template
--
--     $additional_interfaces
--     $super_stream_class
--
-- B E G I N N I N G   O F   T E M P L A T E   dtUnifiedTemplateF
--
%options programming_Language=python3
%options table
%options margin=4
%options prefix=Char_
%options action-block=("*.py", "/.", "./")
%options ParseTable=ParseTable

--
-- The EOF and ERROR symbols are assigned a default here as a
-- convenience.
--
%EOF
    EOF
%End

%Define
    $Header
    /.
                #
                # Rule $rule_number:  $rule_text
                #
                ./

    $BeginAction
    /.$Header$case $rule_number: ./

    $EndAction
    /.          break
                ./

    $BeginJava
    /.$BeginAction
                    $symbol_declarations./

    $EndJava /.$EndAction./

    $NoAction
    /.$Header$case $rule_number:
                    break./

    $NullAction
    /.$Header$case $rule_number:
                    $setResult(None)
                    break./

    $BeginActions
    /.
         void ruleAction(ruleNumber : int )
        
            switch (ruleNumber)
            ./

    $SplitActions
    /.
	            default:
	                ruleAction$rule_number(ruleNumber)
	                break
	        
	        return
	    
	
	     void ruleAction$rule_number(ruleNumber : int )
	    
	        switch (ruleNumber)
	        ./

    $EndActions
    /.
                default:
                    break
            
            return
        ./

    $additional_interfaces /../
    $super_stream_class /.LpgLexStream./
%End

%Globals
    /.
from lpg2 import ArrayList, BadParseException, RuleAction, PrsStream, ParseTable, BacktrackingParser, IToken, ErrorToken, ILexStream, NullExportedSymbolsException, UnimplementedTerminalsException,  UndefinedEofSymbolException, NotBacktrackParseTableException, BadParseSymFileException, IPrsStream, Monitor, DiagnoseParser, IAst, IAstVisitor, IAbstractArrayList, NotDeterministicParseTableException,DeterministicParser, NullTerminalSymbolsException 

    ./
%End

%Headers
    /.
    class $action_type extends $super_stream_class implements $sym_type, RuleAction$additional_interfaces
    
         static  prs : ParseTable =  $prs_type()
          self.dtParser : DeterministicParser

         void setResult(object1 )  self.dtParser.setSym1(object1) 
          getParser()  : DeterministicParser  return self.dtParser 
          getRhsSym(self, i: int) :   return self.dtParser.getSym(i) 
          getRhsTokenIndex(self, i: int) -> int : return self.dtParser.getToken(i) 
          getRhsFirstTokenIndex(self, i: int) -> int : return self.dtParser.getFirstToken(i) 
          getRhsLastTokenIndex(self, i: int) -> int : return self.dtParser.getLastToken(i) 

          getLeftSpan(self) -> int: return self.dtParser.getFirstToken() 
          getRightSpan(self) -> int: return self.dtParser.getLastToken() 
 
       def __init__(self,filename : str, number tab)
        
            super(filename,None, tab)
        

          orderedExportedSymbols(self) ->list : return self.orderedTerminalSymbols 
          getEOFTokenKind(self) -> int : return $prs_type.EOFT_SYMBOL 

          getILexStream() : ILexStream return <$super_stream_class> self 
        
        /**
         * @deprecated replaced by @link #getILexStream()
         */
          getLexStream()  : ILexStream return <$super_stream_class> self 

    
          parser(error_repair_count : int = 0 ,  monitor : Monitor = None) :  $ast_class | None
        
            try:
            
                self.dtParser =  DeterministicParser(self, prs, self)
            
            catch (e)
            
                if( e instanceof NotDeterministicParseTableException)
                    print("****Error: Regenerate $prs_type.ts with -NOBACKTRACK option")
                    process.exit(1)
                
                if( e instanceof NotDeterministicParseTableException)
                    print("****Error: Bad Parser Symbol File -- $sym_type.ts. Regenerate $prs_type.ts")
                    process.exit(1)
                
            

            self.dtParser.setMonitor(monitor)

            try:
            
                return <$ast_type> self.dtParser.parse()
            
            catch (ex)
            
              
               if( ex instanceof BadParseException)
               
                    e = <BadParseException>(e)
                    reset(e.error_token) # point to error token
                    print("Error detected on character " + e.error_token)
                    if (e.error_token < getStreamLength())
                        print(" at line " + getLine(e.error_token) + ", column " + self.getColumn(e.error_token))
                    else print(" at end of file ")
                    print(" with kind " + getKind(e.error_token))
               
               else
                    raise ex
               
            

            return None
        

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
