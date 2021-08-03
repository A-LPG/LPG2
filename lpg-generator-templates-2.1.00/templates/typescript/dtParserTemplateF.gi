--
-- In a parser using this template, the following macro may be redefined:
--
--     $additional_interfaces
--     $ast_class
--
-- B E G I N N I N G   O F   T E M P L A T E   dtParserTemplateF
--
%Options programming_Language=typescript,margin=4
%Options table,error_maps,scopes
%Options prefix=TK_
%Options action-block=("*.ts", "/.", "./")
%Options ParseTable=lpg.runtime.ParseTable

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
                    throw ("No action specified for rule " + $rule_number);./

    $NullAction
    /.$Header$case $rule_number:
                    setResult(null);
                    break;./

    $BeginActions
    /.
        public void ruleAction(ruleNumber : number )
        {
            switch (ruleNumber)
            {
                //#line $next_line "$input_file$"./

    $SplitActions
    /.
	            default:
	                ruleAction$rule_number(ruleNumber);
	                break;
	        }
	        return;
	    }
	
	    public void ruleAction$rule_number(ruleNumber : number )
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
        public $ast_class parse$entry_name()
        {
            return parse$entry_name(null, 0);
        }
            
        public $ast_class parse$entry_name(Monitor monitor)
        {
            return parse$entry_name(monitor, 0);
        }
            
        public $ast_class parse$entry_name(number error_repair_count)
        {
            return parse$entry_name(null, error_repair_count);
        }
            
        public void resetParse$entry_name()
        {
            this.dtParser.resetParserEntry($sym_type.$entry_marker);
        }
        
       public  parse$entry_name(monitor : Monitor=null, error_repair_count= number = 0) :  $ast_class
        {
            this.dtParser.setMonitor(monitor);
            
            try
            {
                return <$ast_class> this.dtParser.parseEntry($sym_type.$entry_marker);
            }
            catch (BadParseException e)
            {
                this.prsStream.reset(e.error_token); // point to error token

                let diagnoseParser = new DiagnoseParser(this.prsStream, $action_type.prsTable);
                diagnoseParser.diagnoseEntry($sym_type.$entry_marker, e.error_token);
            }

            return null;
        }
    ./
        
    $additional_interfaces /../
    $ast_class /.$ast_type./
    $super_class /.any./
    $unimplemented_symbols_warning /.false./
    
    $setSym1 /. // macro setSym1 is deprecated. Use function setResult
                this.getParser().setSym1./
    $setResult /. // macro setResult is deprecated. Use function setResult
                 dtParsergetParser().setSym1./
    $getSym /. // macro getSym is deprecated. Use function getRhsSym
              dtParsergetParser().getSym./
    $getToken /. // macro getToken is deprecated. Use function getRhsTokenIndex
                dtParsergetParser().getToken./
    $getIToken /. // macro getIToken is deprecated. Use function getRhsIToken
                 this.prsStream.getIToken./
    $getLeftSpan /. // macro getLeftSpan is deprecated. Use function getLeftSpan
                   dtParsergetParser().getFirstToken./
    $getRightSpan /. // macro getRightSpan is deprecated. Use function getRightSpan
                    dtParsergetParser().getLastToken./
%End

%Globals
    /.
    
    ./
%End

