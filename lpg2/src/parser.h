#ifndef parser_INCLUDED
#define parser_INCLUDED

#include "jikespg_prs.h" // parsing action functions
#include "jikespg_act.h" // semantic action functions

class Control;
class Parser : public jikespg_act, public jikespg_prs
{
    Control *control;

    inline Location Loc(TokenObject i) { return i; }

public:
    Parser(Control *control_,
           LexStream *lex_stream_,
           VariableLookupTable *variable_table_,
           MacroLookupTable *macro_table_);

    static int t_action(int, int, LexStream *);

    void Parse(int = 0);
};
#endif

