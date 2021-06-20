#include "generator.h"
#include "table.h"
#include "frequency.h"
#include "partition.h"
#include "util.h"

#include <iostream>
using namespace std;

const int Generator::MAX_TABLE_SIZE = INT_MAX - 1;

//
// The array ACTION_COUNT is used to construct a map from each terminal
// into the set (list) of actions defined on that terminal. A count of the
// number of occurences of each action in the automaton is kept.
// This procedure is invoked with a specific shift map which it processes
// and updates the ACTION_COUNT map accordingly.
//
void Generator::process_shift_actions(Array< Tuple<ActionElement> > &action_count, int shift_no)
{
    Dfa::ShiftHeader &sh = pda -> shift[shift_no];
    for (int i = 0; i < sh.Length(); i++)
    {
        int symbol = sh[i].Symbol(),
            act = sh[i].Action(),
            k;
        for (k = 0; k < action_count[symbol].Length(); k++)
        {
            if (action_count[symbol][k].action == act)
                break;
        }

        if (k == action_count[symbol].Length()) // new action not yet seen
        {
            int index = action_count[symbol].NextIndex();
            action_count[symbol][index].action = act;
            action_count[symbol][index].count = 1;
        }
        else (action_count[symbol][k].count)++;
    }

    return;
}


//
// This procedure updates the vector DEFAULT_SHIFT, indexable by the terminals in
// the grammar. Its task is to assign to each element of DEFAULT_SHIFT, the action
// most frequently defined on the symbol in question.
//
void Generator::compute_default_shift(void)
{
    //
    // For each state, invoke PROCESS_SHIFT_ACTIONS to process the
    // shift map associated with that state.
    //
    Array< Tuple<ActionElement> > action_count(grammar -> num_terminals + 1);
    int shift_count = 0,
        shift_reduce_count = 0,
        conflict_count = 0;
    for (int state_no = 1; state_no <= pda -> max_la_state; state_no++)
        process_shift_actions(action_count, pda -> statset[state_no].shift_number);

    //
    // We now iterate over the ACTION_COUNT mapping, and for each
    // terminal t, initialize DEFAULT_SHIFT[t] to the action that is most
    // frequently defined on t.
    //
    default_shift.Resize(grammar -> num_terminals + 1);
    for (int symbol = grammar -> FirstTerminal(); symbol <= grammar -> LastTerminal(); symbol++)
    {
        int max_count = 0,
            default_action = 0;
        for (int k = 0; k < action_count[symbol].Length(); k++)
        {
            if (action_count[symbol][k].count > max_count)
            {
                max_count = action_count[symbol][k].count;
                default_action = action_count[symbol][k].action;
            }
        }

        default_shift[symbol] = default_action;
        if (default_action > pda -> max_la_state) // a conflict action?
             conflict_count += max_count;
        else if (default_action > 0) // A state number?
             shift_count += max_count;
        else shift_reduce_count += max_count;
    }

    if (! option -> quiet)
    {
        option -> report.PutChar('\n');
        option -> report.Put("Number of Shift entries saved by default: ");
        option -> report.Put(shift_count);
        option -> report.PutChar('\n');
        option -> report.Put("Number of Shift/Reduce entries saved by default: ");
        option -> report.Put(shift_reduce_count);
        option -> report.PutChar('\n');
        if (option -> backtrack)
        {
            option -> report.Put("Number of conflict entries saved by default: ");
            option -> report.Put(conflict_count);
            option -> report.PutChar('\n');
        }
    }

    pda -> num_shifts -= shift_count;
    pda -> num_shift_reduces -= shift_reduce_count;
    num_entries = num_entries - shift_count - shift_reduce_count;

    return;
}


//
// COMPUTE_DEFAULT_GOTO constructs the vector DEFAULT_GOTO, which is indexed by
// the non-terminals in the grammar. Its task is to assign to each element
// of the array the Action which is most frequently defined on the symbol in
// question, and remove all such actions from the state automaton.
//
void Generator::compute_default_goto(void)
{
    //
    // The array ACTION_COUNT is used to construct a map from each
    // non-terminal into the set (list) of actions defined on that
    // non-terminal. A count of how many occurences of each action
    // is also kept.
    // This loop is analoguous to the loop in PROCESS_SHIFT_ACTIONS.
    //
    BoundedArray< Tuple<ActionElement> > action_count(grammar -> num_terminals + 1, grammar -> num_symbols);
    for (int state_no = 1; state_no <= pda -> num_states; state_no++)
    {
        Dfa::GotoHeader &go_to = pda -> statset[state_no].go_to;
        for (int i = 0; i < go_to.Length(); i++)
        {
            int symbol = go_to[i].Symbol(),
                act = go_to[i].Action(),
                k;
            for (k = 0; k < action_count[symbol].Length(); k++)
            {
                if (action_count[symbol][k].action == act)
                    break;
            }

            if (k == action_count[symbol].Length()) // new action not yet seen
            {
                int index = action_count[symbol].NextIndex();

                action_count[symbol][index].action = act;
                action_count[symbol][index].count = 1;
            }
            else (action_count[symbol][k].count)++;
        }
    }

    //
    // We now iterate over the mapping created above and for each
    // non-terminal A, initialize DEFAULT_GOTO(A) to the action that
    // is most frequently defined on A.
    //
    default_goto.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
    int goto_count = 0,
        goto_reduce_count = 0;
    for (int symbol = grammar -> FirstNonTerminal(); symbol <= grammar -> LastNonTerminal(); symbol++)
    {
        int max_count = 0,
            default_action = 0;
        for (int k = action_count[symbol].Length() - 1; k >= 0; k--)
        {
            if (action_count[symbol][k].count > max_count)
            {
                max_count = action_count[symbol][k].count;
                default_action = action_count[symbol][k].action;
assert(default_action != 0);
            }
        }

        default_goto[symbol] = default_action;
        if (default_action > 0) // A state number?
             goto_count += max_count;
        else goto_reduce_count += max_count;
    }

    //
    // We now iterate over the automaton and eliminate all GOTO actions
    // for which there is a DEFAULT.
    //
    {
        for (int state_no = 1; state_no <= pda -> num_states; state_no++)
            pda -> statset[state_no].go_to.RemoveDefaults(default_goto);
    }

    if (! option -> quiet)
    {
        option -> report.PutChar('\n');
        option -> report.Put("Number of Goto entries saved by default: ");
        option -> report.Put(goto_count);
        option -> report.PutChar('\n');
        option -> report.Put("Number of Goto/Reduce entries saved by default: ");
        option -> report.Put(goto_reduce_count);
        option -> report.PutChar('\n');
    }

    pda -> num_gotos -= goto_count;
    pda -> num_goto_reduces -= goto_reduce_count;
    num_entries = num_entries - goto_count - goto_reduce_count;

    return;
}


//
// Remap symbols, apply transition default actions  and call
// appropriate table compression routine.
//
void Generator::Process(void)
{
    num_entries = pda -> max_la_state + pda -> num_shifts + pda -> num_shift_reduces
                                      + pda -> num_gotos  + pda -> num_goto_reduces
                                      + pda -> num_reductions;
/* TODO: Remove this ... Debug info for prostheses
for (int i = 1; i <= grammar -> num_symbols; i++)
cout << grammar -> RetrieveString(i) << " " << i << "\n";
*/

    //
    // First, we decrease by 1 the constants NUM_SYMBOLS
    // and NUM_TERMINALS, remove the EMPTY symbol(1) and remap the
    // other symbols beginning at 1.  If default reduction is
    // requested, we assume a special DEFAULT_SYMBOL with number zero.
    //
    if (grammar -> identifier_image != 0)
        grammar -> identifier_image--;
    grammar -> eof_image--;
    grammar -> accept_image--;
    grammar -> start_image--;
    if (option -> error_maps)
    {
        grammar -> error_image--;
        grammar -> eol_image--;
    }
    grammar -> num_terminals--;
    grammar -> num_symbols--;

    //
    // Remap the nullable symbols
    //
    {
        for (int i = 0; i < nullable_nonterminals.Length(); i++)
            nullable_nonterminals[i]--;
    }

    //
    // Remap the keyword symbols
    //
    {
        for (int i = 0; i < grammar -> keywords.Length(); i++)
            grammar -> keywords[i]--;
    }

    //
    // Remap all the symbols used in GOTO and REDUCE actions.
    // Remap all the symbols used in GD_RANGE.
    // Remap all the symbols used in the range of SCOPE.
    //
    for (int state_no = 1; state_no <= pda -> max_la_state; state_no++)
    {
        Dfa::GotoHeader &go_to = pda -> statset[state_no].go_to;
        {
            for (int i = 0; i < go_to.Length(); i++)
                go_to[i].SetSymbol(go_to[i].Symbol() - 1);
        }

        Dfa::ReduceHeader &red = pda -> statset[state_no].reduce;
        {
            for (int i = 1; i < red.Length(); i++)
                red[i].SetSymbol(red[i].Symbol() - 1);
        }

        Dfa::ConflictHeader &conflict = pda -> statset[state_no].conflict;
        {
            for (int i = 1; i < conflict.Length(); i++)
                conflict[i].SetSymbol(conflict[i].Symbol() - 1);
        }
    }

    {
        for (int i = 0; i < pda -> gd_range.Length(); i++)
            pda -> gd_range[i]--;
    }

    {
        for (int i = 0; i < pda -> scope_lhs_symbol.Size(); i++)
        {
            pda -> scope_lhs_symbol[i]--;
            pda -> scope_look_ahead[i]--;
        }
    }

    {
        for (int i = 1; i < pda -> scope_right_side.Size(); i++)
            if (pda -> scope_right_side[i] != 0)
                pda -> scope_right_side[i]--;
    }

    //
    // Remap all symbols in the domain of the Shift maps.
    //
    {
        for (int i = 1; i <= pda -> num_shift_maps; i++)
        {
            Dfa::ShiftHeader &sh = pda -> shift[i];
            for (int j = 0; j < sh.Length(); j++)
                sh[j].SetSymbol(sh[j].Symbol() - 1);
        }
    }

    //
    // Remap the left-hand side of all the rules.
    //
    for (int rule_no = grammar -> FirstRule(); rule_no <= grammar -> LastRule(); rule_no++)
        grammar -> rules[rule_no].lhs--;

    //
    // Remap the dot symbols in ITEM_TABLE.
    //
    if (option -> error_maps)
    {
        for (int i = 1; i <= grammar -> num_items; i++)
            base -> item_table[i].symbol--;
    }

    //
    // We update the SYMNO map.
    //
    for (int symbol = 1; symbol <= grammar -> num_symbols; symbol++)
        grammar -> symbol_index[symbol] = grammar -> symbol_index[symbol + 1];

    //
    // If Goto Default and/or Shift Default were requested, process
    // appropriately.
    //
    if (option -> shift_default)
        compute_default_shift();

    if (option -> goto_default)
        compute_default_goto();

    //
    // We allocate the necessary structures, open the appropriate
    // output file and call the appropriate compression routine.
    //
    if (option -> error_maps)
    {
        naction_symbols.Resize(pda -> num_states + 1);
        for (int i = 1; i <= pda -> num_states; i++)
            naction_symbols[i].Initialize(grammar -> num_nonterminals + 1, grammar -> num_terminals);

        action_symbols.Resize(pda -> num_states + 1);
        {
            for (int i = 1; i <= pda -> num_states; i++)
                action_symbols[i].Initialize(grammar -> num_terminals + 1);
        }
    }

    remap_non_terminals();
    overlap_nt_rows();
    merge_similar_t_rows();
    int num_table_entries = overlay_sim_t_rows();
    overlap_t_rows(num_table_entries);

    return;
}


