--
-- An LPG Parser Template Using lpg.jar
--
-- Macros that may be redefined in an instance of this template
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
                //
                // Rule $rule_number:  $rule_text
                //
                ./

    $BeginAction
    /.$Header$case $rule_number: {./

    $EndAction
    /.          break;
                }./

    $BeginJava
    /.$BeginAction
                    $symbol_declarations./

    $EndJava /.$EndAction./

    $NoAction
    /.$Header$case $rule_number:
                    break;./

    $NullAction
    /.$Header$case $rule_number:
                    $setResult(null);
                    break;./

    $BeginActions
    /.
         void ruleAction(ruleNumber : number )
        {
            switch (ruleNumber)
            {./

    $SplitActions
    /.
	            default:
	                ruleAction$rule_number(ruleNumber);
	                break;
	        }
	        return;
	    }
	
	     void ruleAction$rule_number(ruleNumber : number )
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

    $additional_interfaces /../
    $super_stream_class /.LpgLexStream./
%End

%Globals
    /.
    import {BadParseException, RuleAction, PrsStream, ParseTable, BacktrackingParser, IToken, ErrorToken, ILexStream, NullExportedSymbolsException, 
UnimplementedTerminalsException, Lpg, UndefinedEofSymbolException, NotBacktrackParseTableException, BadParseSymFileException, 
IPrsStream, Monitor, DiagnoseParser, IAst, IAstVisitor, IAbstractArrayList, NotDeterministicParseTableException,
 DeterministicParser, NullTerminalSymbolsException } from "lpg2ts";
    ./
%End

%Headers
    /.
    class $action_type extends $super_stream_class implements $sym_type, RuleAction$additional_interfaces
    {
         static  prs : ParseTable =  $prs_type();
          self.dtParser : DeterministicParser;

         void setResult(object1 : any ) { self.dtParser.setSym1(object1); }
          getParser()  : DeterministicParser { return self.dtParser; }
          getRhsSym(i : number) : any { return self.dtParser.getSym(i); }
          getRhsTokenIndex(i : number) : number { return self.dtParser.getToken(i); }
          getRhsFirstTokenIndex(i : number) : number { return self.dtParser.getFirstToken(i); }
          getRhsLastTokenIndex(i : number) : number { return self.dtParser.getLastToken(i); }

          getLeftSpan() : number{ return self.dtParser.getFirstToken(); }
          getRightSpan() : number{ return self.dtParser.getLastToken(); }
 
       def __init__(self,filename : string, number tab)
        {
            super(filename,null, tab);
        }

          orderedExportedSymbols() : string[]{ return self.orderedTerminalSymbols; }
          getEOFTokenKind() : number { return $prs_type.EOFT_SYMBOL; }

          getILexStream() : ILexStream{ return <$super_stream_class> this; }
        
        /**
         * @deprecated replaced by {@link #getILexStream()}
         */
          getLexStream()  : ILexStream{ return <$super_stream_class> this; }

    
          parser(error_repair_count : number = 0 ,  monitor? : Monitor) :  $ast_class | null
        {
            try
            {
                self.dtParser =  DeterministicParser(this, prs, this);
            }
            catch (e)
            {
                if( e instanceof NotDeterministicParseTableException){
                    Lpg.Lang.System.Out.println("****Error: Regenerate $prs_type.ts with -NOBACKTRACK option");
                    process.exit(1);
                }
                if( e instanceof NotDeterministicParseTableException){
                    Lpg.Lang.System.Out.println("****Error: Bad Parser Symbol File -- $sym_type.ts. Regenerate $prs_type.ts");
                    process.exit(1);
                }
            }

            self.dtParser.setMonitor(monitor);

            try
            {
                return <$ast_type> self.dtParser.parse();
            }
            catch (ex)
            {
              
               if( ex instanceof BadParseException)
               {
                    let e = <BadParseException>(e);
                    reset(e.error_token); // point to error token
                    Lpg.Lang.System.Out.print("Error detected on character " + e.error_token);
                    if (e.error_token < getStreamLength())
                        Lpg.Lang.System.Out.print(" at line " + getLine(e.error_token) + ", column " + self.getColumn(e.error_token));
                    else Lpg.Lang.System.Out.print(" at end of file ");
                    Lpg.Lang.System.Out.println(" with kind " + getKind(e.error_token));
               }
               else{
                    raise ex;
               }
            }

            return null;
        }

    ./

%End

%Rules
    /.$BeginActions./
%End

%Trailers
    /.
        $EndActions
    }
    ./
%End

--
-- E N D   O F   T E M P L A T E
--
