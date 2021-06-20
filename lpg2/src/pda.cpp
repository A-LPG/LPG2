#include "control.h"
#include "resolve.h"
#include "sp.h"

#include <iostream>
using namespace std;

//
// Given an item of the form: [x .A y], where x and y are arbitrary strings,
// and A is a non-terminal, we pretrace the path(s) in the automaton  that
// will be followed in computing the look-ahead set for that item in
// STATE_NO.  A number is assigned to all pairs (S, B), where S is a state,
// and B is a non-terminal, involved in the paths. GOTO_INDEX points to the
// GOTO_ELEMENT of (STATE_NO, A).
//
void Pda::TraceLalrPath(int state_no, int goto_index)
{
    //
    //  If STATE is a state number we first check to see if its base
    // look-ahead set is a special one that does not contain EMPTY and
    // has already been assigned a slot that can be reused.
    // ((LA_BASE[STATE] != OMEGA) signals this condition.)
    // NOTE that look-ahead follow sets are shared only when the maximum
    // look-ahead level allowed is 1 and single productions will not be
    // removed. If either (or both) of these conditions is true, we need
    // to have a unique slot assigned to each pair [S, A] (where S is a
    // state, and A is a non-terminal) in the automaton.
    //
    Dfa::GotoHeader &go_to = statset[state_no].go_to;
    int state = go_to[goto_index].Action();
    if (state > 0 && (la_base[state] != Util::OMEGA &&
                      option -> lalr_level == 1 &&
                      (! option -> single_productions) &&
                      (! option -> backtrack)))
    {
        go_to[goto_index].SetLaptr(la_base[state]);
        return;
    }

    //
    // At this point, R points to a list of items which are the successors
    // of the items needed to initialize the Look-ahead follow sets.  If
    // anyone of these items contains EMPTY, we trace the Digraph for other
    // look-ahead follow sets that may be needed, and signal this fact
    // using the variable CONTAINS_EMPTY.
    //
    la_top++;   // allocate new slot
    assert(la_top <= INT_MAX);
    go_to[goto_index].SetLaptr(la_top);
    bool contains_empty = false;
    Tuple<int> state_list;
    Node *kernel_root = (state > 0 ? statset[state].kernel_items : base -> adequate_item[-state]);
    for (Node *r = kernel_root; r != NULL;  r = r -> next)
    {
        int item = r -> value - 1;
        if (base -> First(base -> item_table[item].suffix_index)[grammar -> empty])
        {
            contains_empty = true;
            int lhs_symbol = grammar -> rules[base -> item_table[item].rule_number].lhs;
            Access(state_list, state_no, item);
            for (int k = 0; k < state_list.Length(); k++)
            {
                int access_state = state_list[k];
                Dfa::GotoHeader &go_to = statset[access_state].go_to;
                int i = go_to.Index(lhs_symbol);
                if (go_to[i].Laptr() == Util::OMEGA)
                    TraceLalrPath(access_state, i);
            }
        }
    }

    //
    // If the look-ahead follow set involved does not contain EMPTY, we
    // mark the state involved (STATE) so that other look-ahead follow
    // sets which may need this set may reuse the same one.
    // NOTE that if CONTAINS_EMPTY is false, then STATE has to denote a
    // state number (positive value) and not a rule number (negative).
    //
    if (! contains_empty)
        la_base[state] = go_to[goto_index].Laptr();

    return;
}