//
// This procedure computes the range of the ACTION_SYMBOLS map after
// Optimal Partitioning has been used to compress that map.  Its
// first argument is an array, STATE_START, that indicates the
// starting location in the compressed vector for each state.  When
// a value of STATE_START is negative it indicates that the state in
// question shares its elements with another state.  Its second
// argument, STATE_STACK, is an array that contains the elements of
// the partition created by PARTSET.  Each element of the partition
// is organized as a circular list where the smallest sets appear
// first in the list.
//
void Generator::compute_action_symbols_range(Array<int> &state_start,
                                             Array<int> &state_stack,
                                             Array<int> &state_list,
                                             Array<int> &action_symbols_range)
{
    //
    // We now write out the range elements of the ACTION_SYMBOLS map.
    // Recall that if STATE_START has a negative value, then the set in
    // question is sharing elements and does not need to be processed.
    //
    int k = 0;
    action_symbols_range[k++] = 0; // The zeroth element is unused
    Array<int> symbol_list(grammar -> num_symbols + 1, Util::OMEGA);
    for (int i = 1; i <= pda -> num_states; i++)
    {
        int state_no = state_list[i];
        if (state_start[state_no] > 0)
        {
            int symbol_root = 0; // Add "fence" element: 0 to list
            symbol_list[symbol_root] = Util::NIL;

            //
            // Pop a state from the stack,  and add each of its elements
            // that has not yet been processed into the list.
            // Continue until stack is empty...
            // Recall that the stack is represented by a circular queue.
            //
            int state;
            for (bool end_node = ((state = state_no) == Util::NIL);
                 ! end_node; end_node = (state == state_no))
            {
                state = state_stack[state];
                Dfa::ShiftHeader &sh = pda -> Shift(state);
                for (int j = 0; j < sh.Length(); j++)
                {
                    int symbol = sh[j].Symbol();
                    if (symbol_list[symbol] == Util::OMEGA)
                    {
                        symbol_list[symbol] = symbol_root;
                        symbol_root = symbol;
                    }
                }

                Dfa::ReduceHeader &red = pda -> statset[state].reduce;
                {
                    for (int j = 1; j < red.Length(); j++)
                    {
                        int symbol = red[j].Symbol();
                        if (symbol_list[symbol] == Util::OMEGA)
                        {
                            symbol_list[symbol] = symbol_root;
                            symbol_root = symbol;
                        }
                    }
                }
            }

            //
            // Write the list out.
            //
            assert(state_start[state_no] == k);
            for (int symbol = symbol_root; symbol != Util::NIL; symbol = symbol_root)
            {
                symbol_root = symbol_list[symbol];

                symbol_list[symbol] = Util::OMEGA;
                action_symbols_range[k++] = symbol;
            }
        }
    }
    assert(action_symbols_range.Size() == k);

    return;
}


//
// This procedure computes the range of the NACTION_SYMBOLS map. Its
// organization is analoguous to COMPUTE_ACTION_SYMBOLS_RANGE.
//
void Generator::compute_naction_symbols_range(Array<int> &state_start,
                                              Array<int> &state_stack,
                                              Array<int> &state_list,
                                              Array<int> &naction_symbols_range)
{
    //
    // We now write out the range elements of the NACTION_SYMBOLS map.
    // Recall that if STATE_START has a negative value, then the set in
    // question is sharing elements and does not need to be processed.
    //
    int k = 0;
    naction_symbols_range[k++] = 0; // The zeroth element is unused
    Array<int> symbol_list(grammar -> num_symbols + 1, Util::OMEGA);
    for (int i = 1; i <= pda -> num_states; i++)
    {
        int state_no = state_list[i];
        if (state_start[state_no] > 0)
        {
            int symbol_root = 0; // Add "fence" element: 0 to list
            symbol_list[symbol_root] = Util::NIL;

            //
            // Pop a state from the stack,  and add each of its elements
            // that has not yet been processed into the list.
            // Continue until stack is empty...
            // Recall that the stack is represented by a circular queue.
            //
            int state;
            for (bool end_node =  ((state = state_no) == Util::NIL);
                 ! end_node; end_node = (state == state_no))
            {
                state = state_stack[state];
                for (int j = pda -> gd_index[state]; j <= pda -> gd_index[state + 1] - 1; j++)
                {
                    int symbol = pda -> gd_range[j];
                    if (symbol_list[symbol] == Util::OMEGA)
                    {
                        symbol_list[symbol] = symbol_root;
                        symbol_root = symbol;
                    }
                }
            }

            //
            // Write the list out.
            //
assert(state_start[state_no] == k);
            for (int symbol = symbol_root; symbol != Util::NIL; symbol = symbol_root)
            {
                symbol_root = symbol_list[symbol];

                symbol_list[symbol] = Util::OMEGA;
                naction_symbols_range[k++] = symbol;
            }
        }
    }

assert(naction_symbols_range.Size() == k);
    return;
}


//
//  REMAP_NON_TERMINALS remaps the non-terminal symbols and states based on
// frequency of entries.
//
void Generator::remap_non_terminals(void)
{
    //
    //   The variable FREQUENCY_SYMBOL is used to hold the non-terminals
    // in the grammar, and  FREQUENCY_COUNT is used correspondingly to
    // hold the number of actions defined on each non-terminal.
    // ORDERED_STATE and ROW_SIZE are used in a similar fashion for states
    //
    BoundedArray<int> frequency_symbol(grammar -> num_terminals + 1, grammar -> num_symbols),
                      frequency_count(grammar -> num_terminals + 1, grammar -> num_symbols);
    frequency_count.MemReset();
    for (int i = grammar -> FirstNonTerminal(); i <= grammar -> LastNonTerminal(); i++)
        frequency_symbol[i] = i;

    Array<int> row_size(pda -> num_states + 1);
    ordered_state.Resize(pda -> max_la_state + 1);
    {
        for (int state_no = 1; state_no <= pda -> num_states; state_no++)
        {
            ordered_state[state_no] = state_no;
            row_size[state_no] = 0;
            Dfa::GotoHeader &go_to = pda -> statset[state_no].go_to;
            for (int i = 0; i < go_to.Length(); i++)
            {
                row_size[state_no]++;
                frequency_count[go_to[i].Symbol()]++;
            }
        }
    }

    //
    // The non-terminals are sorted in descending order based on the
    // number of actions defined on then, and they are remapped based on
    // the new arrangement obtained by the sorting.
    //
    Frequency::sort(frequency_symbol, frequency_count, pda -> num_states);

    symbol_map.Resize(grammar -> num_symbols + 1);
    {
        for (int i = grammar -> FirstNonTerminal(); i <= grammar -> LastNonTerminal(); i++)
            symbol_map[frequency_symbol[i]] = i;
    }

    //
    //    All non-terminal entries in the state automaton are updated
    // accordingly.  We further subtract NUM_TERMINALS from each
    // non-terminal to make them fall in the range [1..NUM_NON_TERMINLS]
    // instead of [NUM_TERMINALS+1..NUM_SYMBOLS].
    //
    for (int state_no = 1; state_no <= pda -> num_states; state_no++)
    {
        Dfa::GotoHeader &go_to = pda -> statset[state_no].go_to;
        for (int i = 0; i < go_to.Length(); i++)
           go_to[i].SetSymbol(symbol_map[go_to[i].Symbol()] - grammar -> num_terminals);
    }

    //
    // If Goto-Default was requested, we find out how many non-terminals
    // were eliminated as a result, and adjust the GOTO-DEFAULT map,
    // based on the new mapping of the non-terminals.
    //
    if (option -> goto_default)
    {
        for (last_symbol = grammar -> num_symbols;
             last_symbol > grammar -> num_terminals;
             last_symbol--)
             if (frequency_count[last_symbol] != 0)
                 break;
        last_symbol -= grammar -> num_terminals;

        if (! option -> quiet)
        {
            option -> report.Put("Number of non-terminals eliminated: ");
            option -> report.Put(grammar -> num_nonterminals - last_symbol);
            option -> report.PutChar('\n');
        }

        //
        // Remap the GOTO-DEFAULT map.
        //
        BoundedArray<int> temp_default_goto(grammar -> num_terminals + 1, grammar -> num_symbols);
        {
            for (int symbol = grammar -> FirstNonTerminal(); symbol <= grammar -> LastNonTerminal(); symbol++)
                temp_default_goto[symbol_map[symbol]] = default_goto[symbol];
        }

        for (int symbol = grammar -> FirstNonTerminal(); symbol <= grammar -> LastNonTerminal(); symbol++)
            default_goto[symbol] = temp_default_goto[symbol];
    }
    else last_symbol = grammar -> num_nonterminals;

    //
    // The states are sorted in descending order based on the number of
    // actions defined on them, and they are remapped based on the new
    // arrangement obtained by the sorting.
    //
    Frequency::sort(ordered_state,
                    row_size,
                    1,
                    pda -> num_states,
                    last_symbol);

    return;
}


//
//
//
void Generator::InitializeTables(int num_table_entries, int minimum_increment)
{
    if (num_table_entries + minimum_increment > MAX_TABLE_SIZE)
    {
        Tuple<const char *> msg;
        IntToString size(MAX_TABLE_SIZE);
        msg.Next() = "Table has exceeded maximum limit of ";
        msg.Next() = size.String();

        option -> EmitError(0, msg);
        control -> Exit(12);
    }

    this -> minimum_increment = minimum_increment;

    int increment_size = Util::Max(num_table_entries, minimum_increment);
    table_size = Util::Min((num_table_entries + increment_size), MAX_TABLE_SIZE);

    next.Resize(table_size + 1);
    previous.Resize(table_size + 1);

    first_index = 1;
    previous[first_index] = Util::NIL;
    next[first_index] = first_index + 1;
    for (int indx = 2; indx < table_size; indx++)
    {
        next[indx] = indx + 1;
        previous[indx] = indx - 1;
    }
    last_index = table_size;
    previous[last_index] = last_index - 1;
    next[last_index] = Util::NIL;

    return;
}


//
// This procedure is invoked when the TABLE being used is not large
// enough.  A new table is allocated, the information from the old table
// is copied, and the old space is released.
//
void Generator::Reallocate(int num_table_entries)
{
    //
    // The next index that will be used is (table_size + 1). Recall that
    // we make sure that there are enough spaces allocated to accomodate
    // at least minimum_increment elements.
    //
    if ((table_size + 1) + minimum_increment > MAX_TABLE_SIZE)
    {
        Tuple<const char *> msg;
        IntToString size(MAX_TABLE_SIZE);
        msg.Next() = "Table has exceeded maximum limit of ";
        msg.Next() = size.String();

        option -> EmitError(0, msg);
        control -> Exit(12);
    }

    int old_size = table_size,
        increment_size = Util::Max(num_table_entries, minimum_increment);
    table_size = Util::Min(((table_size + 1) + increment_size), MAX_TABLE_SIZE);

    if (option -> verbose)
    {
        option -> report.Put("Reallocating storage for tables, adding ");
        option -> report.Put(table_size - old_size);
        option -> report.Put(" entries.\n");
    }

    next.Resize(table_size + 1);
    previous.Resize(table_size + 1);

    if (first_index == Util::NIL)
    {
        first_index = old_size + 1;
        previous[first_index] = Util::NIL;
    }
    else
    {
        next[last_index] = old_size + 1;
        previous[old_size + 1] = last_index;
    }

    next[old_size + 1] = old_size + 2;
    for (int i = old_size + 2; i < table_size; i++)
    {
        next[i] = i + 1;
        previous[i] = i - 1;
    }
    last_index = table_size;
    previous[last_index] = last_index - 1;
    next[last_index] = Util::NIL;

    return;
}


