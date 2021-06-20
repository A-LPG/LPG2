#include "resolve.h"

#include <iostream>
using namespace std;

//
//
//
Resolve::StackElement *Resolve::allocate_stack_element(void)
{
    return &(stack_element_pool.Next());
}


//
// This function takes as argument two pointers to sorted lists of stacks.
// It merges the lists in the proper order and returns the resulting list.
//
Resolve::StackElement *Resolve::union_config_sets(Resolve::StackElement *root1, Resolve::StackElement *root2)
{
    Tuple<Resolve::StackElement *> state_list;

    while (root1 != NULL && root2 != NULL)
    {
        if (root1 -> state_number == root2 -> state_number)
        {
            state_list.Next() = root1;
            root1 = root1 -> next;
            root2 = root2 -> next;
        }
        else if (root1 -> state_number < root2 -> state_number)
        {
            state_list.Next() = root1;
            root1 = root1 -> next;
        }
        else
        {
            state_list.Next() = root2;
            root2 = root2 -> next;
        }
    }

    for (; root1 != NULL; root1 = root1 -> next)
        state_list.Next() = root1;

    for (; root2 != NULL; root2 = root2 -> next)
        state_list.Next() = root2;

    Resolve::StackElement *root = NULL;
    if (state_list.Length() > 0)
    {
        Resolve::StackElement *tail = state_list[0];
        for (int i = 1; i < state_list.Length(); i++)
        {
            tail -> next = state_list[i];
            tail = state_list[i];
        }
        tail -> next = NULL;
        root = state_list[0];
    }

    return root;
}


//
// This function takes as argument a SOURCES_ELEMENT map, an ACTION and a
// set (sorted list) of configurations. It adds the set of configurations
// to the previous set of configurations associated with the ACTION in the
// SOURCES_ELEMENT map.
//
void Resolve::add_configs(Resolve::SourcesElement &sources,
                          int action,
                          Resolve::StackElement *config_root)
{
    if (config_root == NULL) // The new set is empty? Do nothing
        return;

    if (sources.configs[action] == NULL) // The previous was empty?
    {
        sources.list[action] = sources.root;
        sources.root = action;
    }

    sources.configs[action] = union_config_sets(sources.configs[action], config_root);

    return;
}



//
// This function clears out all external space used by the VISITED set and
// resets VISITED to the empty set.
//
void Resolve::clear_visited(void)
{
    for (int state_no = visited.root; state_no != Util::NIL; state_no = visited.list[state_no])
        visited.map[state_no].Reset();
    visited.root = Util::NIL;

    return;
}

//
// This boolean function checks whether or not a given pair [state, symbol]
// was already inserted in the VISITED set.
//
bool Resolve::was_visited(int state_no, int symbol)
{
    for (int i = 0; i < visited.map[state_no].Length(); i++)
    {
        if (visited.map[state_no][i] == symbol)
            return true;
    }

    return false;
}


//
// This function inserts a given pair [state, symbol] into the VISITED set.
//
void Resolve::mark_visited(int state_no, int symbol)
{
    if (visited.map[state_no].Length() == 0) // 1st time we see state_no?
    {
        visited.list[state_no] = visited.root;
        visited.root = state_no;
    }

    visited.map[state_no].Next() = symbol;

    return;
}


//
// This procedure is a modified instantiation of the digraph algorithm
// to compute the CYCLIC set of states.
//
void Resolve::compute_cyclic(int state_no)
{
    stack.Push(state_no);
    int indx = stack.Length();
    cyclic[state_no] = false;
    index_of[state_no] = indx;

    Dfa::GotoHeader &go_to = pda -> statset[state_no].go_to;
    for (int i = 0; i < go_to.Length(); i++)
    {
        int symbol = go_to[i].Symbol(),
            act = go_to[i].Action();
        if (act > 0 && base -> IsNullable(symbol)) // We have a transition on a nullable nonterminal?
        {
            if (index_of[act] == Util::OMEGA)
                compute_cyclic(act);
            else if (index_of[act] != Util::INFINITY_)
                cyclic[state_no] = true;
            cyclic[state_no] = cyclic[state_no] || cyclic[act];

            index_of[state_no] = Util::Min(index_of[state_no], index_of[act]);
        }
    }

    if (index_of[state_no] == indx)
    {
        int act;
        do
        {
            stack.Pop(act);
            index_of[act] = Util::INFINITY_;
        } while(act != state_no);
    }

    return;
}


//
//    In tracing an error, we will be moving backward in the state
// automaton looking for items with the conflict symbol as look-ahead.
// In the case of SLR, we may have to analoguously look at an
// arbitrary set of items involved.  In moving around these graphs, it
// is possible to encounter a cycle, in which case, we simply want to
// back out of the cycle and try another path. We therefore need to
// keep track of which nodes have already been visited.  For LALR
// conflicts, we use the LA_PTR field of the GOTO_ELEMENTs as an index
// to a BOOLEAN array LALR_VISITED.  For SLR conflicts, a boolean
// array, SLR_VISITED, indexable by non-terminals, is used.  For
// trace-backs to the root item, the boolean array SYMBOL_SEEN, also
// also indexable by non-terminals, is used.
//
bool Resolve::trace_root(int lhs_symbol)
{
    if (lhs_symbol == grammar -> accept_image)
        return(true);

    if (symbol_seen[lhs_symbol])
        return(false);

    symbol_seen[lhs_symbol] = true;
    for (int item = nt_items[lhs_symbol]; item != Util::NIL; item = item_list[item])
    {
        if (trace_root(grammar -> rules[base -> item_table[item].rule_number].lhs))
        {
            pda -> PrintItem(item);
            return(true);
        }
    }

    return(false);
}


//
// The procedure below is invoked to retrace a path from the initial
// item to a given item (ITEM_NO) passed to it as argument.
//
void Resolve::print_root_path(int item_no)
{
    symbol_seen.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
    symbol_seen.MemReset();

    if (trace_root(grammar -> rules[base -> item_table[item_no].rule_number].lhs))
        fprintf(option -> syslis, "\n"); // Leave one blank line after root trace.

    return;
}


//
// This procedure takes as argument, a state number, STATE_NO, an
// index into the goto map of state_no, GOTO_INDX, which identifies a
// starting point for a search for the CONFLICT_SYMBOL. It attempts to
// find a path in the automaton (from the starting point) that leads
// to a state where the conflict symbol can be read. If a path is
// found, all items along the path are printed and SUCCESS is returned.
// Otherwise, FAILURE is returned.
//
bool Resolve::lalr_path_retraced(int state_no,
                                 int goto_indx,
                                 int conflict_symbol)
{
    Dfa::GotoHeader &go_to = pda -> statset[state_no].go_to;
    lalr_visited[go_to[goto_indx].Laptr()] = true;
    Tuple<int> state_list;
    int state = go_to[goto_indx].Action();
    for (Node *p = (state > 0 ? pda -> statset[state].kernel_items
                              : base -> adequate_item[-state]);
         p != NULL;
         p = p -> next)
    {
        int item = p -> value - 1;
        if (base -> First(base -> item_table[item].suffix_index)[conflict_symbol]) // Conflict_symbol can be read in state?
        {
            if (option -> trace == Option::FULL)
                print_root_path(item);
            pda -> PrintItem(item);
            return true;
        }
        else if (base -> First(base -> item_table[item].suffix_index)[grammar -> empty])
        {
            int symbol = grammar -> rules[base -> item_table[item].rule_number].lhs;
            pda -> Access(state_list, state_no, item);
            for (int k = 0; k < state_list.Length(); k++)
            {
                int access_state = state_list[k];
                Dfa::GotoHeader &go_to = pda -> statset[access_state].go_to;
                int i = go_to.Index(symbol);
                if (! lalr_visited[go_to[i].Laptr()])
                {
                    if (lalr_path_retraced(access_state, i, conflict_symbol))
                    {
                        pda -> PrintItem(item);
                        return true;
                    }
                }
            }
        }
    }

    return false;
}


//
// In this procedure, we attempt to retrace an LALR conflict path
// (there may be more than one) of CONFLICT_SYMBOL in the state
// automaton that led to ITEM_NO in state STATE_NO.
//
void Resolve::print_relevant_lalr_items(int state_no, int item_no, int conflict_symbol)
{
    int lhs_symbol = grammar -> rules[base -> item_table[item_no].rule_number].lhs;
    if (lhs_symbol == grammar -> accept_image)
        return;

    lalr_visited.Resize(pda -> la_top + 1);
    lalr_visited.MemReset();

    Tuple<int>state_list;
    pda -> Access(state_list, state_no, item_no);
    for (int k = 0; k < state_list.Length(); k++)
    {
        int access_state = state_list[k],
            i = pda -> statset[access_state].go_to.Index(lhs_symbol);
        if (lalr_path_retraced(access_state, i, conflict_symbol))
            break;
    }

    return;
}


//
// The procedure below is invoked to retrace a path that may have
// introduced the CONFLICT_SYMBOL in the FOLLOW set of the nonterminal
// that produces ITEM_NO.  Note that such a path must exist.
//
bool Resolve::slr_trace(int lhs_symbol, int conflict_symbol)
{
    if (slr_visited[lhs_symbol])
        return(false);
    slr_visited[lhs_symbol] = true;
    for (int item = nt_items[lhs_symbol]; item != Util::NIL; item = item_list[item])
    {
        if (base -> First(base -> item_table[item].suffix_index)[conflict_symbol])
        {
            if (option -> trace == Option::FULL)
                print_root_path(item);
             pda -> PrintItem(item);
             return(true);
        }
        if (base -> First(base -> item_table[item].suffix_index)[grammar -> empty])
        {
            if (slr_trace(grammar -> rules[base -> item_table[item].rule_number].lhs,
                          conflict_symbol))
            pda -> PrintItem(item);
            return(true);
        }
    }

    return(false);
}


//
// This procedure is invoked to print an SLR path of items that leads
// to the conflict symbol.
//
void Resolve::print_relevant_slr_items(int item_no, int conflict_symbol)
{
    slr_visited.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
    slr_visited.MemReset();
    if (slr_trace(grammar -> rules[base -> item_table[item_no].rule_number].lhs,
                   conflict_symbol))
    {}

    return;
}


