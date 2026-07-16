from minimalsym import  minimalsym  
class minimalprs(minimalsym):
    ERROR_SYMBOL : int = 0
    def getErrorSymbol(self) -> int: return self.ERROR_SYMBOL

    SCOPE_UBOUND : int = 0
    def getScopeUbound(self) -> int: return self.SCOPE_UBOUND

    SCOPE_SIZE : int = 0
    def getScopeSize(self) -> int: return self.SCOPE_SIZE

    MAX_NAME_LENGTH : int = 0
    def getMaxNameLength(self) -> int: return self.MAX_NAME_LENGTH

    NUM_STATES : int = 2
    def getNumStates(self) -> int: return self.NUM_STATES

    NT_OFFSET : int = 2
    def getNtOffset(self) -> int: return self.NT_OFFSET

    LA_STATE_OFFSET : int = 8
    def getLaStateOffset(self) -> int: return self.LA_STATE_OFFSET

    MAX_LA : int = 0
    def getMaxLa(self) -> int: return self.MAX_LA

    NUM_RULES : int = 1
    def getNumRules(self) -> int: return self.NUM_RULES

    NUM_NONTERMINALS : int = 2
    def getNumNonterminals(self) -> int: return self.NUM_NONTERMINALS

    NUM_SYMBOLS : int = 4
    def getNumSymbols(self) -> int: return self.NUM_SYMBOLS

    START_STATE : int = 2
    def getStartState(self) -> int: return self.START_STATE

    IDENTIFIER_SYMBOL : int = 0
    def getIdentifier_SYMBOL(self) -> int: return self.IDENTIFIER_SYMBOL

    EOFT_SYMBOL : int = 2
    def getEoftSymbol(self) -> int: return self.EOFT_SYMBOL

    EOLT_SYMBOL : int = 3
    def getEoltSymbol(self) -> int: return self.EOLT_SYMBOL

    ACCEPT_ACTION : int = 6
    def getAcceptAction(self) -> int: return self.ACCEPT_ACTION

    ERROR_ACTION : int = 7
    def getErrorAction(self) -> int: return self.ERROR_ACTION

    BACKTRACK : bool = False
    def getBacktrack(self) ->bool: return self.BACKTRACK

    def getStartSymbol(self) -> int: return self.lhs(0)
    def isValidForParser(self) -> bool :  return minimalsym.isValidForParser


    _isNullable : list = [0,
            0,0,0,0
        ]
    def isNullable(self, index: int) ->bool : return  minimalprs._isNullable[index] != 0

    _prosthesesIndex : list = [0,
            2,1
        ]
    def prosthesesIndex(self, index: int) ->int : return  minimalprs._prosthesesIndex[index]

    _isKeyword : list = [0,
            0,0
        ]
    def isKeyword(self, index: int) ->bool : return  minimalprs._isKeyword[index] != 0

    _baseCheck : list = [0,
            1
        ]
    def baseCheck(self, index: int) ->int : return  minimalprs._baseCheck[index]
    _rhs : list = _baseCheck
    def rhs(self, index: int)  -> int: return minimalprs._rhs[index]

    _baseAction : list = [
            1,1,1,4,3,7,7
        ]
    def baseAction(self, index: int) ->int : return  minimalprs._baseAction[index]
    _lhs : list = _baseAction
    def lhs(self, index: int)  -> int: return minimalprs._lhs[index]

    _termCheck : list = [0,
            0,1,0,0,2
        ]
    def termCheck(self, index: int) ->int : return  minimalprs._termCheck[index]

    _termAction : list = [0,
            7,8,7,7,6
        ]
    def termAction(self, index: int) ->int : return  minimalprs._termAction[index]
    def asb(self, index: int) -> int: return 0
    def asr(self, index: int) -> int: return 0
    def nasb(self, index: int)  -> int: return 0
    def nasr(self, index: int)  -> int: return 0
    def terminalIndex(self, index: int) -> int: return 0
    def nonterminalIndex(self, index: int)  -> int: return 0
    def scopePrefix(self, index: int)  -> int: return 0
    def scopeSuffix(self, index: int) -> int: return 0
    def scopeLhs(self, index: int)  -> int: return 0
    def scopeLa(self, index: int) -> int: return 0
    def scopeStateSet(self, index: int)  -> int: return 0
    def scopeRhs(self, index: int)  -> int: return 0
    def scopeState(self, index: int) -> int: return 0
    def inSymb(self, index: int)  -> int: return 0
    def name(self, index: int)  -> str: return ""
    def originalState(self, state : int) -> int: return 0
    def asi(self, state : int) -> int: return 0
    def nasi(self, state : int ) -> int: return 0
    def inSymbol(self, state : int)  -> int: return 0

    #/**
     # assert(! goto_default)
     #/
    def ntAction(self, state : int,  sym : int) -> int:
        return minimalprs._baseAction[state + sym]
    

    #/**
     #* assert(! shift_default)
     #*/
    def tAction(self, state : int,  sym : int)-> int:
        i = minimalprs._baseAction[state]
        k = i + sym
        return minimalprs._termAction[  k if minimalprs._termCheck[k] == sym else i]
    
    def lookAhead(self, la_state : int , sym: int)-> int:
        k = la_state + sym
        return minimalprs._termAction[  k if minimalprs._termCheck[k] == sym else  la_state]
    