//
//
//
int Generator::InsertRow(Tuple<int> &list, int num_table_entries, int last_symbol)
{
    //
    // Look for a suitable index where to overlay the list of symbols
    //
    int index;
    for (index = first_index; true; index = next[index])
    {
        if (index == Util::NIL)
            index = table_size + 1;
        if (index + last_symbol > table_size)
            Reallocate(num_table_entries);

        int i;
        for (i = 0; i < list.Length(); i++)
        {
            assert(list[i] != 0);
            if (next[index + list[i]] == Util::OMEGA)
                break;
        }
        if (i == list.Length()) // did we find a good index?
            break;
    }

    //
    // INDEX marks the starting position for the state, remove all the
    // positions that are claimed by terminal actions in the state.
    //
    for (int j = 0; j < list.Length(); j++)
    {
        int i = index + list[j];
        if (i == last_index)
        {
            last_index = previous[last_index];
            next[last_index] = Util::NIL;
        }
        else
        {
            next[previous[i]] = next[i];
            previous[next[i]] = previous[i];
        }
        next[i] = Util::OMEGA;
    }

    //
    // We now remove the starting position itself from the list, and
    // mark it as taken(CHECK(INDEX) = OMEGA)
    // MAX_INDEX is updated if required.
    // TERM_STATE_INDEX(STATE_NO) is properly set to INDEX as the starting
    // position of STATE_NO.
    //
    if (first_index == last_index)
        first_index = Util::NIL;
    else if (index == first_index)
    {
        first_index = next[first_index];
        previous[first_index] = Util::NIL;
    }
    else if (index == last_index)
    {
        last_index = previous[last_index];
        next[last_index] = Util::NIL;
    }
    else
    {
        next[previous[index]] = next[index];
        previous[next[index]] = previous[index];
    }

    next[index] = Util::OMEGA;

    return index;
}


//
// We now overlap the non-terminal table, or more precisely, we compute the
// starting position in a vector where each of its rows may be placed
// without clobbering elements in another row.  The starting positions are
// stored in the vector STATE_INDEX.
//
void Generator::overlap_nt_rows(void)
{
    int num_table_entries = pda -> num_gotos + pda -> num_goto_reduces + pda -> num_states;
    InitializeTables(num_table_entries, last_symbol);

    int max_index = first_index;

    //
    // We now iterate over all the states in their new sorted order as
    // indicated by the variable STATE_NO, and determine an "overlap"
    // position for them.
    //
    Tuple<int> non_terminal_list;
    state_index.Resize(pda -> max_la_state + 1);
    for (int k = 1; k <= pda -> num_states; k++)
    {
        int state_no = ordered_state[k];

        non_terminal_list.Reset();
        Dfa::GotoHeader &go_to = pda -> statset[state_no].go_to;
        for (int i = 0; i < go_to.Length(); i++)
            non_terminal_list.Next() = go_to[i].Symbol();

        int index = InsertRow(non_terminal_list, num_table_entries, last_symbol);

        if (index > max_index)
            max_index = index;
        state_index[state_no] = index;
    }

    check_size = (option -> goto_default || option -> nt_check
                          ? max_index + grammar -> num_nonterminals
                          : 0);

    for (action_size = max_index + last_symbol; action_size > max_index; action_size--)
    {
        if (next[action_size] == Util::OMEGA)
            break;
    }
    action_size++;

    accept_act = action_size + grammar -> num_rules + 1;
    error_act = accept_act + pda -> conflicts.Length();

    if (! option -> quiet)
    {
        int percentage = ((action_size - num_table_entries) * 1000) / num_table_entries;

        option -> report.PutChar('\n');
        option -> report.Put("Number of entries in base Action Table: ");
        option -> report.Put(num_table_entries);
        option -> report.PutChar('\n');
        option -> report.Put("Additional space required for compaction of Action Table: ");
        option -> report.Put(percentage / 10);
        option -> report.PutChar('.');
        option -> report.Put(percentage % 10);
        option -> report.Put("%\n");
    }

    return;
}


//
// We now try to merge states in the terminal table that are similar.
// Two states S1 and S2 are said to be similar if they contain the
// same shift actions and they reduce to the same set of rules. In
// addition,  there must not exist a terminal symbol "t" such that:
// REDUCE(S1, t) and REDUCE(S2, t) are defined, and
// REDUCE(S1, t) ^= REDUCE(S2, t)
//
void Generator::merge_similar_t_rows(void)
{
    empty_root = Util::NIL;
    single_root = Util::NIL;
    multi_root = Util::NIL;
    top = 0;
    new_state_element.Next(); // skip the 0th element

    Array<int> table(pda -> num_shift_maps + 1, Util::NIL);

    //
    // We now hash all the states into TABLE, based on their shift map
    // number.
    // The rules in the range of the REDUCE MAP are placed in sorted
    // order in a linear linked list headed by REDUCE_ROOT.
    //
    Tuple<int> rule_list;
    Array<int> next_rule(grammar -> num_rules + 1, Util::OMEGA);
    int rule_root = Util::NIL;
    state_list.Resize(pda -> max_la_state + 1);
    for (int state_no = 1; state_no <= pda -> max_la_state; state_no++)
    {
        //
        // Ignore all look-ahead states that are unreachable.
        //
        if (state_no > pda -> num_states && pda -> statset[state_no].predecessors.Length() == 0)
            continue;

        Dfa::ReduceHeader &red = pda -> statset[state_no].reduce;
        for (int i = 1; i < red.Length(); i++)
        {
            int rule_no = red[i].RuleNumber(),
                q,
                trail = Util::NIL;
            for (q = rule_root; q != Util::NIL; trail = q, q = next_rule[q])
            {
                if (q >= rule_no) // Is it in the list already or not?
                    break;
            }
            if (q == rule_no)
                continue;

            if (q == rule_root)
            {
                next_rule[rule_no] = rule_root;
                rule_root = rule_no;
            }
            else
            {
                next_rule[rule_no] = q;
                next_rule[trail] = rule_no;
            }
        }

        //
        // Place the sorted list of rules in rule_list and reset
        // rule_root/next_rule to the empty set...
        //
        rule_list.Reset();
        for (int rule_no = rule_root; rule_no != Util::NIL; rule_no = rule_root)
        {
            rule_root = next_rule[rule_root]; // move to next element
            next_rule[rule_no] = Util::OMEGA;  // reclaim current element
            rule_list.Next() = rule_no; // add element to list
        }

        //
        // We compute the HASH_ADDRESS, mark if the state has a shift
        // action on the ERROR symbol, and search the hash TABLE to see
        // if a state matching the description is already in there.
        //
        Dfa::ShiftHeader &sh = pda -> Shift(state_no);
        int index = sh.Index(grammar -> error_image);
        if (index != Util::OMEGA)
        {
            int act = sh[index].Action();
            if (act < pda -> max_la_state)
                shift_on_error_symbol[state_no] = true;
            else
            {
                int k;
                for (k = act - pda -> max_la_state; pda -> conflicts[k] != 0; k++)
                {
                    if (pda -> conflicts[k] < 0 ||
                        pda -> conflicts[k] > grammar -> num_rules)
                        break;
                }
                shift_on_error_symbol[state_no] = (pda -> conflicts[k] != 0);
            }
        }

        int hash_address = pda -> statset[state_no].shift_number,
            j;
        for (j = table[hash_address]; j != Util::NIL; j = new_state_element[j].link)
        {
            if (rule_list == new_state_element[j].reduce_range)
                break;
        }

        //
        // If the state is a new state to be inserted in the table, we now
        // do so,  and place it in the proper category based on its reduces,
        // In any case, the IMAGE field is updated, and so is the relevant
        // STATE_LIST element.
        //
        // If the state contains a shift action on the error symbol and also
        // contains reduce actions,  we allocate a new element for it and
        // place it in the list headed by MULTI_ROOT.  Such states are not
        // merged, because we do not take default reductions in them.
        //
        if (shift_on_error_symbol[state_no] && rule_list.Length() > 0)
        {
            top++;
            int index = new_state_element.NextIndex();
            if (j == Util::NIL)
            {
                new_state_element[index].link = table[hash_address];
                table[hash_address] = index;
            }

            new_state_element[index].thread = multi_root;
            multi_root = index;

            new_state_element[index].shift_number = hash_address;
            new_state_element[index].reduce_range = rule_list;
            state_list[state_no] = Util::NIL;
            new_state_element[index].image = state_no;
        }
        else if (j == Util::NIL)
        {
            top++;
            int index = new_state_element.NextIndex();
            new_state_element[index].link = table[hash_address];
            table[hash_address] = index;
            if (rule_list.Length() == 0)
            {
                new_state_element[index].thread = empty_root;
                empty_root = index;
            }
            else if (rule_list.Length() == 1)
            {
                new_state_element[index].thread = single_root;
                single_root = index;
            }
            else
            {
                new_state_element[index].thread = multi_root;
                multi_root = index;
            }
            new_state_element[index].shift_number = hash_address;
            new_state_element[index].reduce_range = rule_list;
            state_list[state_no] = Util::NIL;
            new_state_element[index].image = state_no;
        }
        else
        {
            state_list[state_no] = new_state_element[j].image;
            new_state_element[j].image = state_no;
        }
    }

    return;
}