//
// COMPUTE_READ computes the number of intermediate look-ahead sets that
// will be needed (in LA_TOP), allocates space for the sets(LA_SET), and
// initializes them.
// By intermediate look-ahead set, we mean the set of terminals that may
// follow a non-terminal in a given state.
// These sets are initialized to the set of terminals that can immediately
// follow the non-terminal in the state to which it can shift (READ set).
//
void Pda::ComputeRead(void)
{
    if (option -> lalr_level > 1 || option -> single_productions || option -> backtrack)
    {
        read_set.Resize(num_states + 1);
        for (int i = 1; i <= num_states; i++)
            read_set[i].Initialize(grammar -> num_terminals + 1);
    }

    //
    //  We traverse all the states and for all complete items that requires
    // a look-ahead set, we retrace the state digraph (with the help of the
    // routine TRACE_LALR_PATH) and assign a unique number to all look-ahead
    // follow sets that it needs. A look-ahead follow set is a set of
    // terminal symbols associated with a pair [S, A], where S is a state,
    // and A is a non-terminal:
    //
    // [S, A] --> Follow-set
    // Follow-set = {t | t is a terminal that can be shifted on after
    //                      execution of a goto action on A in state S}.
    //
    // Each follow set is initialized with the set of terminals that can be
    // shifted on in state S2, where GOTO(S, A) = S2. After initialization
    // a follow set F that does not contain the special terminal symbol
    // EMPTY is marked with the help of the array LA_BASE, and if the
    // highest level of look-ahead allowed is 1, then only one such set is
    // allocated, and shared for all pairs (S, B) whose follow set is F.
    //
    Tuple<int> state_list;
    la_base.Resize(num_states + 1);
    la_base.Initialize(Util::OMEGA);
    la_top = 0;
    for (int state_no = 1; state_no <= num_states; state_no++)
    {
        for (Node *p = ((option -> lalr_level <= 1 && single_complete_item[state_no])
                        ? NULL
                        : statset[state_no].complete_items);
             p != NULL;
             p = p -> next)
        {
            int item_no = p -> value,
                rule_no = base -> item_table[item_no].rule_number,
                lhs_symbol = grammar -> rules[rule_no].lhs;
            if (lhs_symbol != grammar -> accept_image)
            {
                Access(state_list, state_no, item_no);
                for (int k = 0; k < state_list.Length(); k++)
                {
                    int access_state = state_list[k];
                    Dfa::GotoHeader &go_to = statset[access_state].go_to;
                    int i = go_to.Index(lhs_symbol);
                    if (go_to[i].Laptr() == Util::OMEGA)
                        TraceLalrPath(access_state, i);
                }
            }
        }

        //
        //  If the look-ahead level is greater than 1 or single productions
        // actions are to be removed when possible or we need to trace backtrack
        // information, then we have to compute
        // a Follow-set for all pairs [S, A] in the state automaton. Therefore,
        // we also have to consider Shift-reduce actions as reductions, and
        // trace back to their roots as well.
        // Note that this is not necessary for Goto-reduce actions. Since
        // they terminate with a non-terminal. Each such terminating non-terminal
        // is followed by the empty string and it must produce a
        // rule that either ends up in a reduction, a shift-reduce, or another
        // goto-reduce. It will therefore be taken care of automatically by
        // transitive closure.
        //
        if (option -> lalr_level > 1 || option -> single_productions || option -> backtrack)
        {
            Dfa::ShiftHeader &sh = Shift(state_no);
            for (int j = 0; j < sh.Length(); j++)
            {
                if (sh[j].Action() < 0)
                {
                    int rule_no = - sh[j].Action(),
                        lhs_symbol = grammar -> rules[rule_no].lhs,
                        item_no = base -> adequate_item[rule_no] -> value - 1;
                    Access(state_list, state_no, item_no);
                    for (int k = 0; k < state_list.Length(); k++)
                    {
                        int access_state = state_list[k];
                        Dfa::GotoHeader &go_to = statset[access_state].go_to;
                        int i = go_to.Index(lhs_symbol);
                        if (go_to[i].Laptr() == Util::OMEGA)
                            TraceLalrPath(access_state, i);
                    }
                }
            }

            //
            // When more than one lookahead is requested, we also need
            // to compute the set of terminal symbols that can be read
            // in a state entered via a terminal transition.
            //
            if (option -> lalr_level > 1 && state_no != 1)
            {
                Node *q = statset[state_no].kernel_items;
                int item_no = q -> value - 1;
                if (grammar -> IsTerminal(base -> item_table[item_no].symbol))
                {
                    read_set[state_no] = base -> First(base -> item_table[item_no].suffix_index);
                    for (q = q -> next; q != NULL; q = q -> next)
                    {
                        item_no = q -> value - 1;
                        read_set[state_no] += base -> First(base -> item_table[item_no].suffix_index);
                    }
                }
            }
        }
    }


    //
    //   We now allocate space for LA_INDEX and LA_SET, and initialize
    // all its elements as indicated in reduce.h. The array LA_BASE is
    // used to keep track of Follow sets that have been initialized. If
    // another set needs to be initialized with a value that has been
    // already computed, LA_BASE is used to retrieve the value.
    //
    la_base.Initialize(Util::OMEGA);
    la_index.Resize(la_top + 1);
    la_set.Resize(la_top + 1);
    for (int i = 0; i <= la_top; i++)
        la_set[i].Initialize(grammar -> num_terminals + 1);
    {
        for (int state_no = 1; state_no <= num_states; state_no++)
        {
            Dfa::GotoHeader &go_to = statset[state_no].go_to;
            for (int i = 0; i < go_to.Length(); i++)
            {
                int la_ptr = go_to[i].Laptr();
                if (la_ptr != Util::OMEGA)   // Follow Look-ahead needed
                {
                    int state = go_to[i].Action();
                    Node *q;
                    if (state > 0)
                    {
                        if (la_base[state] != Util::OMEGA)  // already computed
                        {
                            la_index[la_ptr] = la_index[la_base[state]];
                            la_set[la_ptr] = la_set[la_base[state]];
                            q = NULL;
                        }
                        else
                        {
                            la_base[state] = la_ptr;
                            q = statset[state].kernel_items;
                        }
                    }
                    else q = base -> adequate_item[-state];

                    if (q != NULL)
                    {
                        int item_no = q -> value - 1; // initialize with first item
                        la_set[la_ptr] = base -> First(base -> item_table[item_no].suffix_index);
                        for (q = q -> next; q != NULL; q = q -> next)
                        {
                            item_no = q -> value - 1;
                            la_set[la_ptr] += base -> First(base -> item_table[item_no].suffix_index);
                        }
                        la_index[la_ptr] = (la_set[la_ptr][grammar -> empty] ? Util::OMEGA : Util::INFINITY_);
                        if (option -> lalr_level > 1 || option -> single_productions || option -> backtrack)
                        {
                            if (state > 0)
                                read_set[state] = la_set[la_ptr];
                        }
                    }
                }
            }
        }
    }

    return;
}


//
// LA_TRAVERSE takes two major arguments: STATE_NO, and an index (GOTO_INDEX)
// that points to the GOTO_ELEMENT array in STATE_NO for the non-terminal
// left hand side of an item for which look-ahead is to be computed. The
// look-ahead of an item of the form [x. A y] in state STATE_NO is the set
// of terminals that can appear immediately after A in the context summarized
// by STATE_NO. When a look-ahead set is computed, the result is placed in
// an allocation of LA_ELEMENT pointed to by the LA_PTR field of the
// GOTO_ELEMENT array.
//
// The same digraph algorithm used in MKFIRST is used for this computation.
//
void Pda::LaTraverse(int state_no, int goto_index)
{
    Dfa::GotoHeader &go_to = statset[state_no].go_to;
    int la_ptr = go_to[goto_index].Laptr();
    stack.Push(la_ptr);
    int index = stack.Length();
    la_index[la_ptr] = index;

    //
    // Compute STATE, action to perform on Goto symbol in question. If
    // STATE is positive, it denotes a state to which to shift. If it is
    // negative, it is a rule on which to perform a Goto-Reduce.
    //
    Tuple<int> state_list;
    int state = go_to[goto_index].Action();
    for (Node *r = (state > 0 ? statset[state].kernel_items : base -> adequate_item[-state]);
         r != NULL;
         r = r -> next)
    {  // loop over items [A -> x LHS_SYMBOL . y]
        int item = r -> value - 1;
        if (base -> First(base -> item_table[item].suffix_index)[grammar -> empty])
        {
            int symbol = grammar -> rules[base -> item_table[item].rule_number].lhs;
            Access(state_list, state_no, item); // states where RULE was introduced through closure
            for (int k = 0; k < state_list.Length(); k++)
            {
                //
                // Search for GOTO action in access-state after reducing
                // RULE to its left hand side (SYMBOL). Q points to the
                // GOTO_ELEMENT in question.
                //
                int access_state = state_list[k];
                Dfa::GotoHeader &go_to = statset[access_state].go_to;
                int i = go_to.Index(symbol);
                if (la_index[go_to[i].Laptr()] == Util::OMEGA)
                    LaTraverse(access_state, i);
                la_set[la_ptr] += la_set[go_to[i].Laptr()];
                la_index[la_ptr] = Util::Min(la_index[la_ptr], la_index[go_to[i].Laptr()]);
            }
        }
    }

    if (la_index[la_ptr] == index) // Top of a SCC
    {
        for (int i = stack.Top(); i != la_ptr; i = stack.Top())
        {
            la_set[i] = la_set[la_ptr];
            la_index[i] = Util::INFINITY_;
            stack.Pop();
        }
        la_index[la_ptr] = Util::INFINITY_;
        stack.Pop();
    }

    return;
}


