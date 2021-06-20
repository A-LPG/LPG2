#include "sp.h"

//
// ComputeSpMap is an instantiation of the digraph algorithm. It
// is invoked repeatedly by remove_single_productions to:
//
//   1) Partially order the right-hand side of all the single
//      productions (SP) into a list [A1, A2, A3, ..., An]
//      such that if Ai -> Aj then i < j.
//
//   2) As a side effect, it uses the ordering above to order all
//      the SP rules.
//
// The bottom line is that this function computes a set
//     . is_sp_rule which identifies whether or not a given rule
//       is a single production
// and a list
//     . rule_list a partial order of the single production rules
//       as described above.
//
void Sp::ComputeSpMap(int symbol)
{
    stack.Push(symbol);
    int index = stack.Length();
    index_of[symbol] = index;

    //
    // In this instantiation of the digraph algorithm, two symbols (A, B)
    // are related if  A -> B  is a single production and A is the right-hand side of
    // some other single production rule.
    //
    for (int rule_no = sp_rules[symbol]; rule_no != Util::NIL; rule_no = next_rule[rule_no])
    {
        int lhs_symbol = grammar -> rules[rule_no].lhs;
        if (IsSingleProductionRhs(lhs_symbol))
        {
            if (index_of[lhs_symbol] == Util::OMEGA)
                ComputeSpMap(lhs_symbol);

            index_of[symbol] = Util::Min(index_of[symbol], index_of[lhs_symbol]);
        }
    }

    //
    // If the index of symbol is the same index it started with then
    // symbol if the root of a SCC...
    //
    if (index_of[symbol] == index)
    {
        //
        // If symbol is on top of the stack then it is the only
        // symbol in its SCC (thus it is not part of a cycle).
        // If the input grammar is conflict-free, the graph of
        // the single productions will never contain any cycle.
        // Thus, this test will always succeed and all single
        // productions associated with the symbol being processed
        // are added to the list of SP rules here...
        // If the grammar contains cycles, we skip the rules that
        // form such a cycle.
        //
        if (stack.Top() == symbol)
        {
            for (int rule_no = sp_rules[symbol]; rule_no != Util::NIL; rule_no = next_rule[rule_no])
            {
                is_sp_rule.AddElement(rule_no);
                rule_list.Next() = rule_no;
            }
        }

        //
        // As each SCC contains exactly one symbol (as explained above)
        // this loop will always execute exactly once.
        //
        int element;
        do
        {
            element = stack.Pop();
            index_of[element] = Util::INFINITY_;
        } while(element != symbol);
    }

    return;
}


//
// When the parser enters STATE_NO and it is processing SYMBOL, its
// next move is ACTION. Given these 3 parameters, compute_sp_action
// computes the set of reduce actions that may be executed after
// SYMBOL is shifted in STATE_NO.
//
// NOTE that this algorithm works only for the LALR(1) case. When the
// transition on SYMBOL is a lookahead-shift, indicating that the
// parser requires extra lookahead on a particular symbol, the set of
// reduce actions for that symbol is calculated here as the empty set.
//
void Sp::ComputeSingleProductionAction(int state_no, int symbol, int action)
{
    Dfa::GotoHeader &go_to = pda -> statset[state_no].go_to;

    if (sp_action[symbol].Length() == 0) // not yet allocated
        sp_action[symbol].Resize(grammar -> num_terminals + 1);
    sp_action[symbol].Initialize(Util::OMEGA);

    //
    // Note that before this routine is invoked, the global vector
    // index_of identifies the index of each symbol in the goto map of
    // state_no.
    //
    BitSet look_ahead(grammar -> num_terminals + 1);

    if (grammar -> IsTerminal(symbol) && is_conflict_symbol[symbol]) // do nothing for la_shift or conflict.
        ;
    else if (action > 0) // transition action (shift or goto)
    {
        for (Node *item_ptr = pda -> statset[action].complete_items; item_ptr != NULL; item_ptr = item_ptr -> next)
        {
            int item_no = item_ptr -> value,
                rule_no = base -> item_table[item_no].rule_number,
                lhs_symbol = grammar -> rules[rule_no].lhs;
            if (grammar -> RhsSize(rule_no) == 1 && lhs_symbol != grammar -> accept_image)
            {
                if (option -> slr)
                    look_ahead = base -> follow[lhs_symbol];
                else
                {
                    int i = index_of[lhs_symbol],
                        k = go_to[i].Laptr();
                    if (pda -> la_index[k] == Util::OMEGA)
                        pda -> LaTraverse(state_no, i);
                    look_ahead = pda -> la_set[k];
                }
                look_ahead.RemoveElement(grammar -> empty); // empty not valid look-ahead

                for (int i = grammar -> FirstTerminal(); i <= grammar -> LastTerminal(); i++)
                {
                    if (look_ahead[i])
                        sp_action[symbol][i] = rule_no;
                }
            }
        }

        //
        // Remove all lookahead symbols on which conflicts were
        // detected from consideration.
        //
        // TODO: REVIEW THIS!!!
        //
        for (int i = 0; i < pda -> conflict_symbols[action].Length(); i++)
            sp_action[symbol][pda -> conflict_symbols[action][i]] = Util::OMEGA;
    }
    else // read-reduce action
    {
        int rule_no = -action;
        if (grammar -> RhsSize(rule_no) == 1)
        {
            int lhs_symbol = grammar -> rules[rule_no].lhs;

            if (option -> slr)
                look_ahead = base -> follow[lhs_symbol];
            else
            {
                int i = index_of[lhs_symbol],
                    k = go_to[i].Laptr();
                if (pda -> la_index[k] == Util::OMEGA)
                    pda -> LaTraverse(state_no, i);
                look_ahead = pda -> la_set[k];
            }
            look_ahead.RemoveElement(grammar -> empty); // empty not valid look-ahead

            for (int i = grammar -> FirstTerminal(); i <= grammar -> LastTerminal(); i++)
            {
                if (look_ahead[i])
                    sp_action[symbol][i] = rule_no;
            }
        }
    }

    return;
}


