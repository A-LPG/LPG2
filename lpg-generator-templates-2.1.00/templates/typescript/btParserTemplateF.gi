--
-- In a parser using this template, the following macro may be redefined:
--
--     %additional_interfaces
--     %ast_class
--
-- B E G I N N I N G   O F   T E M P L A T E   btParserTemplateF
--
%Options programming_Language=typescript,margin=4,backtrack
%Options table,error_maps,scopes
%options prefix=TK_
%options action-block=("*.ts", "/.", "./")
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
                // Rule %rule_number:  %rule_text
                //
                ./

    $BeginAction
    /.%Header%case %rule_number: {
                   //#line %next_line "%input_file%"./

    $EndAction
    /.            break;
                }./

    $BeginJava
    /.%Header%case %rule_number: {
                    %symbol_declarations
                    //#line %next_line "%input_file%"./

    $EndJava /.%EndAction./

    $NoAction
    /.%Header%case %rule_number:
                    break;./

    $BadAction
    /.%Header%case %rule_number:
                    throw ("No action specified for rule " + %rule_number);./

    $NullAction
    /.%Header%case %rule_number:
                    this.setResult(null);
                    break;./

    $BeginActions
    /.
        
        public  ruleAction(ruleNumber : number) : void
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
       
        public  parse%entry_name(monitor? : Monitor, error_repair_count : number = 0) : %ast_class | null
        {
            this.btParser.setMonitor(monitor);
            
            try
            {
                return <%ast_class> this.btParser.fuzzyParseEntry(%sym_type.%entry_marker, error_repair_count);
            }
            catch (ex)
            {
                if( ex instanceof BadParseException ){
                    let e = <BadParseException>(ex);

                    this.prsStream.reset(e.error_token); // point to error token
                    let diagnoseParser = new DiagnoseParser(this.prsStream, %action_type.prsTable);
                    diagnoseParser.diagnoseEntry(%sym_type.%entry_marker, e.error_token);
                }
                else{
                    throw ex;
                }
            }

            return null;
        }
    ./

    --
    -- Macros that may be needed in a parser using this template
    --
    $additional_interfaces /../
    $ast_class /.%ast_type./
    $super_class /.Object./   
    $unimplemented_symbols_warning /.false./

    --
    -- Old deprecated macros that should NEVER be used.
    --
    $setSym1 /. // macro setSym1 is deprecated. Use function setResult
                this.getParser().setSym1./
    $setResult /. // macro setResult is deprecated. Use function setResult
                 this.getParser().setSym1./
    $getSym /. // macro getSym is deprecated. Use function getRhsSym
              this.getParser().getSym./
    $getToken /. // macro getToken is deprecated. Use function getRhsTokenIndex
                this.getParser().getToken./
    $getIToken /. // macro getIToken is deprecated. Use function getRhsIToken
                 this.prsStream.getIToken./
    $getLeftSpan /. // macro getLeftSpan is deprecated. Use function getLeftSpan
                   this.getParser().getFirstToken./
    $getRightSpan /. // macro getRightSpan is deprecated. Use function getRightSpan
                    this.getParser().getLastToken./
%End

%Globals
    /.
import {BadParseException, RuleAction, PrsStream, ParseTable, BacktrackingParser, IToken, ErrorToken, ILexStream, NullExportedSymbolsException, 
UnimplementedTerminalsException, Lpg, UndefinedEofSymbolException, NotBacktrackParseTableException, BadParseSymFileException, 
IPrsStream, Monitor, DiagnoseParser, IAst, IAstVisitor, IAbstractArrayList, NotDeterministicParseTableException,
 DeterministicParser, NullTerminalSymbolsException } from "lpg2ts";
import { %prs_type } from ".\/%prs_type";
import { %sym_type } from ".\/%sym_type";
    ./
%End

%Headers
    /.
    export class %action_type extends %super_class implements RuleAction%additional_interfaces
    {
        private  prsStream  : PrsStream = new PrsStream();
        
        private  unimplementedSymbolsWarning : boolean = %unimplemented_symbols_warning;

        private static  prsTable : ParseTable = new %prs_type();
        public  getParseTable() : ParseTable { return %action_type.prsTable; }

        private  btParser : BacktrackingParser ;
        public  getParser() : BacktrackingParser{ return this.btParser; }

        private  setResult(object1 : any) : void{ this.btParser.setSym1(object1); }
        public  getRhsSym(i : number) : any{ return this.btParser.getSym(i); }

        public  getRhsTokenIndex(i : number) : number{ return this.btParser.getToken(i); }
        public  getRhsIToken(i : number) : IToken { return this.prsStream.getIToken(this.getRhsTokenIndex(i)); }
        
        public  getRhsFirstTokenIndex(i : number) : number { return this.btParser.getFirstToken(i); }
        public  getRhsFirstIToken(i : number) : IToken{ return this.prsStream.getIToken(this.getRhsFirstTokenIndex(i)); }

        public  getRhsLastTokenIndex(i : number):number { return this.btParser.getLastToken(i); }
        public  getRhsLastIToken(i : number):IToken { return this.prsStream.getIToken(this.getRhsLastTokenIndex(i)); }

        public getLeftSpan() :number { return this.btParser.getFirstToken(); }
        public  getLeftIToken() :IToken { return this.prsStream.getIToken(this.getLeftSpan()); }

        public getRightSpan() : number { return this.btParser.getLastToken(); }
        public  getRightIToken() : IToken { return this.prsStream.getIToken(this.getRightSpan()); }

        public  getRhsErrorTokenIndex(i : number) : number
        {
            let index = this.btParser.getToken(i);
            let err = this.prsStream.getIToken(index);
            return (err instanceof ErrorToken ? index : 0);
        }
        public  getRhsErrorIToken(i : number) : ErrorToken
        {
            let index = this.btParser.getToken(i);
            let err = this.prsStream.getIToken(index);
            return <ErrorToken> (err instanceof ErrorToken ? err : null);
        }

        public  reset(lexStream : ILexStream) : void
        {
            this.prsStream.resetLexStream(lexStream);
            this.btParser.reset(this.prsStream);

            try
            {
                this.prsStream.remapTerminalSymbols(this.orderedTerminalSymbols(), %action_type.prsTable.getEoftSymbol());
            } 
            catch (e)
            {     
                if( e instanceof NullExportedSymbolsException){
                    
                }
                else if( e instanceof UnimplementedTerminalsException){
                    if (this.unimplementedSymbolsWarning) {
                        let unimplemented_symbols = e.getSymbols();
                        Lpg.Lang.System.Out.println("The Lexer will not scan the following token(s):");
                        for (let i : number = 0; i < unimplemented_symbols.size(); i++)
                        {
                            let id = <number>unimplemented_symbols.get(i);
                            Lpg.Lang.System.Out.println("    " + %sym_type.orderedTerminalSymbols[id]);               
                        }
                        Lpg.Lang.System.Out.println();
                    }
                }
                else if( e instanceof UndefinedEofSymbolException){
                    throw  (new UndefinedEofSymbolException
                        ("The Lexer does not implement the Eof symbol " +
                        %sym_type.orderedTerminalSymbols[%action_type.prsTable.getEoftSymbol()]));
                }

            }
        }
        
        constructor(lexStream? :ILexStream)
        {
            super();
            try
            {
                this.btParser = new BacktrackingParser(null, %action_type.prsTable, <RuleAction> this);
            }
            catch (e)
            {
                if(e instanceof NotBacktrackParseTableException)
                throw (new NotBacktrackParseTableException
                                    ("Regenerate %prs_type.ts with -BACKTRACK option"));
                else if(e instanceof BadParseSymFileException){
                    throw (new BadParseSymFileException("Bad Parser Symbol File -- %sym_type.ts"));
                }
                else{
                    throw e;
                }
            }
            if(lexStream){
              this.reset(lexStream);
            }
        }
        
       
        
        public  numTokenKinds() :number { return %sym_type.numTokenKinds; }
        public  orderedTerminalSymbols()  : string[] { return %sym_type.orderedTerminalSymbols; }
        public  getTokenKindName(kind : number ) : string { return %sym_type.orderedTerminalSymbols[kind]; }
        public  getEOFTokenKind() : number{ return %action_type.prsTable.getEoftSymbol(); }
        public  getIPrsStream()  : IPrsStream{ return this.prsStream; }

        /**
         * @deprecated replaced by {@link #getIPrsStream()}
         *
         */
        public  getPrsStream()  : PrsStream{ return this.prsStream; }

        /**
         * @deprecated replaced by {@link #getIPrsStream()}
         *
         */
        public  getParseStream() : PrsStream { return this.prsStream; }

     

        public parser(error_repair_count : number = 0 ,  monitor? : Monitor) :  %ast_class | null
        {
            this.btParser.setMonitor(monitor);
            
            try
            {
                return <%ast_class> this.btParser.fuzzyParse(error_repair_count);
            }
            catch (ex)
            {
               if( ex instanceof BadParseException ){
                     let e = <BadParseException>(ex);
                    this.prsStream.reset(e.error_token); // point to error token

                    let diagnoseParser = new DiagnoseParser(this.prsStream, %action_type.prsTable);
                    diagnoseParser.diagnose(e.error_token);
                }
                else{
                    throw ex;
                }
            }

            return null;
        }

        //
        // Additional entry points, if any
        //
        %entry_declarations
    ./

%End

%Rules
    /.%BeginActions./
%End

%Trailers
    /.
        %EndActions
    }
    ./
%End

--
-- E N D   O F   T E M P L A T E
--