//
// COMPUTE_LA takes as argument a state number (STATE_NO), an item number
// (ITEM_NO), and a set (LOOK_AHEAD).  It computes the look-ahead set of
// terminals for the given item in the given state and places the answer in
// the set LOOK_AHEAD.
//
void Pda::ComputeLa(int state_no, int item_no, BitSet &look_ahead)
{
    stack.Reset(); // empty the stack
    int lhs_symbol = grammar -> rules[base -> item_table[item_no].rule_number].lhs;
    if (lhs_symbol == grammar -> accept_image && base -> item_table[item_no].symbol == grammar -> empty)
    {
        look_ahead = base -> First(base -> item_table[item_no - 1].suffix_index);
        return;
    }

    //
    //
    //
    look_ahead.SetEmpty();
    if (grammar -> IsTerminal(base -> item_table[item_no].symbol)) // note that the symbol may be "empty"
    {
        look_ahead.AddElement(base -> item_table[item_no].symbol);
    }
    else
    {
        look_ahead = base -> NonterminalFirst(base -> item_table[item_no].symbol);
        if (look_ahead[grammar -> empty])
        {
            look_ahead.RemoveElement(grammar -> empty);
            look_ahead += base -> First(base -> item_table[item_no].suffix_index);
        }
    }

    //
    //
    //
    if (look_ahead[grammar -> empty])
    {
        Tuple<int> state_list;
        Access(state_list, state_no, item_no);
        for (int k = 0; k < state_list.Length(); k++)
        {
            //
            // If look-ahead after left hand side is not yet computed,
            // LA_TRAVERSE the graph to compute it.
            //
            int access_state = state_list[k];
            Dfa::GotoHeader &go_to = statset[access_state].go_to;
            int i = go_to.Index(lhs_symbol);
            if (la_index[go_to[i].Laptr()] == Util::OMEGA)
                LaTraverse(access_state, i);
            look_ahead += la_set[go_to[i].Laptr()];
        }
        look_ahead.RemoveElement(grammar -> empty); // empty not valid look-ahead
    }

    return;
}


