public class minimalprs : minimalsym {
    public const int ERROR_SYMBOL = 0;
    public  int getErrorSymbol() { return ERROR_SYMBOL; }

    public const int SCOPE_UBOUND = 0;
    public  int getScopeUbound() { return SCOPE_UBOUND; }

    public const int SCOPE_SIZE = 0;
    public  int getScopeSize() { return SCOPE_SIZE; }

    public const int MAX_NAME_LENGTH = 0;
    public  int getMaxNameLength() { return MAX_NAME_LENGTH; }

    public const int NUM_STATES = 2;
    public  int getNumStates() { return NUM_STATES; }

    public const int NT_OFFSET = 2;
    public  int getNtOffset() { return NT_OFFSET; }

    public const int LA_STATE_OFFSET = 8;
    public  int getLaStateOffset() { return LA_STATE_OFFSET; }

    public const int MAX_LA = 0;
    public  int getMaxLa() { return MAX_LA; }

    public const int NUM_RULES = 1;
    public  int getNumRules() { return NUM_RULES; }

    public const int NUM_NONTERMINALS = 2;
    public  int getNumNonterminals() { return NUM_NONTERMINALS; }

    public const int NUM_SYMBOLS = 4;
    public  int getNumSymbols() { return NUM_SYMBOLS; }

    public const int SEGMENT_SIZE = 8192;
    public  int getSegmentSize() { return SEGMENT_SIZE; }

    public const int START_STATE = 2;
    public  int getStartState() { return START_STATE; }

    public const int IDENTIFIER_SYMBOL = 0;
    public  int getIdentifier_SYMBOL() { return IDENTIFIER_SYMBOL; }

    public const int EOFT_SYMBOL = 2;
    public  int getEoftSymbol() { return EOFT_SYMBOL; }

    public const int EOLT_SYMBOL = 3;
    public  int getEoltSymbol() { return EOLT_SYMBOL; }

    public const int ACCEPT_ACTION = 6;
    public  int getAcceptAction() { return ACCEPT_ACTION; }

    public const int ERROR_ACTION = 7;
    public  int getErrorAction() { return ERROR_ACTION; }

    public const bool BACKTRACK = false;
    public  bool getBacktrack() { return BACKTRACK; }

    public const bool GLR = false;
    public  bool isGLR() { return GLR; }

    public   int getStartSymbol() { return lhs(0); }
    public   bool isValidForParser() { return minimalsym.isValidForParser; }

    public   int getProsthesisIndex(int index) { return 0; }


    public interface IsNullable {
        public static sbyte[] _isNullable = {0,
            0,0,0,0
        };
    };
    public static sbyte[] _isNullable = IsNullable._isNullable;
    public   bool isNullable(int index) { return _isNullable[index] != 0; }

    public interface ProsthesesIndex {
        public static sbyte[] _prosthesesIndex = {0,
            2,1
        };
    };
    public static sbyte[] _prosthesesIndex = ProsthesesIndex._prosthesesIndex;
    public   int prosthesesIndex(int index) { return _prosthesesIndex[index]; }

    public interface IsKeyword {
        public static sbyte[] _isKeyword = {0,
            0,0
        };
    };
    public static sbyte[] _isKeyword = IsKeyword._isKeyword;
    public   bool isKeyword(int index) { return _isKeyword[index] != 0; }

    public interface BaseCheck {
        public static sbyte[] _baseCheck = {0,
            1
        };
    };
    public static sbyte[] _baseCheck = BaseCheck._baseCheck;
    public   int baseCheck(int index) { return _baseCheck[index]; }
    public static sbyte [] _rhs = _baseCheck;
    public   int rhs(int index) { return _rhs[index]; }

    public interface BaseAction {
        public static sbyte[] _baseAction = {
            1,1,1,4,3,7,7
        };
    };
    public static sbyte[] _baseAction = BaseAction._baseAction;
    public   int baseAction(int index) { return _baseAction[index]; }
    public static sbyte [] _lhs = _baseAction;
    public   int lhs(int index) { return _lhs[index]; }

    public interface TermCheck {
        public static sbyte[] _termCheck = {0,
            0,1,0,0,2
        };
    };
    public static sbyte[] _termCheck = TermCheck._termCheck;
    public   int termCheck(int index) { return _termCheck[index]; }

    public interface TermAction {
        public static sbyte[] _termAction = {0,
            7,8,7,7,6
        };
    };
    public static sbyte[] _termAction = TermAction._termAction;
    public   int termAction(int index) { return _termAction[index]; }
    public   int asb(int index) { return 0; }
    public   int asr(int index) { return 0; }
    public   int nasb(int index) { return 0; }
    public   int nasr(int index) { return 0; }
    public   int terminalIndex(int index) { return 0; }
    public   int nonterminalIndex(int index) { return 0; }
    public   int scopePrefix(int index) { return 0;}
    public   int scopeSuffix(int index) { return 0;}
    public   int scopeLhs(int index) { return 0;}
    public   int scopeLa(int index) { return 0;}
    public   int scopeStateSet(int index) { return 0;}
    public   int scopeRhs(int index) { return 0;}
    public   int scopeState(int index) { return 0;}
    public   int inSymb(int index) { return 0;}
    public   string name(int index) { return null; }
    public   int originalState(int state) { return 0; }
    public   int asi(int state) { return 0; }
    public   int nasi(int state) { return 0; }
    public   int inSymbol(int state) { return 0; }

    /**
     * assert(! goto_default);
     */
    public   int ntAction(int state, int sym) {
        return _baseAction[state + sym];
    }

    /**
     * assert(! shift_default);
     */
    public   int tAction(int state, int sym) {
        int i = _baseAction[state],
            k = i + sym;
        return _termAction[_termCheck[k] == sym ? k : i];
    }
    public   int lookAhead(int la_state, int sym) {
        int k = la_state + sym;
        return _termAction[_termCheck[k] == sym ? k : la_state];
    }
}
