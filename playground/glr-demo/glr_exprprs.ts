import { glr_exprsym } from "./glr_exprsym";
import { ParseTable } from "lpg2ts";
export class glr_exprprs implements ParseTable {
    public   readonly ERROR_SYMBOL : number = 4;
    public   getErrorSymbol() : number { return this.ERROR_SYMBOL; }

    public   readonly SCOPE_UBOUND : number = -1;
    public   getScopeUbound() : number { return this.SCOPE_UBOUND; }

    public   readonly SCOPE_SIZE : number = 0;
    public   getScopeSize() : number { return this.SCOPE_SIZE; }

    public   readonly MAX_NAME_LENGTH : number = 11;
    public   getMaxNameLength() : number { return this.MAX_NAME_LENGTH; }

    public   readonly NUM_STATES : number = 4;
    public   getNumStates() : number { return this.NUM_STATES; }

    public   readonly NT_OFFSET : number = 4;
    public   getNtOffset() : number { return this.NT_OFFSET; }

    public   readonly LA_STATE_OFFSET : number = 16;
    public   getLaStateOffset() : number { return this.LA_STATE_OFFSET; }

    public   readonly MAX_LA : number = 1;
    public   getMaxLa() : number { return this.MAX_LA; }

    public   readonly NUM_RULES : number = 2;
    public   getNumRules() : number { return this.NUM_RULES; }

    public   readonly NUM_NONTERMINALS : number = 2;
    public   getNumNonterminals() : number { return this.NUM_NONTERMINALS; }

    public   readonly NUM_SYMBOLS : number = 6;
    public   getNumSymbols() : number { return this.NUM_SYMBOLS; }

    public   readonly START_STATE : number = 3;
    public   getStartState() : number { return this.START_STATE; }

    public   readonly IDENTIFIER_SYMBOL : number = 0;
    public   getIdentifier_SYMBOL() : number { return this.IDENTIFIER_SYMBOL; }

    public   readonly EOFT_SYMBOL : number = 3;
    public   getEoftSymbol() : number { return this.EOFT_SYMBOL; }

    public   readonly EOLT_SYMBOL : number = 3;
    public   getEoltSymbol() : number { return this.EOLT_SYMBOL; }

    public   readonly ACCEPT_ACTION : number = 10;
    public   getAcceptAction() : number { return this.ACCEPT_ACTION; }

    public   readonly ERROR_ACTION : number = 14;
    public   getErrorAction() : number { return this.ERROR_ACTION; }

    public readonly  BACKTRACK : boolean = true;
    public  getBacktrack() :boolean { return this.BACKTRACK; }

    public readonly  GLR : boolean = true;
    public  isGLR() :boolean { return this.GLR; }

    public    getStartSymbol() : number { return this.lhs(0); }
    public   isValidForParser() : boolean { return glr_exprsym.isValidForParser; }


    public static  _isNullable : number[] = [0,
            0,0,0,0,0,0
        ];
    public   isNullable(index : number) : boolean { return  glr_exprprs._isNullable[index] != 0; }

    public static  _prosthesesIndex : number[] = [0,
            2,1
        ];
    public   prosthesesIndex(index : number) : number { return  glr_exprprs._prosthesesIndex[index]; }

    public static  _isKeyword : number[] = [0,
            0,0,0,0
        ];
    public   isKeyword(index : number) : boolean { return  glr_exprprs._isKeyword[index] != 0; }

    public static  _baseCheck : number[] = [0,
            3,1,-1,1,-3,1,-2,-4,0,0
        ];
    public   baseCheck(index : number) : number { return  glr_exprprs._baseCheck[index]; }
    public static  _rhs : number [] = glr_exprprs._baseCheck;
    public    rhs(index : number)  : number { return glr_exprprs._rhs[index]; }

    public static  _baseAction : number[] = [
            1,1,1,3,7,3,8,1,6,14,
            0,5,1,0
        ];
    public   baseAction(index : number) : number { return  glr_exprprs._baseAction[index]; }
    public static   _lhs : number []  = glr_exprprs._baseAction;
    public   lhs(index : number)  : number { return glr_exprprs._lhs[index]; }

    public static  _termCheck : number[] = [0,
            0,1,0,3,2,0,1,0,0,0
        ];
    public   termCheck(index : number) : number { return  glr_exprprs._termCheck[index]; }

    public static  _termAction : number[] = [0,
            14,5,14,10,16,1,11
        ];
    public   termAction(index : number) : number { return  glr_exprprs._termAction[index]; }

    public static  _asb : number[] = [0,
            1,3,1,3
        ];
    public   asb(index : number) : number { return  glr_exprprs._asb[index]; }

    public static  _asr : number[] = [0,
            2,0,3,1,0
        ];
    public   asr(index : number) : number { return  glr_exprprs._asr[index]; }

    public static  _nasb : number[] = [0,
            1,2,1,2
        ];
    public   nasb(index : number) : number { return  glr_exprprs._nasb[index]; }

    public static  _nasr : number[] = [0,
            1,0
        ];
    public   nasr(index : number) : number { return  glr_exprprs._nasr[index]; }

    public static  _terminalIndex : number[] = [0,
            3,2,4,5
        ];
    public   terminalIndex(index : number) : number { return  glr_exprprs._terminalIndex[index]; }

    public static  _nonterminalIndex : number[] = [0,
            6,0
        ];
    public   nonterminalIndex(index : number) : number { return  glr_exprprs._nonterminalIndex[index]; }
    public static _scopePrefix : number[];
    public    scopePrefix(index : number) : number { return 0;}

    public static _scopeSuffix : number[];
    public    scopeSuffix(index : number): number { return 0;}

    public static _scopeLhs : number[];
    public    scopeLhs(index : number): number { return 0;}

    public static _scopeLa : number[];
    public    scopeLa(index : number): number { return 0;}

    public static  _scopeStateSet: number[] ;
    public    scopeStateSet(index : number) : number{ return 0;}

    public static  _scopeRhs: number[] ;
    public    scopeRhs(index : number): number { return 0;}

    public static _scopeState : number[];
    public    scopeState(index : number): number { return 0;}

    public static _inSymb : number[];
    public    inSymb(index : number) : number{ return 0;}


    public static  _name : string[] = [
            "",
            "%empty",
            "NUMBER",
            "PLUS",
            "EOF_TOKEN",
            "ERROR_TOKEN",
            "E"
        ];
    public   name(index : number):string { return glr_exprprs._name[index]; }

    public   originalState(state : number): number {
        return - glr_exprprs._baseCheck[state];
    }
    public    asi(state : number ): number {
        return glr_exprprs._asb[this.originalState(state)];
    }
    public   nasi(state : number ) : number {
        return glr_exprprs._nasb[this.originalState(state)];
    }
    public   inSymbol(state : number)  : number {
        return glr_exprprs._inSymb[this.originalState(state)];
    }

    /**
     * assert(! goto_default);
     */
    public    ntAction(state : number,  sym : number) : number {
        return glr_exprprs._baseAction[state + sym];
    }

    /**
     * assert(! shift_default);
     */
    public    tAction(state : number,  sym : number): number {
        let i = glr_exprprs._baseAction[state],
            k = i + sym;
        return glr_exprprs._termAction[ glr_exprprs._termCheck[k] == sym ? k : i];
    }
    public    lookAhead( la_state : number ,  sym : number): number {
        let k = la_state + sym;
        return glr_exprprs._termAction[ glr_exprprs._termCheck[k] == sym ? k : la_state];
    }
}
