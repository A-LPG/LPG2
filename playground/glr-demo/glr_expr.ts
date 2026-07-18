
    //#line 139 "glrParserTemplateF.gi

import {BadParseException, RuleAction, PrsStream, ParseTable, GLRParser, BacktrackingParser, IToken, ErrorToken, ILexStream, NullExportedSymbolsException, 
UnimplementedTerminalsException, Lpg, UndefinedEofSymbolException, NotGLRParseTableException, BadParseSymFileException, 
IPrsStream, Monitor, DiagnoseParser, IAst, IAstVisitor, IAbstractArrayList, NotDeterministicParseTableException,
DeterministicParser, NullTerminalSymbolsException } from "lpg2ts";
import { glr_exprprs } from ".\/glr_exprprs";
import { glr_exprsym } from ".\/glr_exprsym";

    //#line 150 "glrParserTemplateF.gi

export class glr_expr extends Object implements RuleAction
{
    private  prsStream  : PrsStream = new PrsStream();
    
    private  unimplementedSymbolsWarning : boolean = false;

    private static  prsTable : ParseTable = new glr_exprprs();
    public  getParseTable() : ParseTable { return glr_expr.prsTable; }

    private  glrParser : GLRParser ;
    public  getParser() : GLRParser{ return this.glrParser; }

    // During GLR→BT recover fallback, rule actions must read BT stacks.
    private  recoverParser : BacktrackingParser | null = null;
    public  setRecoverParser(parser : BacktrackingParser | null) : void { this.recoverParser = parser; }
    public  getRecoverParser() : BacktrackingParser | null { return this.recoverParser; }

    private  setResult(object1 : any) : void{
        if (this.recoverParser != null) this.recoverParser.setSym1(object1);
        else this.glrParser.setSym1(object1);
    }
    public  getRhsSym(i : number) : any{
        return this.recoverParser != null ? this.recoverParser.getSym(i) : this.glrParser.getSym(i);
    }

    public  getRhsTokenIndex(i : number) : number{
        return this.recoverParser != null ? this.recoverParser.getToken(i) : this.glrParser.getToken(i);
    }
    public  getRhsIToken(i : number) : IToken { return this.prsStream.getIToken(this.getRhsTokenIndex(i)); }
    
    public  getRhsFirstTokenIndex(i : number) : number {
        return this.recoverParser != null ? this.recoverParser.getFirstToken(i) : this.glrParser.getFirstToken(i);
    }
    public  getRhsFirstIToken(i : number) : IToken{ return this.prsStream.getIToken(this.getRhsFirstTokenIndex(i)); }

    public  getRhsLastTokenIndex(i : number):number {
        return this.recoverParser != null ? this.recoverParser.getLastToken(i) : this.glrParser.getLastToken(i);
    }
    public  getRhsLastIToken(i : number):IToken { return this.prsStream.getIToken(this.getRhsLastTokenIndex(i)); }

    public getLeftSpan() :number {
        return this.recoverParser != null ? this.recoverParser.getFirstToken() : this.glrParser.getFirstToken();
    }
    public  getLeftIToken() :IToken { return this.prsStream.getIToken(this.getLeftSpan()); }

    public getRightSpan() : number {
        return this.recoverParser != null ? this.recoverParser.getLastToken() : this.glrParser.getLastToken();
    }
    public  getRightIToken() : IToken { return this.prsStream.getIToken(this.getRightSpan()); }

    public  getRhsErrorTokenIndex(i : number) : number
    {
        let index = this.getRhsTokenIndex(i);
        let err = this.prsStream.getIToken(index);
        return (err instanceof ErrorToken ? index : 0);
    }
    public  getRhsErrorIToken(i : number) : ErrorToken
    {
        let index = this.getRhsTokenIndex(i);
        let err = this.prsStream.getIToken(index);
        return <ErrorToken> (err instanceof ErrorToken ? err : null);
    }