//
// DefaultAction takes as parameter a state, state_no and a rule,
// rule_no that may be reduced when the parser enters state_no.
// DefaultAction tries to determine the highest rule that may be
// reached via a sequence of single production reductions.
//
int Sp::DefaultAction(int state_no, int rule_no)
{
    //
    // While the rule we have at hand is a single production, ...
    //
    Dfa::GotoHeader &go_to = pda -> statset[state_no].go_to;
    while (IsSingleProductionRule(rule_no))
    {
        int lhs_symbol = grammar -> rules[rule_no].lhs,
            action = go_to.Action(lhs_symbol);

        assert(action != Util::OMEGA);

        if (action < 0) // goto-reduce action?
        {
            action = -action;
            if (grammar -> RhsSize(action) != 1)
                break;

            rule_no = action;
        }
        else
        {
            int best_rule = Util::OMEGA;

            //
            // Enter the state action and look for preferably a SP rule
            // or some rule with right-hand side of size 1.
            //
            Dfa::ReduceHeader &red = pda -> statset[action].reduce;
            for (int i = 1; i < red.Length(); i++)
            {
                action = red[i].RuleNumber();
                if (IsSingleProductionRule(action))
                {
                    best_rule = action;
                    break;
                }
                if (grammar -> RhsSize(action) == 1)
                    best_rule = action;
            }
            if (best_rule == Util::OMEGA)
                break;

            rule_no = best_rule;
        }
    }

    return rule_no;
}

//
// This routine takes as parameter a state, state_no, a nonterminal,
// lhs_symbol (that is the right-hand side of a SP or a rule with
// right-hand side of size 1, but not identified as a SP) on which
// there is a transition in state_no and a lookahead symbol
// la_symbol that may be processed after taking the transition. It
// returns the reduce action that follows the transition if an
// action on la_symbol is found, otherwise it returns the most
// suitable default action.
//
int Sp::NonterminalAction(int state_no, int lhs_symbol, int la_symbol)
{
    Dfa::GotoHeader &go_to = pda -> statset[state_no].go_to;
    int action = go_to.Action(lhs_symbol);

    if (action < 0)
        action = -action;
    else
    {
        int state_no = action;
        Dfa::ReduceHeader &red = pda -> statset[state_no].reduce;
        action = Util::OMEGA;
        for (int i = 1; i < red.Length(); i++)
        {
            int rule_no = red[i].RuleNumber();
            if (red[i].Symbol() == la_symbol)
            {
                action = rule_no;
                break;
            }
            else if (action == Util::OMEGA && IsSingleProductionRule(rule_no))
            {
                 action = rule_no;
            }
        }
    }

    return action;
}


//
// Let red.RuleNumber() be a rule  A -> X.  The item [A -> .X] is in STATE1
// and STATE2 (update.state).  After shifting on X (in STATE1 and STATE2),
// if the lookahead is red.Symbol() then red.RuleNumber() is reduced.
// In STATE1, a sequence of single-production reductions is executed ending
// with a reduction of RULE1. In STATE2, a sequence of single-productions
// is also executed ending with RULE2.
// The goal of this function is to find the greatest ancestor of
// red.RuleNumber() that is also a descendant of both RULE1 and RULE2.
//
int Sp::GreatestCommonAncestor(Dfa::Reduce &red,
                               int state1,
                               int rule1,
                               UpdateActionElement &update)
{
    int action1 = red.RuleNumber(),
        action2 = action1,
        rule_no;
    do
    {
        assert(action1 != Util::OMEGA);
        rule_no = action1;
        if (action1 == rule1 || action2 == update.action)
            break;
        int lhs_symbol = grammar -> rules[rule_no].lhs;
        action1 = NonterminalAction(state1, lhs_symbol, red.Symbol());
        action2 = NonterminalAction(update.state, lhs_symbol, red.Symbol());
    } while (action1 == action2 && action1 != Util::OMEGA);

    return rule_no;
}