//
// If shift-default actions are requested, the shift actions
// associated with each state are factored out of the Action matrix
// and all identical rows are merged.  This merged matrix is used to
// create a boolean vector that may be used to confirm whether or not
// there is a shift action in a given state S on a given symbol t.
// If we can determine that there is a shift action on a pair (S, t)
// we can apply shift default to the Shift actions just like we did
// for the Goto actions.
//
void Generator::merge_shift_domains(void)
{
    //
    // Some of the rows in the shift action map have already been merged
    // by the merging of compatible states... We simply need to increase
    // the size of the granularity by merging these new terminal states
    // based only on their shift actions.
    //
    // The array SHIFT_DOMAIN_TABLE is used as the base for a hash table.
    // Each submap represented by a row of the shift action map is hashed
    // into this table by summing the terminal symbols in its domain.
    // The submap is then entered in the hash table and assigned a unique
    // number if such a map was not already placed in the hash table.
    // Otherwise, the number assigned to the previous submap is also
    // associated with the new submap.
    //
    // The vector SHIFT_IMAGE is used to keep track of the unique number
    // associated with each unique shift submap.
    // The vector REAL_SHIFT_NUMBER is the inverse of SHIFT_IMAGE. It is
    // used to associate each unique number to its shift submap.
    // The integer NUM_TABLE_ENTRIES is used to count the number of
    // elements in the new merged shift map.
    //
    // The arrays ORDERED_SHIFT and ROW_SIZE are also initialized here.
    // They are used to sort the rows of the shift actions map later...
    //
    Array<int> shift_domain_table(SHIFT_TABLE_SIZE, Util::NIL),
               shift_domain_link(num_terminal_states + 1),
               ordered_shift(pda -> num_shift_maps + 1),
               terminal_list(grammar -> num_terminals + 1);

    Array<bool> shift_symbols(grammar -> num_terminals + 1);

    shift_domain_count = 0;

    shift_image.Resize(pda -> max_la_state + 1);
    real_shift_number.Resize(pda -> num_shift_maps + 1);

    //
    // The NULL shift map is assigned index 0.
    //
    real_shift_number[0] = 0;
    ordered_shift[0] = 0;
    terminal_row_size[0] = 0;

    int num_table_entries = 0;
    {
        for (int state_no = 1; state_no <= num_terminal_states; state_no++)
        {
            int shift_no = new_state_element[state_no].shift_number;
            Dfa::ShiftHeader &sh = pda -> shift[shift_no];
            int shift_size = sh.Length();
            if (shift_size == 0) // The empty shift map is not considered
            {
                assert(shift_no == 0);
                shift_image[state_no] = shift_no;
            }
            else
            {
                shift_symbols.MemReset();
                unsigned hash_address = shift_size;
                for (int k = 0; k < shift_size; k++)
                {
                    int symbol = sh[k].Symbol();
                    hash_address += symbol;
                    shift_symbols[symbol] = true;
                }

                hash_address %= SHIFT_TABLE_SIZE;

                int i;
                for (i = shift_domain_table[hash_address]; i != Util::NIL; i = shift_domain_link[i])
                {
                    Dfa::ShiftHeader &sh = pda -> shift[new_state_element[i].shift_number];
                    if (sh.Length() == shift_size)
                    {
                        int j;
                        for (j = 0; j < shift_size; j++)
                        {
                            if (! shift_symbols[sh[j].Symbol()])
                                break;
                        }
                        if (j == shift_size) // If a perfect match was found, point to it.
                        {
                            shift_image[state_no] = shift_image[i];
                            break;
                        }
                    }
                }

                //
                // If the shift map was not found, add it to the table.
                //
                if (i == Util::NIL)
                {
                    shift_domain_link[state_no] = shift_domain_table[hash_address];
                    shift_domain_table[hash_address] = state_no;

                    shift_domain_count++;
                    shift_image[state_no] = shift_domain_count;
                    real_shift_number[shift_domain_count] = shift_no;
                    ordered_shift[shift_domain_count] = shift_domain_count;
                    terminal_row_size[shift_domain_count] = shift_size;
                    num_table_entries += shift_size;
                }
            }
        }
    }

    //
    // Compute the frequencies, and remap the terminal symbols
    // accordingly.
    //
    {
        for (int symbol = grammar -> FirstTerminal(); symbol <= grammar -> LastTerminal(); symbol++)
        {
            terminal_frequency_symbol[symbol] = symbol;
            terminal_frequency_count[symbol] = 0;
        }
    }

    {
        for (int i = 1; i <= shift_domain_count; i++)
        {
            int shift_no = real_shift_number[i];
            Dfa::ShiftHeader &sh = pda -> shift[shift_no];
            for (int j = 0; j < sh.Length(); j++)
                terminal_frequency_count[sh[j].Symbol()]++;
        }
    }

    if (option -> remap_terminals)
        Frequency::sort(terminal_frequency_symbol,
                        terminal_frequency_count,
                        1,
                        grammar -> num_terminals,
                        shift_domain_count);

    {
        for (int symbol = grammar -> FirstTerminal(); symbol <= grammar -> LastTerminal(); symbol++)
            symbol_map[terminal_frequency_symbol[symbol]] =  symbol;
    }

    symbol_map[Grammar::DEFAULT_SYMBOL] = Grammar::DEFAULT_SYMBOL;
    grammar -> identifier_image = (grammar -> identifier_image == 0
                                       ? 0
                                       : symbol_map[grammar -> identifier_image]);
    grammar -> eof_image = symbol_map[grammar -> eof_image];
    if (option -> error_maps)
    {
        grammar -> error_image = symbol_map[grammar -> error_image];
        grammar -> eol_image = symbol_map[grammar -> eol_image];
    }

    for (int i = 1; i <= pda -> num_shift_maps; i++)
    {
        Dfa::ShiftHeader &sh = pda -> shift[i];
        for (int j = 0; j < sh.Length(); j++)
             sh[j].SetSymbol(symbol_map[sh[j].Symbol()]);
    }

    for (int state_no = 1; state_no <= num_terminal_states; state_no++)
    {
        Dfa::ReduceHeader &red = new_state_element[state_no].reduce;
        for (int i = 1; i < red.Length(); i++)
            red[i].SetSymbol(symbol_map[red[i].Symbol()]);
    }

    //
    // If ERROR_MAPS are requested, we also have to remap the original
    // REDUCE maps.
    //
    if (option -> error_maps)
    {
        for (int state_no = 1; state_no <= pda -> num_states; state_no++)
        {
            Dfa::ReduceHeader &red = pda -> statset[state_no].reduce;
            for (int i = 1; i < red.Length(); i++)
                red[i].SetSymbol(symbol_map[red[i].Symbol()]);
        }
    }

    //
    // Remap the DEFAULT_SHIFT map.
    //
    Array<int> temp_default_shift(grammar -> num_terminals + 1);
    {
        for (int symbol = grammar -> FirstTerminal(); symbol <= grammar -> LastTerminal(); symbol++)
            temp_default_shift[symbol_map[symbol]] = default_shift[symbol];
    }

    {
        for (int symbol = grammar -> FirstTerminal(); symbol <= grammar -> LastTerminal(); symbol++)
            default_shift[symbol] = temp_default_shift[symbol];
    }

    //
    // We now compute the starting position for each Shift check row
    // as we did for the terminal states.  The starting positions are
    // stored in the vector SHIFT_CHECK_INDEX.
    //
    Frequency::sort(ordered_shift,
                    terminal_row_size,
                    1,
                    shift_domain_count,
                    grammar -> num_terminals);

    InitializeTables(num_table_entries, grammar -> num_terminals);

    int max_indx = first_index;

    //
    // Look for a suitable index where to overlay the shift check row.
    // Note that we start this loop at index 0 because we have to also
    // overlay the empty shift map in the shift_check table.
    //
    shift_check_index.Resize(pda -> num_shift_maps + 1);
    for (int k = 0; k <= shift_domain_count; k++)
    {
        int shift_no = ordered_shift[k];
        Dfa::ShiftHeader &sh = pda -> shift[real_shift_number[shift_no]];

        int indx;
        for (indx = first_index; true; indx = next[indx])
        {
            if (indx == Util::NIL)
                indx = table_size + 1;
            if (indx + grammar -> num_terminals > table_size)
                Reallocate(num_table_entries);

            int i;
            for (i = 0; i < sh.Length(); i++)
            {
                int symbol = sh[i].Symbol();
                if (next[indx + symbol] == Util::OMEGA)
                    break;
            }
            if (i == sh.Length()) // did we find a good index?
                break;
        }

        //
        // INDX marks the starting position for the row, remove all the
        // positions that are claimed by that shift check row.
        // If a position has the value 0,   then it is the starting position
        // of a Shift row that was previously processed, and that element
        // has already been removed from the list of available positions.
        //
        for (int j = 0; j < sh.Length(); j++)
        {
            int symbol = sh[j].Symbol();
            int i = indx + symbol;
            if (next[i] != 0)
            {
                if (i == last_index)
                {
                    last_index = previous[last_index];
                    next[last_index] = Util::NIL;
                }
                else
                {
                    next[previous[i]] = next[i];
                    previous[next[i]] = previous[i];
                }
            }
            next[i] = Util::OMEGA;
        }

        //
        // We now remove the starting position itself from the list without
        // marking it as taken, since it can still be used for a shift check.
        // MAX_INDX is updated if required.
        // SHIFT_CHECK_INDEX(SHIFT_NO) is properly set to INDX as the
        // starting position of STATE_NO.
        //
        if (first_index == last_index)
            first_index = Util::NIL;
        else if (indx == first_index)
        {
            first_index = next[first_index];
            previous[first_index] = Util::NIL;
        }
        else if (indx == last_index)
        {
            last_index = previous[last_index];
            next[last_index] = Util::NIL;
        }
        else
        {
            next[previous[indx]] = next[indx];
            previous[next[indx]] = previous[indx];
        }
        next[indx] = 0;

        if (indx > max_indx)
            max_indx = indx;
        shift_check_index[shift_no] = indx;
    }

    //
    // Update all counts, and report statistics.
    //
    shift_check_size = max_indx + grammar -> num_terminals;

    int j;
    for (j = shift_check_size; j >= max_indx; j--)
    {
        if (next[j] == Util::OMEGA)
            break;
    }

    if (! option -> quiet)
    {
        int percentage = ((j - num_table_entries) * 1000) / num_table_entries;

        option -> report.PutChar('\n');
        option -> report.Put("Number of entries in Shift Check Table: ");
        option -> report.Put(num_table_entries);
        option -> report.PutChar('\n');
        option -> report.Put("Additional space required for compaction of Shift Check Table: ");
        option -> report.Put(percentage /10);
        option -> report.PutChar('.');
        option -> report.Put(percentage % 10);
        option -> report.Put("%\n");
    }

    return;
}


