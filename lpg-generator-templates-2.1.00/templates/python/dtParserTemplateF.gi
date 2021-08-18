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
                //
                // Rule $rule_number:  $rule_text
                //
                ./

    $BeginAction
    /.$Header$case $rule_number: {
                    //#line $next_line "$input_file$"./

    $EndAction
    /.            break;
                }./

    $BeginJava
    /.$Header$case $rule_number: {
                    $symbol_declarations
                    //#line $next_line "$input_file$"./

    $EndJava /.$EndAction./

    $NoAction
    /.$Header$case $rule_number:
                    break;./

    $BadAction
    /.$Header$case $rule_number:
                    raise Error("No action specified for rule " + $rule_number);./

    $NullAction
    /.$Header$case $rule_number:
                    self.setResult(null);
                    break;./

    $BeginActions
    /.
          ruleAction(ruleNumber : number ) : void
        {
            switch (ruleNumber)
            {
                //#line $next_line "$input_file$"./

    $SplitActions
    /.
	            default:
	                self.ruleAction$rule_number(ruleNumber);
	                break;
	        }
	        return;
	    }
	
	      ruleAction$rule_number(ruleNumber : number ) : void
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

    $entry_declarations
    /.
       
          resetParse$entry_name() : void
        {
            self.dtParser.resetParserEntry($sym_type.$entry_marker);
        }
        
          parse$entry_name(monitor? : Monitor | null, error_repair_count: number = 0) : $ast_class | null
        {
            if(monitor)
                self.dtParser.setMonitor(monitor);
            
            try
            {
                return <$ast_class> self.dtParser.parseEntry($sym_type.$entry_marker);
            }
            catch (ex)
            {
                if( ex instanceof BadParseException ){
                  let e = <BadParseException>(ex);
                  self.prsStream.reset(e.error_token); // point to error token

                  let diagnoseParser =  DiagnoseParser(self.prsStream, $action_type.prsTable);
                  diagnoseParser.diagnoseEntry($sym_type.$entry_marker, e.error_token);
                }
                else{
                    raise ex;
                }
            }

            return null;
        }
    ./
        
    $additional_interfaces /../
    $ast_class /.$ast_type./
    $super_class /.object./
    $unimplemented_symbols_warning /.False./
    
    $setSym1 /. // macro setSym1 is deprecated. Use function setResult
                self.getParser().setSym1./
    $setResult /. // macro setResult is deprecated. Use function setResult
                 self.dtParsergetParser().setSym1./
    $getSym /. // macro getSym is deprecated. Use function getRhsSym
              self.dtParsergetParser().getSym./
    $getToken /. // macro getToken is deprecated. Use function getRhsTokenIndex
                self.dtParsergetParser().getToken./
    $getIToken /. // macro getIToken is deprecated. Use function getRhsIToken
                 self.prsStream.getIToken./
    $getLeftSpan /. // macro getLeftSpan is deprecated. Use function getLeftSpan
                   self.dtParsergetParser().getFirstToken./
    $getRightSpan /. // macro getRightSpan is deprecated. Use function getRightSpan
                   self.dtParsergetParser().getLastToken./
%End

%Globals
    /.
import {BadParseException, RuleAction, PrsStream, ParseTable, BacktrackingParser, IToken, ErrorToken, ILexStream, NullExportedSymbolsException, 
UnimplementedTerminalsException, Lpg, UndefinedEofSymbolException, NotBacktrackParseTableException, BadParseSymFileException, 
IPrsStream, Monitor, DiagnoseParser, IAst, IAstVisitor, IAbstractArrayList, NotDeterministicParseTableException,
 DeterministicParser, NullTerminalSymbolsException } from "lpg2ts";
import { $prs_type } from ".\/$prs_type";
import { $sym_type } from ".\/$sym_type";
    ./
%End

%Headers
    /.
    class $action_type extends $super_class implements RuleAction$additional_interfaces
    {
          prsStream  : PrsStream =  PrsStream();
        
          unimplementedSymbolsWarning : boolean= $unimplemented_symbols_warning;

         static  prsTable  : ParseTable=  $prs_type();
          getParseTable() : ParseTable{ return $action_type.prsTable; }

          dtParser : DeterministicParser ;
          getParser() : DeterministicParser{ return self.dtParser; }

          setResult(object1 : any ) :void{ self.dtParser.setSym1(object1); }
          getRhsSym(i : number) : any { return self.dtParser.getSym(i); }

          getRhsTokenIndex(i : number) : number { return self.dtParser.getToken(i); }
          getRhsIToken(i : number) : IToken { return self.prsStream.getIToken(self.getRhsTokenIndex(i)); }
        
          getRhsFirstTokenIndex(i : number) : number{ return self.dtParser.getFirstToken(i); }
          getRhsFirstIToken(i : number)  : IToken{ return self.prsStream.getIToken(self.getRhsFirstTokenIndex(i)); }

          getRhsLastTokenIndex(i : number) : number{ return self.dtParser.getLastToken(i); }
          getRhsLastIToken(i : number)  : IToken{ return self.prsStream.getIToken(self.getRhsLastTokenIndex(i)); }

          getLeftSpan() : number{ return self.dtParser.getFirstToken(); }
          getLeftIToken() : IToken { return self.prsStream.getIToken(self.getLeftSpan()); }

          getRightSpan() : number { return self.dtParser.getLastToken(); }
          getRightIToken() : IToken { return self.prsStream.getIToken(self.getRightSpan()); }

          getRhsErrorTokenIndex(i : number) : number
        {
            let index = self.dtParser.getToken(i);
            let err = self.prsStream.getIToken(index);
            return (err instanceof ErrorToken ? index : 0);
        }
          getRhsErrorIToken(i : number) : ErrorToken
        {
            let index = self.dtParser.getToken(i);
            let err = self.prsStream.getIToken(index);
            return <ErrorToken> (err instanceof ErrorToken ? err : null);
        }

          reset(lexStream : ILexStream) : void
        {
            self.prsStream.resetLexStream(lexStream);
            self.dtParser.reset(self.prsStream);

            try
            {
                self.prsStream.remapTerminalSymbols(self.orderedTerminalSymbols(), $action_type.prsTable.getEoftSymbol());
            }
            catch(ex)
            {
                if( ex  instanceof NullExportedSymbolsException) {
                }
                else if(ex  instanceof NullTerminalSymbolsException) {
                }
                else if(ex  instanceof UnimplementedTerminalsException)
                {
                    if (self.unimplementedSymbolsWarning) {
                        let e = <UnimplementedTerminalsException>(ex);
                        let unimplemented_symbols = e.getSymbols();
                        Lpg.Lang.System.Out.println("The Lexer will not scan the following token(s):");
                        for (let i : number = 0; i < unimplemented_symbols.size(); i++)
                        {
                            let  id  : number = unimplemented_symbols.get(i);
                            Lpg.Lang.System.Out.println("    " + $sym_type.orderedTerminalSymbols[id]);               
                        }
                        Lpg.Lang.System.Out.println();
                    }
                }
                else if(ex  instanceof UndefinedEofSymbolException )
                {
                    raise ( UndefinedEofSymbolException
                                        ("The Lexer does not implement the Eof symbol " +
                                        $sym_type.orderedTerminalSymbols[$action_type.prsTable.getEoftSymbol()]));
                }
                else{
                    raise ex;
                }
            }


        }
        
       def __init__(self,lexStream? :ILexStream)
        {
            super();
          
            try
            {
                self.dtParser =  DeterministicParser(null, $action_type.prsTable, <RuleAction> this);
            }
            catch (e)
            {
                if( e instanceof NotDeterministicParseTableException)
                raise ( NotDeterministicParseTableException
                                    ("Regenerate $prs_type.ts with -NOBACKTRACK option"));
                else if( e instanceof BadParseSymFileException){
                 raise ( BadParseSymFileException("Bad Parser Symbol File -- $sym_type.ts. Regenerate $prs_type.ts"));
                }
                else{
                    raise e;
                }
            }
            if(lexStream){
              self.reset(lexStream);
            }
        }

      

          numTokenKinds() : number{ return $sym_type.numTokenKinds; }
          orderedTerminalSymbols()  : string[] { return $sym_type.orderedTerminalSymbols; }
          getTokenKindName(kind : number ) : string{ return $sym_type.orderedTerminalSymbols[kind]; }            
          getEOFTokenKind() : number{ return $action_type.prsTable.getEoftSymbol(); }
          getIPrsStream()  : IPrsStream{ return self.prsStream; }

        /**
         * @deprecated replaced by {@link #getIPrsStream()}
         *
         */
          getPrsStream() : PrsStream{ return self.prsStream; }

        /**
         * @deprecated replaced by {@link #getIPrsStream()}
         *
         */
          getParseStream() : PrsStream{ return self.prsStream; }

         parser(error_repair_count : number = 0 ,  monitor? : Monitor) :  $ast_class | null
        {
            self.dtParser.setMonitor(monitor);

            try
            {
                return <$ast_class> self.dtParser.parseEntry();
            }
            catch ( ex)
            {
                if( ex instanceof BadParseException ){
                    let e = <BadParseException>(ex);
                    self.prsStream.reset(e.error_token); // point to error token

                    let diagnoseParser =  DiagnoseParser(self.prsStream, $action_type.prsTable);
                    diagnoseParser.diagnose(e.error_token);
                }
                else{
                    raise ex;
                }
            }

            return null;
        }

        //
        // Additional entry points, if any
        //
        $entry_declarations
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