//
// TODO: TAKE A HARD LOOK AT THIS!!!
//
void Resolve::PriorityRuleTrace(int state_no, int goto_indx, int conflict_symbol)
{
    Dfa::GotoHeader &go_to = pda -> statset[state_no].go_to;
    int &priority_rank = lalr_priority[go_to[goto_indx].Laptr()];
    Tuple<int> state_list;
    int state = go_to[goto_indx].Action();
    for (Node *p = (state > 0 ? pda -> statset[state].kernel_items
                              : base -> adequate_item[-state]);
         p != NULL;
         p = p -> next)
    {
        int item = p -> value - 1,
            rule_no = base -> item_table[item].rule_number,
            lhs_symbol = grammar -> rules[rule_no].lhs;
        priority_rank = (grammar -> rules[rule_no].IsPriorityProduction() && base -> rank[rule_no] < priority_rank
                                  ? base -> rank[rule_no]
                                  : priority_rank);
        if (lhs_symbol != grammar -> accept_image)
        {
            pda -> Access(state_list, state_no, item);
            for (int k = 0; k < state_list.Length(); k++)
            {
                int access_state = state_list[k];
                Dfa::GotoHeader &go_to = pda -> statset[access_state].go_to;
                int i = go_to.Index(lhs_symbol);

                if (pda -> la_index[go_to[i].Laptr()] == Util::OMEGA)
                    pda -> LaTraverse(access_state, i);

                if (pda -> la_set[go_to[i].Laptr()][conflict_symbol])
                {
                    if (lalr_priority[go_to[i].Laptr()] == Util::OMEGA)
                    {
                        lalr_priority[go_to[i].Laptr()] = priority_rank;
                        PriorityRuleTrace(access_state, i, conflict_symbol);
                    }
                    priority_rank = Util::Min(priority_rank, lalr_priority[go_to[i].Laptr()]);
                }
            }
        }
    }

    return;
}


//
// TODO: TAKE A VERY, VERY HARD LOOK AT THIS!!!
//
int Resolve::ComputeReducePriority(int state_no, int item_no, int conflict_symbol)
{
    int base_rule = base -> item_table[item_no].rule_number,
        priority_rank = base -> rank[base_rule],
        lhs_symbol = grammar -> rules[base_rule].lhs;
    if (lhs_symbol != grammar -> accept_image)
    {
        Tuple<int>state_list;
        lalr_priority.Resize(pda -> la_top + 1);
        lalr_priority.Initialize(Util::OMEGA);

        pda -> Access(state_list, state_no, item_no);
        for (int k = 0; k < state_list.Length(); k++)
        {
            int access_state = state_list[k];
            Dfa::GotoHeader &go_to = pda -> statset[access_state].go_to;
            int i = go_to.Index(lhs_symbol);

            if (pda -> la_index[go_to[i].Laptr()] == Util::OMEGA)
                pda -> LaTraverse(access_state, i);

            if (pda -> la_set[go_to[i].Laptr()][conflict_symbol])
            {
                if (lalr_priority[go_to[i].Laptr()] == Util::OMEGA)
                {
                    lalr_priority[go_to[i].Laptr()] = base -> rank[base_rule];
                    PriorityRuleTrace(access_state, i, conflict_symbol);
                }
                priority_rank = Util::Min(priority_rank, lalr_priority[go_to[i].Laptr()]);
            }
        }
    }

    return priority_rank;
}


//
// TODO: TAKE A VERY HARD LOOK AT THIS!!!
//
int Resolve::ComputeShiftPriority(int state_no, int action, int conflict_symbol)
{
    //
    // Process priority for shift action only if the precedence option is in effect. Otherwise, give 
    // shift actions the highest priority.
    //
    if (! option -> precedence)
        return 0;

    Tuple<int>state_list;
    lalr_priority.Resize(pda -> la_top + 1);
    lalr_priority.Initialize(Util::OMEGA);

    int priority_rank = grammar -> num_rules;
    for (Node *p = (action > 0 ? pda -> statset[action].kernel_items
                               : base -> adequate_item[-action]);
         p != NULL;
         p = p -> next)
    {
        int item_no = p -> value - 1,
            rule_no = base -> item_table[item_no].rule_number;
        priority_rank = Util::Min(priority_rank, base -> rank[rule_no]);
        int lhs_symbol = grammar -> rules[rule_no].lhs;
        if (lhs_symbol != grammar -> accept_image)
        {
            pda -> Access(state_list, state_no, item_no);
            for (int k = 0; k < state_list.Length(); k++)
            {
                int access_state = state_list[k];
                Dfa::GotoHeader &go_to = pda -> statset[access_state].go_to;
                int  i = go_to.Index(lhs_symbol);

                if (pda -> la_index[go_to[i].Laptr()] == Util::OMEGA)
                    pda -> LaTraverse(access_state, i);

                if (pda -> la_set[go_to[i].Laptr()][conflict_symbol])
                {
                    if (lalr_priority[go_to[i].Laptr()] == Util::OMEGA)
                    {
                        lalr_priority[go_to[i].Laptr()] = base -> rank[rule_no];
                        PriorityRuleTrace(access_state, i, conflict_symbol);
                    }
                    priority_rank = Util::Min(priority_rank, lalr_priority[go_to[i].Laptr()]);
                }
            }
        }
    }

    return priority_rank;
}


//
// This routine is invoked when a grammar contains conflicts, and the
// first conflict is detected.
//
void Resolve::ConflictsInitialization(void)
{
    //
    // NT_ITEMS and ITEM_LIST are used in reporting SLR conflicts, and
    // in recreating paths from the Start item. See the routines
    // PRINT_RELEVANT_SLR_ITEMS and PRINT_ROOT_PATH.
    //
    nt_items.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
    nt_items.Initialize(Util::NIL);
    item_list.Resize(grammar -> num_items + 1);

    int k = (Control::PRINT_LINE_SIZE - 11) / 2 - 1;
    char msg_line[Control::PRINT_LINE_SIZE];
    control -> PrintHeading();
    FillIn(msg_line, k, '-');
    fprintf(option -> syslis, "\n%s CONFLICTS %s\n", msg_line, msg_line);

    //
    //   SLR conflicts may be caused by a symbol in the FOLLOW set of a
    // left hand side, which is not actually in the LALR look-ahead set in
    // that context.  Therefore, there may not exist a path in the state
    // automaton from the state where the conflict was detected to another
    // state where it was introduced.  In such a case, we try to retrace a
    // path that contributed the conflict-symbol to the FOLLOW set via a
    // sequence of productions.
    //
    // To assist in this task, we build below a map from each non-terminal
    // A to the set of items of which A is the dot SYMBOL. I.e., all items
    // of the form [x .A y] where x and y are arbitrary strings, and A is
    // a non-terminal. This map is also used in retracing a path from the
    // Start item to any other item.
    //
    for (int i = 1; i <= grammar -> num_items; i++)
    {
        if (grammar -> IsNonTerminal(base -> item_table[i].symbol))
        {
            item_list[i] = nt_items[base -> item_table[i].symbol];
            nt_items[base -> item_table[i].symbol] =  i;
        }
    }

    return;
}