//
// MKRDCTS constructs the REDUCE map and detects conflicts in the grammar ->
// When constructing an LALR parser, the subroutine COMPUTE_LA is invoked to
// compute the lalr look-ahead sets.  For an SLR parser, the FOLLOW map
// computed earlier in the procedure MKFIRST is used.
//
// For a complete description of the lookahead algorithm used in this
// program, see Charles, PhD thesis, NYU 1991.
//
void Pda::MakeReductions(void)
{
    //
    // Set up a pool of temporary space. If LALR(k), k > 1 is requested,
    // INIT_LALRK_PROCESS sets up the necessary environment for the
    // computation of multiple lookahead.
    //
    Resolve *resolve = new Resolve(control, this);

    //
    // Check whether or not the grammar is not LR(k) for any k
    // because there exist a nonterminal A such that
    //
    //                     A =>+rm A
    //
    if (option -> lalr_level > 1)
    {
        for (int symbol = grammar -> FirstNonTerminal(); (! not_lrk) && symbol <= grammar -> LastNonTerminal(); symbol++)
            not_lrk = base -> rmpself[symbol];

        for (int state_no = 1; (! not_lrk) && state_no <= num_states; state_no++)
            not_lrk = resolve -> cyclic[state_no];
    }

    //
    // RULE_COUNT is an array used to count the number of reductions on
    // particular rules within a given state.
    //
    // SYMBOL_LIST is used to construct temporary lists of terminals on
    // which reductions are defined.
    //
    // When default actions are requested, the vector SINGLE_COMPLETE_ITEM
    // is used to identify states that contain exactly one final item.
    // NOTE that when the READ_REDUCE options is turned on, the LR(0)
    // automaton constructed contains no such state.
    //
    // ACTION is an array that is used as the base for a mapping from
    // each terminal symbol into a list of actions that can be executed
    // on that symbol in a given state.
    //
    // LOOK_AHEAD is used to compute lookahead sets.
    //

    //
    // If we will be removing single productions, we need to keep
    // track of all (state, symbol) pairs on which a conflict is
    // detected. The structure conflict_symbols is used as a base
    // to construct that map. See ADD_CONFLICT_SYMBOL below.
    //
    if (option -> single_productions)
        conflict_symbols.Resize(num_states + 1);

    //
    // Iterate over the states to construct two boolean vectors.  One
    // indicates whether there is a
    // shift action on the ERROR symbol when the DEFAULT_OPT is 5.  The
    // other indicates whether it is all right to take default action in
    // states containing exactly one final item.
    //
    // We also check whether the grammar is LR(0). I.e., whether it needs
    // any look-ahead at all.
    //
    single_complete_item.Resize(num_states + 1);
    {
        for (int state_no = 1; state_no <= num_states; state_no++)
        {
            //
            //   Compute whether this state is a final state.  I.e., a state that
            // contains only a single complete item. If so, mark it as a default
            // state. Note that if the READ-REDUCE option is used, the automaton
            // will not contain such states. Also, states are marked only when
            // default actions are requested.
            //
            Node *item_ptr = statset[state_no].kernel_items;
            int item_no = item_ptr -> value;
            single_complete_item[state_no] =
                   ((! option -> read_reduce) &&
                    (! option -> single_productions) &&
                    (! option -> backtrack) &&
                    (item_ptr  -> next == NULL) &&
                    (base -> item_table[item_no].symbol == grammar -> empty));

            //
            // If a state has a complete item, and more than one kernel item
            // which is different from the complete item, then this state
            // requires look-ahead for the complete item.
            //
            if (highest_level == 0)
            {
                Node *p = statset[state_no].complete_items;
                if (p != NULL)
                {
                    if (item_ptr -> next != NULL ||
                        item_ptr -> value != p -> value)
                        highest_level = 1;
                }
            }
        }
    }

    //
    // We call COMPUTE_READ to perform the following tasks:
    // 1) Count how many elements are needed in LA_ELEMENT: LA_TOP
    // 2) Allocate space for and initialize LA_SET and LA_INDEX
    //
    if (! option -> slr)
        ComputeRead();

    //
    // Allocate space for REDUCE which will be used to map each
    // into its reduce map. We also initialize RULE_COUNT which
    // will be used to count the number of reduce actions on each
    // rule with in a given state.
    //
    Array< Tuple<int> > action(grammar -> num_terminals + 1);
    Array<int> symbol_list(grammar -> num_terminals + 1),
               rule_count(grammar -> num_rules + 1);
    rule_count.MemReset();

    //
    // If the soft_keywords option is on then we need to keep
    // track of the lookahead computed for a given rule in a
    // given state.
    //
    Array<BitSet> rule_look_ahead(grammar -> num_rules + 1);
    for (int i = 0; i < rule_look_ahead.Size(); i++)
        rule_look_ahead[i].Initialize(grammar -> num_terminals + 1);

    //
    // We iterate over the states, compute the lookahead sets,
    // resolve conflicts (if multiple lookahead is requested) and/or
    // report the conflicts if requested...
    //
    for (int state_no = 1; state_no <= num_states; state_no++)
    {
        int default_rule = Util::OMEGA,
            symbol_root = Util::NIL;
        Node *item_ptr = statset[state_no].complete_items;
        if (item_ptr != NULL)
        {
            //
            // Check if it is possible to take default reduction. The DEFAULT_OPT
            // parameter indicates what kind of default options are requested.
            // The various values it can have are:
            //
            //    a)   0 => no default reduction.
            //    b)   1 => default reduction only on adequate states. I.e.,
            //              states with only one complete item in their kernel.
            //    c)   2 => Default on all states that contain exactly one
            //              complete item not derived from an empty rule.
            //    d)   3 => Default on all states that contain exactly one
            //              complete item including items from empty rules.
            //    e)   4 => Default reduction on all states that contain exactly
            //              one item. If a state contains more than one item we
            //              take Default on the item that generated the most
            //              reductions. If there is a tie, one is selected at
            //              random.
            //    f)   5 => Same as 4 except that no default actions are computed
            //              for states that contain a shift action on the ERROR
            //              symbol.
            //
            //  In the code below, we first check for category 3.  If it is not
            // satisfied, then we check for the others. Note that in any case,
            // default reductions are never taken on the ACCEPT rule.
            //
            int item_no = item_ptr -> value,
                rule_no = base -> item_table[item_no].rule_number,
                symbol = grammar -> rules[rule_no].lhs;
            if (single_complete_item[state_no] && symbol != grammar -> accept_image)
            {
                default_rule = rule_no;
                item_ptr = NULL; // No need to check for conflicts
            }

            //
            // Iterate over all complete items in the state, build action
            // map, and check for conflicts.
            //
            BitSet look_ahead(grammar -> num_terminals + 1);
            for (; item_ptr != NULL; item_ptr = item_ptr -> next) // for each complete item, ...
            {
                int item_no = item_ptr -> value,
                    rule_no = base -> item_table[item_no].rule_number;

                if (option -> slr)  // SLR table? use Follow
                     look_ahead = base -> follow[grammar -> rules[rule_no].lhs];
                else ComputeLa(state_no, item_no, look_ahead);

//TODO: Do I need this?
//                if (option -> soft_keywords)
//                 {
//                     rule_look_ahead[rule_no] = look_ahead;
//                     if (look_ahead[grammar -> identifier_image])
//                         look_ahead += grammar -> KeywordSet();
//                 }

                for (int symbol = grammar -> FirstTerminal(); symbol <= grammar -> LastTerminal(); symbol++) // for all symbols in la set
                {
                    if (look_ahead[symbol])
                    {
                        int k = action[symbol].NextIndex();
                        action[symbol][k] = item_no;

                        if (k == 0) // first element of action[symbol]?
                        {
                            symbol_list[symbol] = symbol_root;
                            symbol_root = symbol;
                        }
                        else
                        {
                            //
                            // Always place the rule with the largest
                            // right-hand side first in the list.
                            //
                            int n = base -> item_table[action[symbol][0]].rule_number;
                            if (grammar -> RhsSize(n) >= grammar -> RhsSize(rule_no))
                            {
                                action[symbol][k] = action[symbol][0];
                                action[symbol][0] = item_no;
                            }
                        }
                    }
                }
            }

            //
            // At this stage, we have constructed the ACTION map for STATE_NO.
            // ACTION is a map from each symbol into a set of final items.
            // The rules associated with these items are the rules by which
            // to reduce when the lookahead is the symbol in question.
            // SYMBOL_LIST/SYMBOL_ROOT is a list of the non-empty elements of
            // ACTION. If the number of elements in a set ACTION(t), for some
            // terminal t, is greater than one or it is not empty and STATE_NO
            // contains a shift action on t then STATE_NO has one or more
            // conflict(s). The procedure RESOLVE_CONFLICTS takes care of
            // resolving the conflicts appropriately and returns an ACTION
            // map where each element has either 0 (if the conflicts were
            // shift-reduce conflicts, the shift is given precedence) or 1
            // element (if the conflicts were reduce-reduce conflicts, only
            // the first element in the ACTION(t) list is returned).
            //
            if (symbol_root != Util::NIL)
            {
                if (option -> soft_keywords)
                     resolve -> ResolveConflictsAndAddSoftActions(state_no, action, symbol_list, symbol_root, rule_count);
                else resolve -> ResolveConflicts(state_no, action, symbol_list, symbol_root);

                for (symbol = symbol_root; symbol != Util::NIL; symbol = symbol_list[symbol])
                {
                    if (action[symbol].Length() > 0)
                    {
                        item_no = action[symbol][0];
                        rule_count[base -> item_table[item_no].rule_number]++;
                    }
                }
            }
        }
        else if (option -> soft_keywords) // check for shift/shift conflicts on keywords
        {
            resolve -> ResolveConflictsAndAddSoftActions(state_no, action, symbol_list, symbol_root, rule_count);
        }

        //
        // We are now ready to compute the size of the reduce map for
        // STATE_NO (reduce_size) and the default rule.
        // If the state being processed contains only a single complete item
        // then the DEFAULT_RULE was previously computed and the list of
        // symbols is empty.
        // NOTE: a REDUCE_ELEMENT will be allocated for all states, even
        // those that have no reductions at all. This will facilitate the
        // Table Compression routines, for they can assume that such an
        // object exists, and can be used for Default values.
        //
        int reduce_size = 0;
        if (symbol_root != Util::NIL)
        {
            //
            // Compute REDUCE_SIZE, the number of reductions in the state and
            // DEFAULT_RULE: the rule with the highest number of reductions
            // to it.
            //
            int max_count = 0;
            for (Node *q = statset[state_no].complete_items; q != NULL; q = q -> next)
            {
                int item_no = q -> value,
                    rule_no = base -> item_table[item_no].rule_number,
                    symbol = grammar -> rules[rule_no].lhs;
                reduce_size += rule_count[rule_no];
                if (rule_count[rule_no] > max_count && symbol != grammar -> accept_image)
                {
                    max_count = rule_count[rule_no];
                    default_rule = rule_no;
                }
            }

            num_reductions += reduce_size;
        }

        //
        //   NOTE that the default fields are set for all states,
        // whether or not DEFAULT actions are requested. This is
        // all right since one can always check whether (DEFAULT > 0)
        // before using these fields.
        //
        ReduceHeader &red = statset[state_no].reduce;
        int index = red.NextIndex();
        red[index].SetSymbol(Grammar::DEFAULT_SYMBOL);
        red[index].SetRuleNumber(default_rule);
        for (int symbol = symbol_root; symbol != Util::NIL; symbol = symbol_list[symbol])
        {
            if (action[symbol].Length() > 0)
            {
                int index = red.NextIndex();
                red[index].SetSymbol(symbol);
                red[index].SetRuleNumber(base -> item_table[action[symbol][0]].rule_number);

                action[symbol].Reset();
            }
        }

        //
        // Reset RULE_COUNT elements used in this state.
        //
        for (Node *q = statset[state_no].complete_items; q != NULL; q = q -> next)
        {
            int rule_no = base -> item_table[q -> value].rule_number;
            rule_count[rule_no] = 0;
        }
    } // end for ALL_STATES

    fprintf(option -> syslis, "\n\n");

    //
    // If the automaton required multiple lookahead, construct the
    // permanent lookahead states.
    //
    if (max_la_state > num_states)
    {
        assert(resolve);
        resolve -> CreateLastats();
    }

    //
    // We are now finished with the LALR(k) construction of the
    // automaton. Clear all temporary space that was used in that
    // process and calculate the maximum lookahead level that was
    // needed.
    //
    delete resolve;

    //
    // Print informational messages and free all temporary space that
    // was used to compute lookahead information.
    //
    if (not_lrk)
    {
        Tuple<const char *> msg;
        msg.Next() = "Output file \"";
        msg.Next() = option -> grm_file;
        msg.Next() = " is not LR(K).";
        option -> EmitWarning(0, msg);

        for (int symbol = grammar -> FirstNonTerminal(); symbol <= grammar -> LastNonTerminal(); symbol++)
        {
            if (base -> rmpself[symbol])
            {
                msg.Reset();
                char tok[Control::SYMBOL_SIZE + 1];
                grammar -> RestoreSymbol(tok, grammar -> RetrieveString(symbol));

                msg.Next() = "Nonterminal ";
                msg.Next() = tok;
                msg.Next() = " is cyclic.";
                option -> EmitError(grammar -> RetrieveTokenLocation(symbol), msg);
            }
        }

        for (int state_no = 1; state_no <= num_states; state_no++)
        {
            if (resolve -> cyclic[state_no])
            {
                msg.Reset();
                IntToString state(state_no);
                msg.Next() = "State ";
                msg.Next() = state.String();
                msg.Next() = " is cyclic.";
                option -> EmitError(0, msg);
            }
        }
    }
    else
    {
        LexStream *ls = grammar -> GetLexStream();

        if (num_reduce_reduce_conflicts > 0 || num_shift_reduce_conflicts > 0)
        {
            Tuple<const char *> msg;

            msg.Next() = "Grammar is not ";
            if (! option -> slr)
            {
                if (highest_level != Util::INFINITY_)
                {
                    IntToString hl_str(highest_level);

                    msg.Next() = "LALR(";
                    msg.Next() = hl_str.String();
                    msg.Next() = ")";
                }
                else msg.Next() = " LALR(K)";
            }
            else msg.Next() = " SLR(1)";
            msg.Next() = " - it contains ";
            IntToString rr_count(num_reduce_reduce_conflicts);
            IntToString sr_count(num_shift_reduce_conflicts);
            if (num_reduce_reduce_conflicts > 0)
            {
                msg.Next() = rr_count.String();
                msg.Next() = " reduce/reduce conflicts";
                if (num_shift_reduce_conflicts > 0)
                    msg.Next() = " and ";
            }
            if (num_shift_reduce_conflicts > 0)
            {
                msg.Next() = sr_count.String();
                msg.Next() = " shift/reduce conflicts";
            }
            msg.Next() = ".";
            option -> EmitWarning(ls -> GetTokenReference(grammar -> rules[0].first_token_index), msg);
        }
        else if (! option -> quiet)
        {
            Tuple<const char *> msg;

            msg.Next() = "Grammar is ";

            if (highest_level == 0)
                msg.Next() = " LR(0).";
            else if (option -> slr)
                msg.Next() = " SLR(1).";
            else
            {
                IntToString hl_str(highest_level);

                msg.Next() = " LALR(";
                msg.Next() = hl_str.String();
                msg.Next() = ").";
            }
            option -> EmitInformative(ls -> GetTokenReference(grammar -> rules[0].first_token_index), msg);
        }
    }

    //
    // If the removal of single productions is requested, do that.
    //
    if (option -> single_productions)
    {
        Sp *sp = new Sp(control, this);
        sp -> RemoveSingleProductions();
        delete sp;
    }

    //
    // If either more than one lookahead was needed or the removal
    // of single productions was requested, the automaton was
    // transformed with the addition of new states and new
    // transitions. In such a case, we reconstruct the predecessor map.
    //
    if (highest_level > 1 || option -> single_productions)
        BuildPredecessorsMap();

    //
    // Save the conflict actions and update the automaton.
    //
    if (option -> backtrack)
        ProcessConflictActions();

    option -> FlushReport();

    return;
}


