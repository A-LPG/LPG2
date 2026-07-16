#pragma once
 #include <string>
  #include "lpg2/ParseTable.h"
 #include "minimalsym.h"
 struct minimalprs :public minimalsym {
             typedef  unsigned char byte;
     constexpr   static int ERROR_SYMBOL = 0;
     int getErrorSymbol() { return ERROR_SYMBOL; }

     constexpr   static int SCOPE_UBOUND = 0;
     int getScopeUbound() { return SCOPE_UBOUND; }

     constexpr   static int SCOPE_SIZE = 0;
     int getScopeSize() { return SCOPE_SIZE; }

     constexpr   static int MAX_NAME_LENGTH = 0;
     int getMaxNameLength() { return MAX_NAME_LENGTH; }

     constexpr   static int NUM_STATES = 2;
     int getNumStates() { return NUM_STATES; }

     constexpr   static int NT_OFFSET = 2;
     int getNtOffset() { return NT_OFFSET; }

     constexpr   static int LA_STATE_OFFSET = 8;
     int getLaStateOffset() { return LA_STATE_OFFSET; }

     constexpr   static int MAX_LA = 0;
     int getMaxLa() { return MAX_LA; }

     constexpr   static int NUM_RULES = 1;
     int getNumRules() { return NUM_RULES; }

     constexpr   static int NUM_NONTERMINALS = 2;
     int getNumNonterminals() { return NUM_NONTERMINALS; }

     constexpr   static int NUM_SYMBOLS = 4;
     int getNumSymbols() { return NUM_SYMBOLS; }

     constexpr   static int START_STATE = 2;
     int getStartState() { return START_STATE; }

     constexpr   static int IDENTIFIER_SYMBOL = 0;
     int getIdentifier_SYMBOL() { return IDENTIFIER_SYMBOL; }

     constexpr   static int EOFT_SYMBOL = 2;
     int getEoftSymbol() { return EOFT_SYMBOL; }

     constexpr   static int EOLT_SYMBOL = 3;
     int getEoltSymbol() { return EOLT_SYMBOL; }

     constexpr   static int ACCEPT_ACTION = 6;
     int getAcceptAction() { return ACCEPT_ACTION; }

     constexpr   static int ERROR_ACTION = 7;
     int getErrorAction() { return ERROR_ACTION; }

   constexpr   static bool BACKTRACK = false;
      bool getBacktrack() { return BACKTRACK; }

     int getStartSymbol() { return lhs(0); }
      bool isValidForParser() { return minimalsym::isValidForParser; }

inline static char _isNullable[] = {0,
            0,0,0,0
        };
char * get_isNullable_data(){ return  _isNullable;}
      bool isNullable(int index) { return _isNullable[index] != 0; }
inline static signed int _prosthesesIndex[] = {0,
            2,1
        };
signed int * get_prosthesesIndex_data(){ return  _prosthesesIndex;}
      int prosthesesIndex(int index) { return _prosthesesIndex[index]; }
inline static char _isKeyword[] = {0,
            0,0
        };
char * get_isKeyword_data(){ return  _isKeyword;}
      bool isKeyword(int index) { return _isKeyword[index] != 0; }
inline static signed int _baseCheck[] = {0,
            1
        };
signed int * get_baseCheck_data(){ return  _baseCheck;}
      int baseCheck(int index) { return _baseCheck[index]; }
inline      static signed int*  _rhs = _baseCheck;
     int rhs(int index) { return _rhs[index]; };
signed int*  get_rhs_data(){ return _rhs;}
inline static signed int _baseAction[] = {
            1,1,1,4,3,7,7
        };
signed int * get_baseAction_data(){ return  _baseAction;}
      int baseAction(int index) { return _baseAction[index]; }
 inline     static signed int * _lhs = _baseAction;
     int lhs(int index) { return _lhs[index]; };
signed int*  get_lhs_data(){ return _lhs;}
inline static signed int _termCheck[] = {0,
            0,1,0,0,2
        };
signed int * get_termCheck_data(){ return  _termCheck;}
      int termCheck(int index) { return _termCheck[index]; }
inline static signed int _termAction[] = {0,
            7,8,7,7,6
        };
signed int * get_termAction_data(){ return  _termAction;}
      int termAction(int index) { return _termAction[index]; }
     int asb(int index) { return 0; }
     int asr(int index) { return 0; }
     int nasb(int index) { return 0; }
     int nasr(int index) { return 0; }
     int terminalIndex(int index) { return 0; }
     int nonterminalIndex(int index) { return 0; }
     int scopePrefix(int index) { return 0;}
     int scopeSuffix(int index) { return 0;}
     int scopeLhs(int index) { return 0;}
     int scopeLa(int index) { return 0;}
     int scopeStateSet(int index) { return 0;}
     int scopeRhs(int index) { return 0;}
     int scopeState(int index) { return 0;}
     int inSymb(int index) { return 0;}
      std::wstring name(int index) { return {}; }
     int originalState(int state) { return 0; }
     int asi(int state) { return 0; }
     int nasi(int state) { return 0; }
     int inSymbol(int state) { return 0; }

    /**
     * assert(! goto_default);
     */
     int ntAction(int state, int sym) {
        return _baseAction[state + sym];
    }

    /**
     * assert(! shift_default);
     */
     int tAction(int state, int sym) {
        int i = _baseAction[state],
            k = i + sym;
        return _termAction[_termCheck[k] == sym ? k : i];
    }
     int lookAhead(int la_state, int sym) {
        int k = la_state + sym;
        return _termAction[_termCheck[k] == sym ? k : la_state];
    }
};