//
//   If conflicts are detected, tehy are placed in two lists headed by
// SR_CONFLICT_ROOT and RR_CONFLICT_ROOT.  We scan these lists, and
// report the conflicts.
//
void Resolve::process_conflicts(int state_no)
{
    if (nt_items.Size() == 0)
        ConflictsInitialization();

    //
    // Flush whatever output is still left in the buffer
    //
    option -> FlushReport();

    //
    // Print a header in the output console and print the state
    // information in the listing file.
    //
    option -> report.Put("\n*** In state ");
    option -> report.Put(state_no);
    option -> report.Put(":\n\n");
    option -> report.Flush(stdout);

    pda -> PrintShiftState(state_no); // Print state containing conflicts
    fprintf(option -> syslis, "\n\n"); // Leave some space after printing state

    //
    // Process shift-reduce conflicts.
    //
    for (int i = 0; i < sr_conflicts.Length(); i++)
    {
        SrConflictElement *p = &sr_conflicts[i];
        int symbol = p -> symbol,
            rule_no = base -> item_table[p -> item].rule_number;
        char temp[Control::SYMBOL_SIZE + 1];
        grammar -> RestoreSymbol(temp, grammar -> RetrieveString(symbol));

//      option -> report.Put("*** Shift/reduce conflict on \"");
//      option -> report.Put(temp);
//      option -> report.Put("\" with rule ");
//      option -> report.Put(rule_no);
//      option -> report.PutChar('\n');

	// RMF 4/25/2008 - Print location information for the rule involved
        Grammar::RuleElement thisRule = grammar -> rules[rule_no];
        int ruleTokenStart = thisRule.first_token_index;
        int ruleTokenEnd   = thisRule.last_token_index;
	LexStream *ls = grammar -> GetLexStream();
	Token *startToken = ls -> GetTokenReference(ruleTokenStart);
        Token *endToken   = ls -> GetTokenReference(ruleTokenEnd);

	Tuple<const char *> msg;
	IntToString state_no_str(state_no);
	IntToString rule_no_str(rule_no);

	msg.Next() = "Shift/reduce conflict in state ";
	msg.Next() = state_no_str.String();
	msg.Next() = " on \"";
	msg.Next() = temp;
	msg.Next() = "\" with rule ";
	msg.Next() = rule_no_str.String();

	option -> EmitWarning(startToken, endToken, msg);

        option -> FlushReport();

        if (option -> trace != Option::NONE)
        {
            if (! option -> slr)
                 print_relevant_lalr_items(state_no, p -> item, symbol);
            else print_relevant_slr_items(p -> item, symbol);
            pda -> PrintItem(p -> item);
            fprintf(option -> syslis, "\n\n"); // Leave some space after printing items
        }
    }

    //
    // Process reduce-reduce conflicts.
    //
    for (int j = 0; j < rr_conflicts.Length(); j++)
    {
        RrConflictElement *p = &rr_conflicts[j];

        int symbol = p -> symbol,
            n = base -> item_table[p -> item1].rule_number,
            rule_no = base -> item_table[p -> item2].rule_number;
        char temp[Control::SYMBOL_SIZE + 1];
        grammar -> RestoreSymbol(temp, grammar -> RetrieveString(symbol));

//      option -> report.Put("*** Reduce/reduce conflict on \"");
//      option -> report.Put(temp);
//      option -> report.Put("\" between rule ");
//      option -> report.Put(n);
//      option -> report.Put(" and ");
//      option -> report.Put(rule_no);
//      option -> report.PutChar('\n');

	// RMF 4/25/2008 - Print location information for the rules involved
	Grammar::RuleElement thisRule  = grammar -> rules[n];
        Grammar::RuleElement otherRule = grammar -> rules[rule_no];
        int ruleTokenStart  = thisRule.first_token_index;
        int ruleTokenEnd    = thisRule.last_token_index;
        int otherTokenStart = otherRule.first_token_index;
        // int otherTokenEnd   = otherRule.last_token_index;
	LexStream *ls = grammar -> GetLexStream();
	Token *startToken      = ls -> GetTokenReference(ruleTokenStart);
        Token *endToken        = ls -> GetTokenReference(ruleTokenEnd);
        Token *otherStartToken = ls -> GetTokenReference(otherTokenStart);
        // Token *otherEndToken   = ls -> GetTokenReference(otherTokenEnd);

	Tuple<const char *> msg;
	IntToString state_no_str(state_no);
	IntToString n_str(n);
	IntToString rule_no_str(rule_no);
	IntToString other_start_line_str(otherStartToken -> Line());
	IntToString other_start_col_str(otherStartToken -> Column());

	msg.Next() = "Reduce/reduce conflict in state ";
	msg.Next() = state_no_str.String();
	msg.Next() = " on \"";
	msg.Next() = temp;
	msg.Next() = "\" between rule ";
	msg.Next() = n_str.String();
	msg.Next() = " and rule ";
	msg.Next() = rule_no_str.String();
	msg.Next() = " starting at line ";
	msg.Next() = other_start_line_str.String();
	msg.Next() = ", column ";
	msg.Next() = other_start_col_str.String();

	option -> EmitWarning(startToken, endToken, msg);

        option -> FlushReport();

        if (option -> trace != Option::NONE)
        {
            if (! option -> slr)
                 print_relevant_lalr_items(state_no, p -> item1, symbol);
            else print_relevant_slr_items(p -> item1, symbol);
            pda -> PrintItem(p -> item1);
            char msg_line[Control::PRINT_LINE_SIZE];
            FillIn(msg_line, Control::PRINT_LINE_SIZE - 3, '-');
            fprintf(option -> syslis, "\n%s", msg_line);
            if (! option -> slr)
                 print_relevant_lalr_items(state_no, p -> item2, symbol);
            else print_relevant_slr_items(p -> item2, symbol);
            pda -> PrintItem(p -> item2);
            fprintf(option -> syslis, "\n\n"); // Leave some space after printing items
        }
    }

    //
    // Process Keyword/Identifier conflicts.
    //
    if (option -> verbose)
    {
        {
            for (int i = 0; i < soft_ss_conflicts.Length(); i++)
            {
                SsConflictElement *p = &soft_ss_conflicts[i];
                int symbol = p -> symbol;
                char temp[Control::SYMBOL_SIZE + 1];
                grammar -> RestoreSymbol(temp, grammar -> RetrieveString(symbol));

                option -> report.Put("*** Keyword/Identifier Shift conflict on \"");
                option -> report.Put(temp);
                option -> report.Put("\"\n");
            }
        }

        {
            for (int i = 0; i < soft_sr_conflicts.Length(); i++)
            {
                SrConflictElement *p = &soft_sr_conflicts[i];
                int symbol = p -> symbol,
                    rule_no = base -> item_table[p -> item].rule_number;
                char temp[Control::SYMBOL_SIZE + 1];
                grammar -> RestoreSymbol(temp, grammar -> RetrieveString(symbol));

                option -> report.Put("*** Keyword/Identifier Shift/reduce conflict on \"");
                option -> report.Put(temp);
                option -> report.Put("\" with rule ");
                option -> report.Put(rule_no);
                option -> report.PutChar('\n');
            }
        }

        for (int i = 0; i < soft_rr_conflicts.Length(); i++)
        {
            RrConflictElement *p = &soft_rr_conflicts[i];
            int symbol = p -> symbol,
                n = base -> item_table[p -> item1].rule_number,
                rule_no = base -> item_table[p -> item2].rule_number;
            char temp[Control::SYMBOL_SIZE + 1];
            grammar -> RestoreSymbol(temp, grammar -> RetrieveString(symbol));

            option -> report.Put("*** Keyword/Identifier Reduce/reduce conflict on \"");
            option -> report.Put(temp);
            option -> report.Put("\" between rule ");
            option -> report.Put(n);
            option -> report.Put(" and ");
            option -> report.Put(rule_no);
            option -> report.PutChar('\n');
        }
    }
    else
    {
        if (soft_ss_conflicts.Length() > 0)
        {
            option -> report.Put("*** ");
            option -> report.Put(soft_ss_conflicts.Length());
            option -> report.Put(" Keyword/Identifier Shift/shift conflicts detected\n");
        }

        if (soft_sr_conflicts.Length() > 0)
        {
            option -> report.Put("*** ");
            option -> report.Put(soft_sr_conflicts.Length());
            option -> report.Put(" Keyword/Identifier Shift/reduce conflicts detected\n");
        }

        if (soft_rr_conflicts.Length() > 0)
        {
            option -> report.Put("*** ");
            option -> report.Put(soft_rr_conflicts.Length());
            option -> report.Put(" Keyword/Identifier Reduce/reduce conflicts detected\n");
        }
    }

    option -> FlushReport();

    return;
}


//
// This function takes as argument a configuration STACK, a SYMBOL on
// which a transition can be made in the configuration and a terminal
// lookahead symbol, LA_SYMBOL. It executes the transition on SYMBOL
// and simulates all paths taken in the automaton after that transition
// until new state(s) are reached where a transition is possible on
// the lookahead symbol. It then returns the new set of configurations
// found on which a transition on LA_SYMBOL is possible.
//
Resolve::StackElement *Resolve::follow_sources(Resolve::StackElement *stack,
                                               int symbol, int la_symbol)
{
    Resolve::StackElement *configs = NULL; // Initialize the output set of configurations

    //
    // If the starting configuration consists of a single state and
    // the initial [state, symbol] pair has already been visited,
    // return the null set. Otherwise, mark the pair visited and ...
    //
    int state_no = stack -> state_number;
    if (stack -> size == 1)
    {
        if (was_visited(state_no, symbol) ||
            (state_no == 1 && symbol == grammar -> accept_image))
            return configs;

        mark_visited(state_no, symbol);
    }

    //
    // Find the transition defined on the symbol...
    // If the SYMBOL is a nonterminal and we can determine that the
    // lookahead symbol (LA_SYMBOL) cannot possibly follow the
    // nonterminal in question in this context, we simply abandon the
    // search and return the NULL set.
    //
    int act;
    if (grammar -> IsTerminal(symbol))
        act = pda -> Shift(state_no).Action(symbol);
    else
    {
        Dfa::GotoHeader &go_to = pda -> statset[state_no].go_to;
        int i = go_to.Index(symbol);
        if (pda -> la_index[go_to[i].Laptr()] == Util::OMEGA)
            pda -> LaTraverse(state_no, i);

        if (! pda -> la_set[go_to[i].Laptr()][la_symbol])
            return configs;

        act = go_to[i].Action();
    }

    //
    // If the ACTion on the symbol is a shift or a goto, ...
    //
    if (act > 0)
    {
        //
        // We check to see if the new state contains an action on the
        // lookahead symbol. If that's the case then we create a new
        // configuration by appending ACT to the starting configuration
        // and add this newly formed configuration to the set(list) of
        // configurations...
        //
        Dfa::ShiftHeader &sh = pda -> Shift(act);
        for (int i = 0; i < sh.Length(); i++)
        {
            if (sh[i].Symbol() == la_symbol)
            {
                Resolve::StackElement *q = allocate_stack_element();
                q -> state_number = act;
                q -> size = stack -> size + 1;
                q -> previous = stack;
                q -> next = NULL;

                configs = q;

                break;
            }
        }

        //
        // If the new state cannot get into a cycle of null
        // transitions, we check to see if it contains any transition
        // on a nullable nonterminal. For each such transition, we
        // append the new state to the stack and recursively invoke
        // FOLLOW_SOURCES to check if a transition on LA_SYMBOL cannot
        // follow such a null transition.
        //
        if (! cyclic[act])
        {
            Dfa::GotoHeader &go_to = pda -> statset[act].go_to;
            for (int i = 0; i < go_to.Length(); i++)
            {
                symbol = go_to[i].Symbol();
                if (base -> IsNullable(symbol))
                {
                    Resolve::StackElement *new_configs,
                                          *q = allocate_stack_element();
                    q -> state_number = act;
                    q -> size = stack -> size + 1;
                    q -> previous = stack;
                    q -> next = NULL;

                    new_configs = follow_sources(q, symbol, la_symbol);
                    if (new_configs != NULL)
                        configs = union_config_sets(configs, new_configs);
                }
            }
        }
    }

    //
    // We now iterate over the kernel set of items associated with the
    // ACTion defined on SYMBOL...
    //
    Tuple<int>state_list;
    for (Node *item_ptr = (act > 0 ? pda -> statset[act].kernel_items
                                   : base -> adequate_item[-act]);
         item_ptr != NULL;
         item_ptr = item_ptr -> next)
    {
        int item_no = item_ptr -> value;

        //
        // For each item that is a final item whose left-hand side
        // is neither the starting symbol nor a symbol that can
        // right-most produce itself...
        //
        if (base -> item_table[item_no].symbol == grammar -> empty)
        {
            int rule_no = base -> item_table[item_no].rule_number,
                lhs_symbol = grammar -> rules[rule_no].lhs;
            if (lhs_symbol != grammar -> accept_image && (! base -> rmpself[lhs_symbol]))
            {
                //
                // If the length of the prefix of the item preceeding
                // the dot is shorter that the length of the stack, we
                // retrace the item's path within the stack and
                // invoke FOLLOW_SOURCES with the prefix of the stack
                // where the item was introduced through closure, the
                // left-hand side of the item and the lookahead symbol.
                //
                if (base -> item_table[item_no].dot < stack -> size)
                {
                    Resolve::StackElement *q = stack;
                    for (int i = 1; i < base -> item_table[item_no].dot; i++)
                        q = q -> previous;
                    q = follow_sources(q, lhs_symbol, la_symbol);
                    configs = union_config_sets(configs, q);
                }
                else
                {
                    //
                    // Compute the item in the root state of the stack,
                    // and find the root state...
                    //
                    item_no -= stack -> size;
                    Resolve::StackElement *q;
                    for (q = stack; q -> size != 1; q = q -> previous)
                        ;

                    //
                    // We are now back in the main automaton, find all
                    // sources where the item was introduced through
                    // closure start a new configuration and invoke
                    // FOLLOW_SOURCES with the appropriate arguments to
                    // calculate the set of configurations associated
                    // with these sources.
                    //
                    pda -> Access(state_list, q -> state_number, item_no);
                    for (int k = 0; k < state_list.Length(); k++)
                    {
                        int access_state = state_list[k];
                        Resolve::StackElement *new_configs,
                                              *q = allocate_stack_element();
                        q -> state_number = access_state;
                        q -> size = 1;
                        q -> previous = NULL;
                        q -> next = NULL;

                        new_configs = follow_sources(q, lhs_symbol, la_symbol);
                        if (new_configs != NULL)
                            configs = union_config_sets(configs, new_configs);
                    }
                }
            }
        }
    }

    return configs;
}