//
// In SOURCE_STATE there is a transition on SYMBOL into STATE_NO.
// SYMBOL is the right-hand side of a SP rule and the global map
// sp_action[SYMBOL] yields a set of update reduce actions that may
// follow the transition on SYMBOL into STATE_NO.
//
void Sp::ComputeUpdateActions(int source_state, int state_no, int symbol)
{
    Dfa::ReduceHeader &red = pda -> statset[state_no].reduce;
    for (int i = 1; i < red.Length(); i++)
    {
        if (IsSingleProductionRule(red[i].RuleNumber()))
        {
            int rule_no = sp_action[symbol][red[i].Symbol()];
            if (rule_no == Util::OMEGA)
                rule_no = DefaultAction(source_state, red[i].RuleNumber());

            //
            // Lookup the update map to see if a previous update was made
            // in STATE_NO on SYMBOL...
            //
            int k;
            for (k = 0; k < update_action[state_no].Length(); k++)
            {
                if (update_action[state_no][k].symbol == red[i].Symbol())
                    break;
            }

            //
            // If no previous update action was defined on STATE_NO and
            // SYMBOL, simply add it. Otherwise, chose as the greatest
            // common ancestor of the initial reduce action and the two
            // contending updates as the update action.
            //
            if (k == update_action[state_no].Length())
            {
                int index = update_action[state_no].NextIndex();

                update_action[state_no][index].symbol = red[i].Symbol();
                update_action[state_no][index].action = rule_no;
                update_action[state_no][index].state  = source_state;
            }
            else if ((rule_no != update_action[state_no][k].action) &&
                     (update_action[state_no][k].action != red[i].RuleNumber()))
            {
                update_action[state_no][k].action = GreatestCommonAncestor(red[i],
                                                                           source_state,
                                                                           rule_no,
                                                                           update_action[state_no][k]);
            }
        }
    }

    return;
}


//
// StateMap is invoked to create a new state using the reduce map
// sp_symbol[SYMBOL]. The new state will be entered via a transition
// on SYMBOL which is the right-hand side of the SP rule of which
// ITEM_NO is the final item.
//
// RULE_HEAD is the root of a list of rules in the global vector
// next_rule.  This list of rules identifies the range of the reduce
// map sp_symbol[SYMBOL]. The value SP_RULE_COUNT is the number of
// rules in the list. The value SP_ACTION_COUNT is the number of
// actions in the map sp_symbol[SYMBOL].
//
int Sp::StateMap(int rule_head, int item_no, int sp_rule_count, int sp_action_count, int symbol)
{
    //
    // These new SP states are defined by their reduce maps. Hash the
    // reduce map based on the set of rules in its range - simply add
    // them up and reduce modulo STATE_TABLE_SIZE.
    //
    unsigned hash_address = 0;
    for (int rule_no = rule_head; rule_no != Util::NIL; rule_no = next_rule[rule_no])
        hash_address += rule_no;
    hash_address %= STATE_TABLE_SIZE;

    //
    // Search the hash table for a compatible state. Two states S1
    // and S2 are compatible if
    //     1) the set of rules in their reduce map is identical.
    //     2) for each terminal symbol t, either
    //            reduce[S1][t] == reduce[S2][t] or
    //            reduce[S1][t] == OMEGA         or
    //            reduce[S2][t] == OMEGA
    //
    int state_no;
    for (state_no = sp_table[hash_address]; state_no != Util::NIL; state_no = sp_state[state_no].link)
    {
        SpStateElement &state = sp_state[state_no];
        if (state.rule.Length() == sp_rule_count) // same # of rules?
        {
            int i;
            for (i = 0; i < state.rule.Length(); i++)
            {
                if (next_rule[state.rule[i]] == Util::OMEGA)   // not in list?
                    break;
            }

            //
            // If the set of rules are identical, we proceed to compare the
            // actions for compatibility. The idea is to make sure that all
            // actions in the hash table do not clash in the actions in the
            // map sp_action[SYMBOL].
            //
            if (i == state.rule.Length()) // all the rules match?
            {
                int j;
                for (j = 0; j < state.action.Length(); j++)
                {
                    if (sp_action[symbol][state.action[j].symbol] != Util::OMEGA &&
                        sp_action[symbol][state.action[j].symbol] != state.action[j].action)
                         break;
                }

                //
                // If the two states are compatible merge them into the map
                // sp_action[SYMBOL]. (Note that this effectively destroys
                // the original map.) Also, keep track of whether or not an
                // actual merging action was necessary with the boolean
                // variable no_overwrite.
                //
                if (j == state.action.Length()) // compatible states
                {
                    //
                    // If the item was not previously associated with this
                    // state, add it.
                    //
                    Node *p;
                    for (p = state.complete_items; p != NULL; p = p -> next)
                    {
                        if (p -> value == item_no)
                            break;
                    }
                    if (p == NULL)
                    {
                        p = node_pool -> AllocateNode();
                        p -> value = item_no;
                        p -> next = state.complete_items;
                        state.complete_items = p;
                    }

                    //
                    // If the two maps are identical (there was no merging),
                    // return the state number otherwise, free the old map
                    // and break out of the search loop.
                    //
                    bool no_overwrite = true;
                    int k;
                    for (k = 0; k < state.action.Length(); k++)
                    {
                        if (sp_action[symbol][state.action[k].symbol] == Util::OMEGA)
                        {
                            sp_action[symbol][state.action[k].symbol] = state.action[k].action;
                            no_overwrite = false;
                        }
                    }

                    if (no_overwrite && state.action.Length() == sp_action_count)
                        return state_no;
                    break; // for (state = sp_table[hash_address]; ...
                }
            }
        }
    }

    //
    // If we did not find a compatible state, construct a new one.
    // Add it to the list of state and add it to the hash table.
    //
    if (state_no == Util::NIL)
    {
        state_no = sp_state.NextIndex();

        sp_state[state_no].link = sp_table[hash_address];
        sp_table[hash_address] = state_no;

        Node *p = node_pool -> AllocateNode();
        p -> value = item_no;
        p -> next = NULL;
        sp_state[state_no].complete_items = p;

        for (int rule_no = rule_head; rule_no != Util::NIL; rule_no = next_rule[rule_no])
            sp_state[state_no].rule.Next() = rule_no;
        assert(sp_state[state_no].rule.Length() == sp_rule_count);
    }

    //
    // If the state is new or had its reduce map merged with another
    // map, we update the reduce map here.
    //
    sp_state[state_no].action.Reset();
    for (int i = grammar -> FirstTerminal(); i <= grammar -> LastTerminal(); i++)
    {
        if (sp_action[symbol][i] != Util::OMEGA)
        {
            int index = sp_state[state_no].action.NextIndex();
            sp_state[state_no].action[index].symbol = i;
            sp_state[state_no].action[index].action = sp_action[symbol][i];
        }
    }

    return state_no;
}