//
// PRINT_STATE prints all the items in a state.  NOTE that when single
// productions are eliminated, certain items that were added in a state by
// CLOSURE, will no longer show up in the output.  Example: If we have the
// item [A ::= .B]  in a state, and the GOTO_REDUCE generated on B has been
// replaced by say the GOTO or GOTO_REDUCE of A, the item above can no longer
// be retrieved, since transitions in a given state are reconstructed from
// the KERNEL and ADEQUATE items of the actions in the GOTO and SHIFT maps.
//
void Pda::PrintShiftState(int state_no)
{
    assert(state_no > 0 && state_no <= num_states);

    char buffer[Control::PRINT_LINE_SIZE + 1],
         line[Control::PRINT_LINE_SIZE + 1];

    //
    // ITEM_SEEN is used to construct sets of items, to help avoid
    // adding duplicates in a list.  Duplicates can occur because an
    // item from the kernel set will either be shifted on if it is not a
    // complete item, or it will be a member of the Complete_items set.
    // Duplicates can also occur because of the elimination of single
    // productions.
    //
    BitSet state_seen(max_la_state + 1, BitSet::EMPTY),
           item_seen(grammar -> num_items + 1, BitSet::EMPTY);

    int l = NumberLength(state_no) + 8; // 8 = length("STATE") + 2 spaces + newline
    FillIn(buffer, (Control::PRINT_LINE_SIZE - l), '-');
    fprintf(option -> syslis, "\n\n\nSTATE %d %s",state_no, buffer);

    //
    // Print the set of states that have transitions to STATE_NO.
    //
    strcpy(line, "( ");
    for (int k = 0; k < statset[state_no].predecessors.Length(); k++)
    {// copy list of transition symbols into array
        int previous_state = statset[state_no].predecessors[k];
        if (! state_seen[previous_state])
        {
            state_seen.AddElement(previous_state);
            if (strlen(line) + NumberLength(previous_state) > Control::PRINT_LINE_SIZE-2)
            {
                fprintf(option -> syslis,"\n%s",line);
                strcpy(line, "  ");
            }
            if (previous_state != 0)
            {
                IntToString num(previous_state);
                strcat(line, num.String());
                strcat(line, " ");
            }
        }
    }
    strcat(line, ")");
    fprintf(option -> syslis,"\n%s\n", line);

    //
    // Add the set of kernel items to the array ITEM_LIST, and mark all
    // items seen to avoid duplicates.
    //
    Tuple<int> item_list;
    for (Node *p = statset[state_no].kernel_items; p != NULL; p = p -> next)
    {
        int item_no = p -> value;
        item_list.Next() = item_no; // add to array
        item_seen.AddElement(item_no); // Mark as "seen"
    }
    int kernel_length = item_list.Length();

    //
    // Add the Complete Items to the array ITEM_LIST, and mark used.
    //
    for (Node *q = statset[state_no].complete_items; q != NULL; q = q -> next)
    {
        int item_no = q -> value;
        if (! item_seen[item_no])
        {
            item_seen.AddElement(item_no); // Mark item "seen"
            item_list.Next() = item_no;
        }
    }

    //
    // If some items were removed because they were single productions,
    // Add them here to the array ITEM_LIST, and mark them used.
    //
    for (Node *r = statset[state_no].single_production_items; r != NULL; r = r -> next)
    {
        int item_no = r -> value;
        if (! item_seen[item_no])
        {
            item_seen.AddElement(item_no); // Mark item "seen"
            item_list.Next() = item_no;
        }
    }

    //
    // Iterate over the shift map.  Shift-Reduce actions are identified
    // by a negative integer that indicates the rule in question , and
    // the associated item can be retrieved by indexing the array
    // ADEQUATE_ITEMS at the location of the rule.  For shift-actions, we
    // simply take the predecessor-items of all the items in the kernel
    // of the following state.
    // If the shift-action is a look-ahead shift, we check to see if the
    // look-ahead state contains shift actions, and retrieve the next
    // state from one of those shift actions.
    //
    Dfa::ShiftHeader &sh = Shift(state_no);
    for (int i = 0; i < sh.Length(); i++)
    {
        int next_state = sh[i].Action();
        while (next_state > num_states && next_state < max_la_state)
        {
            Dfa::ShiftHeader &next_sh = shift[statset[next_state].shift_number];
            next_state = (next_sh.Length() > 0 ? next_sh[0].Action() : 0);
        }

        Node *q;
        if (next_state > max_la_state) // If shift/reduce conflict, process the shift
        {
            q = NULL;
            for (int k = next_state - max_la_state; conflicts[k] != 0; k++)
            {
                int act = conflicts[k];
                if (act < 0)
                {
                     q = base -> adequate_item[-act];
                     break;
                }
                else if (act > grammar -> num_rules)
                {
                     act -= grammar -> num_rules;
                     q = statset[act].kernel_items;
                     if (q == NULL) // single production state?
                         q = statset[act].complete_items;
                     break;
                }
            }
        }
        else if (next_state == 0)
             q = NULL;
        else if (next_state < 0)
             q = base -> adequate_item[-next_state];
        else
        {
             q = statset[next_state].kernel_items;
             if (q == NULL) // single production state?
                 q = statset[next_state].complete_items;
        }
        for (; q != NULL; q = q -> next)
        {
            int item_no = q -> value - 1;
            if (! item_seen[item_no])
            {
                item_seen.AddElement(item_no);
                item_list.Next() = item_no;
            }
        }
    }

    //
    // GOTOS and GOTO-REDUCES are analogous to SHIFTS and SHIFT-REDUCES.
    //
    Dfa::GotoHeader &go_to = statset[state_no].go_to;
    for (int m = 0; m < go_to.Length(); m++)
    {
        Node *q;
        if (go_to[m].Action() > 0)
        {
             q = statset[go_to[m].Action()].kernel_items;
             if (q == NULL) // single production state?
                 q = statset[go_to[m].Action()].complete_items;
        }
        else q = base -> adequate_item[- go_to[m].Action()];

        for (; q != NULL; q = q -> next)
        {
            int item_no = q -> value - 1;
            if (! item_seen[item_no])
            {
                item_seen.AddElement(item_no);
                item_list.Next() = item_no;
            }
        }
    }

    //
    // Print the Kernel items.  If there are any closure items, skip a
    // line, sort then, then print them.  The kernel items are in sorted
    // order.
    //
    for (int j = 0; j < kernel_length; j++)
    {
        int item_no = item_list[j];
        PrintItem(item_no);
    }

    if (kernel_length < item_list.Length())
    {
        fprintf(option -> syslis, "\n");
        Util::QuickSort(item_list, kernel_length, item_list.Length() - 1);
        for (int k = kernel_length; k < item_list.Length(); k++)
        {
            int item_no = item_list[k];
            PrintItem(item_no);
        }
    }

    return;
}