    public  reset(lexStream : ILexStream) : void
    {
        this.prsStream.resetLexStream(lexStream);
        this.glrParser.reset(this.prsStream);

        try
        {
            this.prsStream.remapTerminalSymbols(this.orderedTerminalSymbols(), glr_expr.prsTable.getEoftSymbol());
        } 
        catch (e)
        {     
            if( e instanceof NullExportedSymbolsException){
                
            }
            else if( e instanceof NullTerminalSymbolsException){
                
            }
            else if( e instanceof UnimplementedTerminalsException){
                if (this.unimplementedSymbolsWarning) {
                    let unimplemented_symbols = e.getSymbols();
                    Lpg.Lang.System.Out.println("The Lexer will not scan the following token(s):");
                    for (let i : number = 0; i < unimplemented_symbols.size(); i++)
                    {
                        let id = <number>unimplemented_symbols.get(i);
                        Lpg.Lang.System.Out.println("    " + glr_exprsym.orderedTerminalSymbols[id]);               
                    }
                    Lpg.Lang.System.Out.println();
                }
            }
            else if( e instanceof UndefinedEofSymbolException){
                throw  (new UndefinedEofSymbolException
                    ("The Lexer does not implement the Eof symbol " +
                    glr_exprsym.orderedTerminalSymbols[glr_expr.prsTable.getEoftSymbol()]));
            }

        }
    }
    
    constructor(lexStream? :ILexStream)
    {
        super();
        try
        {
            this.glrParser = new GLRParser(null, glr_expr.prsTable, <RuleAction> this);
        }
        catch (e)
        {
            if(e instanceof NotGLRParseTableException)
            throw (new NotGLRParseTableException
                                ("Regenerate glr_exprprs.ts with -GLR option"));
            else if(e instanceof BadParseSymFileException){
                throw (new BadParseSymFileException("Bad Parser Symbol File -- glr_exprsym.ts"));
            }
            else{
                throw e;
            }
        }
        if(lexStream){
          this.reset(lexStream);
        }
    }
    
   
    
    public  numTokenKinds() :number { return glr_exprsym.numTokenKinds; }
    public  orderedTerminalSymbols()  : string[] { return glr_exprsym.orderedTerminalSymbols; }
    public  getTokenKindName(kind : number ) : string { return glr_exprsym.orderedTerminalSymbols[kind]; }
    public  getEOFTokenKind() : number{ return glr_expr.prsTable.getEoftSymbol(); }
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

 