//
// This program is invoked to remove as many single production actions as
// possible from the automaton.
//
void Sp::RemoveSingleProductions(void)
{
    //
    // Construct a set of all symbols used in the right-hand side of
    // single production in symbol_list. The variable symbol_root
    // points to the root of the list. Also, construct a mapping from
    // each symbol into the set of single productions of which it is
    // the right-hand side. sp_rules is the base of that map and the
    // relevant sets are stored in the vector next_rule.
    //
    int symbol_root = Util::NIL;
    Array<int> symbol_list(grammar -> num_symbols + 1, Util::OMEGA);
    sp_rules.Resize(grammar -> num_symbols + 1);
    sp_rules.Initialize(Util::NIL);
    next_rule.Resize(grammar -> num_rules + 1);
    for (int rule_no = grammar -> FirstRule(); rule_no <= grammar -> LastRule(); rule_no++)
    {
        if (grammar -> IsUnitProduction(rule_no))
        {
            int symbol = grammar -> rhs_sym[grammar -> rules[rule_no].rhs_index];
            next_rule[rule_no] = sp_rules[symbol];
            sp_rules[symbol] = rule_no;
            if (symbol_list[symbol] == Util::OMEGA) // first sighting of symbol
            {
                symbol_list[symbol] = symbol_root;
                symbol_root = symbol;
            }
        }
    }

    //
    // If no single productions were found, return.
    //
    if (symbol_root == Util::NIL)
        return;

    //
    // Iterate over the list of right-hand side symbols used in
    // single productions and invoke compute_sp_map to partially
    // order these symbols (based on the ::= (or ->) relationship) as
    // well as their associated rules. (See compute_sp_map for detail)
    //
    index_of.Resize(grammar -> num_symbols + 1);
    index_of.Initialize(Util::OMEGA);
    is_sp_rule.Initialize(grammar -> num_rules + 1);
    for (int i = symbol_root; i != Util::NIL; i = symbol_list[i])
    {
        if (index_of[i] == Util::OMEGA)
            ComputeSpMap(i);
    }

    //
    // Clear out all the sets in sp_rules and using the new revised
    // list of SP rules mark the new set of right-hand side symbols.
    // Note this code is important for consistency in case we are
    // removing single productions in an automaton containing
    // conflicts. If an automaton does not contain any conflict, the
    // new set of SP rules is always the same as the initial set.
    // See function IsSingleProductionRhs().
    //
    for (int j = symbol_root; j != Util::NIL; j = symbol_list[j])
        sp_rules[j] = Util::NIL;
    for (int k = 0; k < rule_list.Length(); k++)
    {
        int rule_no = rule_list[k],
            symbol = grammar -> rhs_sym[grammar -> rules[rule_no].rhs_index];
        sp_rules[symbol] = Util::OMEGA;
    }

    //
    // Traverse all regular states and process the relevant ones.
    //
    is_conflict_symbol.Initialize(grammar -> num_terminals + 1);
    sp_action.Resize(grammar -> num_symbols + 1);
    sp_table.Resize(STATE_TABLE_SIZE);
    sp_table.Initialize(Util::NIL);
    new_action.Resize(pda -> num_states + 1);
    update_action.Resize(pda -> num_states + 1);
    //
    // Initialize the set/list (symbol_root, symbol_list) to the
    // empty set/list.
    //
    symbol_root = Util::NIL;
    symbol_list.Initialize(Util::OMEGA);
    for (int state_no = 1; state_no <= pda -> num_states; state_no++)
    {
        //
        // If the state has no goto actions, it is not considered, as
        // no single productions could have been introduced in it.
        // Otherwise, we initialize index_of to the empty map and
        // presume that symbol_list is initialized to the empty set.
        //
        Dfa::GotoHeader &go_to = pda -> statset[state_no].go_to;
        if (go_to.Length() > 0)
        {
            is_conflict_symbol.SetEmpty();
            for (int i = 0; i < pda -> conflict_symbols[state_no].Length(); i++)
                is_conflict_symbol.AddElement(pda -> conflict_symbols[state_no][i]);

            //
            // First, use index_of to map each nonterminal symbol on
            // which there is a transition in state_no into its index
            // in the goto map of state_no.
            // Note that this initialization must be executed first
            // before we process the next loop, because index_of is
            // a global variable that is used in the routine
            // ComputeSingleProductionAction.
            //
            // Traverse first the goto map, then the shift map and
            // for each symbol that is the right-hand side of a single
            // production on which there is a transition, compute the
            // lookahead set that can follow this transition and add
            // the symbol to the set of candidates (in symbol_list).
            //
            index_of.Initialize(Util::OMEGA);
            for (int n = 0; n < go_to.Length(); n++)
                index_of[go_to[n].Symbol()] = n;

            for (int j = 0; j < go_to.Length(); j++)
            {
                int symbol = go_to[j].Symbol();
                if (IsSingleProductionRhs(symbol))
                {
                    ComputeSingleProductionAction(state_no, symbol, go_to[j].Action());
                    symbol_list[symbol] = symbol_root;
                    symbol_root = symbol;
                }
            }

            Dfa::ShiftHeader &sh = pda -> Shift(state_no);
            for (int k = 0; k < sh.Length(); k++)
            {
                int symbol = sh[k].Symbol();
                index_of[symbol] = k;
                if (IsSingleProductionRhs(symbol))
                {
                    ComputeSingleProductionAction(state_no, symbol, sh[k].Action());
                    symbol_list[symbol] = symbol_root;
                    symbol_root = symbol;
                }
            }

            //
            // We now traverse the set of single productions in order
            // and for each rule that was introduced through closure
            // in the state (there is an action on both the left- and
            // right-hand side)...
            //
            for (int l = 0; l < rule_list.Length(); l++)
            {
                int rule_no = rule_list[l],
                    symbol = grammar -> rhs_sym[grammar -> rules[rule_no].rhs_index];
                if (symbol_list[symbol] != Util::OMEGA)
                {
                    int lhs_symbol = grammar -> rules[rule_no].lhs;
                    if (index_of[lhs_symbol] != Util::OMEGA)
                    {
                        if (symbol_list[lhs_symbol] == Util::OMEGA)
                        {
                            ComputeSingleProductionAction(state_no,
                                                          lhs_symbol,
                                                          go_to[index_of[lhs_symbol]].Action());
                            symbol_list[lhs_symbol] = symbol_root;
                            symbol_root = lhs_symbol;
                        }

                        //
                        // Copy all reduce actions defined after the
                        // transition on the left-hand side into the
                        // corresponding action defined after the transition
                        // on the right-hand side. If an action is defined
                        // for the left-hand side -
                        //
                        //     sp_action[lhs_symbol][i] != OMEGA
                        //
                        // - but not for the right-hand side -
                        //
                        //     sp_action[symbol][i] == OMEGA
                        //
                        // it is an indication that after the transition on
                        // symbol, the action on i is a lookahead shift or a
                        // conflict. In that case, no action is copied.
                        //
                        for (int i = grammar -> FirstTerminal(); i <= grammar -> LastTerminal(); i++)
                        {
                            if (sp_action[lhs_symbol][i] != Util::OMEGA &&
                                sp_action[symbol][i] != Util::OMEGA)
                                    sp_action[symbol][i] = sp_action[lhs_symbol][i];
                        }
                    }
                }
            }

            //
            // For each symbol that is the right-hand side of some SP
            // for which a reduce map is defined, we either construct
            // a new state if the transition is into a final state,
            // or we update the relevant reduce action of the state
            // into which the transition is made, otherwise.
            //
            // When execution of this loop is terminated the set
            // symbol_root/symbol_list is reinitialized to the empty
            // set.
            //
            for (int symbol = symbol_root; symbol != Util::NIL; symbol_list[symbol] = Util::OMEGA, symbol = symbol_root)
            {
                symbol_root = symbol_list[symbol];

                if (IsSingleProductionRhs(symbol))
                {
                    int original_action = (grammar -> IsTerminal(symbol)
                                                    ? sh[index_of[symbol]].Action()
                                                    : go_to[index_of[symbol]].Action()),
                        action = original_action;

                    //
                    // If the transition is a lookahead shift or a coflict,
                    // do nothing. If the action is a goto- or shift-reduce,
                    // compute the relevant rule and item involved.
                    // Otherwise, the action is a shift or a goto. If the
                    // transition is into a final state then it is
                    // processed as the case of read-reduce above. If
                    // not, we invoke compute_update_actions to update
                    // the relevant actions.
                    //
                    int item_no = Util::OMEGA,
                        rule_no;
                    if (grammar -> IsTerminal(symbol) && is_conflict_symbol[symbol]) // a conflict or a lookahead shift
                        rule_no = Util::OMEGA;
                    else if (action < 0) // read-reduce action
                    {
                        rule_no = -action;
                        item_no = base -> adequate_item[rule_no] -> value;
                    }
                    else                // transition action
                    {
                        Node *item_ptr = pda -> statset[action].kernel_items;
                        item_no = item_ptr -> value;
                        if ((item_ptr -> next == NULL) &&
                            (base -> item_table[item_no].symbol == grammar -> empty))
                             rule_no = base -> item_table[item_no].rule_number;
                        else
                        {
                            ComputeUpdateActions(state_no, action, symbol);
                            rule_no = Util::OMEGA;
                        }
                    }

                    //
                    // If we have a valid SP rule we first construct the
                    // set of rules in the range of the reduce map of the
                    // right-hand side of the rule. If that set contains
                    // a single rule then the action on the right-hand
                    // side is redefined as the same action on the left-
                    // hand side of the rule in question. Otherwise, we
                    // create a new state for the final item of the SP
                    // rule consisting of the reduce map associated with
                    // the right-hand side of the SP rule and the new
                    // action on the right-hand side is a transition into
                    // this new state.
                    //
                    if (rule_no != Util::OMEGA)
                    {
                        if (IsSingleProductionRule(rule_no))
                        {
                            int sp_rule_count = 0,
                                sp_action_count = 0,
                                rule_head = Util::NIL;
                            next_rule.Initialize(Util::OMEGA);
                            for (int i = grammar -> FirstTerminal(); i <= grammar -> LastTerminal(); i++)
                            {
                                rule_no = sp_action[symbol][i];
                                if (rule_no != Util::OMEGA)
                                {
                                    sp_action_count++;
                                    if (next_rule[rule_no] == Util::OMEGA)
                                    {
                                        sp_rule_count++;
                                        next_rule[rule_no] = rule_head;
                                        rule_head = rule_no;
                                    }
                                }
                            }

                            if (sp_rule_count == 1 && IsSingleProductionRule(rule_head))
                            {
                                int lhs_symbol = grammar -> rules[rule_head].lhs;
                                action = go_to[index_of[lhs_symbol]].Action();
                            }
                            else
                            {
                                int sp_state_no = StateMap(rule_head,
                                                           item_no,
                                                           sp_rule_count,
                                                           sp_action_count,
                                                           symbol);
                                action = pda -> num_states + 1 + sp_state_no;
                            }

                            int index = new_action[state_no].NextIndex();
                            new_action[state_no][index].symbol = symbol;
                            new_action[state_no][index].action = action;

                            //
                            // If the states have to be printed, then save the item
                            // associated with a single production that was eliminated.
                            //
                            if (option -> states && original_action < 0)
                            {
                                Node *p = node_pool -> AllocateNode();
                                p -> value = item_no - 1;
                                p -> next = pda -> statset[state_no].single_production_items;
                                pda -> statset[state_no].single_production_items = p;
                            }
                        }
                    }
                }
            }
        }
    } // for (state_no = 1; state_no <= pda -> num_states; state_no++)

    AddSingleProductionStates();

    UpdateBaseAutomaton();
    UpdateConflictActions();
    UpdateLookaheadAutomata();

    return;
}