//
//
//
void Pda::PrintLaState(int state_no)
{
    assert(state_no > num_states && state_no <= max_la_state);

    char buffer[Control::PRINT_LINE_SIZE + 1];
    int i = NumberLength(state_no) + 11; // 11 = length of "LA STATE"
                                         // + 2 spaces + newline
    FillIn(buffer, (Control::PRINT_LINE_SIZE - i), '-');
    fprintf(option -> syslis, "\n\n\nLA STATE %d %s", state_no, buffer);

    //
    // Print the set of states that have transitions to STATE_NO.
    //
    if (statset[state_no].predecessors.Length() == 0)
         fprintf(option -> syslis,"\n(Unreachable State)\n");
    else fprintf(option -> syslis,"\n(%d)\n", statset[state_no].predecessors[0]);

    return;
}


//
// PrintShifts prints all the shift actions in a given state.
//
void Pda::PrintShifts(Dfa::ShiftHeader &sh, int max_size)
{
    if (sh.Length() > 0)
    {
        fprintf(option -> syslis, "\n");
        for (int i = 0; i < sh.Length(); i++)
        {
            int symbol = sh[i].Symbol();
            char temp[Control::SYMBOL_SIZE + 1],
                 line[Control::SYMBOL_SIZE + 1];
            grammar -> RestoreSymbol(temp, grammar -> RetrieveString(symbol));
            grammar -> PrintLargeToken(line, temp, "", max_size);
            if (sh[i].Action() > max_la_state)
            {
                for (int k = sh[i].Action() - max_la_state; conflicts[k] != 0; k++)
                {
                    int act = conflicts[k];
                    if (act > grammar -> num_rules)
                    {
                        fprintf(option -> syslis,
                                "\n%-*s    Shift  %d",
                                max_size, line, act - grammar -> num_rules);
                    }
                    else if (act < 0)
                    {
                        fprintf(option -> syslis,
                                "\n%-*s    Sh/Rd  %d",
                                max_size, line, -act);
                    }
                    else
                    {
                        fprintf(option -> syslis,
                                "\n%-*s    Reduce %d",
                                max_size, line, act);
                    }
                    strcpy(line, ""); // do not reprint the symbol
                }
            }
            else
            {
                int number = Util::Abs(sh[i].Action());
                if (sh[i].Action() > num_states)
                {
                    fprintf(option -> syslis,
                            "\n%-*s    La/Sh  %d",
                            max_size, line, number);
                }
                else if (sh[i].Action() > 0)
                {
                    fprintf(option -> syslis,
                            "\n%-*s    Shift  %d",
                            max_size, line, number);
                }
                else
                {
                    fprintf(option -> syslis,
                            "\n%-*s    Sh/Rd  %d",
                            max_size, line, number);
                }
            }
        }
    }

    return;
}


