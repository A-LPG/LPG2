#include "table.h"

#include <iostream>
using namespace std;

void Table::PrintStateMaps()
{
    //
    // First, flush any data left in the report buffer.
    //
    option -> FlushReport();

    control -> PrintHeading();
    fprintf(option -> syslis,
            "\nMapping of new state numbers into "
            "original numbers:\n");
    for (int i = 1; i <= pda -> num_states; i++)
         fprintf(option -> syslis, "\n%5d  ==>>  %5d", sorted_state[i], original_state[i]);
    fprintf(option -> syslis,"\n");

    return;
}

//
//
//
void Table::PrintReport(void)
{
    if (option -> quiet)
        return;

    option -> report.Put("\nActions in Compressed Tables:");
    option -> report.PutChar('\n');
    option -> report.Put("     Number of Shifts: ");
    option -> report.Put(shift_count);
    option -> report.PutChar('\n');
    option -> report.Put("     Number of Shift/Reduces: ");
    option -> report.Put(shift_reduce_count);
    option -> report.PutChar('\n');

    if (option -> backtrack)
    {
        option -> report.Put("     Number of conflict points: ");
        option -> report.Put(conflict_count);
        option -> report.PutChar('\n');
    }

    if (pda -> max_la_state > pda -> num_states)
    {
        option -> report.Put("     Number of Look-Ahead Shifts: ");
        option -> report.Put(la_shift_count);
        option -> report.PutChar('\n');
    }

    option -> report.Put("     Number of Gotos: ");
    option -> report.Put(goto_count);
    option -> report.PutChar('\n');
    option -> report.Put("     Number of Goto/Reduces: ");
    option -> report.Put(goto_reduce_count);
    option -> report.PutChar('\n');
    option -> report.Put("     Number of Reduces: ");
    option -> report.Put(reduce_count);
    option -> report.PutChar('\n');
    option -> report.Put("     Number of Defaults: ");
    option -> report.Put(default_count);
    option -> report.PutChar('\n');

    IntArrayInfo *base_check_info = NULL,
                 *base_action_info = NULL,
                 *term_check_info = NULL,
                 *term_action_info = NULL,
                 *default_goto_info = NULL,
                 *default_reduce_info = NULL,
                 *shift_state_info = NULL,
                 *shift_check_info = NULL,
                 *default_shift_info = NULL,
                 *action_symbols_base_info = NULL,
                 *action_symbols_range_info = NULL,
                 *naction_symbols_base_info = NULL,
                 *naction_symbols_range_info = NULL,
                 *terminal_index_info = NULL,
                 *nonterminal_index_info = NULL,
                 *scope_prefix_info = NULL,
                 *scope_suffix_info = NULL,
                 *scope_lhs_symbol_info = NULL,
                 *scope_look_ahead_info = NULL,
                 *scope_state_set_info = NULL,
                 *scope_right_side_info = NULL,
                 *scope_state_info = NULL,
                 *in_symb_info = NULL;

    for (int i = 0; i < data.Length(); i++)
    {
        IntArrayInfo &array_info = data[i];
        switch(array_info.name_id)
        {
            case BASE_CHECK:
                 base_check_info = &array_info;
                 break;
            case BASE_ACTION:
                 base_action_info = &array_info;
                 break;
            case TERM_CHECK:
                 term_check_info = &array_info;
                 break;
            case TERM_ACTION:
                 term_action_info = &array_info;
                 break;
            case DEFAULT_GOTO:
                 default_goto_info = &array_info;
                 break;
            case DEFAULT_REDUCE:
                 default_reduce_info = &array_info;
                 break;
            case SHIFT_STATE:
                 shift_state_info = &array_info;
                 break;
            case SHIFT_CHECK:
                 shift_check_info = &array_info;
                 break;
            case DEFAULT_SHIFT:
                 default_shift_info = &array_info;
                 break;
            case ACTION_SYMBOLS_BASE:
                 action_symbols_base_info = &array_info;
                 break;
            case ACTION_SYMBOLS_RANGE:
                 action_symbols_range_info = &array_info;
                 break;
            case NACTION_SYMBOLS_BASE:
                 naction_symbols_base_info = &array_info;
                 break;
            case NACTION_SYMBOLS_RANGE:
                 naction_symbols_range_info = &array_info;
                 break;
            case TERMINAL_INDEX:
                 terminal_index_info = &array_info;
                 break;
            case NONTERMINAL_INDEX:
                 nonterminal_index_info = &array_info;
                 break;
            case SCOPE_PREFIX:
                 scope_prefix_info = &array_info;
                 break;
            case SCOPE_SUFFIX:
                 scope_suffix_info = &array_info;
                 break;
            case SCOPE_LHS_SYMBOL:
                 scope_lhs_symbol_info = &array_info;
                 break;
            case SCOPE_LOOK_AHEAD:
                 scope_look_ahead_info = &array_info;
                 break;
            case SCOPE_STATE_SET:
                 scope_state_set_info = &array_info;
                 break;
            case SCOPE_RIGHT_SIDE:
                 scope_right_side_info = &array_info;
                 break;
            case SCOPE_STATE:
                 scope_state_info = &array_info;
                 break;
            case IN_SYMB:
                 in_symb_info = &array_info;
                 break;
            default:
                 break;
        }
    }

    option -> report.Put("\nParsing Tables storage:");
    option -> report.PutChar('\n');

    option -> report.Put("    Storage required for BASE_CHECK: ");
    option -> report.Put(base_check_info -> num_bytes);
    option -> report.Put(" Bytes");
    option -> report.PutChar('\n');

    option -> report.Put("    Storage required for BASE_ACTION: ");
    option -> report.Put(base_action_info -> num_bytes);
    option -> report.Put(" Bytes");
    option -> report.PutChar('\n');

    option -> report.Put("    Storage required for TERM_CHECK: ");
    option -> report.Put(term_check_info -> num_bytes);
    option -> report.Put(" Bytes");
    option -> report.PutChar('\n');

    option -> report.Put("    Storage required for TERM_ACTION: ");
    option -> report.Put(term_action_info -> num_bytes);
    option -> report.Put(" Bytes");
    option -> report.PutChar('\n');

    if (default_goto_info)
    {
        option -> report.Put("    Storage required for DEFAULT_GOTO: ");
        option -> report.Put(default_goto_info -> num_bytes);
        option -> report.Put(" Bytes");
        option -> report.PutChar('\n');
    }

    if (option -> shift_default)
    {
        assert(default_reduce_info);
        option -> report.Put("    Storage required for DEFAULT_REDUCE: ");
        option -> report.Put(default_reduce_info -> num_bytes);
        option -> report.Put(" Bytes");
        option -> report.PutChar('\n');

        assert(shift_state_info);
        option -> report.Put("    Storage required for SHIFT_STATE: ");
        option -> report.Put(shift_state_info -> num_bytes);
        option -> report.Put(" Bytes");
        option -> report.PutChar('\n');

        assert(shift_check_info);
        option -> report.Put("    Storage required for SHIFT_CHECK: ");
        option -> report.Put(shift_check_info -> num_bytes);
        option -> report.Put(" Bytes");
        option -> report.PutChar('\n');

        assert(default_shift_info);
        option -> report.Put("    Storage required for DEFAULT_SHIFT: ");
        option -> report.Put(default_shift_info -> num_bytes);
        option -> report.Put(" Bytes");
        option -> report.PutChar('\n');
    }

    if (option -> error_maps)
    {
        option -> report.PutChar('\n');
        option -> report.Put("Error maps storage:");
        option -> report.PutChar('\n');

        option -> report.Put("    Storage required for ACTION_SYMBOLS_BASE map: ");
        option -> report.Put(action_symbols_base_info -> num_bytes);
        option -> report.Put(" Bytes");
        option -> report.PutChar('\n');

        option -> report.Put("    Storage required for ACTION_SYMBOLS_RANGE map: ");
        option -> report.Put(action_symbols_range_info -> num_bytes);
        option -> report.Put(" Bytes");
        option -> report.PutChar('\n');

        option -> report.Put("    Storage required for NACTION_SYMBOLS_BASE map: ");
        option -> report.Put(naction_symbols_base_info -> num_bytes);
        option -> report.Put(" Bytes");
        option -> report.PutChar('\n');
        option -> report.Put("    Storage required for NACTION_SYMBOLS_RANGE map: ");
        option -> report.Put(naction_symbols_range_info -> num_bytes);
        option -> report.Put(" Bytes");
        option -> report.PutChar('\n');

        option -> report.Put("    Storage required for TERMINAL_INDEX map: ");
        option -> report.Put(terminal_index_info -> num_bytes);
        option -> report.Put(" Bytes");
        option -> report.PutChar('\n');

        option -> report.Put("    Storage required for NON_TERMINAL_INDEX map: ");
        option -> report.Put(nonterminal_index_info -> num_bytes);
        option -> report.Put(" Bytes");
        option -> report.PutChar('\n');

        if (scope_prefix_info)
        {
            option -> report.PutChar('\n');

            option -> report.Put("    Storage required for SCOPE_PREFIX map: ");
            option -> report.Put(scope_prefix_info -> num_bytes);
            option -> report.Put(" Bytes");
            option -> report.PutChar('\n');
            option -> report.Put("    Storage required for SCOPE_SUFFIX map: ");
            option -> report.Put(scope_suffix_info -> num_bytes);
            option -> report.Put(" Bytes");
            option -> report.PutChar('\n');
            option -> report.Put("    Storage required for SCOPE_LHS_SYMBOL map: ");
            option -> report.Put(scope_lhs_symbol_info -> num_bytes);
            option -> report.Put(" Bytes");
            option -> report.PutChar('\n');
            option -> report.Put("    Storage required for SCOPE_LOOK_AHEAD map: ");
            option -> report.Put(scope_look_ahead_info -> num_bytes);
            option -> report.Put(" Bytes");
            option -> report.PutChar('\n');
            option -> report.Put("    Storage required for SCOPE_STATE_SET map: ");
            option -> report.Put(scope_state_set_info -> num_bytes);
            option -> report.Put(" Bytes");
            option -> report.PutChar('\n');
            option -> report.Put("    Storage required for SCOPE_RIGHT_SIDE map: ");
            option -> report.Put(scope_right_side_info -> num_bytes);
            option -> report.Put(" Bytes");
            option -> report.PutChar('\n');
            option -> report.Put("    Storage required for SCOPE_STATE map: ");
            option -> report.Put(scope_state_info -> num_bytes);
            option -> report.Put(" Bytes");
            option -> report.PutChar('\n');
            option -> report.Put("    Storage required for IN_SYMB map: ");
            option -> report.Put(in_symb_info -> num_bytes);
            option -> report.Put(" Bytes");
            option -> report.PutChar('\n');
        }

        //
        // It is possible to accurately calculate how much storage is required
        // for the Names map only when generating C or C++ tables. For the other
        // cases, we simply write out the basic information.
        //
        if (option -> programming_language == Option::C 
            || option -> programming_language == Option::CPP
            || option->programming_language == Option::CPP2)
        {
            option -> report.PutChar('\n');

            option -> report.Put("    Storage required for NAME_START map: ");
            option -> report.Put(name_start.num_bytes);
            option -> report.Put(" Bytes");
            option -> report.PutChar('\n');

            option -> report.Put("    Storage required for STRING_BUFFER map: ");
            option -> report.Put(name_start.array[name_start.array.Size() - 1] - 1);
            option -> report.Put(" Bytes");
            option -> report.PutChar('\n');
        }
        else
        {
            option -> report.PutChar('\n');

            option -> report.Put("    Number of names: ");
            option -> report.Put(name_start.array.Size());
            option -> report.PutChar('\n');

            option -> report.Put("    Number of characters in name: ");
            option -> report.Put(name_start.array[name_start.array.Size() - 1] - 1);
            option -> report.PutChar('\n');
        }
    }

    option -> FlushReport();

    return;
}
