type minimalprs struct{}
func Newminimalprs() *minimalprs{
    return &minimalprs{}
}
const minimalprs_ERROR_SYMBOL int = 0
func (my * minimalprs) GetErrorSymbol() int {
     return minimalprs_ERROR_SYMBOL
}
const minimalprs_SCOPE_UBOUND int = 0
func (my * minimalprs) GetScopeUbound() int {
     return minimalprs_SCOPE_UBOUND
}
const minimalprs_SCOPE_SIZE int = 0
func (my * minimalprs) GetScopeSize() int {
     return minimalprs_SCOPE_SIZE
}
const minimalprs_MAX_NAME_LENGTH int = 0
func (my * minimalprs) GetMaxNameLength() int {
     return minimalprs_MAX_NAME_LENGTH
}
const minimalprs_NUM_STATES int = 2
func (my * minimalprs) GetNumStates() int {
     return minimalprs_NUM_STATES
}
const minimalprs_NT_OFFSET int = 2
func (my * minimalprs) GetNtOffset() int {
     return minimalprs_NT_OFFSET
}
const minimalprs_LA_STATE_OFFSET int = 8
func (my * minimalprs) GetLaStateOffset() int {
     return minimalprs_LA_STATE_OFFSET
}
const minimalprs_MAX_LA int = 0
func (my * minimalprs) GetMaxLa() int {
     return minimalprs_MAX_LA
}
const minimalprs_NUM_RULES int = 1
func (my * minimalprs) GetNumRules() int {
     return minimalprs_NUM_RULES
}
const minimalprs_NUM_NONTERMINALS int = 2
func (my * minimalprs) GetNumNonterminals() int {
     return minimalprs_NUM_NONTERMINALS
}
const minimalprs_NUM_SYMBOLS int = 4
func (my * minimalprs) GetNumSymbols() int {
     return minimalprs_NUM_SYMBOLS
}
const minimalprs_START_STATE int = 2
func (my * minimalprs) GetStartState() int {
     return minimalprs_START_STATE
}
const minimalprs_IDENTIFIER_SYMBOL int = 0
func (my * minimalprs) getIdentifier_SYMBOL() int {
     return minimalprs_IDENTIFIER_SYMBOL
}
const minimalprs_EOFT_SYMBOL int = 2
func (my * minimalprs) GetEoftSymbol() int {
     return minimalprs_EOFT_SYMBOL
}
const minimalprs_EOLT_SYMBOL int = 3
func (my * minimalprs) GetEoltSymbol() int {
     return minimalprs_EOLT_SYMBOL
}
const minimalprs_ACCEPT_ACTION int = 6
func (my * minimalprs) GetAcceptAction() int {
     return minimalprs_ACCEPT_ACTION
}
const minimalprs_ERROR_ACTION int = 7
func (my * minimalprs) GetErrorAction() int {
     return minimalprs_ERROR_ACTION
}
const minimalprs_BACKTRACK bool = false
func (my * minimalprs) GetBacktrack() bool {
     return minimalprs_BACKTRACK
}
func (my * minimalprs) GetStartSymbol() int{
    return my.Lhs(0)
}
func (my * minimalprs) IsValidForParser() bool{
    return minimalsym.IsValidForParser
}

var  minimalprs_IsNullable []int=[]int{0,
            0,0,0,0,
}
func (my * minimalprs) IsNullable(index int)bool{
    return minimalprs_IsNullable[index] != 0
}

var  minimalprs_ProsthesesIndex []int=[]int{0,
            2,1,
}
func (my * minimalprs) ProsthesesIndex(index int)int{
    return minimalprs_ProsthesesIndex[index]
}

var  minimalprs_IsKeyword []int=[]int{0,
            0,0,
}
func (my * minimalprs) IsKeyword(index int)bool{
    return minimalprs_IsKeyword[index] != 0
}

var  minimalprs_BaseCheck []int=[]int{0,
            1,
}
func (my * minimalprs) BaseCheck(index int)int{
    return minimalprs_BaseCheck[index]
}
var minimalprs_Rhs  = minimalprs_BaseCheck
func (my * minimalprs) Rhs(index int) int{ return minimalprs_Rhs[index] }

var  minimalprs_BaseAction []int=[]int{
            1,1,1,4,3,7,7,
}
func (my * minimalprs) BaseAction(index int)int{
    return minimalprs_BaseAction[index]
}
var minimalprs_Lhs  = minimalprs_BaseAction
func (my * minimalprs) Lhs(index int) int{ return minimalprs_Lhs[index] }

var  minimalprs_TermCheck []int=[]int{0,
            0,1,0,0,2,
}
func (my * minimalprs) TermCheck(index int)int{
    return minimalprs_TermCheck[index]
}

var  minimalprs_TermAction []int=[]int{0,
            7,8,7,7,6,
}
func (my * minimalprs) TermAction(index int)int{
    return minimalprs_TermAction[index]
}
func (my * minimalprs) Asb(index int) int{ return 0 }
func (my * minimalprs) Asr(index int) int{ return 0 }
func (my * minimalprs) Nasb(index int) int{ return 0 }
func (my * minimalprs) Nasr(index int) int{ return 0 }
func (my * minimalprs) TerminalIndex(index int) int{ return 0 }
func (my * minimalprs) NonterminalIndex(index int) int{ return 0 }
func (my * minimalprs) ScopePrefix(index int) int{ return 0 }
func (my * minimalprs) ScopeSuffix(index int) int{ return 0 }
func (my * minimalprs) ScopeLhs(index int) int{ return 0 }
func (my * minimalprs) ScopeLa(index int) int{ return 0 }
func (my * minimalprs) ScopeStateSet(index int) int{ return 0 }
func (my * minimalprs) ScopeRhs(index int) int{ return 0 }
func (my * minimalprs) ScopeState(index int) int{ return 0 }
func (my * minimalprs) InSymb(index int) int{ return 0 }
func (my * minimalprs) Name(index int)   string{ return "" }
func (my * minimalprs) OriginalState(state int) int{
    return 0
}
func (my * minimalprs) Asi(state int) int{
    return 0
}
func (my * minimalprs) Nasi(state int ) int{
    return 0
}
func (my * minimalprs) InSymbol(state int) int{
    return 0
}

    /**
     * assert(! goto_default);
     */
    func (my * minimalprs) NtAction(state int,  sym int) int{
        return minimalprs_BaseAction[state + sym]
    }

    /**
     * assert(! shift_default);
     */
    func (my * minimalprs) TAction(state int,  sym int)int{
        var i = minimalprs_BaseAction[state]
        var k = i + sym
        var index int
        if minimalprs_TermCheck[k] == sym {
           index = k
        }else{
           index = i
        }
        return minimalprs_TermAction[index]
    }
    func (my * minimalprs) LookAhead(la_state int , sym int)int{
        var k = la_state + sym
        var index int
        if minimalprs_TermCheck[k] == sym {
           index = k
        }else{
           index = la_state
        }
        return minimalprs_TermAction[ index]
    }