//
// This function has a similar structure as FOLLOW_SOURCES.  But,
// instead of computing configurations that can be reached, it
// computes lookahead symbols that can be reached.  It takes as
// argument a configuration STACK, a SYMBOL on which a transition can
// be made in the configuration and a set variable, LOOK_AHEAD, where
// the result is to be stored.  When NEXT_LA is invoked from the
// outside, LOOK_AHEAD is assumed to be initialized to the empty set.
// NEXT_LA first executes the transition on SYMBOL and thereafter, all
// terminal symbols that can be read are added to LOOKAHEAD.
//
void Resolve::next_la(Resolve::StackElement *stack, int symbol, BitSet &look_ahead)
{
    //
    // The only symbol that can follow the end-of-file symbol is the
    // end-of-file symbol.
    //
    if (symbol == grammar -> eof_image)
    {
        look_ahead.AddElement(grammar -> eof_image);
        return;
    }

    //
    // act is the transition defined on the symbol...
    //
    int state_no = stack -> state_number,
        act = (grammar -> IsTerminal(symbol)? pda -> Shift(state_no).Action(symbol)
                                            : pda -> statset[state_no].go_to.Action(symbol));
    assert(act != Util::OMEGA);

    //
    // If the ACTion on the symbol is a shift or a goto, then all
    // terminal symbols that can be read in ACT are added to
    // LOOK_AHEAD.
    //
    if (act > 0)
        look_ahead += pda -> read_set[act];

    //
    // We now iterate over the kernel set of items associated with the
    // ACTion defined on SYMBOL...
    // Recall that the READ_SET of ACT is but the union of the FIRST
    // map defined on the suffixes of the items in the kernel of ACT.
    //
    Tuple<int>state_list;
    for (Node * item_ptr = (act > 0 ? pda -> statset[act].kernel_items
                                    : base -> adequate_item[-act]);
         item_ptr != NULL;
         item_ptr = item_ptr -> next)
    {
        int item_no = item_ptr -> value;

        //
        // For each item that is a final item whose left-hand side
        // is neither the starting symbol nor a symbol that can
        // right-most produce itself...
        //
        if (base -> First(base -> item_table[item_no - 1].suffix_index)[grammar -> empty])
        {
            int rule_no = base -> item_table[item_no].rule_number,
                lhs_symbol = grammar -> rules[rule_no].lhs;
            if (lhs_symbol != grammar -> accept_image && (! base -> rmpself[lhs_symbol]))
            {
                //
                // If the length of the prefix of the item preceeding
                // the dot is shorter that the length of the stack, we
                // retrace the item's path within the stack and
                // invoke NEXT_LA with the prefix of the stack
                // where the item was introduced through closure, the
                // left-hand side of the item and LOOK_AHEAD.
                //
                Resolve::StackElement *q = stack;
                if (base -> item_table[item_no].dot < stack -> size)
                {
                    for (int i = 1; i < base -> item_table[item_no].dot; i++)
                        q = q -> previous;
                    next_la(q, lhs_symbol, look_ahead);
                }
                else
                {
                    //
                    // Compute the item in the root state of the stack,
                    // and find the root state...
                    //
                    item_no -= stack -> size;
                    while(q -> size != 1)
                        q = q -> previous;

                    //
                    // We are now back in the main automaton, find all
                    // sources where the item was introduced through
                    // closure and add all terminal symbols in the
                    // follow set of the left-hand side symbol in each
                    // source to LOOK_AHEAD.
                    //
                    pda -> Access(state_list, q -> state_number, item_no);
                    for (int k = 0; k < state_list.Length(); k++)
                    {
                        int access_state = state_list[k];

                        //
                        // If look-ahead after left hand side is not
                        // yet computed,call LA_TRAVERSE to compute it.
                        //
                        Dfa::GotoHeader &go_to = pda -> statset[access_state].go_to;
                        int i = go_to.Index(lhs_symbol);
                        if (pda -> la_index[go_to[i].Laptr()] == Util::OMEGA)
                            pda -> LaTraverse(access_state, i);
                        look_ahead += pda -> la_set[go_to[i].Laptr()];
                    }
                }
            }
        }
    }

    return;
}


//
// This function takes as argument an array, STACK_SEEN, with
// STATE_TABLE_SIZE elements (indexable in the range
// 0..STATE_TABLE_SIZE-1) which is the base of a hash table and a
// STACK. It searches the hash table to see if it already contained
// the stack in question. If yes, it returns true. Otherwise, it
// inserts the stack into the table and returns false.
//
bool Resolve::stack_was_seen(Array<StackElement *> &stack_seen, Resolve::StackElement *stack)
{
    unsigned hash_address = stack -> size; // Initialize hash address
    {
        for (Resolve::StackElement *p = stack; p != NULL; p = p -> previous)
            hash_address += p -> state_number;
        hash_address %= SourcesElement::STATE_TABLE_SIZE;
    }

    for (Resolve::StackElement *p = stack_seen[hash_address]; p != NULL; p = p -> link)
    {
        if (stack -> size == p -> size)
        {
            Resolve::StackElement *q,
                                  *r;
            for (q = stack, r = p;
                 q != NULL;
                 q = q -> previous, r = r -> previous)
            {
                if (q -> state_number != r -> state_number)
                    break;
            }
            if (q == NULL)
                return true;
        }
    }

    stack -> link = stack_seen[hash_address];
    stack_seen[hash_address] = stack;

    return false;
}


