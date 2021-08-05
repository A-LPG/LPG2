--
-- An LPG Parser Template Using lpg.jar
--
-- Macros that may be redefined in an instance of this template
--
--     %additional_interfaces
--     %super_stream_class
--
-- B E G I N N I N G   O F   T E M P L A T E   dtUnifiedTemplateF
--
%Options programming_Language=typescript
%Options table
%Options margin=4
%Options prefix=Char_
%Options action-block=("*.ts", "/.", "./")
%Options ParseTable=ParseTable

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
                // Rule %rule_number:  %rule_text
                //
                ./

    $BeginAction
    /.%Header%case %rule_number: {./

    $EndAction
    /.          break;
                }./

    $BeginJava
    /.%BeginAction
                    %symbol_declarations./

    $EndJava /.%EndAction./

    $NoAction
    /.%Header%case %rule_number:
                    break;./

    $NullAction
    /.%Header%case %rule_number:
                    %setResult(null);
                    break;./

    $BeginActions
    /.
        public void ruleAction(ruleNumber : number )
        {
            switch (ruleNumber)
            {./

    $SplitActions
    /.
	            default:
	                ruleAction%rule_number(ruleNumber);
	                break;
	        }
	        return;
	    }
	
	    public void ruleAction%rule_number(ruleNumber : number )
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
    
    ./
%End

%Headers
    /.
    public class %action_type extends %super_stream_class implements %sym_type, RuleAction%additional_interfaces
    {
        private static  prs : ParseTable = new %prs_type();
        private  this.dtParser : DeterministicParser;

        private void setResult(object1 : any ) { this.dtParser.setSym1(object1); }
        public  getParser()  : DeterministicParser { return this.dtParser; }
        public  getRhsSym(i : number) : any { return this.dtParser.getSym(i); }
        public  getRhsTokenIndex(i : number) : number { return this.dtParser.getToken(i); }
        public  getRhsFirstTokenIndex(i : number) : number { return this.dtParser.getFirstToken(i); }
        public  getRhsLastTokenIndex(i : number) : number { return this.dtParser.getLastToken(i); }

        public  getLeftSpan() : number{ return this.dtParser.getFirstToken(); }
        public  getRightSpan() : number{ return this.dtParser.getLastToken(); }
 
       constructor(string filename, number tab)
        {
            super(filename,null, tab);
        }

        public  orderedExportedSymbols() : string[]{ return this.orderedTerminalSymbols; }
        public  getEOFTokenKind() : number { return %prs_type.EOFT_SYMBOL; }

        public  getILexStream() : ILexStream{ return <%super_stream_class> this; }
        
        /**
         * @deprecated replaced by {@link #getILexStream()}
         */
        public  getLexStream()  : ILexStream{ return <%super_stream_class> this; }

    
         public parser(error_repair_count : number = 0 ,  monitor : Monitor = null) :  %ast_class
        {
            try
            {
                this.dtParser = new DeterministicParser(this, prs, this);
            }
            catch (NotDeterministicParseTableException e)
            {
                Java.system.out.println("****Error: Regenerate %prs_type.ts with -NOBACKTRACK option");
                process.exit(1);
            }
            catch (BadParseSymFileException e)
            {
                Java.system.out.println("****Error: Bad Parser Symbol File -- %sym_type.ts. Regenerate %prs_type.ts");
                process.exit(1);
            }
            this.dtParser.setMonitor(monitor);

            try
            {
                return <%ast_type> this.dtParser.parse();
            }
            catch (BadParseException e)
            {
                reset(e.error_token); // point to error token

                Java.system.out.print("Error detected on character " + e.error_token);
                if (e.error_token < getStreamLength())
                     Java.system.out.print(" at line " + getLine(e.error_token) + ", column " + this.getColumn(e.error_token));
                else Java.system.out.print(" at end of file ");
                Java.system.out.println(" with kind " + getKind(e.error_token));
            }

            return null;
        }

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