//
// PrintGotos prints all the shift actions in a given state.
//
void Pda::PrintGotos(Dfa::GotoHeader &go_to, int max_size)
{
    if (go_to.Length() > 0)
    {
        fprintf(option -> syslis, "\n");
        for (int i = 0; i < go_to.Length(); i++)
        {
            int symbol = go_to[i].Symbol();
            char temp[Control::SYMBOL_SIZE + 1],
                 line[Control::SYMBOL_SIZE + 1];
            grammar -> RestoreSymbol(temp, grammar -> RetrieveString(symbol));
            grammar -> PrintLargeToken(line, temp, "", max_size);
            int number = Util::Abs(go_to[i].Action());
            if (go_to[i].Action() > 0)
            {
                fprintf(option -> syslis,
                        "\n%-*s    Goto   %d",
                        max_size, line, number);
            }
            else
            {
                fprintf(option -> syslis,
                        "\n%-*s    Gt/Rd  %d",
                        max_size, line, number);
            }
        }
    }

    return;
}


//
// PrintReductions prints all the shift actions in a given state.
//
void Pda::PrintReductions(Dfa::ReduceHeader &red, int max_size)
{
    if (red.Length() > 0)
    {
        fprintf(option -> syslis, "\n");
        for (int i = 1; i < red.Length(); i++)
        {
            int symbol = red[i].Symbol();
            char temp[Control::SYMBOL_SIZE + 1],
                 line[Control::SYMBOL_SIZE + 1];
            grammar -> RestoreSymbol(temp, grammar -> RetrieveString(symbol));
            grammar -> PrintLargeToken(line, temp, "", max_size);
            int number = red[i].RuleNumber();
            if (grammar -> rules[number].lhs != grammar -> accept_image)
            {
                fprintf(option -> syslis,
                        "\n%-*s    Reduce %d",
                        max_size, line, number);
            }
            else
            {
                fprintf(option -> syslis, "\n%-*s    Accept", max_size, line);
            }
        }
    }

    if (red[0].RuleNumber() != Util::OMEGA)
    {
        fprintf(option -> syslis,
                "\n\nDefault reduction to rule  %d",
                red[0].RuleNumber());
    }

    return;
}


//
//
//
int Pda::MaximumSymbolNameLength(int state_no)
{
    int max_size = 0;
    char temp[Control::SYMBOL_SIZE + 1];

    //
    // Compute the size of the largest symbol.  The MAX_SIZE cannot
    // be larger than PRINT_LINE_SIZE - 17 to allow for printing of
    // headers for actions to be taken on the symbols.
    //
    Dfa::ShiftHeader &sh = Shift(state_no);
    for (int i = 0; i < sh.Length(); i++)
    {
        int symbol = sh[i].Symbol();
        grammar -> RestoreSymbol(temp, grammar -> RetrieveString(symbol));
        max_size = Util::Max(max_size, strlen(temp));
    }

    Dfa::GotoHeader &go_to = statset[state_no].go_to;
    for (int j = 0; j < go_to.Length(); j++)
    {
        int symbol = go_to[j].Symbol();
        grammar -> RestoreSymbol(temp, grammar -> RetrieveString(symbol));
        max_size = Util::Max(max_size, strlen(temp));
    }

    Dfa::ReduceHeader &red = statset[state_no].reduce;
    for (int k = 1; k < red.Length(); k++)
    {
        int symbol = red[k].Symbol();
        grammar -> RestoreSymbol(temp, grammar -> RetrieveString(symbol));
        max_size = Util::Max(max_size, strlen(temp));
    }

    return Util::Min(max_size, Control::PRINT_LINE_SIZE - 17);
}