//
// STATE_TO_RESOLVE_CONFLICTS is a function that attempts to resolve
// conflicts by doing more look-ahead.  If the conflict resolution
// is successful, then a new state is created and returned; otherwise,
// the NULL pointer is returned.
//
int Resolve::state_to_resolve_conflicts(Resolve::SourcesElement &sources, int la_symbol, int level)
{
    //
    // Initialize new lookahead state. Initialize counters. Check and
    // adjust HIGHEST_LEVEL reached so far, if necessary.
    //
    int state = Util::OMEGA,
        num_shift_actions = 0,
        num_reduce_actions = 0;

    if (level > pda -> highest_level)
        pda -> highest_level = level;

    //
    // One of the parameters received is a SOURCES map whose domain is
    // a set of actions and each of these actions is mapped into a set
    // of configurations that can be reached after that action is
    // executed (in the state where the conflicts were detected).
    // In this loop, we compute an ACTION map which maps each each
    // terminal symbol into 0 or more actions in the domain of SOURCES.
    //
    // NOTE in a sources map, if a configuration is associated with
    // more than one action then the grammar is not LALR(k) for any k.
    // We check for that condition below. However, this check is there
    // for purely cosmetic reason. It is not necessary for the
    // algorithm to work correctly and its removal will speed up this
    // loop somewhat (for conflict-less input).
    // The first loop below initializes the hash table used for
    // lookups ...
    //
    sources.stack_seen.MemReset();
    Array<int> rule_count(grammar -> num_rules + 1);
    Array<Node *> action(grammar -> num_terminals + 1, NULL);
    Tuple<int> symbol_list;
    BitSet look_ahead(grammar -> num_terminals + 1);
    int act;
    for (act = sources.root; act != Util::NIL; act = sources.list[act])
    {
        //
        // For each action we iterate over its associated set of
        // configurations and invoke NEXT_LA to compute the lookahead
        // set for that configuration. These lookahead sets are in
        // turn unioned together to form a lookahead set for the
        // action in question.
        //
        look_ahead.SetEmpty();
        Resolve::StackElement *stack;
        for (stack = sources.configs[act];
             stack != NULL; stack = stack -> next)
        {
            if (stack_was_seen(sources.stack_seen, stack))
            {// This is the superfluous code mentioned above!
                pda -> highest_level = Util::INFINITY_;
                break;
            }

            next_la(stack, la_symbol, look_ahead);
        }
        if (stack != NULL) // if we exited the above loop prematurely
            break;
        look_ahead.RemoveElement(grammar -> empty);      // EMPTY never in LA set

        //
        // For each lookahead symbol computed for this action, add an
        // action to the ACTION map and keep track of the symbols on
        // which any action is defined.
        // If new conflicts are detected and we are already at the
        // lookahead level requested, we terminate the computation...
        //
        int count = 0,
            symbol;
        for (symbol = grammar -> FirstTerminal(); symbol <= grammar -> LastTerminal(); symbol++)
        {
            if (look_ahead[symbol])
            {
                count++;

                if (action[symbol] == NULL)
                     symbol_list.Next() = symbol;
                else if (level == option -> lalr_level)
                     break;

                Node *p = node_pool -> AllocateNode();
                p -> value = act;
                p -> next = action[symbol];
                action[symbol] = p;
            }
        }
        if (symbol <= grammar -> LastTerminal()) // if we exited the above loop prematurely
            break;

        //
        // If the action in question is a reduction then we keep track
        // of how many times it was used.
        //
        if (act >= 0 && act <= grammar -> num_rules)
            rule_count[act] = count;
    }

    //
    //
    //
    if (act != Util::NIL) // if we exited the above loop prematurely
    {
        clear_action(action, symbol_list);
        return Util::OMEGA;
    }

    //
    // We now iterate over the symbols on which actions are defined.
    // If we detect conflicts on any symbol, we compute new sources
    // and try to recover by computing more lookahead. Otherwise, we
    // update the counts and create two lists: a list of symbols on
    // which shift actions are defined and a list of symbols on which
    // reduce actions are defined.
    //
    Resolve::SourcesElement new_sources(grammar -> num_rules, pda -> num_states);
    Tuple<int> shift_list,
               reduce_list;
    int j;
    for (j = 0; j < symbol_list.Length(); j++)
    {
        int symbol = symbol_list[j];

        //
        // We have four cases to consider:
        //    1. There are conflicts on SYMBOL
        //    2. The action on SYMBOL is a shift-reduce
        //    3. The action on SYMBOL is a shift
        //    4. The action on SYMBOL is a reduce
        //
        if (action[symbol] -> next != NULL)
        {
            new_sources.Clear();
            Node *tail = NULL;
            for (Node *p = action[symbol]; p != NULL; tail = p, p = p -> next)
            {
                int act = p -> value;
                if (act >= 0 && act <= grammar -> num_rules)
                    rule_count[act]--;

                clear_visited();


                for (Resolve::StackElement *stack = sources.configs[act];
                     stack != NULL;
                     stack = stack -> next)
                {
                    Resolve::StackElement *new_configs;

                    new_configs = follow_sources(stack, la_symbol, symbol);
                    add_configs(new_sources, act, new_configs);
                }
            }
            node_pool -> FreeNodes(action[symbol], tail);
            action[symbol] = NULL;

            int la_state = state_to_resolve_conflicts(new_sources, symbol, level + 1);

            if (la_state == Util::OMEGA)
                break;

            Node *q = node_pool -> AllocateNode();
            q -> value = la_state;
            q -> next = NULL;
            action[symbol] = q;

            num_shift_actions++;
            shift_list.Next() = symbol;
        }
        else if (action[symbol] -> value < 0)
        {
            num_shift_actions++;
            shift_list.Next() = symbol;
        }
        else if (action[symbol] -> value > grammar -> num_rules)
        {
            num_shift_actions++;
            (action[symbol] -> value) -= grammar -> num_rules;
            shift_list.Next() = symbol;
        }
        else
        {
            num_reduce_actions++;
            reduce_list.Next() = symbol;
        }
    }

    //
    //
    //
    if (j < symbol_list.Length()) // if we exited the loop above prematurely
    {
        clear_action(action, symbol_list);
        return Util::OMEGA;
    }

    //
    // We now iterate over the reduce actions in the domain of sources
    // and compute a default reduce action.
    //
    int default_rule = Util::OMEGA,
        count = 0;
    for (act = sources.root; act != Util::NIL; act = sources.list[act])
    {
        if (act >= 0 && act <= grammar -> num_rules)
        {
            if (rule_count[act] > count)
            {
                count = rule_count[act];
                default_rule = act;
            }
        }
    }

    //
    // Could not resolve this conflict - a state with no shift and no reduce actions is encountered
    //
    if (num_shift_actions == 0 && default_rule == Util::OMEGA) {
        clear_action(action, symbol_list);
        return Util::OMEGA;
    }

    //
    // By now, we are ready to create a new look-ahead state. The
    // actions for the state are in the ACTION vector, and the
    // constants: NUM_SHIFT_ACTIONS and NUM_REDUCE_ACTIONS indicate
    // the number of shift and reduce actions in the ACTION vector.
    // Note that the IN_STATE field of each look-ahead state created
    // is initially set to the number associated with that state. If
    // all the conflicts detected in the state, S, that requested the
    // creation of a look-ahead state are resolved, then this field
    // is updated with S.
    // Otherwise, this field indicates that this look-ahead state is
    // dangling - no other state point to it.
    //
    pda -> max_la_state++;
    assert(pda -> max_la_state == pda -> statset.Length());

    state = pda -> statset.NextIndex();

    pda -> statset[state].kernel_items = NULL;
    pda -> statset[state].complete_items = NULL;
    pda -> statset[state].single_production_items = NULL;
    pda -> statset[state].transition_symbol = la_symbol;

    //
    // If there are any shift-actions in this state, we create a shift
    // map for them if one does not yet exist, otherwise, we reuse the
    // old existing one.
    //
    if (num_shift_actions > 0)
    {
        Array<int> shift_action(grammar -> num_terminals + 1, Util::OMEGA);

        //
        // In this loop, we compute the hash address as the number of
        // shift actions, plus the sum of all the symbols on which a
        // shift action is defined.  As a side effect, we also take
        // care of some other issues. Shift actions which were encoded
        // to distinguish them from reduces action are decoded.
        // The counters for shift and shift-reduce actions are updated.
        // For all Shift actions to look-ahead states, the IN_STATE
        // field of these look-ahead target states are updated.
        //
        for (int i = 0; i < shift_list.Length(); i++)
        {
            int symbol = shift_list[i];

            if (action[symbol] -> value < 0)
                pda -> num_shift_reduces++;
            else if (action[symbol] -> value <= pda -> num_states)
                pda -> num_shifts++;
            else // lookahead-shift
                pda -> AddPredecessor(action[symbol] -> value, state);

            shift_action[symbol] = action[symbol] -> value;
        }

        pda -> statset[state].shift_number = pda -> UpdateShiftMaps(shift_list, shift_action);
    }
    else
    {
        pda -> statset[state].shift_number = 0;
    }

    //
    // Construct Reduce map.
    //
    {
        assert(reduce_list.Length() == num_reduce_actions);

        pda -> num_reductions += num_reduce_actions;

        Dfa::ReduceHeader &red = pda -> statset[state].reduce;
        assert(red.Length() == 0);
        red.NextIndex(); // allocate zeroth element.
        red[0].SetSymbol(Grammar::DEFAULT_SYMBOL);
        red[0].SetRuleNumber(default_rule);
        for (int i = 0; i < reduce_list.Length(); i++)
        {
            int symbol = reduce_list[i],
                index = red.NextIndex();
            red[index].SetSymbol(symbol);
assert(grammar -> IsTerminal(symbol));
            red[index].SetRuleNumber(action[symbol] -> value);
        }
    }

    clear_action(action, symbol_list);

    return state;
}


//
// Release all space allocated to process this lookahead state and
// return.
//
void Resolve::clear_action(Array<Node *> &action, Tuple<int> &symbol_list)
{
    for (int k = 0; k < symbol_list.Length(); k++)
    {
        int symbol = symbol_list[k];
        Node *tail = action[symbol];
        if (tail != NULL)
        {
            for (Node *p = tail -> next; p != NULL; tail = p, p = p -> next)
                 ;
            node_pool -> FreeNodes(action[symbol], tail);
        }
    }

    return;
}


//
// If conflicts were detected and LALR(k) processing was requested,
// where k > 1, then we attempt to resolve the conflicts by computing
// more lookaheads.  Shift-Reduce conflicts are processed first,
// followed by Reduce-Reduce conflicts.
//
void Resolve::ResolveConflicts(int state_no, Array< Tuple<int> > &action, Array<int> &symbol_list, int symbol_root)
{
    sr_conflicts.Reset();
    Dfa::ShiftHeader &sh = pda -> Shift(state_no);
    for (int i = 0; i < sh.Length(); i++)
    {
        int symbol = sh[i].Symbol(),
            act = sh[i].Action();
        ResolveShiftReduceConflicts(state_no, symbol, act, action);
    }

    rr_conflicts.Reset();
    for (int symbol = symbol_root; symbol != Util::NIL; symbol = symbol_list[symbol])
        ResolveReduceReduceConflicts(state_no, symbol, action);

    //
    // If any unresolved conflicts were detected, process them.
    //
    if (sr_conflicts.Length() > 0 || rr_conflicts.Length() > 0)
        process_conflicts(state_no);

    stack_element_pool.Reset();

    return;
}


