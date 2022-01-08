#ifndef jikespg_prs_INCLUDED
#define jikespg_prs_INCLUDED

#include "jikespg_sym.h"

#include <stdio.h> // Need definition for NULL

class jikespg_prs
{
public:

    enum {
          ERROR_SYMBOL      = 37,
          SCOPE_UBOUND      = -1,
          SCOPE_SIZE        = 0,
          MAX_NAME_LENGTH   = 27,
          MAX_TERM_LENGTH   = 27,
          NUM_STATES        = 85,

          NT_OFFSET         = 37,
          LA_STATE_OFFSET   = 484,
          MAX_LA            = 2147483647,
          NUM_RULES         = 134,
          NUM_TERMINALS     = 37,
          NUM_NONTERMINALS  = 47,
          NUM_SYMBOLS       = 84,
          START_STATE       = 173,
          IDENTIFIER_SYMBOL = 0,
          EOFT_SYMBOL       = 29,
          EOLT_SYMBOL       = 29,
          ACCEPT_ACTION     = 349,
          ERROR_ACTION      = 350,
          BACKTRACK         = 0
         };

    static inline int original_state(int state) { return -base_check[state]; }
    static inline int asi(int state) { return asb[original_state(state)]; }
    static inline int nasi(int state) { return nasb[original_state(state)]; }
    static inline int in_symbol(int state) { return in_symb[original_state(state)]; }

    static const char is_nullable[];
    static const unsigned char prostheses_index[];
    static const char is_keyword[];
    static const signed char base_check[];
    static const signed char *rhs;
    static const unsigned short base_action[];
    static const unsigned short *lhs;
    static const unsigned char term_check[];
    static const unsigned short term_action[];
    static const unsigned char asb[];
    static const unsigned char asr[];
    static const unsigned char nasb[];
    static const unsigned char nasr[];
    static const unsigned char terminal_index[];
    static const unsigned char non_terminal_index[];
    static const unsigned char scope_prefix[];
    static const unsigned char scope_suffix[];
    static const unsigned char scope_lhs[];
    static const unsigned char scope_la[];
    static const unsigned char scope_state_set[];
    static const unsigned char scope_rhs[];
    static const unsigned char scope_state[];
    static const unsigned char in_symb[];
    static const unsigned short name_start[];
    static const char string_buffer[];
    static inline int name_length(int i) { return name_start[i + 1] - name_start[i]; }

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

#endif /* jikespg_prs_INCLUDED */