//
// Printstatates prints all the states of the parser.
//
void Pda::PrintStates(void)
{
    //
    // Update the predecessor map. Some regular states may look like
    // they are unreachable because the original transition into
    // them was changed into a shift action to a lookahead state
    // instead. In such a case, we make the lookahead state where
    // the transition to the original state will take place a
    // predecessor of the original state.
    //
    if (max_la_state > num_states)
    {
        for (int state_no = num_states + 1; state_no <= max_la_state; state_no++)
        {
            if (statset[state_no].predecessors.Length() == 1) // a reachable lookahead state
            {
                Dfa::ShiftHeader &sh = Shift(state_no);
                for (int j = 0; j < sh.Length(); j++)
                {
                    int action = sh[j].Action();
                    if (action > 0 && action <= num_states) // a shift action
                        AddPredecessor(action, state_no);
                }
            }
        }
    }

    //
    // Print the shift states
    //
    control -> PrintHeading();
    fprintf(option -> syslis,"Shift STATES: ");

    for (int state_no = 1; state_no <= num_states; state_no++) // iterate over the states
    {
        PrintShiftState(state_no);

        int max_size = MaximumSymbolNameLength(state_no);

        PrintShifts(Shift(state_no), max_size);
        PrintGotos(statset[state_no].go_to, max_size);
        PrintReductions(statset[state_no].reduce, max_size);
    }

    //
    // Print the lookahead states
    //
    if (max_la_state > num_states)
    {
        control -> PrintHeading();
        fprintf(option -> syslis, "Look-Ahead STATES:");

        for (int state_no = num_states + 1; state_no <= max_la_state; state_no++)
        {
            PrintLaState(state_no);
            if (statset[state_no].predecessors.Length() != 0) // This state is reachable
            {
                int max_size = MaximumSymbolNameLength(state_no);
                PrintShifts(Shift(state_no), max_size);
                PrintReductions(statset[state_no].reduce, max_size);
            }
        }
    }

    fprintf(option -> syslis, "\n");

    return;
}


//
// When a state contains conflicting actions, we need to
//
//  1. update the reduction count
//  2. for each symbol on which there is a shift/reduce
//     conflict, remove the shift action and update the
//     shift map of the relevant state.
//
void Pda::ProcessConflictActions()
{
    //
    // Allocate temporary space used to construct final lookahead
    // states.
    // The array shift_action will be used to construct a shift map
    // for a given state. It is initialized here to the empty map.
    // The array shift_count is used to count how many references
    // there are to each shift map.
    //
    Array<int> shift_action(grammar -> num_terminals + 1, Util::OMEGA),
               initial_shift_action(grammar -> num_terminals + 1, Util::OMEGA),
               shift_count(max_la_state + 1, 0);

    {
        for (int state_no = 1; state_no <= max_la_state; state_no++)
            shift_count[statset[state_no].shift_number]++;
    }

    //
    // We now process all states that contain conflicting actions.
    //
    Tuple<int> shift_list;
    for (int state_no = 1; state_no <= num_states; state_no++)
    {
        //
        // If any conflict action was detected in state_no,
        // Remap the shift map associated with it.
        //
        Dfa::ConflictHeader &conflict = statset[state_no].conflict;
        Tuple<int> &soft_keywords = statset[state_no].soft_keywords;
        if (conflict.Length() > 0 || soft_keywords.Length() > 0)
        {
            //
            // Copy the shift map associated with STATE_NO into the direct
            // access map SHIFT_ACTION.
            //
            int shift_no = statset[state_no].shift_number;
            Dfa::ShiftHeader &sh = shift[shift_no];
            {
                for (int i = 0; i < sh.Length(); i++)
                    initial_shift_action[sh[i].Symbol()] = sh[i].Action();
            }

            //
            // First, record the conflicting actions and remove
            // all reductions associated with a symbol on which
            // confliting actions are defined.
            //
            Dfa::ReduceHeader &red = statset[state_no].reduce;
            for (int k = 0; k < conflict.Length(); k++)
            {
                int symbol = conflict[k].Symbol(),
                    index = conflict[k].ConflictIndex();
                shift_action[symbol] = max_la_state + index;
                shift_list.Next() = symbol;

                //
                // Update the reduce count with the conflicting reduce actions.
                //
                for (int i = index; conflicts[i] != 0; i++)
                {
                    int act = conflicts[i];
                    if (act > 0 && act <= grammar -> num_rules)
                         num_reductions++;
                    else
                    {
                        if (act > grammar -> num_rules)
                            act -= grammar -> num_rules;
                        if (initial_shift_action[symbol] != act)
                        {
                            if (act < 0)
                                 num_shift_reduces++;
                            else num_shifts++;
                        }
                    }
                }

                index = red.Index(symbol);
                if (index != Util::OMEGA)
                {
                    int last_index = red.Length() - 1;
                    if (index != last_index)
                    {
                        red[index].SetSymbol(red[last_index].Symbol());
                        red[index].SetRuleNumber(red[last_index].RuleNumber());
                    }
                    red.Reset(last_index);
                    num_reductions--;
                }
            }

            //
            // Copy the shift map associated with STATE_NO into the direct
            // access map SHIFT_ACTION.
            //
            for (int i = 0; i < sh.Length(); i++)
            {
                int symbol = sh[i].Symbol();
                if (shift_action[symbol] == Util::OMEGA)
                {
                    shift_action[symbol] = sh[i].Action();
                    shift_list.Next() = symbol;
                }
            }

            //
            //
            //
            for (int j = 0; j < soft_keywords.Length(); j++)
            {
                int keyword = soft_keywords[j];

                assert(shift_action[keyword] == Util::OMEGA);
                assert(shift_action[grammar -> identifier_image] != Util::OMEGA);

                shift_action[keyword] = shift_action[grammar -> identifier_image];
                shift_list.Next() = keyword;

                if (shift_action[grammar -> identifier_image] < 0)
                     num_shift_reduces++;
                else num_shifts++;
            }

            //
            // If the initial shift map was shared by two or more states
            // then we have to construct a brand new shift map. Otherwise,
            // we reused the shift map.
            //
            if (shift_count[shift_no] > 1)
            {
                 shift_count[shift_no]--;
                 statset[state_no].shift_number = UpdateShiftMaps(shift_list, shift_action);
                 shift_count[statset[state_no].shift_number]++;
            }
            else ResetShiftMap(shift_no, shift_list, shift_action);

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
    }

    return;
}