//
// When a grammar contains soft keywords, then it is processed as
// LALR(1). Hence, conflicts are not resolved. In this function,
// we check to see if the state in question (state_no) contains
// actions on IDENTIFIER. If so, we add all such actions to each
// keyword in the grammar in that state. Finally, all conflicts:
// hard and soft, are reported.
//
void Resolve::ResolveConflictsAndAddSoftActions(int state_no,
                                                Array< Tuple<int> > &action,
                                                Array<int> &symbol_list,
                                                int symbol_root,
                                                Array<int> &rule_count)
{
    //
    // Note that a shift action to a state "S" is encoded with the
    // value (S+NUM_RULES) to help distinguish it from reduce actions.
    // Reduce actions lie in the range [0..NUM_RULES].   Shift-reduce
    // actions lie in the range [-NUM_RULES..-1].
    //
    soft_sr_conflicts.Reset();
    soft_rr_conflicts.Reset();
    soft_ss_conflicts.Reset();
    sr_conflicts.Reset();
    rr_conflicts.Reset();

    //
    //
    //
    Array<int> shift_action(grammar -> num_terminals + 1, Util::OMEGA);
    Tuple<int> shifted_keywords;

    Dfa::ShiftHeader &sh = pda -> Shift(state_no);
    for (int i = 0; i < sh.Length(); i++)
    {
        int symbol = sh[i].Symbol();
        shift_action[symbol] = sh[i].Action();

        if (grammar -> IsKeyword(symbol))
            shifted_keywords.Next() = symbol;
        else if (symbol != grammar -> identifier_image)
            (void)  ResolveShiftReduceConflicts(state_no, symbol, shift_action[symbol], action);
    }

    //
    // For a shift action, the identifier_action can be decoded as follows:
    //
    //  . Values in the range -num_rules..-1 represent shift-reduce actions.
    //  . Values in the range 1..num_states represent shift actions.
    //  . Values in the range num_states.. represent conflicts actions.
    //
    // For a reduce action, the identifier_action can be decoded as follows:
    //
    //  . Values in the range 1..num_rules represent reduce actions.
    //  . Values in the range num_rules.. represent conflict actions.
    //
    int identifier_action = ResolveIdentifierConflicts(state_no, shift_action, action);

    Dfa::ConflictHeader &conflict = pda -> statset[state_no].conflict;

    if (shift_action[grammar -> identifier_image] != Util::OMEGA)
    {
        {
            for (int i = 0; i < shifted_keywords.Length(); i++)
            {
                int keyword = shifted_keywords[i];
                ResolveKeywordIdentifierShiftReduceConflicts(state_no, keyword, shift_action[keyword], identifier_action, action);
            }
        }

        //
        // We now process the keywords on which one or more reduce actions
        // is defined but no shift action is defined. For each such keyword,
        // we add a conflicting action that treats the keyword as an identifier.
        //
        for (int i = 0; i < grammar -> keywords.Length(); i++)
        {
            //
            // Note that if there is no conflict involved in adding the "soft" action
            // (i.e., when action[keyword].Length() == 0 && shift_action[keyword] == OMEGA),
            // then we do nothing here. This case is processed later in the function
            // ProcessConflictActions in pda.cpp.
            //
            int keyword = grammar -> keywords[i];
            if (shift_action[keyword] == Util::OMEGA)
            {
                if (action[keyword].Length() == 0)
                {
                    //
                    // In this case, we know that no action was associated
                    // with this keyword in this state. The soft action to
                    // be associated with it is the action defined on Identifier.
                    // This step is taken in the function ProcessConflictActions
                    // in pda.cpp.
                    //
                    pda -> statset[state_no].soft_keywords.Next() = keyword;
                }
                else
                {
                    Array<bool> rule_seen(grammar -> num_rules + 1);
                    rule_seen.MemReset();

                    Tuple<Dfa::ConflictCell> cells;
                    for (int k = 0; k < action[keyword].Length(); k++)
                    {
                        int item_no = action[keyword][k];
                        Dfa::ConflictCell &cell = cells.Next();
                        cell.action = base -> item_table[item_no].rule_number;
                        cell.priority = ComputeReducePriority(state_no, item_no, keyword);
                        rule_seen[cell.action] = true;
                    }

                    //
                    //
                    //
                    if (identifier_action > pda -> num_states)
                    {
                        int offset = grammar -> num_rules; // the highest priority of a real action
                        for (int k = identifier_action - pda -> num_states; pda -> conflicts[k] != 0; k++)
                        {
                            int act = pda -> conflicts[k];
                            if (act < 0 || act > grammar -> num_rules || (! rule_seen[act]))
                            {
                                Dfa::ConflictCell &identifier_cell = cells.Next();
                                identifier_cell.action = act;
                                identifier_cell.priority = offset + k; // keep these actions in the same order
                            }
                        }
                    }
                    else
                    {
                        Dfa::ConflictCell &identifier_cell = cells.Next();
                        identifier_cell.action = identifier_action;
                        identifier_cell.priority = Util::INFINITY_;
                    }

                    Dfa::Conflict &conf = conflict.Next();
                    conf.SetSymbol(keyword);
                    conf.SetConflictIndex(pda -> MapConflict(cells));

                    //
                    // Report any soft conflict detected and clear space.
                    //
                    {
                        for (int k = 0; k < action[keyword].Length(); k++)
                        {
                            int item_no = action[keyword][k];
                            if (option -> conflicts)
                            {
                                int index = soft_sr_conflicts.NextIndex();
                                soft_sr_conflicts[index].state_number = Util::OMEGA;
                                soft_sr_conflicts[index].item = item_no;
                                soft_sr_conflicts[index].symbol = keyword;
                            }
                            pda -> num_soft_shift_reduce_conflicts++;
                        }
                    }
                    action[keyword].Reset();
                }
            }
        }

        //
        //
        //
        for (int symbol = symbol_root; symbol != Util::NIL; symbol = symbol_list[symbol])
        {
            if (shift_action[symbol] == Util::OMEGA)
                ResolveReduceReduceConflicts(state_no, symbol, action);
        }
    }
    else if (identifier_action != Util::OMEGA) // (action[grammar -> identifier_image].Length() > 0)
    {
        Array<bool> keyword_processed(grammar -> num_terminals + 1);
        keyword_processed.MemReset();

        //
        // We now process keywords on which a shift action is defined when
        // state_no contains reduce actions but no shift action on IDENTIFIER.
        //
        for (int i = 0; i < shifted_keywords.Length(); i++)
        {
            int keyword = shifted_keywords[i];
            keyword_processed[keyword] = true;

            Array<bool> rule_seen(grammar -> num_rules + 1);
            rule_seen.MemReset();

            Tuple<Dfa::ConflictCell> cells;
            for (int k = 0; k < action[keyword].Length(); k++)
            {
                int item_no = action[keyword][k];

                Dfa::ConflictCell &cell = cells.Next();
                cell.action = base -> item_table[item_no].rule_number;
                cell.priority = ComputeReducePriority(state_no, item_no, keyword);
                rule_seen[cell.action] = true;
            }

            int act = shift_action[keyword];
            Dfa::ConflictCell &cell = cells.Next();
            cell.action = (act < 0 ? act : act + grammar -> num_rules);
            cell.priority = ComputeShiftPriority(state_no, act, keyword);

            //
            //
            //
            if (identifier_action > grammar -> num_rules)
            {
                int offset = grammar -> num_rules; // the highest priority of a real action
                for (int k = identifier_action - grammar -> num_rules; pda -> conflicts[k] != 0; k++)
                {
                    int act = pda -> conflicts[k];
                    assert(act > 0 && act <= grammar -> num_rules && shift_action[grammar -> identifier_image] == Util::OMEGA);
                    if (! rule_seen[act])
                    {
                        Dfa::ConflictCell &identifier_cell = cells.Next();
                        identifier_cell.action = act;
                        identifier_cell.priority = offset + k; // keep these actions in the same order

                        int item_no = base -> adequate_item[act] -> value;
                        if (option -> conflicts)
                        {
                            int index = soft_sr_conflicts.NextIndex();
                            soft_sr_conflicts[index].state_number = Util::OMEGA;
                            soft_sr_conflicts[index].item = item_no;
                            soft_sr_conflicts[index].symbol = keyword;
                        }
                        pda -> num_soft_shift_reduce_conflicts++;
                    }
                }
            }
            else
            {
                Dfa::ConflictCell &identifier_cell = cells.Next();
                identifier_cell.action = identifier_action;
                identifier_cell.priority = Util::INFINITY_;

                assert(identifier_action > 0 && identifier_action <= grammar -> num_rules && shift_action[grammar -> identifier_image] == Util::OMEGA);
                int item_no = base -> adequate_item[identifier_action] -> value;
                if (option -> conflicts)
                {
                    int index = soft_sr_conflicts.NextIndex();
                    soft_sr_conflicts[index].state_number = Util::OMEGA;
                    soft_sr_conflicts[index].item = item_no;
                    soft_sr_conflicts[index].symbol = keyword;
                }
                pda -> num_soft_shift_reduce_conflicts++;
            }

            Dfa::Conflict &conf = conflict.Next();
            conf.SetSymbol(keyword);
            conf.SetConflictIndex(pda -> MapConflict(cells));

            {
                for (int k = 0; k < action[keyword].Length(); k++)
                {
                    if (option -> conflicts)
                    {
                        int index = sr_conflicts.NextIndex();
                        sr_conflicts[index].state_number = act;
                        sr_conflicts[index].item = action[keyword][k];
                        sr_conflicts[index].symbol = keyword;
                    }

                    pda -> num_shift_reduce_conflicts++;
                }
            }

            //
            // Remove reduce actions defined on symbol so as to give
            // precedence to the shift.
            //
            action[keyword].Reset();
        }

        //
        // Now process all non-keywords and keywords on which reduce
        // actions but no shift-actions are defined.
        //
        int tail_symbol = Util::NIL;
        for (int symbol = symbol_root; symbol != Util::NIL; tail_symbol = symbol, symbol = symbol_list[symbol])
        {
            if (grammar -> IsKeyword(symbol) && shift_action[symbol] == Util::OMEGA)
            {
                keyword_processed[symbol] = true;
                ResolveKeywordIdentifierReduceReduceConflicts(state_no, symbol, identifier_action, action);
            }
            else if (symbol != grammar -> identifier_image)
                ResolveReduceReduceConflicts(state_no, symbol, action);
        }

        //
        // Now process all the remaining keywords that have not yet been
        // processed in this state.
        //
        {
            for (int i = 0; i < grammar -> keywords.Length(); i++)
            {
                int keyword = grammar -> keywords[i];
                if (! keyword_processed[keyword])
                {
                    //
                    //
                    //
                    if (identifier_action > grammar -> num_rules)
                    {
                        int conflict_index = identifier_action - grammar -> num_rules;

                        Dfa::Conflict &conf = conflict.Next();
                        conf.SetSymbol(keyword);
                        conf.SetConflictIndex(conflict_index);
                    }
                    else
                    {
                        action[keyword].Next() = base -> adequate_item[identifier_action] -> value;
                        symbol_list[tail_symbol] = keyword;
                        symbol_list[keyword] = Util::NIL;
                        tail_symbol = keyword;

                        assert(identifier_action > 0 && identifier_action <= grammar -> num_rules);
                        rule_count[identifier_action]++;
                    }
                }
            }
        }
    }
    else // No action is defined on identifier in this state
    {
        //
        // Process all the shifted keywords normally
        //
        for (int i = 0; i < shifted_keywords.Length(); i++)
        {
            int keyword = shifted_keywords[i];
            ResolveShiftReduceConflicts(state_no, keyword, shift_action[keyword], action);
        }

        //
        // Process all remaining reduce actions normally.
        //
        for (int symbol = symbol_root; symbol != Util::NIL; symbol = symbol_list[symbol])
        {
            if (shift_action[symbol] == Util::OMEGA)
                ResolveReduceReduceConflicts(state_no, symbol, action);
        }
    }

    //
    // If any unresolved conflicts were detected, process them.
    //
    if (sr_conflicts.Length() > 0 || rr_conflicts.Length() > 0)
        process_conflicts(state_no);

    stack_element_pool.Reset();

    return;
}


