import { minimalsym } from "./minimalsym";
export class minimalprs {
    public   readonly ERROR_SYMBOL : number = 0;
    public   getErrorSymbol() : number { return this.ERROR_SYMBOL; }

    public   readonly SCOPE_UBOUND : number = 0;
    public   getScopeUbound() : number { return this.SCOPE_UBOUND; }

    public   readonly SCOPE_SIZE : number = 0;
    public   getScopeSize() : number { return this.SCOPE_SIZE; }

    public   readonly MAX_NAME_LENGTH : number = 0;
    public   getMaxNameLength() : number { return this.MAX_NAME_LENGTH; }

    public   readonly NUM_STATES : number = 2;
    public   getNumStates() : number { return this.NUM_STATES; }

    public   readonly NT_OFFSET : number = 2;
    public   getNtOffset() : number { return this.NT_OFFSET; }

    public   readonly LA_STATE_OFFSET : number = 8;
    public   getLaStateOffset() : number { return this.LA_STATE_OFFSET; }

    public   readonly MAX_LA : number = 0;
    public   getMaxLa() : number { return this.MAX_LA; }

    public   readonly NUM_RULES : number = 1;
    public   getNumRules() : number { return this.NUM_RULES; }

    public   readonly NUM_NONTERMINALS : number = 2;
    public   getNumNonterminals() : number { return this.NUM_NONTERMINALS; }

    public   readonly NUM_SYMBOLS : number = 4;
    public   getNumSymbols() : number { return this.NUM_SYMBOLS; }

    public   readonly START_STATE : number = 2;
    public   getStartState() : number { return this.START_STATE; }

    public   readonly IDENTIFIER_SYMBOL : number = 0;
    public   getIdentifier_SYMBOL() : number { return this.IDENTIFIER_SYMBOL; }

    public   readonly EOFT_SYMBOL : number = 2;
    public   getEoftSymbol() : number { return this.EOFT_SYMBOL; }

    public   readonly EOLT_SYMBOL : number = 3;
    public   getEoltSymbol() : number { return this.EOLT_SYMBOL; }

    public   readonly ACCEPT_ACTION : number = 6;
    public   getAcceptAction() : number { return this.ACCEPT_ACTION; }

    public   readonly ERROR_ACTION : number = 7;
    public   getErrorAction() : number { return this.ERROR_ACTION; }

    public readonly  BACKTRACK : boolean = false;
    public  getBacktrack() :boolean { return this.BACKTRACK; }

    public    getStartSymbol() : number { return this.lhs(0); }
    public   isValidForParser() : boolean { return minimalsym.isValidForParser; }


    public static  _isNullable : number[] = [0,
            0,0,0,0
        ];
    public   isNullable(index : number) : boolean { return  minimalprs._isNullable[index] != 0; }

    public static  _prosthesesIndex : number[] = [0,
            2,1
        ];
    public   prosthesesIndex(index : number) : number { return  minimalprs._prosthesesIndex[index]; }

    public static  _isKeyword : number[] = [0,
            0,0
        ];
    public   isKeyword(index : number) : boolean { return  minimalprs._isKeyword[index] != 0; }

    public static  _baseCheck : number[] = [0,
            1
        ];
    public   baseCheck(index : number) : number { return  minimalprs._baseCheck[index]; }
    public static  _rhs : number [] = minimalprs._baseCheck;
    public    rhs(index : number)  : number { return minimalprs._rhs[index]; }

    public static  _baseAction : number[] = [
            1,1,1,4,3,7,7
        ];
    public   baseAction(index : number) : number { return  minimalprs._baseAction[index]; }
    public static   _lhs : number []  = minimalprs._baseAction;
    public   lhs(index : number)  : number { return minimalprs._lhs[index]; }

    public static  _termCheck : number[] = [0,
            0,1,0,0,2
        ];
    public   termCheck(index : number) : number { return  minimalprs._termCheck[index]; }

    public static  _termAction : number[] = [0,
            7,8,7,7,6
        ];
    public   termAction(index : number) : number { return  minimalprs._termAction[index]; }
    public    asb(index : number) : number { return 0; }
    public    asr(index : number) : number { return 0; }
    public    nasb(index : number)  : number{ return 0; }
    public    nasr(index : number)  : number{ return 0; }
    public    terminalIndex(index : number) : number { return 0; }
    public    nonterminalIndex(index : number)  : number{ return 0; }
    public    scopePrefix(index : number)  : number{ return 0;}
    public    scopeSuffix(index : number) : number { return 0;}
    public    scopeLhs(index : number)  : number{ return 0;}
    public    scopeLa(index : number) : number { return 0;}
    public    scopeStateSet(index : number)  : number{ return 0;}
    public    scopeRhs(index : number)  : number{ return 0;}
    public    scopeState(index : number) : number { return 0;}
    public    inSymb(index : number)  : number{ return 0;}
    public    name(index : number)  : string{ return ""; }
    public   originalState(state : number): number { return 0; }
    public  asi(state : number ): number { return 0; }
    public    nasi(state : number ) : number { return 0; }
    public    inSymbol(state : number)  : number { return 0; }

    /**
     * assert(! goto_default);
     */
    public    ntAction(state : number,  sym : number) : number {
        return minimalprs._baseAction[state + sym];
    }

    /**
     * assert(! shift_default);
     */
    public    tAction(state : number,  sym : number): number {
        let i = minimalprs._baseAction[state],
            k = i + sym;
        return minimalprs._termAction[ minimalprs._termCheck[k] == sym ? k : i];
    }
    public    lookAhead( la_state : number ,  sym : number): number {
        let k = la_state + sym;
        return minimalprs._termAction[ minimalprs._termCheck[k] == sym ? k : la_state];
    }
}