//
//
//
void Sp::AddSingleProductionStates()
{
    if (sp_state.Length() > 0)
    {
        //
        // If the automaton contained lookahead states, these states appeared
        // immmediately after the regular states in the range
        // [pda->num_states+1 .. pda->max_la_state]. At this point we have
        // created sp_state.Length() new single production states that we
        // need to lie in the range [pda->num_states+1 .. pda->num_states+sp_state.Length()].
        // Therefore, we need to first move the lookahead states to the range
        // [pda->num_states+sp_state.Length()+1 .. pda->max_la_state+sp_state.Length()].
        //
        MoveLaStates();

        //
        // We now permanently construct all the new SP states, if any.
        //
        Array<int> rule_count(grammar -> num_rules + 1);
        for (int offset = 0; offset < sp_state.Length(); offset++)
        {
            SpStateElement &state = sp_state[offset];

            int state_no = pda -> num_states + offset + 1; // new state number

            //
            // These states are identified as special SP states since
            // they have no kernel items. They also have no goto and
            // shift actions.
            //
            pda -> statset[state_no].kernel_items = NULL;
            pda -> statset[state_no].complete_items = state.complete_items;
            pda -> statset[state_no].single_production_items = NULL;
            pda -> statset[state_no].shift_number = 0;
            pda -> statset[state_no].transition_symbol = grammar -> empty;

            assert(pda -> statset[state_no].go_to.Length() == 0);
            assert(pda -> statset[state_no].predecessors.Length() == 0);

            //
            // Count the number of actions defined on each rule in the
            // range of the reduce map.
            //
            for (int i = 0; i < state.rule.Length(); i++)
                rule_count[state.rule[i]] = 0;

            for (int j = 0; j < state.action.Length(); j++)
                rule_count[state.action[j].action]++;

            //
            // Count the total number of reduce actions in the reduce map
            // and calculate the default.
            //
            int default_rule = Util::OMEGA,
                reduce_size = 0,
                sp_rule_count = 0;
            for (int k = 0; k < state.rule.Length(); k++)
            {
                reduce_size += rule_count[state.rule[k]];
                if (rule_count[state.rule[k]] > sp_rule_count)
                {
                    sp_rule_count = rule_count[state.rule[k]];
                    default_rule = state.rule[k];
                }
            }

            //
            // Construct a permanent reduce map for this SP state.
            //
            pda -> num_reductions += reduce_size;

            Dfa::ReduceHeader &red = pda -> statset[state_no].reduce;
            assert(state.action.Length() == reduce_size);
            red.Resize(state.action.Length() + 1);
            red[0].SetSymbol(Grammar::DEFAULT_SYMBOL);
            red[0].SetRuleNumber(default_rule);
            for (int l = 0; l < state.action.Length(); l++)
            {
                red[reduce_size].SetSymbol(state.action[l].symbol);
                red[reduce_size].SetRuleNumber(state.action[l].action);
                reduce_size--;
            }
        }
    }

    return;
}

