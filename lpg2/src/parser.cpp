#include "parser.h"

#include <assert.h>
#include <iostream>
using namespace std;

#include "control.h"

Parser::Parser(Control *control_,
               LexStream *lex_stream_,
               VariableLookupTable *variable_table_,
               MacroLookupTable *macro_table_) : jikespg_act(control_,
                                                             lex_stream_,
                                                             variable_table_,
                                                             macro_table_),
                                                 control(control_)
{}


//
//
//
int Parser::t_action(int act, int sym, LexStream *stream)
{
    act = jikespg_prs::t_action(act, sym);
    if (act > LA_STATE_OFFSET)
    {
        for (TokenObject tok = stream -> Peek();
             ;
             tok = stream -> Next(tok))
        {
           act = look_ahead(act - LA_STATE_OFFSET, stream -> Kind(tok));
           if (act <= LA_STATE_OFFSET)
               break;
        } 
    }
    return act;
}

//
//
//
void Parser::Parse(int start_index)
{
    lex_stream -> Reset(start_index);

    curtok = lex_stream -> Gettoken();
    int act = START_STATE,
              current_kind = lex_stream -> Kind(curtok);

    //
    // Start parsing.
    //
    state_stack_top = -1;

    //
    // Process a terminal
    //
    for (;;)
    {
        if (++state_stack_top >= stack.Size())
            ReallocateStacks();

        stack[state_stack_top] = act;
        location_stack[state_stack_top] = Loc(curtok);

        act = t_action(act, current_kind, lex_stream);

        if (act <= NUM_RULES)
        {
            state_stack_top--; // make reduction look like a shift-reduce
        }
        else if (act > ERROR_ACTION)
        {
            curtok = lex_stream -> Gettoken();
            current_kind = lex_stream -> Kind(curtok);

            act -= ERROR_ACTION;
        }
        else if (act < ACCEPT_ACTION)
        {
            curtok = lex_stream -> Gettoken();
            current_kind = lex_stream -> Kind(curtok);

            continue;
        }
        else break;

        //
        // Process a non_terminal
        //
        do
        {
            state_stack_top -= (rhs[act] - 1);
            (this ->* rule_action[act])();
            act = nt_action(stack[state_stack_top], lhs[act]);
        } while(act <= NUM_RULES);
    } // process_termainal

    if (act == ERROR_ACTION)
    {
        //
        // Normally, we would want to do this:
        //
        //    DiagnoseParser *diagnose_parser = new DiagnoseParser(lex_stream);
        //    delete diagnose_parser;
        //
        Tuple<const char *> msg;
        int  prevtok = lex_stream -> Previous(curtok);
        if (lex_stream -> Kind(prevtok) == TK_NOTICE_KEY ||
            lex_stream -> Kind(prevtok) == TK_GLOBALS_KEY ||
            lex_stream -> Kind(prevtok) == TK_AST_KEY  ||
            lex_stream -> Kind(prevtok) == TK_HEADERS_KEY)
        {
             BlockSymbol *block = option -> DefaultBlock();
             msg.Next() = "A block ";
             msg.Next() = block -> BlockBegin();
             msg.Next() = "  ...  ";
             msg.Next() = block -> BlockEnd();
             msg.Next() = " is expected here instead of this token";
        }
        else
            msg.Next() = "Syntax error detected on this token";

        option -> EmitError(curtok, msg);
        control -> Exit(12);
    }

    return;
}