//
//
//
int Resolve::ResolveIdentifierConflicts(int state_no,
                                        Array<int> &shift_action,
                                        Array< Tuple<int> > &action)
{
    Dfa::ConflictHeader &conflict = pda -> statset[state_no].conflict;
    int symbol = grammar -> identifier_image,
        symbol_action = shift_action[symbol];

    //
    // Initially, symbol_action is the value of the shift action defined
    // on identifier;
    //
    if (symbol_action != Util::OMEGA)
    {
        //
        // If unresolved shift-reduce conflicts are detected on symbol,
        // add them to the list of conflicts so they can be reported
        // (if the CONFLICT option is on) and count them.
        //
        if (action[symbol].Length() > 0)
        {
            int act = symbol_action;
            if (option -> backtrack)
            {
                Tuple<Dfa::ConflictCell> cells;
                for (int k = 0; k < action[symbol].Length(); k++)
                {
                    int item_no = action[symbol][k];

                    Dfa::ConflictCell &cell = cells.Next();
                    cell.action = base -> item_table[item_no].rule_number;
                    cell.priority = ComputeReducePriority(state_no, item_no, symbol);
                }

                Dfa::ConflictCell &cell = cells.Next();
                cell.action = (act < 0 ? act : act + grammar -> num_rules);
                cell.priority = ComputeShiftPriority(state_no, act, symbol);

                Dfa::Conflict &conf = conflict.Next();
                conf.SetSymbol(symbol);
                conf.SetConflictIndex(pda -> MapConflict(cells));

                //
                // Identify the action on this symbol as a conflict action.
                // This code is only useful when processing an identifier
                // symbol for a grammar with soft keywords. Note that since
                // in such a case the maximum number of lookahead must be
                // 1, we can simply offset the conflict index by NUM_STATES
                // to distinguish it from other valid terminal actions such
                // a shift, shift-reduce or reduce action.
                //
                symbol_action = conf.ConflictIndex() + pda -> num_states;
            }

            for (int k = 0; k < action[symbol].Length(); k++)
            {
                if (option -> conflicts)
                {
                    int index = sr_conflicts.NextIndex();
                    sr_conflicts[index].state_number = act;
                    sr_conflicts[index].item = action[symbol][k];
                    sr_conflicts[index].symbol = symbol;
                }

                pda -> num_shift_reduce_conflicts++;
            }

            //
            // Remove reduce actions defined on symbol so as to give
            // precedence to the shift.
            //
            action[symbol].Reset();
        }
    }
    else if (action[symbol].Length() > 0) // any Reduce/Reduce conflict
    {
        symbol_action = base -> item_table[action[symbol][0]].rule_number;

        //
        // If unresolved reduce-reduce conflicts are detected on
        // symbol, add them to the list of conflicts so they can be
        // reported (if the CONFLICT option is on) and count them.
        //
        if (action[symbol].Length() > 1) // any Reduce/Reduce conflict
        {
            if (option -> backtrack)
            {
                Tuple<Dfa::ConflictCell> cells;
                for (int k = 0; k < action[symbol].Length(); k++)
                {
                    int item_no = action[symbol][k];

                    Dfa::ConflictCell &cell = cells.Next();
                    cell.action = base -> item_table[item_no].rule_number;
                    cell.priority = ComputeReducePriority(state_no, item_no, symbol);
                }

                Dfa::Conflict &conf = conflict.Next();
                conf.SetSymbol(symbol);
                conf.SetConflictIndex(pda -> MapConflict(cells));

                //
                // Identify the action on this symbol as a conflict action.
                // This code is only useful when processing an identifier
                // symbol for a grammar with soft keywords. Note that since
                // in such a case the maximum number of lookahead must be
                // 1, we can simply offset the conflict index by NUM_STATES
                // to distinguish it from other valid terminal actions such
                // a shift, shift-reduce or reduce action.
                //
                symbol_action = conf.ConflictIndex() + grammar -> num_rules;
            }

            int base_item = action[symbol][0];
            for (int k = 1; k < action[symbol].Length(); k++)
            {
                if (option -> conflicts)
                {
                    int index = rr_conflicts.NextIndex();
                    rr_conflicts[index].symbol = symbol;
                    rr_conflicts[index].item1  = base_item;
                    rr_conflicts[index].item2  = action[symbol][k];
                }

                pda -> num_reduce_reduce_conflicts++;
            }

            //
            // Remove all reduce actions that are defined on symbol
            // except the first one. That rule is the one with the
            // longest right-hand side that was associated with symbol.
            // See code in pda.cpp.
            //
            action[symbol].Reset(1);
        }
    }

    return symbol_action;
}


//
//
//
void Resolve::ResolveShiftReduceConflicts(int state_no,
                                          int symbol,
                                          int symbol_action,
                                          Array< Tuple<int> > &action)
{
    Dfa::ConflictHeader &conflict = pda -> statset[state_no].conflict;
    Tuple<int> state_list;

    if (action[symbol].Length() > 0)
    {
        if (option -> single_productions)
            pda -> AddConflictSymbol(state_no, symbol);

        if (option -> lalr_level > 1)
        {
            sources.Clear();

            Resolve::StackElement *q = allocate_stack_element();
            q -> state_number = state_no;
            q -> size = 1;
            q -> previous = NULL;
            q -> next = NULL;

            //
            // Note that a shift action to a state "S" is encoded with the
            // value (S+NUM_RULES) to help distinguish it from reduce actions.
            // Reduce actions lie in the range [0..NUM_RULES].   Shift-reduce
            // actions lie in the range [-NUM_RULES..-1].
            //
            int act = symbol_action;
            if (act > 0)
                 add_configs(sources, act + grammar -> num_rules, q);
            else add_configs(sources, act, q);

            for (int k = 0; k < action[symbol].Length(); k++)
            {
                Resolve::StackElement *new_configs;
                int item_no = action[symbol][k],
                    act = base -> item_table[item_no].rule_number,
                    lhs_symbol = grammar -> rules[act].lhs;

                clear_visited();
                pda -> Access(state_list, state_no, item_no);
                for (int j = 0; j < state_list.Length(); j++)
                {
                    int access_state = state_list[j];

                    q = allocate_stack_element();
                    q -> state_number = access_state;
                    q -> size = 1;
                    q -> previous = NULL;
                    q -> next = NULL;

                    new_configs = follow_sources(q, lhs_symbol, symbol);
                    if (new_configs != NULL)
                        add_configs(sources, act, new_configs);
                }
            }

            //
            // The function STATE_TO_RESOLVE_CONFLICTS returns a lookahead
            // state that has been constructed to
            // resolve the conflicts in question. If the value returned by
            // that function is OMEGA, then it was not possible to resolve
            // the conflicts.  In any case, STATE_TO_RESOLVE_CONFLICTS
            // frees the space that is used by the action map headed by
            // ACTION_ROOT.
            //
            int la_state = state_to_resolve_conflicts(sources, symbol, 2);
            if (la_state != Util::OMEGA)
            {
                pda -> AddPredecessor(la_state, state_no);
                action[symbol].Reset();
            }
        }

        //
        // If unresolved shift-reduce conflicts are detected on symbol,
        // add them to the list of conflicts so they can be reported
        // (if the CONFLICT option is on) and count them.
        //
        if (action[symbol].Length() > 0)
        {
            int act = symbol_action;
            if (option -> backtrack)
            {
                Tuple<Dfa::ConflictCell> cells;
                for (int k = 0; k < action[symbol].Length(); k++)
                {
                    int item_no = action[symbol][k];

                    Dfa::ConflictCell &cell = cells.Next();
                    cell.action = base -> item_table[item_no].rule_number;
                    cell.priority = ComputeReducePriority(state_no, item_no, symbol);
                }

                Dfa::ConflictCell &cell = cells.Next();
                cell.action = (act < 0 ? act : act + grammar -> num_rules);
                cell.priority = ComputeShiftPriority(state_no, act, symbol);

                Dfa::Conflict &conf = conflict.Next();
                conf.SetSymbol(symbol);
                conf.SetConflictIndex(pda -> MapConflict(cells));
            }

            for (int k = 0; k < action[symbol].Length(); k++)
            {
                if (option -> conflicts)
                {
                    int index = sr_conflicts.NextIndex();
                    sr_conflicts[index].state_number = act;
                    sr_conflicts[index].item = action[symbol][k];
                    sr_conflicts[index].symbol = symbol;
                }

                pda -> num_shift_reduce_conflicts++;
            }

            //
            // Remove reduce actions defined on symbol so as to give
            // precedence to the shift.
            //
            action[symbol].Reset();
        }
    }

    return;
}


//
//
//
void Resolve::ResolveReduceReduceConflicts(int state_no,
                                           int symbol,
                                           Array< Tuple<int> > &action)
{
    Dfa::ConflictHeader &conflict = pda -> statset[state_no].conflict;
    Tuple<int> state_list;
    if (action[symbol].Length() > 1) // any Reduce/Reduce conflict
    {
        if (option -> single_productions)
            pda -> AddConflictSymbol(state_no, symbol);

        //
        // Note that if the symbol in question is the end-of-file symbol,
        // the reduce-reduce conflict is unresolvable.
        //
        if (option -> lalr_level > 1 && symbol != grammar -> eof_image)
        {
            sources.Clear();
            for (int k = 0; k < action[symbol].Length(); k++)
            {
                Resolve::StackElement *new_configs;
                int item_no = action[symbol][k],
                    act = base -> item_table[item_no].rule_number,
                    lhs_symbol = grammar -> rules[act].lhs;

                clear_visited();
                pda -> Access(state_list, state_no, item_no);
                for (int j = 0; j < state_list.Length(); j++)
                {
                    int access_state = state_list[j];

                    Resolve::StackElement *q = allocate_stack_element();
                    q -> state_number = access_state;
                    q -> size = 1;
                    q -> previous = NULL;
                    q -> next = NULL;

                    new_configs = follow_sources(q, lhs_symbol, symbol);
                    if (new_configs != NULL)
                        add_configs(sources, act, new_configs);
                }
            }

            //
            // STATE_TO_RESOLVE_CONFLICTS will return a pointer to a
            // LaStateElement if the conflicts were resolvable with more
            // lookaheads, otherwise, it returns NULL.
            //
            int la_state = state_to_resolve_conflicts(sources, symbol, 2);
            if (la_state != Util::OMEGA)
            {
                pda -> AddPredecessor(la_state, state_no);
                action[symbol].Reset();
            }
        }

        //
        // If unresolved reduce-reduce conflicts are detected on
        // symbol, add them to the list of conflicts so they can be
        // reported (if the CONFLICT option is on) and count them.
        //
        if (action[symbol].Length() > 1)
        {
            if (option -> backtrack)
            {
                Tuple<Dfa::ConflictCell> cells;
                for (int k = 0; k < action[symbol].Length(); k++)
                {
                    int item_no = action[symbol][k];

                    Dfa::ConflictCell &cell = cells.Next();
                    cell.action = base -> item_table[item_no].rule_number;
                    cell.priority = ComputeReducePriority(state_no, item_no, symbol);
                }

                Dfa::Conflict &conf = conflict.Next();
                conf.SetSymbol(symbol);
                conf.SetConflictIndex(pda -> MapConflict(cells));
            }

            int base_item = action[symbol][0];
            for (int k = 1; k < action[symbol].Length(); k++)
            {
                if (option -> conflicts)
                {
                    int index = rr_conflicts.NextIndex();
                    rr_conflicts[index].symbol = symbol;
                    rr_conflicts[index].item1  = base_item;
                    rr_conflicts[index].item2  = action[symbol][k];
                }

                pda -> num_reduce_reduce_conflicts++;
            }

            //
            // Remove all reduce actions that are defined on symbol
            // except the first one. That rule is the one with the
            // longest right-hand side that was associated with symbol.
            // See code in pda.cpp.
            //
            action[symbol].Reset(1);
        }
    }

    return;
}


