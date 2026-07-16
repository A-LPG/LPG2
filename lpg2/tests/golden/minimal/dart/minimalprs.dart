import 'minimalsym.dart';
 class minimalprs {
    static const  int ERROR_SYMBOL = 0;
    int   getErrorSymbol(){ return ERROR_SYMBOL; }

    static const  int SCOPE_UBOUND = 0;
    int   getScopeUbound(){ return SCOPE_UBOUND; }

    static const  int SCOPE_SIZE = 0;
    int   getScopeSize(){ return SCOPE_SIZE; }

    static const  int MAX_NAME_LENGTH = 0;
    int   getMaxNameLength(){ return MAX_NAME_LENGTH; }

    static const  int NUM_STATES = 2;
    int   getNumStates(){ return NUM_STATES; }

    static const  int NT_OFFSET = 2;
    int   getNtOffset(){ return NT_OFFSET; }

    static const  int LA_STATE_OFFSET = 8;
    int   getLaStateOffset(){ return LA_STATE_OFFSET; }

    static const  int MAX_LA = 0;
    int   getMaxLa(){ return MAX_LA; }

    static const  int NUM_RULES = 1;
    int   getNumRules(){ return NUM_RULES; }

    static const  int NUM_NONTERMINALS = 2;
    int   getNumNonterminals(){ return NUM_NONTERMINALS; }

    static const  int NUM_SYMBOLS = 4;
    int   getNumSymbols(){ return NUM_SYMBOLS; }

    static const  int START_STATE = 2;
    int   getStartState(){ return START_STATE; }

    static const  int IDENTIFIER_SYMBOL = 0;
    int   getIdentifier_SYMBOL(){ return IDENTIFIER_SYMBOL; }

    static const  int EOFT_SYMBOL = 2;
    int   getEoftSymbol(){ return EOFT_SYMBOL; }

    static const  int EOLT_SYMBOL = 3;
    int   getEoltSymbol(){ return EOLT_SYMBOL; }

    static const  int ACCEPT_ACTION = 6;
    int   getAcceptAction(){ return ACCEPT_ACTION; }

    static const  int ERROR_ACTION = 7;
    int   getErrorAction(){ return ERROR_ACTION; }

    static const bool  BACKTRACK = false;
    bool  getBacktrack() { return BACKTRACK; }

    int  getStartSymbol() { return lhs(0); }
    bool  isValidForParser()  { return minimalsym.isValidForParser; }


     static  const List<int> _isNullable = [0,
            0,0,0,0
        ];
       bool isNullable(int index){ return  minimalprs._isNullable[index] != 0; }

     static  const List<int> _prosthesesIndex = [0,
            2,1
        ];
       int prosthesesIndex(int index){ return  minimalprs._prosthesesIndex[index]; }

     static  const List<int> _isKeyword = [0,
            0,0
        ];
       bool isKeyword(int index){ return  minimalprs._isKeyword[index] != 0; }

     static  const List<int> _baseCheck = [0,
            1
        ];
       int baseCheck(int index){ return  minimalprs._baseCheck[index]; }
     static const List<int> _rhs = minimalprs._baseCheck;
    int    rhs(int index){ return minimalprs._rhs[index]; }

     static  const List<int> _baseAction = [
            1,1,1,4,3,7,7
        ];
       int baseAction(int index){ return  minimalprs._baseAction[index]; }
     static const List<int>  _lhs =minimalprs._baseAction;
   int   lhs(int index){ return minimalprs._lhs[index]; }

     static  const List<int> _termCheck = [0,
            0,1,0,0,2
        ];
       int termCheck(int index){ return  minimalprs._termCheck[index]; }

     static  const List<int> _termAction = [0,
            7,8,7,7,6
        ];
       int termAction(int index){ return  minimalprs._termAction[index]; }
      int  asb(int index) { return 0; }
      int  asr(int index) { return 0; }
      int  nasb(int index)  { return 0; }
      int  nasr(int index)  { return 0; }
      int  terminalIndex(int index)  { return 0; }
      int  nonterminalIndex(int index)  { return 0; }
      int  scopePrefix(int index)  { return 0;}
      int  scopeSuffix(int index)  { return 0;}
      int  scopeLhs(int index)  { return 0;}
      int  scopeLa(int index)  { return 0;}
      int  scopeStateSet(int index)  { return 0;}
      int  scopeRhs(int index)  { return 0;}
      int  scopeState(int index)  { return 0;}
      int  inSymb(int index)  { return 0;}
      String  name(int index) { return ""; }
   int  originalState(int state){ return 0; }
     int asi(int state ){ return 0; }
     int   nasi(int state ){ return 0; }
     int   inSymbol(int state){ return 0; }

    /**
     * assert(! goto_default);
     */
    int ntAction(int state,  int sym){
        return minimalprs._baseAction[state + sym];
    }

    /**
     * assert(! shift_default);
     */
     int  tAction(int state,  int sym){
        var i = minimalprs._baseAction[state],
            k = i + sym;
        return minimalprs._termAction[ minimalprs._termCheck[k] == sym ? k : i];
    }
    int   lookAhead( int la_state ,  int sym){
        var k = la_state + sym;
        return minimalprs._termAction[ minimalprs._termCheck[k] == sym ? k : la_state];
    }
}