    // error_repair_count>0: GLR failure falls back to BacktrackingParser
    // fuzzyParse (Recover prosthesis); Diagnose is last resort.
    public parser(error_repair_count : number = 0 ,  monitor? : Monitor) :  Ast | null
    {
        this.glrParser.setMonitor(monitor);
        
        try
        {
            return <Ast> this.glrParser.parse(error_repair_count);
        }
        catch (ex)
        {
           if( ex instanceof BadParseException ){
                 let e = <BadParseException>(ex);
                this.prsStream.reset(e.error_token); // point to error token

                let diagnoseParser = new DiagnoseParser(this.prsStream, glr_expr.prsTable);
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
    

    //#line 334 "glrParserTemplateF.gi

    
    public  ruleAction(ruleNumber : number) : void
    {
        switch (ruleNumber)
        {

            //
            // Rule 1:  E ::= E PLUS E
            //
            case 1: {
               //#line 18 "glr_expr.g"
                this.setResult(
                    //#line 18 glr_expr.g
                    new E(this.getLeftIToken(), this.getRightIToken(),
                          //#line 18 glr_expr.g
                          <E>this.getRhsSym(1),
                          //#line 18 glr_expr.g
                          <E>this.getRhsSym(3))
                //#line 18 glr_expr.g
                );
            break;
            }
            //
            // Rule 2:  E ::= NUMBER
            //
            case 2: {
               //#line 19 "glr_expr.g"
                this.setResult(
                    //#line 19 glr_expr.g
                    new E(this.getLeftIToken(), this.getRightIToken(),
                          //#line 19 glr_expr.g
                          null,
                          //#line 19 glr_expr.g
                          null)
                //#line 19 glr_expr.g
                );
            break;
            }
    //#line 338 "glrParserTemplateF.gi

    
            default:
                break;
        }
        return;
    }
}
    export abstract class Ast implements IAst
    {
        private nextAst : IAst | null = null;
        public getNextAst() : IAst  | null{ return this.nextAst; }
        public setNextAst(n : IAst) : void{ this.nextAst = n; }
        public resetNextAst() : void { this.nextAst = null; }
        protected leftIToken : IToken ;
        protected rightIToken: IToken ;
        public getParent(): IAst | null 
        {
            throw new Error("noparent-saved option in effect");
        }
        public getRuleIndex(): number { return 0; }

        public getLeftIToken() : IToken { return this.leftIToken; }
        public getRightIToken() : IToken { return this.rightIToken; }
        public getPrecedingAdjuncts() : IToken[] { return this.leftIToken.getPrecedingAdjuncts(); }
        public getFollowingAdjuncts() : IToken[] { return this.rightIToken.getFollowingAdjuncts(); }

        public  toString() : string 
        {
            let str = this.leftIToken.getILexStream()?.toString(this.leftIToken.getStartOffset(), this.rightIToken.getEndOffset());
            return str? str : "";
        }

    constructor(leftIToken : IToken , rightIToken? : IToken )
        {
            this.leftIToken = leftIToken;
            if(rightIToken) this.rightIToken = rightIToken;
            else            this.rightIToken = leftIToken;
        }

      public   initialize() : void {}

        public  getChildren() : Lpg.Util.ArrayList<IAst>
        {
            throw new Error("noparent-saved option in effect");
        }
         public   getAllChildren() : Lpg.Util.ArrayList<IAst> { return this.getChildren(); }

        public abstract acceptWithVisitor(v : Visitor) : void;
        public abstract  acceptWithArg(v : ArgumentVisitor, o : any) : void;
        public abstract acceptWithResult(v : ResultVisitor) : any;
        public abstract acceptWithResultArgument(v : ResultArgumentVisitor, o : any) : any;
        public  accept(v : IAstVisitor ) : void {}
    }

    export abstract class AbstractAstList extends Ast implements IAbstractArrayList<Ast>
    {
        private leftRecursive : boolean ;
        public  list  = new Lpg.Util.ArrayList<Ast>();
        public  size() : number { return this.list.size(); }
        public   getList() : Lpg.Util.ArrayList<Ast    > { return this.list; }
        public  getElementAt(i : number ) : Ast { return <Ast> this.list.get(this.leftRecursive ? i : this.list.size() - 1 - i); }
        public  getArrayList() : Lpg.Util.ArrayList<Ast>
        {
            if (! this.leftRecursive) // reverse the list 
            {
                for (let i = 0, n = this.list.size() - 1; i < n; i++, n--)
                {
                    let ith = this.list.get(i),
                           nth = this.list.get(n);
                    this.list.set(i, nth);
                    this.list.set(n, ith);
                }
                this.leftRecursive = true;
            }
            return this.list;
        }
        /**
         * @deprecated replaced by {@link #addElement()}
         *
         */
        public  add(element : Ast) : boolean
        {
            this.addElement(element);
            return true;
        }

        public  addElement(element : Ast) : void
        {
            this.list.add(element);
            if (this.leftRecursive)
                 this.rightIToken = element.getRightIToken();
            else this.leftIToken = element.getLeftIToken();
        }

        constructor(leftToken : IToken, rightToken : IToken , leftRecursive : boolean )        {
              super(leftToken, rightToken);
              this.leftRecursive = leftRecursive;
        }

    }

    export class AstToken extends Ast implements IAstToken
    {
        constructor(token : IToken ) { super(token); }
        public  getIToken() : IToken{ return this.leftIToken; }
        public  toString() : string  { return this.leftIToken.toString(); }


        public  acceptWithVisitor(v : Visitor) : void{ v.visitAstToken(this); }
        public  acceptWithArg(v : ArgumentVisitor, o : any) : void { v.visitAstToken(this, o); }
        public  acceptWithResult(v : ResultVisitor) : any{ return v.visitAstToken(this); }
        public   acceptWithResultArgument(v : ResultArgumentVisitor, o : any) : any { return v.visitAstToken(this, o); }
    }

    export interface IRootForglr_expr
    {
         getLeftIToken() : IToken;
         getRightIToken() : IToken;

        acceptWithVisitor(v : Visitor) : void;
         acceptWithArg(v : ArgumentVisitor, o : any) : void;
        acceptWithResult(v : ResultVisitor) : any;
        acceptWithResultArgument(v : ResultArgumentVisitor, o : any) : any;
    }

    /**
     * is always implemented by <b>AstToken</b>. It is also implemented by <b>E</b>
     */
    export interface IAstToken extends IRootForglr_expr    {
    }

    /**
     * is implemented by <b>E</b>
     */
    export interface IE extends IAstToken {}

    /**
     *<b>
    *<li>Rule 1:  E ::= E PLUS E
    *<li>Rule 2:  E ::= NUMBER
     *</b>
     */
    export class E extends Ast implements IE
    {
        private  _E : E | null;
        private  _E3 : E | null;

        /**
         * The value returned by <b>getE</b> may be <b>null</b>
         */
        public  getE() : E | null { return this._E; }
        /**
         * The value returned by <b>getE3</b> may be <b>null</b>
         */
        public  getE3() : E | null { return this._E3; }

        constructor(leftIToken : IToken , rightIToken : IToken ,
                     _E : E | null,
                     _E3 : E | null)
        {
            super(leftIToken, rightIToken);

            this._E = _E;
            this._E3 = _E3;
            this.initialize();
        }

        public  acceptWithVisitor(v : Visitor) : void{ v.visitE(this); }
        public  acceptWithArg(v : ArgumentVisitor, o : any) : void { v.visitE(this, o); }
        public  acceptWithResult(v : ResultVisitor) : any{ return v.visitE(this); }
        public   acceptWithResultArgument(v : ResultArgumentVisitor, o : any) : any { return v.visitE(this, o); }
        public getRuleIndex(): number { return 2 ;}
    }

    export interface Visitor
    {
        visitAstToken(n : AstToken) : void;
        visitE(n : E) : void;

        visit(n : Ast) : void;
    }
    export interface ArgumentVisitor
    {
        visitAstToken(n : AstToken, o : any) : void;
        visitE(n : E, o : any) : void;

        visit(n : Ast, o : any) : void;
    }
    export interface ResultVisitor
    {
        visitAstToken(n : AstToken) : any;
        visitE(n : E) : any;

        visit(n : Ast) : any;
    }
    export interface ResultArgumentVisitor
    {
        visitAstToken(n : AstToken, o : any) : any;
        visitE(n : E, o : any) : any;

        visit(n : Ast, o : any) : any;
    }
    export abstract class AbstractVisitor implements Visitor,ArgumentVisitor
    {
        public abstract  unimplementedVisitor(s : string) : void;

        public  visitAstToken(n : AstToken, o? : any) : void { this.unimplementedVisitor("visitAstToken(AstToken, any)"); }

        public  visitE(n : E, o? : any) : void { this.unimplementedVisitor("visitE(E, any)"); }


        public  visit(n : Ast, o? : any) : void
        {
            if (n instanceof AstToken) this.visitAstToken(<AstToken> n, o);
            else if (n instanceof E) this.visitE(<E> n, o);
            else throw new Error("visit(" + n.toString() + ")");
        }
    }
    export abstract class AbstractResultVisitor implements ResultVisitor,ResultArgumentVisitor
    {
        public abstract  unimplementedVisitor(s : string) : any;

        public visitAstToken(n : AstToken, o? : any) : any{ return  this.unimplementedVisitor("visitAstToken(AstToken, any)"); }

        public visitE(n : E, o? : any) : any{ return  this.unimplementedVisitor("visitE(E, any)"); }


        public visit(n : Ast, o? : any) : any
        {
            if (n instanceof AstToken) return this.visitAstToken(<AstToken> n, o);
            else if (n instanceof E) return this.visitE(<E> n, o);
            else throw new Error("visit(" + n.toString() + ")");
        }
    }