//
//
//
void Resolve::ResolveKeywordIdentifierShiftReduceConflicts(int state_no,
                                                           int symbol,
                                                           int symbol_action,
                                                           int identifier_action,
                                                           Array< Tuple<int> > &action)
{
    Dfa::ConflictHeader &conflict = pda -> statset[state_no].conflict;
    Tuple<int> state_list;

    if (action[symbol].Length() > 0)
    {
        Array<bool> rule_seen(grammar -> num_rules + 1);
        rule_seen.MemReset();

        int act = symbol_action;

        Tuple<Dfa::ConflictCell> cells;
        for (int k = 0; k < action[symbol].Length(); k++)
        {
            int item_no = action[symbol][k];
            Dfa::ConflictCell &cell = cells.Next();
            cell.action = base -> item_table[item_no].rule_number;
            cell.priority = ComputeReducePriority(state_no, item_no, symbol);

            rule_seen[cell.action] = true;
        }

        Dfa::ConflictCell &cell = cells.Next();
        cell.action = (act < 0 ? act : act + grammar -> num_rules);
        cell.priority = ComputeShiftPriority(state_no, act, symbol);

        //
        // If more than one actions are defined on identifier, add
        // them all here.
        //
        if (identifier_action > pda -> num_states)
        {
            int offset = grammar -> num_rules; // the highest priority of a real action
            for (int k = identifier_action - pda -> num_states; pda -> conflicts[k] != 0; k++)
            {
                int act = pda -> conflicts[k];
                if (act < 0 || act > grammar -> num_rules || (! rule_seen[act]))
                {
                    Dfa::ConflictCell &identifier_cell = cells.Next();
                    identifier_cell.action = act;
                    identifier_cell.priority = offset + k; // keep these actions in the same order
                }
            }
        }
        else // no conflict on identifier.
        {
            Dfa::ConflictCell &identifier_cell = cells.Next();
            identifier_cell.action = identifier_action;
            identifier_cell.priority = Util::INFINITY_;
        }

        Dfa::Conflict &conf = conflict.Next();
        conf.SetSymbol(symbol);
        conf.SetConflictIndex(pda -> MapConflict(cells));

        //
        // Process conflicts and clear space.
        //
        {
            for (int k = 0; k < action[symbol].Length(); k++)
            {
                if (option -> conflicts)
                {
                    int index = sr_conflicts.NextIndex();
                    sr_conflicts[index].state_number = act;
                    sr_conflicts[index].item = action[symbol][k];
                    sr_conflicts[index].symbol = symbol;
                }

                pda -> num_shift_reduce_conflicts++;
            }
        }

        //
        // Remove reduce actions defined on symbol so as to give
        // precedence to the shift.
        //
        action[symbol].Reset();
    }
    else // No reductions. only a shift/shift conflict?
    {
        int act = symbol_action;
        Tuple<Dfa::ConflictCell> cells;

        Dfa::ConflictCell &cell = cells.Next();
        cell.action = (act < 0 ? act : act + grammar -> num_rules);
        cell.priority = 0;

        //
        //
        //
        if (identifier_action > pda -> num_states)
        {
            int offset = grammar -> num_rules; // the highest priority of a real action
            for (int k = identifier_action - pda -> num_states; pda -> conflicts[k] != 0; k++)
            {
                int act = pda -> conflicts[k];

                Dfa::ConflictCell &identifier_cell = cells.Next();
                identifier_cell.action = act;
                identifier_cell.priority = offset + k; // keep these actions in the same order
            }
        }
        else
        {
            Dfa::ConflictCell &identifier_cell = cells.Next();
            identifier_cell.action = identifier_action;
            identifier_cell.priority = Util::INFINITY_;
        }

        Dfa::Conflict &conf = conflict.Next();
        conf.SetSymbol(symbol);
        conf.SetConflictIndex(pda -> MapConflict(cells));
    }

    if (option -> conflicts)
    {
        int index = soft_ss_conflicts.NextIndex();
        soft_ss_conflicts[index].symbol = symbol;
    }
    pda -> num_shift_shift_conflicts++;

    return;
}


//
//
//
void Resolve::ResolveKeywordIdentifierReduceReduceConflicts(int state_no,
                                                            int symbol,
                                                            int identifier_action,
                                                            Array< Tuple<int> > &action)

{
    Dfa::ConflictHeader &conflict = pda -> statset[state_no].conflict;
    Tuple<int> state_list;

    assert(action[symbol].Length() > 0);
    int first_rule = base -> item_table[action[symbol][0]].rule_number;
    if (action[symbol].Length() > 1 || first_rule != identifier_action)
    {
        Array<bool> rule_seen(grammar -> num_rules + 1);
        rule_seen.MemReset();

        Tuple<Dfa::ConflictCell> cells;
        for (int k = 0; k < action[symbol].Length(); k++)
        {
            int item_no = action[symbol][k];

            Dfa::ConflictCell &cell = cells.Next();
            cell.action = base -> item_table[item_no].rule_number;
            cell.priority = ComputeReducePriority(state_no, item_no, symbol);
            rule_seen[cell.action] = true;
        }

        //
        //
        //
        if (identifier_action > grammar -> num_rules)
        {
            int offset = grammar -> num_rules; // the highest priority of a real action
            for (int i = identifier_action - grammar -> num_rules; pda -> conflicts[i] != 0; i++)
            {
                int act = pda -> conflicts[i];
                assert(act > 0 && act <= grammar -> num_rules);
                if (! rule_seen[act])
                {
                    Dfa::ConflictCell &identifier_cell = cells.Next();
                    identifier_cell.action = act;
                    identifier_cell.priority = offset + i; // keep these actions in the same order
                }
            }
        }
        else if (! rule_seen[identifier_action])
        {
            Dfa::ConflictCell &identifier_cell = cells.Next();
            assert(identifier_action > 0 && identifier_action <= grammar -> num_rules);
            identifier_cell.action = identifier_action;
            identifier_cell.priority = Util::INFINITY_;
        }

        Dfa::Conflict &conf = conflict.Next();
        conf.SetSymbol(symbol);
        conf.SetConflictIndex(pda -> MapConflict(cells));

        if (action[symbol].Length() > 1) // any real conflicts?
        {
            int base_item = action[symbol][0];
            for (int k = 1; k < action[symbol].Length(); k++)
            {
                if (option -> conflicts)
                {
                    int index = rr_conflicts.NextIndex();
                    rr_conflicts[index].symbol = symbol;
                    rr_conflicts[index].item1  = base_item;
                    rr_conflicts[index].item2  = action[symbol][k];
                }
                pda -> num_reduce_reduce_conflicts++;
            }
        }

        if (option -> conflicts)
        {
            int index = soft_rr_conflicts.NextIndex();
            soft_rr_conflicts[index].symbol = symbol;
            soft_rr_conflicts[index].item1  = action[symbol][0];
            int rule_no2 = (identifier_action <= grammar -> num_rules
                                ? identifier_action
                                : pda -> conflicts[identifier_action - grammar -> num_rules]);
            int item_no2 = base -> adequate_item[rule_no2] -> value;
            soft_rr_conflicts[index].item2 = item_no2;
        }
        pda -> num_soft_reduce_reduce_conflicts++;

        //
        // Remove all reduce actions that are defined on symbol
        // except the first one. That rule is the one with the
        // longest right-hand side that was associated with symbol.
        // See code in pda.cpp.
        //
        action[symbol].Reset(1);
    }

    return;
}


//
// Transfer the look-ahead states to their permanent destination, the
// array LASTATS and update the original automaton with the relevant
// transitions into the lookahead states.
//
void Resolve::CreateLastats(void)
{
    pda -> statset.Resize(pda -> max_la_state + 1);

    //
    // Allocate temporary space used to construct final lookahead
    // states.
    // The array shift_action will be used to construct a shift map
    // for a given state. It is initialized here to the empty map.
    // The array shift_count is used to count how many references
    // there are to each shift map.
    //
    Array<int> shift_action(grammar -> num_terminals + 1, Util::OMEGA),
               shift_count(pda -> max_la_state + 1, 0);

    for (int state_no = 1; state_no <= pda -> max_la_state; state_no++)
        shift_count[pda -> statset[state_no].shift_number]++;

    //
    // Traverse the list of lookahead states and initialize the
    // final lastat element appropriately. Also, construct a mapping
    // from each relevant initial state into the list of lookahead
    // states into which it can shift. We also keep track of these
    // initial states in a list headed by state_root.
    //
    Array<Tuple <int> > new_shift_actions(pda -> num_states + 1);
    Tuple<int> state_list;
    for (int la_state = pda -> num_states + 1; la_state <= pda -> max_la_state; la_state++)
    {
        if (pda -> statset[la_state].predecessors.Length() > 0) // unreachable states have no predecessors
        {
            assert(pda -> statset[la_state].predecessors.Length() == 1); // there is exactly one predecessor
            int state_no = pda -> statset[la_state].predecessors[0];
            if (state_no <= pda -> num_states)
            {
                if (new_shift_actions[state_no].Length() == 0)
                    state_list.Next() = state_no;
                new_shift_actions[state_no].Next() = la_state;
            }
        }
    }

    //
    // We now traverse the list of initial states that can shift into
    // lookahead states and update their shift map appropriately.
    //
    Tuple<int> shift_list;
    for (int j = 0; j < state_list.Length(); j++)
    {
        int state_no = state_list[j];
        //
        // Copy the shift map associated with STATE_NO into the direct
        // access map SHIFT_ACTION.
        //
        int shift_no = pda -> statset[state_no].shift_number;
        Dfa::ShiftHeader &sh = pda -> shift[shift_no];
        assert(shift_list.Length() == 0);
        for (int i = 0; i < sh.Length(); i++)
        {
            int symbol = sh[i].Symbol();
            shift_action[symbol] = sh[i].Action();

            shift_list.Next() = symbol;
        }

        //
        // Add the lookahead shift transitions to the initial shift
        // map.
        //
        int shift_size = sh.Length();
        for (int k = 0; k < new_shift_actions[state_no].Length(); k++)
        {
            int la_state = new_shift_actions[state_no][k],
                symbol = pda -> statset[la_state].transition_symbol;

            if (shift_action[symbol] == Util::OMEGA)
            {
                 shift_size++;
                 shift_list.Next() = symbol;
            }
            else if (shift_action[symbol] < 0)
                 pda -> num_shift_reduces--;
            else pda -> num_shifts--;

            shift_action[symbol] = la_state;
        }
        assert(shift_list.Length() == shift_size);

        //
        // If the initial shift map was shared by two or more states
        // then we have to construct a brand new shift map. Otherwise,
        // we reused the shift map.
        //
        if (shift_count[shift_no] > 1)
        {
             shift_count[shift_no]--;
             pda -> statset[state_no].shift_number = pda -> UpdateShiftMaps(shift_list, shift_action);
             shift_count[pda -> statset[state_no].shift_number]++;
        }
        else pda -> ResetShiftMap(shift_no, shift_list, shift_action);

        //
        // Reset the elements of shift_action that were used.
        //
        for (int l = 0; l < shift_list.Length(); l++)
        {
            int symbol = shift_list[l];
            shift_action[symbol] = Util::OMEGA;
        }
        shift_list.Reset();
    }

    return;
}