%Headers
    /.
    public class $action_type extends $super_class implements RuleAction$additional_interfaces
    {
        private PrsStream this.prsStream = null;
        
        private boolean unimplementedSymbolsWarning = $unimplemented_symbols_warning;

        private static  prsTable  : ParseTable= new $prs_type();
        public  getParseTable() : ParseTable{ return $action_type.prsTable; }

        private  dtParser : DeterministicParser = null;
        public  getParser() : DeterministicParser{ return this.dtParser; }

        private  setResult(object1 : any ) :void{ this.dtParser.setSym1(object1); }
        public  getRhsSym(i : number) : any { return this.dtParser.getSym(i); }

        public  getRhsTokenIndex(i : number) : number { return this.dtParser.getToken(i); }
        public  getRhsIToken(i : number) : IToken { return this.prsStream.getIToken(getRhsTokenIndex(i)); }
        
        public  getRhsFirstTokenIndex(i : number) : number{ return this.dtParser.getFirstToken(i); }
        public  getRhsFirstIToken(i : number)  : IToken{ return this.prsStream.getIToken(getRhsFirstTokenIndex(i)); }

        public  getRhsLastTokenIndex(i : number) : number{ return this.dtParser.getLastToken(i); }
        public  getRhsLastIToken(i : number)  : IToken{ return this.prsStream.getIToken(getRhsLastTokenIndex(i)); }

        public  getLeftSpan() : number{ return this.dtParser.getFirstToken(); }
        public  getLeftIToken() : IToken { return this.prsStream.getIToken(getLeftSpan()); }

        public  getRightSpan() : number { return this.dtParser.getLastToken(); }
        public  getRightIToken() : IToken { return this.prsStream.getIToken(getRightSpan()); }

        public  getRhsErrorTokenIndex(i : number) : number
        {
            let index = this.dtParser.getToken(i);
            let err = this.prsStream.getIToken(index);
            return (err instanceof ErrorToken ? index : 0);
        }
        public  getRhsErrorIToken(i : number) : ErrorToken
        {
            let index = this.dtParser.getToken(i);
            let err = this.prsStream.getIToken(index);
            return <ErrorToken> (err instanceof ErrorToken ? err : null);
        }

        public void reset(lexStream : ILexStream)
        {
            this.prsStream = new PrsStream(lexStream);
            this.dtParser.reset(this.prsStream);

            try
            {
                this.prsStream.remapTerminalSymbols(orderedTerminalSymbols(), $action_type.prsTable.getEoftSymbol());
            }
            catch(NullExportedSymbolsException e) {
            }
            catch(NullTerminalSymbolsException e) {
            }
            catch(UnimplementedTerminalsException e)
            {
                if (unimplementedSymbolsWarning) {
                    java.util.ArrayList unimplemented_symbols = e.getSymbols();
                    Java.system.out.println("The Lexer will not scan the following token(s):");
                    for (i : number = 0; i < unimplemented_symbols.length; i++)
                    {
                        Integer id = (Integer) unimplemented_symbols.get(i);
                        Java.system.out.println("    " + $sym_type.orderedTerminalSymbols[id.intValue()]);               
                    }
                    Java.system.out.println();
                }
            }
            catch(UndefinedEofSymbolException e)
            {
                throw (new UndefinedEofSymbolException
                                    ("The Lexer does not implement the Eof symbol " +
                                     $sym_type.orderedTerminalSymbols[$action_type.prsTable.getEoftSymbol()]));
            }
        }
        
       constructor(lexStream? :ILexStream)
        {
            try
            {
                this.dtParser = new DeterministicParser(this.prsStream, $action_type.prsTable, (RuleAction) this);
            }
            catch (NotDeterministicParseTableException e)
            {
                throw (new NotDeterministicParseTableException
                                    ("Regenerate $prs_type.ts with -NOBACKTRACK option"));
            }
            catch (BadParseSymFileException e)
            {
                throw (new BadParseSymFileException("Bad Parser Symbol File -- $sym_type.ts. Regenerate $prs_type.ts"));
            }
            if(lexStream){
              this.reset(lexStream);
            }
        }

      

        public  numTokenKinds() : number{ return $sym_type.numTokenKinds; }
        public  orderedTerminalSymbols()  : string[] { return $sym_type.orderedTerminalSymbols; }
        public  getTokenKindName(number kind) : string{ return $sym_type.orderedTerminalSymbols[kind]; }            
        public  getEOFTokenKind() : number{ return $action_type.prsTable.getEoftSymbol(); }
        public  getIPrsStream()  : IPrsStream{ return this.prsStream; }

        /**
         * @deprecated replaced by {@link #getIPrsStream()}
         *
         */
        public  getPrsStream() : PrsStream{ return this.prsStream; }

        /**
         * @deprecated replaced by {@link #getIPrsStream()}
         *
         */
        public  getParseStream() : PrsStream{ return this.prsStream; }

        public parser(error_repair_count : number = 0 ,  monitor : Monitor = null) :  $ast_class
        {
            this.dtParser.setMonitor(monitor);

            try
            {
                return <$ast_class> this.dtParser.parse();
            }
            catch (BadParseException e)
            {
                this.prsStream.reset(e.error_token); // point to error token

                let diagnoseParser = new DiagnoseParser(this.prsStream, $action_type.prsTable);
                diagnoseParser.diagnose(e.error_token);
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