//
// By now, similar states have been grouped together, and placed in
// one of three linear linked lists headed by the root pointers:
// MULTI_ROOT, SINGLE_ROOT, and EMPTY_ROOT.
// We iterate over each of these lists and construct new states out
// of these groups of similar states when they are compatible. Then,
// we remap the terminal symbols.
//
int Generator::overlay_sim_t_rows(void)
{
    int num_shifts_saved = 0,
        num_conflicts_saved = 0,
        num_reductions_saved = 0,
        default_saves = 0;

    //
    // We first iterate over the groups of similar states in the
    // MULTI_ROOT list.  These states have been grouped together,
    // because they have the same Shift map, and reduce to the same
    // rules, but we must check that they are fully compatible by making
    // sure that no two states contain reduction to a different rule on
    // the same symbol.
    //     The idea is to take a state out of the group, and merge with
    // it as many other compatible states from the group as possible.
    // remaining states from the group that caused clashes are thrown
    // back into the MULTI_ROOT list as a new group of states.
    //
    Array<int> rule_count(grammar -> num_rules + 1),
               reduce_action(grammar -> num_terminals + 1);
    for (int i = multi_root; i != Util::NIL; i = new_state_element[i].thread)
    {
        for (int l = 0; l < new_state_element[i].reduce_range.Length(); l++)
        {
            int rule_no = new_state_element[i].reduce_range[l];
            rule_count[rule_no] = 0;
        }

        //
        // REDUCE_ACTION is used to keep track of reductions that are to be
        // applied on terminal symbols as the states are merged.  We pick
        // out the first state (STATE_NO) from the group of states involved,
        // initialize REDUCE_ACTION with its reduce map, and count the number
        // of reductions associated with each rule in that state.
        //
        reduce_action.Initialize(Util::OMEGA);
        int state_no = new_state_element[i].image;
        Dfa::ReduceHeader &red = pda -> statset[state_no].reduce;
        for (int j = 1; j < red.Length(); j++)
        {
            int rule_no = red[j].RuleNumber();
            reduce_action[red[j].Symbol()] = rule_no;
            rule_count[rule_no]++;
        }

        //
        // STATE_SET_ROOT is used to traverse the rest of the list that form
        // the group of states being processed.  STATE_SUBSET_ROOT is used
        // to construct the new list that will consist of all states in the
        // group that are compatible starting with the initial state.
        // STATE_ROOT is used to construct a list of all states in the group
        // that are not compatible with the initial state.
        //
        int state_set_root = state_list[state_no],
            state_subset_root = state_no,
            state_root = Util::NIL;
        state_list[state_subset_root] = Util::NIL;
        {
            for (int state_no = state_set_root; state_no != Util::NIL; state_no = state_set_root)
            {
                state_set_root = state_list[state_set_root];

                //
                // We traverse the reduce map of the state taken out from the group
                // and check to see if it is compatible with the subset being
                // constructed so far.
                //
                Dfa::ReduceHeader &red = pda -> statset[state_no].reduce;
                int j;
                for (j = 1; j < red.Length(); j++)
                {
                    int symbol = red[j].Symbol();
                    if (reduce_action[symbol] != Util::OMEGA)
                    {
                        if (reduce_action[symbol] != red[j].RuleNumber())
                            break;
                    }
                }

                //
                // If J > Q->REDUCE_ELEMENT.N then we traversed the whole reduce map,
                // and all the reductions involved are compatible with the new state
                // being constructed.  The state involved is added to the subset, the
                // rule counts are updated, and the REDUCE_ACTIONS map is updated.
                //     Otherwise, we add the state involved to the STATE_ROOT list
                // which will be thrown back in the MULTI_ROOT list.
                //
                if (j == red.Length())
                {
                    state_list[state_no] = state_subset_root;
                    state_subset_root = state_no;
                    for (j = 1; j < red.Length(); j++)
                    {
                        int symbol = red[j].Symbol();
                        if (reduce_action[symbol] == Util::OMEGA)
                        {
                            int rule_no = red[j].RuleNumber();
                            if (grammar -> rules[rule_no].lhs == grammar -> accept_image)
                                rule_no = 0;
                            reduce_action[symbol] = rule_no;
                            rule_count[rule_no]++;
                        }
                        else num_reductions_saved++;
                    }
                }
                else
                {
                    state_list[state_no] = state_root;
                    state_root = state_no;
                }
            }
        }

        //
        // Figure out the best default rule candidate, and update
        // DEFAULT_SAVES.
        // Recall that all accept actions were changed into reduce actions
        // by rule 0.
        //
        int k = 0,
            reduce_size = 0,
            default_rule = error_act;
        for (int e = 0; e < new_state_element[i].reduce_range.Length(); e++)
        {
            int rule_no = new_state_element[i].reduce_range[e];
            reduce_size += rule_count[rule_no];
            if ((rule_count[rule_no] > k) && (rule_no != 0)
                && ! shift_on_error_symbol[state_subset_root])
            {
                k = rule_count[rule_no];
                default_rule = rule_no;
            }
        }
        default_saves += k;
        reduce_size -= k;

        //
        // If STATE_ROOT is not NIL then there are states in the group that
        // did not meet the compatibility test.  Throw those states back in
        // front of MULTI_ROOT as a group.
        //
        if (state_root != Util::NIL)
        {
            top++;
            int index = new_state_element.NextIndex();
            new_state_element[index].thread = new_state_element[i].thread;
            new_state_element[i].thread = index;
            new_state_element[index].shift_number = pda -> statset[state_root].shift_number;
            new_state_element[index].reduce_range = new_state_element[i].reduce_range;
            new_state_element[index].image = state_root;
        }

        //
        // Create Reduce map for the newly created terminal state.
        // We may assume that SYMBOL field of defaults is already set to
        // the DEFAULT_SYMBOL value.
        //
        Dfa::ReduceHeader &new_red = new_state_element[i].reduce;
        new_red.Resize(reduce_size + 1);
        new_red[0].SetSymbol(Grammar::DEFAULT_SYMBOL);
        new_red[0].SetRuleNumber(default_rule);
        for (int symbol = grammar -> FirstTerminal(); symbol <= grammar -> LastTerminal(); symbol++)
        {
            if (reduce_action[symbol] != Util::OMEGA)
            {
                if (reduce_action[symbol] != default_rule)
                {
                    new_red[reduce_size].SetSymbol(symbol);
                    new_red[reduce_size].SetRuleNumber(reduce_action[symbol] == 0
                                                                              ? accept_act
                                                                              : reduce_action[symbol]);
                    reduce_size--;
                }
            }
        }
        assert(reduce_size == 0);
        new_state_element[i].image = state_subset_root;
    }

    //
    // We now process groups of states that have reductions to a single
    // rule.  Those states are fully compatible, and the default is the
    // rule in question.
    // Any of the REDUCE_ELEMENT maps that belongs to a state in the
    // group of states being processed may be reused for the new  merged
    // state.
    //
    {
        for (int i = single_root; i != Util::NIL; i = new_state_element[i].thread)
        {
            int state_no = new_state_element[i].image,
                rule_no = new_state_element[i].reduce_range[0];
            if (grammar -> rules[rule_no].lhs == grammar -> accept_image)
            {
                Dfa::ReduceHeader &red = pda -> statset[state_no].reduce;
                Dfa::ReduceHeader &new_red = new_state_element[i].reduce;
                new_red.Resize(red.Length());

                new_red[0].SetSymbol(Grammar::DEFAULT_SYMBOL);
                new_red[0].SetRuleNumber(error_act);
                for (int j = 1; j < new_red.Length(); j++)
                {
                    new_red[j].SetSymbol(red[j].Symbol());
                    new_red[j].SetRuleNumber(accept_act);
                }
            }
            else
            {
                reduce_action.Initialize(Util::OMEGA);

                for (; state_no != Util::NIL; state_no = state_list[state_no])
                {
                    Dfa::ReduceHeader &red = pda -> statset[state_no].reduce;
                    for (int j = 1; j < red.Length(); j++)
                    {
                        int symbol = red[j].Symbol();
                        if (reduce_action[symbol] == Util::OMEGA)
                        {
                            reduce_action[symbol] = rule_no;
                            default_saves++;
                        }
                        else num_reductions_saved++;
                    }
                }

                Dfa::ReduceHeader &new_red = new_state_element[i].reduce;
                new_red.Resize(1);
                new_red[0].SetSymbol(Grammar::DEFAULT_SYMBOL);
                new_red[0].SetRuleNumber(rule_no);
            }
        }
    }

    //
    // Groups of states that have no reductions are also compatible.
    // Their default is ERROR_ACTION.
    //
    {
        for (int i = empty_root; i != Util::NIL; i = new_state_element[i].thread)
        {
            Dfa::ReduceHeader &new_red = new_state_element[i].reduce;
            new_red.Resize(1);
            new_red[0].SetSymbol(Grammar::DEFAULT_SYMBOL);
            new_red[0].SetRuleNumber(error_act);
        }
    }

    assert(top == new_state_element.Length() - 1);
    num_terminal_states = new_state_element.Length() - 1;

    terminal_frequency_symbol.Resize(grammar -> num_terminals + 1);
    terminal_frequency_count.Resize(grammar -> num_terminals + 1);
    terminal_row_size.Resize(pda -> max_la_state + 1);
    if (option -> shift_default)
        merge_shift_domains();

    //
    // We now reorder the terminal states based on the number of actions
    // in them, and remap the terminal symbols if they were not already
    // remapped in the previous block for the SHIFT_CHECK vector.
    //
    for (int symbol = grammar -> FirstTerminal(); symbol <= grammar -> LastTerminal(); symbol++)
    {
        terminal_frequency_symbol[symbol] = symbol;
        terminal_frequency_count[symbol] = 0;
    }

    int num_table_entries = num_terminal_states;
    {
        for (int i = 1; i <= num_terminal_states; i++)
        {
            ordered_state[i] = i;
            terminal_row_size[i] = 0;
            int num_conflicts = 0;
            Dfa::ShiftHeader &sh = pda -> shift[new_state_element[i].shift_number];
            for (int j = 0; j < sh.Length(); j++)
            {
                int symbol = sh[j].Symbol();
                if ((! option -> shift_default) || (sh[j].Action() != default_shift[symbol]))
                {
                    terminal_row_size[i]++;
                    if (sh[j].Action() > pda -> max_la_state)
                        num_conflicts++;
                    terminal_frequency_count[symbol]++;
                }
            }

            for (int state_no = state_list[new_state_element[i].image];
                 state_no != Util::NIL; state_no = state_list[state_no])
            {
                num_shifts_saved += (terminal_row_size[i] - num_conflicts);
                num_conflicts_saved += num_conflicts;
            }

            //
            // Note that the Default action is skipped !!!
            //
            Dfa::ReduceHeader &red = new_state_element[i].reduce;
            {
                for (int j = 1; j < red.Length(); j++)
                {
                    terminal_row_size[i]++;
                    terminal_frequency_count[red[j].Symbol()]++;
                }
            }
            num_table_entries += terminal_row_size[i];
        }
    }

    if (! option -> quiet)
    {
        option -> report.PutChar('\n');
        option -> report.Put("Number of unique terminal states: ");
        option -> report.Put(num_terminal_states);
        option -> report.PutChar('\n');
        option -> report.Put("Number of Shift actions saved by merging: ");
        option -> report.Put(num_shifts_saved);
        option -> report.PutChar('\n');
        option -> report.Put("Number of Conflict points saved by merging: ");
        option -> report.Put(num_conflicts_saved);
        option -> report.PutChar('\n');
        option -> report.Put("Number of Reduce actions saved by merging: ");
        option -> report.Put(num_reductions_saved);
        option -> report.PutChar('\n');
        option -> report.Put("Number of Reduce saved by default: ");
        option -> report.Put(default_saves);
        option -> report.PutChar('\n');
    }

    Frequency::sort(ordered_state,
                    terminal_row_size,
                    1,
                    num_terminal_states,
                    grammar -> num_terminals);

    if (! option -> shift_default)
    {
        if (option -> remap_terminals)
            Frequency::sort(terminal_frequency_symbol,
                            terminal_frequency_count,
                            1,
                            grammar -> num_terminals,
                            num_terminal_states);

        for (int symbol = grammar -> FirstTerminal(); symbol <= grammar -> LastTerminal(); symbol++)
            symbol_map[terminal_frequency_symbol[symbol]] =  symbol;
        symbol_map[Grammar::DEFAULT_SYMBOL] = Grammar::DEFAULT_SYMBOL;
        grammar -> identifier_image = (grammar -> identifier_image == 0
                                           ? 0
                                           : symbol_map[grammar -> identifier_image]);
        grammar -> eof_image = symbol_map[grammar -> eof_image];
        if (option -> error_maps)
        {
            grammar -> error_image = symbol_map[grammar -> error_image];
            grammar -> eol_image = symbol_map[grammar -> eol_image];
        }

        for (int i = 1; i <= pda -> num_shift_maps; i++)
        {
            Dfa::ShiftHeader &sh = pda -> shift[i];
            for (int j = 0; j < sh.Length(); j++)
                sh[j].SetSymbol(symbol_map[sh[j].Symbol()]);
        }

        for (int state_no = 1; state_no <= num_terminal_states; state_no++)
        {
            Dfa::ReduceHeader &red = new_state_element[state_no].reduce;
            for (int i = 1; i < red.Length(); i++)
                red[i].SetSymbol(symbol_map[red[i].Symbol()]);
        }

        //
        // If ERROR_MAPS are requested, we also have to remap the original
        // REDUCE maps.
        //
        if (option -> error_maps)
        {
            for (int state_no = 1; state_no <= pda -> num_states; state_no++)
            {
                Dfa::ReduceHeader &red = pda -> statset[state_no].reduce;
                for (int i = 1; i < red.Length(); i++)
                    red[i].SetSymbol(symbol_map[red[i].Symbol()]);
            }
        }
    }

    return num_table_entries;
}


