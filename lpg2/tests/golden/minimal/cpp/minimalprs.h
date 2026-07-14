#ifndef minimalprs_INCLUDED
#define minimalprs_INCLUDED

#include "minimalsym.h"

#include <stdio.h> // Need definition for NULL

class minimalprs
{
public:

    enum {
          NT_OFFSET         = 2,
          LA_STATE_OFFSET   = 8,
          MAX_LA            = 0,
          NUM_RULES         = 1,
          NUM_TERMINALS     = 2,
          NUM_NONTERMINALS  = 2,
          NUM_SYMBOLS       = 4,
          START_STATE       = 2,
          IDENTIFIER_SYMBOL = 0,
          EOFT_SYMBOL       = 2,
          EOLT_SYMBOL       = 3,
          ACCEPT_ACTION     = 6,
          ERROR_ACTION      = 7,
          BACKTRACK         = 0
         };


    static const char is_nullable[];
    static const unsigned char prostheses_index[];
    static const char is_keyword[];
    static const unsigned char base_check[];
    static const unsigned char *rhs;
    static const unsigned char base_action[];
    static const unsigned char *lhs;
    static const unsigned char term_check[];
    static const unsigned char term_action[];
    static inline int nt_action(int state, int sym)
    {
        return base_action[state + sym];
    }

    static inline int t_action(int state, int sym)
    {
        int i = base_action[state],
            k = i + sym;
        return term_action[term_check[k] == sym ? k : i];
    }

    static inline int look_ahead(int la_state, int sym)
    {
        int k = la_state + sym;
        return term_action[term_check[k] == sym ? k : la_state];
    }
};

#endif /* minimalprs_INCLUDED */