//
// For each state with updates or new actions, take appropriate
// actions.
//
void Sp::UpdateBaseAutomaton()
{
    Array<int> shift_count(pda -> max_la_state + 1, 0);
    {
        for (int state_no = 1; state_no <= pda -> max_la_state; state_no++)
            shift_count[pda -> statset[state_no].shift_number]++;
    }

    for (int state_no = 1; state_no <= pda -> num_states; state_no++)
    {
        //
        // Update reduce actions for final items of single production
        // that are in non-final states.
        //
        if (update_action[state_no].Length() != 0)
        {
            Dfa::ReduceHeader &red = pda -> statset[state_no].reduce;
            //
            // Keep track of the index associated with in symbol in the
            // domain of the reduce map.
            //
            for (int i = 1; i < red.Length(); i++)
                index_of[red[i].Symbol()] = i;

            //
            // Update the action associated with each symbol in the reduce map.
            //
            for (int k = 0; k < update_action[state_no].Length(); k++)
                red[index_of[update_action[state_no][k].symbol]].SetRuleNumber(update_action[state_no][k].action);
        }

        //
        // Update initial automaton with transitions into new SP
        // states.
        //
        if (new_action[state_no].Length() != 0)
        {
            Array<int> shift_action(grammar -> num_terminals + 1, Util::OMEGA);

            //
            // Mark the index of each symbol on which there is a
            // transition and copy the shift map into the vector
            // shift_action.
            //
            Dfa::GotoHeader &go_to = pda -> statset[state_no].go_to;
            for (int i = 0; i < go_to.Length(); i++)
                index_of[go_to[i].Symbol()] = i;

            Dfa::ShiftHeader &sh = pda -> Shift(state_no);
            for (int j = 0; j < sh.Length(); j++)
            {
                index_of[sh[j].Symbol()] = j;
                shift_action[sh[j].Symbol()] = sh[j].Action();
            }

            //
            // Iterate over the new action and update the goto map
            // directly for goto actions but update shift_action
            // for shift actions. Also, keep track as to whether or
            // not there were any shift transitions at all...
            //
            bool any_shift_action = false;
            for (int k = 0; k < new_action[state_no].Length(); k++)
            {
                if (grammar -> IsNonTerminal(new_action[state_no][k].symbol))
                {
                    if (go_to[index_of[new_action[state_no][k].symbol]].Action() < 0 &&
                        new_action[state_no][k].action > 0)
                    {
                        pda -> num_goto_reduces--;
                        pda -> num_gotos++;
                    }
                    go_to[index_of[new_action[state_no][k].symbol]].SetAction(new_action[state_no][k].action);
                }
                else
                {
                    if (sh[index_of[new_action[state_no][k].symbol]].Action() < 0 && new_action[state_no][k].action > 0)
                    {
                        pda -> num_shift_reduces--;
                        pda -> num_shifts++;
                    }
                    shift_action[new_action[state_no][k].symbol] = new_action[state_no][k].action;

                    any_shift_action = true;
                }
            }

            //
            // If there were any shift actions, a new shift map may
            // have been created. Hash shift_action into the
            // shift hash table.
            //
            if (any_shift_action)
            {
                Tuple<int> shift_list;
                for (int i = 0; i < sh.Length(); i++) // Compute Hash location
                    shift_list.Next() = sh[i].Symbol();

                //
                // If the initial shift map was shared by two or more states
                // then we have to construct a brand new shift map. Otherwise,
                // we reused the shift map.
                //
                int shift_no = pda -> statset[state_no].shift_number;
                if (shift_count[shift_no] > 1)
                {
                     shift_count[shift_no]--;
                     pda -> statset[state_no].shift_number = pda -> UpdateShiftMaps(shift_list, shift_action);
                     shift_count[pda -> statset[state_no].shift_number]++;
                }
                else pda -> ResetShiftMap(shift_no, shift_list, shift_action);
            }
        }
    }

    //
    // All updates have now been made to the base automaton, adjust
    // the number of regular states to include the new SP states.
    //
    pda -> num_states += sp_state.Length();

    return;
}