//
// We now compute the starting position for each terminal state just
// as we did for the non-terminal states.
// The starting positions are stored in the vector TERM_STATE_INDEX.
//
void Generator::overlap_t_rows(int num_table_entries)
{
    InitializeTables(num_table_entries, grammar -> num_terminals);

    int max_indx = first_index;
    Tuple<int> terminal_list;
    term_state_index.Resize(pda -> max_la_state + 1);
    for (int k = 1; k <= num_terminal_states; k++)
    {
        int state_no = ordered_state[k];

        //
        // For the terminal table, we are dealing with two lists, the SHIFT
        // list, and the REDUCE list. Those lists are merged together first
        // in TERMINAL_LIST.  Since we have to iterate over the list twice,
        // this merging makes things easy.
        //
        terminal_list.Reset();
        Dfa::ShiftHeader &sh = pda -> shift[new_state_element[state_no].shift_number];
        for (int i = 0; i < sh.Length(); i++)
        {
            int symbol = sh[i].Symbol();
            if ((! option -> shift_default) || (sh[i].Action() != default_shift[symbol]))
                terminal_list.Next() = symbol;
        }

        Dfa::ReduceHeader &red = new_state_element[state_no].reduce;
        {
            for (int i = 1; i < red.Length(); i++)
                terminal_list.Next() = red[i].Symbol();
        }

        int indx = InsertRow(terminal_list, num_table_entries, grammar -> num_terminals);
        if (indx > max_indx)
            max_indx = indx;
        term_state_index[state_no] = indx;
    }

    //
    // Update all counts, and report statistics.
    //
    term_check_size = max_indx + grammar -> num_terminals;
    for (term_action_size = max_indx + grammar -> num_terminals;
         term_action_size >= max_indx; term_action_size--)
        if (next[term_action_size] == Util::OMEGA)
            break;

    if (! option -> quiet)
    {
        int percentage = ((term_action_size - num_table_entries) * 1000) / num_table_entries;

        option -> report.PutChar('\n');
        option -> report.Put("Number of entries in Terminal Action Table: ");
        option -> report.Put(num_table_entries);
        option -> report.PutChar('\n');
        option -> report.Put("Additional space required for compaction of Terminal Table: ");
        option -> report.Put(percentage / 10);
        option -> report.PutChar('.');
        option -> report.Put(percentage % 10);
        option -> report.Put("%\n");
    }

    //
    // compute the max size for table_size.
    //
    table_size = Util::Max(check_size, term_check_size);
    table_size = Util::Max(table_size, shift_check_size);
    table_size = Util::Max(table_size, action_size);
    table_size = Util::Max(table_size, term_action_size);

    return;
}