//
//
//
void Sp::UpdateConflictActions()
{
    Tuple<ConflictLocation> candidates;
    Array<int> conflict_count(pda -> conflicts.Length());
    conflict_count.MemReset();
    for (int state_no = 1; state_no <= pda -> num_states; state_no++)
    {
        Dfa::ShiftHeader &sh = pda -> Shift(state_no);
        Dfa::ConflictHeader &conflict = pda -> statset[state_no].conflict;
        for (int i = 0; i < conflict.Length(); i++)
        {
            int symbol = conflict[i].Symbol(),
                action = sh.Action(symbol);

            //
            // Do we have a shift-reduce action by a single production rule?
            //
            if (action != Util::OMEGA && action < 0 && IsSingleProductionRule(-action))
            {
                int index = candidates.NextIndex();
                candidates[index].state = state_no;
                candidates[index].conflict_index = i;
                conflict_count[conflict[i].ConflictIndex()]++;
            }
        }
    }

    Tuple<Dfa::ConflictCell> cells;
    for (int i = 0; i < candidates.Length(); i++)
    {
        int state_no = candidates[i].state;
        Dfa::GotoHeader &go_to = pda -> statset[state_no].go_to;
        Dfa::ConflictHeader &conflict = pda -> statset[state_no].conflict;
        cells.Reset();
        int conflict_index = conflict[candidates[i].conflict_index].ConflictIndex();
        for (int j = conflict_index; pda -> conflicts[j] != 0; j++)
        {
            int index = cells.NextIndex();
            cells[index].priority = cells.Length(); // keep current order.
            if (pda -> conflicts[j] < 0) // found the shift reduce action
            {
                int rule_no = -(pda -> conflicts[j]),
                    lhs_symbol = grammar -> rules[rule_no].lhs;
                cells[index].action = go_to.Action(lhs_symbol);
            }
            else cells[index].action = pda -> conflicts[j];
        }

        if (conflict_count[conflict_index] > 1)
        {
             conflict_count[conflict_index]--;
             conflict[candidates[i].conflict_index].SetConflictIndex(pda -> MapConflict(cells));
        }
        else pda -> RemapConflict(conflict_index, cells);
    }

    return;
}


//
//
//
void Sp::UpdateLookaheadAutomata()
{
    Array<int> shift_count(pda -> max_la_state + 1, 0);
    for (int state_no = 1; state_no <= pda -> max_la_state; state_no++)
        shift_count[pda -> statset[state_no].shift_number]++;

    //
    // We now process action on single productions in the lookahead states
    //
    for (int la_state = pda -> num_states + 1; la_state < pda -> max_la_state; la_state++)
    {
        int source_state = SourceState(la_state);
        if (source_state != Util::OMEGA) // Is the la_state accessible?
        {
            Dfa::GotoHeader &go_to = pda -> statset[source_state].go_to;
            for (int i = 0; i < go_to.Length(); i++)
                index_of[go_to[i].Symbol()] = i;

            Tuple<int> shift_list;
            Array<int> shift_action(grammar -> num_terminals + 1, Util::OMEGA);

            bool any_new_action = false;
            Dfa::ShiftHeader &sh = pda -> Shift(la_state);
            for (int j = 0; j < sh.Length(); j++)
            {
                int symbol = sh[j].Symbol(),
                    action = sh[j].Action();
                if (action < 0) // a shift-reduce?
                {
                    int rule_no = -action;
                    if (IsSingleProductionRule(rule_no))
                    {
                        int lhs_symbol = grammar -> rules[rule_no].lhs;
                        action = go_to[index_of[lhs_symbol]].Action();
                        any_new_action = true;
                    }
                }
                shift_action[symbol] = action;
                shift_list.Next() = symbol;
            }

            //
            // If any single production was removed, a new shift map
            // must be created.
            //
            if (any_new_action)
            {
                //
                // If the initial shift map was shared by two or more states
                // then we have to construct a brand new shift map. Otherwise,
                // we reused the shift map.
                //
                int shift_no = pda -> statset[la_state].shift_number;
                if (shift_count[shift_no] > 1)
                {
                     shift_count[shift_no]--;
                     pda -> statset[la_state].shift_number = pda -> UpdateShiftMaps(shift_list, shift_action);
                     shift_count[pda -> statset[la_state].shift_number]++;
                }
                else pda -> ResetShiftMap(shift_no, shift_list, shift_action);
            }
        }
    }

    return;
}

//
//
//
void Sp::MoveLaStates()
{
    //
    // We are now ready to extend all global maps based on states and
    // permanently install the new states.
    //
    int offset = sp_state.Length();
    pda -> statset.Resize(pda -> max_la_state + offset + 1);
    if (pda -> gd_index.Length() > 0) // see class PRODUCE
    {
        //
        // Each element gd_index[i] points to the starting location
        // of a slice in another array, gd_range. The last element of
        // the slice can be computed as (gd_index[i+1] - 1). We now extend
        // gd_index by setting each new element to the end of gd_range, thus
        // associating a null slice with these new states.
        //
        for (int i = pda -> gd_index.Length(); i <= pda -> statset.Length(); i++)
            pda -> gd_index.Next() = pda -> gd_range.Length(); // gate !!!
    }

    //
    // Shift all the lookahead states down. Note that it is important to
    // iterate in descending order over the lookahead states as is done
    // below since there may be overlapping indexes between the old
    // range (num_states + 1 .. max_la_state) and the new range
    // (num_states + 1 + offset .. max_la_state + offset).
    //
    for (int state_num = pda -> max_la_state; state_num > pda -> num_states; state_num--)
    {
        Dfa::StateElement &new_state = pda -> statset[state_num + offset],
                          &old_state = pda -> statset[state_num];

        //
        // A Lookahead state does not contain a kernel items set
        // A Lookahead state does not contain a complete items set
        // A Lookahead state does not contain a single_production items set
        //
        assert(old_state.predecessors.Length() <= 1); // A good lookahead state has one predeccessor. A bad one has none!
        if (old_state.predecessors.Length() == 1)
        {
            int state_no = old_state.predecessors[0];
            new_state.predecessors.Next() = (state_no > pda -> num_states ? state_no + offset
                                                                          : state_no);
        }

        // A Lookahead state does not contain a goto map.
        new_state.reduce = old_state.reduce;
        new_state.shift_number = old_state.shift_number;
        new_state.transition_symbol = old_state.transition_symbol;

        //
        // Reset data in old state...
        //
        old_state.predecessors.Reset();
        old_state.reduce.Reset();
    }
    pda -> max_la_state += offset;

    //
    // We now adjust all references to a lookahead state. The idea is
    // to offset the number associated with each lookahead state by
    // the number of new SP states that were added.
    // Note that the shift map does not include conflict actions as
    // these actions, if present, are added to the shift map after
    // single productions have been removed.
    //
    for (int k = 1; k <= pda -> num_shift_maps; k++)
    {
        Dfa::ShiftHeader &sh = pda -> shift[k];
        for (int i = 0; i < sh.Length(); i++)
        {
            if (sh[i].Action() > pda -> num_states)
                sh[i].SetAction(sh[i].Action() + offset);
        }
    }

    return;
}