//
//
//
void Generator::Generate(Table *parse_table)
{
    //
    // Check whether or not table can be generated.
    //
    int la_state_offset = (option -> read_reduce
                                   ? error_act + grammar -> num_rules
                                   : error_act);
    if (la_state_offset > (MAX_TABLE_SIZE + 1)) // are we off limit?
    {
        Tuple<const char *> msg;
        IntToString size(MAX_TABLE_SIZE + 1);
        msg.Next() = "Table contains entries that are > ";
        msg.Next() = size.String();
        msg.Next() = "; Processing stopped.";

        option -> EmitError(0, msg);
        control -> Exit(12);
    }

    //
    // Build nullables array.
    //
    Table::IntArrayInfo &nullables = parse_table -> data.Next();
    nullables.name_id = Table::NULLABLES;
    nullables.type_id = Table::B;
    nullables.array.Resize(grammar -> num_symbols + 1);
    nullables.array.MemReset();
    for (int i = 0; i < nullable_nonterminals.Length(); i++)
    {
        int symbol = nullable_nonterminals[i];
        nullables.array[symbol_map[symbol]] = 1; // set to true
    }

    //
    // Build the prostheses index array.
    //
    /*
    Table::IntArrayInfo &prostheses_index = parse_table -> data.Next();
    prostheses_index.name_id = Table::PROSTHESES_INDEX;
    prostheses_index.type_id = parse_table -> Type(0, grammar -> num_nonterminals);
    prostheses_index.array.Resize(grammar -> num_symbols + 1);
    prostheses_index.array.MemReset();
    prostheses_index.array[0] = 0;
    for (int symbol = grammar -> FirstTerminal(); symbol <= grammar -> LastNonTerminal(); symbol++)
    {
        prostheses_index.array[symbol_map[symbol]] = symbol + 1;
    }
    */
    Table::IntArrayInfo &prostheses_index = parse_table -> data.Next();
    prostheses_index.name_id = Table::PROSTHESES_INDEX;
    prostheses_index.type_id = parse_table -> Type(0, grammar -> num_nonterminals);
    prostheses_index.array.Resize(grammar -> num_nonterminals + 1);
    prostheses_index.array.MemReset();
    prostheses_index.array[0] = 0;
    for (int symbol = grammar -> FirstNonTerminal(); symbol <= grammar -> LastNonTerminal(); symbol++)
    {
        prostheses_index.array[symbol_map[symbol] - grammar -> num_terminals] = symbol - grammar -> num_terminals;
    }

/* TODO: Remove this ... Debug info for prostheses
cout << "\n\nAgain:\n\n";
for (int i = 1; i <= grammar -> num_symbols; i++)
cout << grammar -> RetrieveString(i) << " " << i << "\n";
cout << "\n";
for (int i = 1; i <= grammar -> num_nonterminals; i++)
cout << grammar -> RetrieveString(prostheses_index.array[i] + grammar -> num_terminals) << "; prostheses_index[" << i << "] = " << prostheses_index.array[i] << "\n";
*/

    //
    // Build keywords array.
    //
    Table::IntArrayInfo &keywords = parse_table -> data.Next();
    keywords.name_id = Table::KEYWORDS;
    keywords.type_id = Table::B;
    keywords.array.Resize(grammar -> num_terminals + 1);
    keywords.array.MemReset();
    {
        for (int i = 0; i < grammar -> keywords.Length(); i++)
        {
            int symbol = grammar -> keywords[i];
            keywords.array[symbol_map[symbol]] = 1; // set to true
        }
    }

    //
    // Do we need an extended check table for error recovery or debugging?
    //
    if (check_size == 0 && (option -> error_maps || option -> debug))
        check_size = action_size;

    assert(Grammar::DEFAULT_SYMBOL == 0);
    Table::IntArrayInfo &rhs_and_baseCheck = parse_table -> data.Next();
    rhs_and_baseCheck.name_id = Table::BASE_CHECK;
    rhs_and_baseCheck.array.Resize(grammar -> num_rules + check_size + 1);
    rhs_and_baseCheck.array.MemReset();
    Table::IntArrayInfo &lhs_and_baseAction = parse_table -> data.Next();
    lhs_and_baseAction.name_id = Table::BASE_ACTION;
    lhs_and_baseAction.array.Resize(grammar -> num_rules + action_size + pda -> conflicts.Length() + 1);
    lhs_and_baseAction.array.Initialize(error_act);
    int high_index = (option -> backtrack ? error_act + grammar -> num_rules : error_act);
    {
        for (int i = 1; i <= num_terminal_states; i++)
        {
            int indx = term_state_index[i],
                state_no = new_state_element[i].image;

            //
            // Update the action link between the non-terminal and terminal
            // tables.  If error-maps are requested, an indirect linking is made
            // as follows:
            // Each non-terminal row identifies its original state number, and
            // a new vector START_TERMINAL_STATE indexable by state numbers
            // identifies the starting point of each state in the terminal table.
            //
            for (; state_no != Util::NIL; state_no = state_list[state_no])
            {
                if (state_no <= pda -> num_states)
                {
                    lhs_and_baseAction.array[grammar -> num_rules + state_index[state_no]] = indx;
                    high_index = Util::Max(high_index, indx);
                }
                else // a lookahead state?
                {
                    int act = la_state_offset + indx;
                    state_index[state_no] = act;
                }
            }
        }
    }

    //
    //  Now update the non-terminal tables with the non-terminal actions.
    //
    {
        for (int state_no = 1; state_no <= pda -> num_states; state_no++)
        {
            int indx = state_index[state_no];
            Dfa::GotoHeader &go_to = pda -> statset[state_no].go_to;
            for (int j = 0; j < go_to.Length(); j++)
            {
                int symbol = go_to[j].Symbol(),
                    act = go_to[j].Action(),
                    i = grammar -> num_rules + indx + symbol;
                if (option -> goto_default || option -> nt_check)
                    rhs_and_baseCheck.array[i] = symbol;
                if (act > 0)
                {
                    lhs_and_baseAction.array[i] = state_index[act] + grammar -> num_rules;
                    parse_table -> goto_count++;
                }
                else
                {
                    lhs_and_baseAction.array[i] = -act;
                    parse_table -> goto_reduce_count++;
                }
            }
        }
    }

    if (option -> backtrack)
    {
        for (int i = 0; i < pda -> conflicts.Length(); i++)
        {
            int act = pda -> conflicts[i];
            lhs_and_baseAction.array[accept_act + i] =
              (act > grammar -> num_rules // shift
                   ? state_index[act - grammar -> num_rules] + grammar -> num_rules
                   : act < 0 // shift-reduce
                         ? -act + error_act
                         : act);
        }
    }

    //
    // Write left hand side symbol of rules followed by ACTION table.
    //
    lhs_and_baseAction.array[0] = symbol_map[grammar -> start_image] - grammar -> num_terminals;
    {
        for (int i = 1; i <= grammar -> num_rules; i++)
            lhs_and_baseAction.array[i] = symbol_map[grammar -> rules[i].lhs] - grammar -> num_terminals;
    }
    lhs_and_baseAction.type_id = parse_table -> Type(0, high_index);

    //
    // If we need to store the original state numbers, we store them as 
    // negative values. Thus, in such a case, we need to adjust the low_index.
    // Next, we write the size of the right-hand side of each rule in the top
    // portion of the CHECK table.
    //
    int low_index = 0;
    high_index = (option -> goto_default || option -> nt_check ? grammar -> num_symbols : 0);
    if (option -> error_maps || option -> debug)
    {
        low_index = -(pda -> num_states);
        for (int state_no = 1; state_no <= pda -> num_states; state_no++)
            rhs_and_baseCheck.array[grammar -> num_rules + state_index[state_no]] = -state_no;
    }

    {
        for (int i = 1; i <= grammar -> num_rules; i++)
        {
            rhs_and_baseCheck.array[i] = grammar -> RhsSize(i);
            high_index = Util::Max(high_index, grammar -> RhsSize(i));
        }
    }
    rhs_and_baseCheck.type_id = parse_table -> Type(low_index, high_index);

    //
    // Initialize the terminal tables,and update with terminal actions.
    //
    Table::IntArrayInfo &termCheck = parse_table -> data.Next();
    termCheck.name_id = Table::TERM_CHECK;
    termCheck.type_id = parse_table -> Type(0, grammar -> num_terminals);
    termCheck.array.Resize(term_check_size + 1);
    termCheck.array.MemReset();
    Table::IntArrayInfo &termAction = parse_table -> data.Next();
    termAction.name_id = Table::TERM_ACTION;
    termAction.array.Resize(term_action_size + 1);
    termAction.array.Initialize(error_act);
    termAction.array[0] = 0;
    high_index = Util::Max(grammar -> num_rules, num_terminal_states);
    for (int state_no = 1; state_no <= num_terminal_states; state_no++)
    {
        int indx = term_state_index[state_no];
        Dfa::ShiftHeader &sh = pda -> shift[new_state_element[state_no].shift_number];
        for (int j = 0; j < sh.Length(); j++)
        {
            int symbol = sh[j].Symbol(),
                act = sh[j].Action();
            if ((! option -> shift_default) || (act != default_shift[symbol]))
            {
                int i = indx + symbol,
                    result_act;
                termCheck.array[i] = symbol;

                if (act > pda -> max_la_state)
                {
                    result_act = accept_act + (act - pda -> max_la_state);
                    parse_table -> conflict_count++;
                }
                else if (act > pda -> num_states)
                {
                    result_act = state_index[act];
                    parse_table -> la_shift_count++;
                }
                else if (act > 0)
                {
                    result_act = state_index[act] + grammar -> num_rules;
                    parse_table -> shift_count++;
                }
                else
                {
                    result_act = -act + error_act;
                    parse_table -> shift_reduce_count++;
                }

                if (result_act > (MAX_TABLE_SIZE + 1))
                {
                    Tuple<const char *> msg;
                    IntToString size(MAX_TABLE_SIZE + 1);
                    msg.Next() = "Table contains look-ahead shift entry that is > ";
                    msg.Next() = size.String();
                    msg.Next() = "; Processing stopped.";

                    option -> EmitError(0, msg);
                    control -> Exit(12);
                }

                termAction.array[i] = result_act;
                high_index = Util::Max(high_index, result_act);
            }
        }

        Dfa::ReduceHeader &red = new_state_element[state_no].reduce;
        {
            for (int j = 1; j < red.Length(); j++)
            {
                int symbol = red[j].Symbol(),
                    rule_no = red[j].RuleNumber(),
                    i = indx + symbol;
                termCheck.array[i] = symbol;
                termAction.array[i] = rule_no;
                parse_table -> reduce_count++;
            }
        }

        int rule_no = red[0].RuleNumber();
        if (rule_no != error_act)
            parse_table -> default_count++;
        termCheck.array[indx] = Grammar::DEFAULT_SYMBOL;
        if (option -> shift_default)
             termAction.array[indx] = state_no;
        else termAction.array[indx] = rule_no;
    }
    termAction.type_id = parse_table -> Type(0, high_index);

    //
    //
    //
    if (option -> goto_default)
    {
        Table::IntArrayInfo &defaultGoto = parse_table -> data.Next();
        defaultGoto.name_id = Table::DEFAULT_GOTO;
        defaultGoto.type_id = parse_table -> Type(0, error_act);
        defaultGoto.array.Resize(grammar -> num_nonterminals + 1);
        defaultGoto.array[0] = 0;
        for (int i = 1, symbol = grammar -> FirstNonTerminal(); symbol <= grammar -> LastNonTerminal(); i++, symbol++)
        {
           int act = default_goto[symbol];
           if (act < 0)
                defaultGoto.array[i] = -act;
           else if (act == 0)
                defaultGoto.array[i] = error_act;
           else defaultGoto.array[i] = state_index[act] + grammar -> num_rules;
        }
    }

    //
    //
    //
    if (option -> shift_default)
    {
        //
        //
        //
        Table::IntArrayInfo &defaultReduce = parse_table -> data.Next();
        defaultReduce.name_id = Table::DEFAULT_REDUCE;
        defaultReduce.type_id = parse_table -> Type(0, error_act);
        defaultReduce.array.Resize(num_terminal_states + 1);
        defaultReduce.array[0] = 0;
        {
            for (int i = 1; i < defaultReduce.array.Size(); i++)
            {
                Dfa::ReduceHeader &red = new_state_element[i].reduce;
                defaultReduce.array[i] = red[0].RuleNumber();
            }
        }

        //
        //
        //
        Table::IntArrayInfo &shiftState = parse_table -> data.Next();
        shiftState.name_id = Table::SHIFT_STATE;
        shiftState.type_id = parse_table -> Type(0, shift_check_size);
        shiftState.array.Resize(num_terminal_states + 1);
        shiftState.array[0] = 0;
        {
            for (int i = 1; i < shiftState.array.Size(); i++)
                shiftState.array[i] = shift_check_index[shift_image[i]];
        }

        //
        //
        //
        Table::IntArrayInfo &shiftCheck = parse_table -> data.Next();
        shiftCheck.name_id = Table::SHIFT_CHECK;
        shiftCheck.type_id = parse_table -> Type(0, grammar -> num_terminals); //unsigned
        shiftCheck.array.Resize(shift_check_size + 1);
        shiftCheck.array.Initialize(Grammar::DEFAULT_SYMBOL);
        {
            for (int i = 1; i <= shift_domain_count; i++)
            {
                int indx = shift_check_index[i];
                Dfa::ShiftHeader &sh = pda -> shift[real_shift_number[i]];
                for (int j = 0; j < sh.Length(); j++)
                {
                    int symbol = sh[j].Symbol();
                    shiftCheck.array[indx + symbol] = symbol;
                }
            }
        }

        //
        //
        //
        Table::IntArrayInfo &defaultShift = parse_table -> data.Next();
        defaultShift.name_id = Table::DEFAULT_SHIFT;
        defaultShift.array.Resize(grammar -> num_terminals + 1);
        defaultShift.array[0] = 0;
        high_index = 0;
        for (int symbol = grammar -> FirstTerminal(); symbol <= grammar -> LastTerminal(); symbol++)
        {
            int act = default_shift[symbol];
            if (act < 0)
                 defaultShift.array[symbol] = -act + error_act;
            else if (act == 0)
                 defaultShift.array[symbol] = error_act;
            else if (act > pda -> max_la_state)
                 defaultShift.array[symbol] = accept_act + (act - pda -> max_la_state);
            else if (act > pda -> num_states)
                 defaultShift.array[symbol] = state_index[act];
            else defaultShift.array[symbol] = state_index[act] + grammar -> num_rules;

            high_index = Util::Max(high_index, defaultShift.array[symbol]);

            if (defaultShift.array[symbol] > (MAX_TABLE_SIZE + 1))
            {
                Tuple<const char *> msg;
                IntToString size(MAX_TABLE_SIZE + 1);
                msg.Next() = "Table contains look-ahead shift entry that is > ";
                msg.Next() = size.String();
                msg.Next() = "; Processing stopped.";

                option -> EmitError(0, msg);
                control -> Exit(12);
            }
        }

        defaultShift.type_id = parse_table -> Type(0, high_index);
    }

    //
    //
    //
    if (option -> error_maps || option -> states)
    {
        int max_indx = accept_act - grammar -> num_rules - 1;
        Array<int> temp(max_indx + 1, Util::OMEGA);
        for (int state_no = 1; state_no <= pda -> num_states; state_no++)
            temp[state_index[state_no]] = state_no;

        int j = pda -> num_states + 1;
        for (int i = max_indx; i >= 1; i--)
        {
            int state_no = temp[i];
            if (state_no != Util::OMEGA)
            {
                j--;
                ordered_state[j] = i + grammar -> num_rules;
                state_list[j] = state_no;
            }
        }

        //
        // If needed, Initialize the arrays original_state
        // and sorted_state.
        //
        if (option -> states)
        {
            //
            //
            //
            parse_table -> original_state.Resize(pda -> num_states + 1); // print only the regular (not lookahead) states
            {
                for (int i = 0; i < parse_table -> original_state.Size(); i++)
                    parse_table -> original_state[i] = state_list[i];
            }
    
            //
            //
            //
            parse_table -> sorted_state.Resize(pda -> num_states + 1);  // print only the regular (not lookahead) states
            {
                for (int i = 0; i < parse_table -> sorted_state.Size(); i++)
                    parse_table -> sorted_state[i] = ordered_state[i];
            }
        }
    }

    //
    //
    //
    if (option -> error_maps)
    {
        //
        // We now construct a bit map for the set of terminal symbols that
        // may appear in each state. Then, we invoke PARTSET to apply the
        // Partition Heuristic and print it.
        //
        Array<int> as_size(pda -> num_states + 1);
        for (int state_no = 1; state_no <= pda -> num_states; state_no++)
        {
            Dfa::ShiftHeader &sh = pda -> Shift(state_no);
            as_size[state_no] = sh.Length();
            {
                for (int i = 0; i < sh.Length(); i++)
                    action_symbols[state_no].AddElement(sh[i].Symbol());
            }

            Dfa::ReduceHeader &red = pda -> statset[state_no].reduce;
            as_size[state_no] += (red.Length() - 1);
            {
                for (int i = 1; i < red.Length(); i++)
                    action_symbols[state_no].AddElement(red[i].Symbol());
            }
        }

        Array<int> state_start(pda -> num_states + 2),
                   state_stack(pda -> num_states + 1);

        Partition<BitSet>::partset(action_symbols,
                                   pda -> num_states,
                                   as_size,
                                   state_list,
                                   state_start,
                                   state_stack,
                                   grammar -> num_terminals);

        //
        // Compute and write out the base of the ACTION_SYMBOLS map.
        //
        Table::IntArrayInfo &action_symbols_base = parse_table -> data.Next();
        action_symbols_base.name_id = Table::ACTION_SYMBOLS_BASE;
        action_symbols_base.type_id = parse_table -> Type(0, state_start[pda -> num_states + 1]); // see action_symbols_range
        action_symbols_base.array.Resize(pda -> num_states + 1);
        action_symbols_base.array[0] = 0;

        //
        // Since the original state number associated with a
        // state k is stored in the baseCheck table (actually, its negation is stored there).
        // The mapping of action_symbols_base is based on the original state number directly.
        // The mapping of action_symbols_base is based on the original state number directly.
        // In the case, the elements in the range of action_symbols can be accessed directly.
        //
        {
            for (int i = 1; i < action_symbols_base.array.Size(); i++)
                action_symbols_base.array[state_list[i]] = Util::Abs(state_start[state_list[i]]);
        }

        //
        // Compute and write out the range of the ACTION_SYMBOLS map.
        //
        Table::IntArrayInfo &action_symbols_range = parse_table -> data.Next();
        action_symbols_range.name_id = Table::ACTION_SYMBOLS_RANGE;
        action_symbols_range.type_id = parse_table -> Type(0, grammar -> num_terminals);
        action_symbols_range.array.Resize(state_start[pda -> num_states + 1]);
        action_symbols_range.array[0] = 0;
        compute_action_symbols_range(state_start, state_stack,
                                     state_list, action_symbols_range.array);

        //
        // We now repeat the same process for the domain of the GOTO table.
        //
        {
            for (int state_no = 1; state_no <= pda -> num_states; state_no++)
            {
                as_size[state_no] = pda -> gd_index[state_no + 1] - pda -> gd_index[state_no];

                for (int i = pda -> gd_index[state_no]; i <= pda -> gd_index[state_no + 1] - 1; i++)
                     naction_symbols[state_no].AddElement(pda -> gd_range[i]);
            }
        }

        Partition<BitSetWithOffset>::partset(naction_symbols,
                                             pda -> num_states,
                                             as_size,
                                             state_list,
                                             state_start,
                                             state_stack,
                                             grammar -> num_nonterminals);

        {
            for (int i = 0; i < pda -> gd_range.Length(); i++) // Remap non-terminals
                pda -> gd_range[i] = symbol_map[pda -> gd_range[i]] - grammar -> num_terminals;
        }

        //
        // Compute and write out the base of the NACTION_SYMBOLS map.
        //
        Table::IntArrayInfo &naction_symbols_base = parse_table -> data.Next();
        naction_symbols_base.name_id = Table::NACTION_SYMBOLS_BASE;
        naction_symbols_base.type_id = parse_table -> Type(0, state_start[pda -> num_states + 1]); // see naction_symbols_range
        naction_symbols_base.array.Resize(pda -> num_states + 1);
        naction_symbols_base.array[0] = 0;

        //
        //
        //
        {
            for (int i = 1; i < action_symbols_base.array.Size(); i++)
                naction_symbols_base.array[state_list[i]] = Util::Abs(state_start[state_list[i]]);
        }

        //
        // Compute and write out the range of the NACTION_SYMBOLS map.
        //
        Table::IntArrayInfo &naction_symbols_range = parse_table -> data.Next();
        naction_symbols_range.name_id = Table::NACTION_SYMBOLS_RANGE;
        naction_symbols_range.type_id = parse_table -> Type(0, grammar -> num_symbols);
        naction_symbols_range.array.Resize(state_start[pda -> num_states + 1]);
        naction_symbols_range.array[0] = 0;
        compute_naction_symbols_range(state_start, state_stack,
                                      state_list, naction_symbols_range.array);

        //
        // We write the name_index of each terminal symbol by remapping
        // the NAME_INDEX values based on the new symbol numbers.
        //
        Table::IntArrayInfo &terminalIndex = parse_table -> data.Next();
        terminalIndex.name_id = Table::TERMINAL_INDEX;
        terminalIndex.type_id = parse_table -> Type(0, grammar -> num_names);
        terminalIndex.array.Resize(grammar -> num_terminals + 1);
        terminalIndex.array[0] = 0;
        for (int symbol = grammar -> FirstTerminal(); symbol <= grammar -> LastTerminal(); symbol++)
            terminalIndex.array[symbol_map[symbol]] = grammar -> symbol_index[symbol].external_name_index;

        //
        // We write the name_index of each non_terminal symbol. The array
        // TEMP is used to remap the NAME_INDEX values based on the new
        // symbol numbers. Next, TEMP is copied to its final destination
        // with its lower bound index adjusted to 1...
        //
        BoundedArray<int> temp_index(grammar -> FirstNonTerminal(), grammar -> LastNonTerminal());
        {
            for (int symbol = grammar -> FirstNonTerminal(); symbol <= grammar -> LastNonTerminal(); symbol++)
                temp_index[symbol_map[symbol]] = grammar -> symbol_index[symbol].external_name_index;
        }
        Table::IntArrayInfo &nonterminalIndex = parse_table -> data.Next();
        nonterminalIndex.name_id = Table::NONTERMINAL_INDEX;
        nonterminalIndex.type_id = parse_table -> Type(0, grammar -> num_names);
        nonterminalIndex.array.Resize(temp_index.Size() + 1);
        nonterminalIndex.array[0] = 0;
        {
            for (int i = grammar -> FirstNonTerminal(); i <= grammar -> LastNonTerminal(); i++)
                nonterminalIndex.array[i - grammar -> num_terminals] = temp_index[i];
        }

        //
        //  Update the scope maps.
        //
        if (pda -> scope_prefix.Size() > 0)
        {
            Table::IntArrayInfo &scope_prefix = parse_table -> data.Next();
            scope_prefix.name_id = Table::SCOPE_PREFIX;
            scope_prefix.type_id = parse_table -> Type(0, pda -> scope_right_side.Size());
            scope_prefix.array.Resize(pda -> scope_prefix.Size());

            Table::IntArrayInfo &scope_suffix = parse_table -> data.Next();
            scope_suffix.name_id = Table::SCOPE_SUFFIX;
            scope_suffix.type_id = parse_table -> Type(0, pda -> scope_right_side.Size());
            scope_suffix.array.Resize(pda -> scope_suffix.Size());

            Table::IntArrayInfo &scope_lhs_symbol = parse_table -> data.Next();
            scope_lhs_symbol.name_id = Table::SCOPE_LHS_SYMBOL;
            scope_lhs_symbol.type_id = parse_table -> Type(0, grammar -> num_symbols);
            scope_lhs_symbol.array.Resize(pda -> scope_lhs_symbol.Size());

            Table::IntArrayInfo &scope_look_ahead = parse_table -> data.Next();
            scope_look_ahead.name_id = Table::SCOPE_LOOK_AHEAD;
            scope_look_ahead.type_id = parse_table -> Type(0, grammar -> num_terminals);
            scope_look_ahead.array.Resize(pda -> scope_look_ahead.Size());

            Table::IntArrayInfo &scope_state_set = parse_table -> data.Next();
            scope_state_set.name_id = Table::SCOPE_STATE_SET;
            scope_state_set.type_id = parse_table -> Type(0, pda -> scope_state.Size());
            scope_state_set.array.Resize(pda -> scope_state_set.Size());

            for (int j = 0; j < pda -> scope_look_ahead.Size(); j++)
            {
                scope_prefix.array[j]     = pda -> scope_prefix[j];
                scope_suffix.array[j]     = pda -> scope_suffix[j];
                scope_lhs_symbol.array[j] = symbol_map[pda -> scope_lhs_symbol[j]]
                                             - grammar -> num_terminals;
                scope_look_ahead.array[j] = symbol_map[pda -> scope_look_ahead[j]];
                scope_state_set.array[j]  = pda -> scope_state_set[j];
            }

            //
            // Mark all elements of prefix strings.
            //
            Table::IntArrayInfo &scope_right_side = parse_table -> data.Next();
            scope_right_side.name_id = Table::SCOPE_RIGHT_SIDE;
            scope_right_side.type_id = parse_table -> Type(0, grammar -> num_symbols);
            scope_right_side.array.Resize(pda -> scope_right_side.Size());
            for (int i = 0; i < scope_right_side.array.Size(); i++)
            {
                scope_right_side.array[i] = (pda -> scope_right_side[i] == 0
                                                  ? 0
                                                  : symbol_map[pda -> scope_right_side[i]]);
            }

            //
            // Map each symbol in a scope suffix directly into its name index.
            // Recall that the scope suffix is used only in reporting errors
            // and not in diagnosing them. As those indexes may be shared, we
            // need to make sure that each suffix element is mapped only once.
            //
            // TODO: REMOVE THIS CODE. IT TURNS OUT THAT WE NEED THE SYMBOL
            // (NOT FOR DIAGNOSIS BUT) FOR RECOVERY!!!
            //
            // Array<bool> index_seen(scope_right_side.array.Size(), false);
            // for (int i = 0; i < scope_suffix.array.Size(); i++)
            // {
            //     int root = scope_suffix.array[i];
            //     if (! index_seen[root])
            //     {
            //         index_seen[root] = true;
            //
            //         for (int j = root; scope_right_side.array[j] != 0; j++)
            //         {
            //             int symbol = scope_right_side.array[j];
            //             scope_right_side.array[j] =
            //                  (grammar -> IsTerminal(symbol)
            //                       ? terminalIndex.array[symbol]
            //                       : nonterminalIndex.array[symbol - grammar -> num_terminals]);
            //         }
            //     }
            // }

            Table::IntArrayInfo &scope_state = parse_table -> data.Next();
            scope_state.name_id = Table::SCOPE_STATE;
            scope_state.type_id = parse_table -> Type(0, accept_act); // 0..max_state_index
            scope_state.array.Resize(pda -> scope_state.Size());
            for (int l = 0; l < pda -> scope_state.Size(); l++)
            {
                int state_no = pda -> scope_state[l];
                scope_state.array[l] = (state_no == 0
                                                  ? 0
                                                  : state_index[state_no] + grammar -> num_rules);
            }

            //
            // Transition symbol
            //
            Table::IntArrayInfo &inSymb = parse_table -> data.Next();
            inSymb.name_id = Table::IN_SYMB;
            inSymb.type_id = parse_table -> Type(0, grammar -> num_symbols);
            inSymb.array.Resize(pda -> num_states + 1);
            inSymb.array[0] = 0;
            inSymb.array[1] = 0;
            for (int state_no = 2; state_no < inSymb.array.Size(); state_no++)
            {
                Node *q = pda -> statset[state_no].kernel_items;
                int symbol = 0;
                if (q != NULL)
                {
                    int item_no = q -> value - 1;
                    symbol = base -> item_table[item_no].symbol;
                }

                inSymb.array[state_no] = symbol_map[symbol];
            }
        }

        //
        //
        //
        Array<const char *> name(grammar -> num_names + 1);
        for (int k = 0; k < name.Size(); k++)
            name[k] = grammar -> RetrieveName(k);
        parse_table -> initialize(name, Table::NAME_START, parse_table -> name_start, parse_table -> name_info, parse_table -> max_name_length);
    }

    //
    // Calculate number of bytes required for each integer table.
    // In addition, the number of bytes required for the name table
    // is (name_start[name_start.Size() - 1] - 1);
    //
    Array<int> type_size(parse_table -> num_type_ids);
    type_size[Table::B] = type_size[Table::I8] = type_size[Table::U8] = 1;
    type_size[Table::I16] = type_size[Table::U16] = 2;
    type_size[Table::I32] = 4;
    {
        for (int i = 0; i < parse_table -> data.Length(); i++)
        {
            parse_table -> data[i].num_bytes = type_size[parse_table -> data[i].type_id] * parse_table -> data[i].array.Size();
            // assert(parse_table -> data[i].type_id >= parse_table -> Type(parse_table -> data[i].array)); 
        }
    }
    if (parse_table -> name_start.array.Size() > 0)
        parse_table -> name_start.num_bytes = type_size[parse_table -> name_start.type_id] * parse_table -> name_start.array.Size();

    //
    // Copy all basic information that need to be carried over
    // to the table object.
    //
    parse_table -> symbol_map.Resize(this -> symbol_map.Size());
    {
        for (int i = 0; i < this -> symbol_map.Size(); i++)
            parse_table -> symbol_map[i] = this -> symbol_map[i];
    }
    parse_table -> last_symbol = this -> last_symbol;
    parse_table -> start_state = this -> state_index[1] + grammar -> num_rules;
    parse_table -> accept_act = this -> accept_act;
    parse_table -> error_act = this -> error_act;

    return;
}
