#include "control.h"
#include "produce.h"
#include "partition.h"

//
// This procedure computes for each state the set of non-terminal symbols
// that are required as candidates for secondary error recovery.  If the
// option NAMES=OPTIMIZED is requested, the NAME map is optimized and SYMNO
// is updated accordingly.
//
void Produce::Process(void)
{
    //
    // TOP, STACK, and INDEX are used for the digraph algorithm
    // in the routines COMPUTE_PRODUCES.
    //
    // The array PRODUCES is used to construct two maps:
    //
    // 1) PRODUCES, a mapping from each non-terminal A to the set of
    // non-terminals C such that:
    //
    //                   A  =>*  x C w
    //
    // 2) RIGHT_MOST_PRODUCES, a mapping from each non-terminal A to
    // the set of non-terminals C such that:
    //
    //                   C =>+ A x   and   x =>* %empty.
    //
    // NOTE: This is really a reverse right-most produces mapping,
    //       since given the above rule, we say that
    //       C right-most produces A.
    //
    //
    direct_produces.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
    right_produces.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
    for (int i = grammar -> FirstNonTerminal(); i <= grammar -> LastNonTerminal(); i++) // Initialize each set in PRODUCES array to Empty set.
        right_produces[i].Initialize(grammar -> num_nonterminals + 1, grammar -> num_terminals);

    //
    // Note that the elements of RIGHT_PRODUCES are automatically set to
    // the empty set when initialized. Also, all the elements of
    // DIRECT_PRODUCES are set to NULL.
    //
    for (int nt = grammar -> FirstNonTerminal(); nt <= grammar -> LastNonTerminal(); nt++)
    {
        for (int k = 0; k < base -> clitems[nt].Length(); k++)
        {
            int item_no = base -> clitems[nt][k],
                symbol = base -> item_table[item_no].symbol;
            if (grammar -> IsNonTerminal(symbol))
            {
                int i = base -> item_table[item_no].suffix_index;
                if (base -> First(i)[grammar -> empty] && (! right_produces[symbol][nt]))
                {
                    right_produces[symbol].AddElement(nt);
                    direct_produces[symbol].Next() = nt;
                }
            }
        }
    }

    //
    // Complete the construction of the RIGHT_PRODUCES map for
    // non-terminals using the digraph algorithm.
    // We make sure that each non-terminal A is not present in its own
    // PRODUCES set since we are interested in the non-reflexive
    // (positive) transitive closure.
    //
    index_of.Resize(grammar -> num_symbols + 1);
    index_of.Initialize(Util::OMEGA);
    {
        for (int nt = grammar -> FirstNonTerminal(); nt <= grammar -> LastNonTerminal(); nt++)
        {
            if (index_of[nt] == Util::OMEGA)
                compute_produces(right_produces, nt);
            right_produces[nt].RemoveElement(nt); // Not reflexive
        }
    }

    //
    // Construct the minimum subset of the domain of the GOTO map
    // needed for automatic secondary level error recovery.   For each
    // state, we start out with the set of all nonterminals on which
    // there is a transition in that state, and pare it down to a
    // subset S, by removing all nonterminals B in S such that there
    // is a goto-reduce action on B by a single production.  If the
    // READ-REDUCE option is not turned on, then, we check whether or
    // not the goto action on B is to an LR(0) reduce state.Once we have
    // our subset S, we further reduce its size as follows.  For each
    // nonterminal A in S such that there exists another nonterminal
    // B in S, where B != A,  A ->+ Bx  and  x =>* %empty, we remove A
    // from S.
    //
    // Initially, all the elements of the array NT_LIST (which is
    // indexable by the nonterminals) except for the element associated
    // with the "accept" nonterminal are set to OMEGA.
    // At the end of this process, the nonterminal elements whose
    // NT_LIST values are still OMEGA are precisely the nonterminal
    // symbols that will never be used as candidates.
    //
    //
    assert(gd_index.Length() == 0);
    assert(gd_range.Length() == 0);
    gd_index.Next() = 0; // There is no state 0
    BitSetWithOffset set(grammar -> num_nonterminals + 1, grammar -> num_terminals);
    BoundedArray<bool> useful(grammar -> num_terminals + 1, grammar -> num_symbols, false);
    for (int state_no = 1; state_no <= num_states; state_no++)
    {
        Tuple<int> nt_set;
        assert (gd_index.Length() == state_no);
        gd_index.Next() = gd_range.Length();

        set.SetEmpty();

        Dfa::GotoHeader &go_to = statset[state_no].go_to;
        for (int i = 0; i < go_to.Length(); i++)
        {
            int symbol = go_to[i].Symbol(),
                state = go_to[i].Action(),
                rule_no;
            if (state < 0)
                rule_no = -state;
            else
            {
                Node *q = statset[state].kernel_items;
                int item_no = q -> value;
                if (q -> next != NULL)
                     rule_no = 0;
                else rule_no = base -> item_table[item_no].rule_number;
            }
            if (rule_no == 0 || grammar -> RhsSize(rule_no) != 1)
            {
                nt_set.Next() = symbol;
                set += right_produces[symbol];
                useful[symbol] = true; // mark element "symbol" as useful.
            }
        }

        for (int k = 0; k < nt_set.Length(); k++)
        {
            int symbol = nt_set[k];
            if (! set[symbol])
                gd_range.Next() = symbol;
        }
    }
    assert (gd_index.Length() == num_states + 1);
    gd_index.Next() = gd_range.Length(); // add a gate!!!

    //
    // Remove names assigned to nonterminals that are never used as
    // error candidates.
    //
    if (option -> names == Option::OPTIMIZED)
    {
        //
        // In addition to nonterminals that are never used as candidates,
        // if a nullable nonterminal was assigned a name by default
        // (nonterminals that were "named" by default are identified
        // with negative indices), that name is also removed.
        //
        for (int symbol = grammar -> FirstNonTerminal(); symbol <= grammar -> LastNonTerminal(); symbol++)
        {
            if (! useful[symbol])
                grammar -> symbol_index[symbol].external_name_index = grammar -> symbol_index[grammar -> accept_image].external_name_index;
            else if (grammar -> symbol_index[symbol].external_name_index < 0)
            {
                if (base -> IsNullable(symbol))
                    grammar -> symbol_index[symbol].external_name_index = grammar -> symbol_index[grammar -> accept_image].external_name_index;
                else
                    grammar -> symbol_index[symbol].external_name_index = - grammar -> symbol_index[symbol].external_name_index;
            }
        }

        //
        // Adjust name map to remove unused elements and update SYMNO map.
        //
        //
        // First, compute the set of names that are useful
        //
        Array<bool> name_used(grammar -> num_names + 1, false);
        {
            for (int symbol = 1; symbol <= grammar -> num_symbols; symbol++)
                name_used[grammar -> symbol_index[symbol].external_name_index] = true;
        }

        //
        // Next, construct a new name map that only contains the useful names
        //
        Tuple<char *> new_names;
        Array<int> names_map(grammar -> num_names + 1);
        for (int i = 0; i <= grammar -> num_names; i++)
        {
            if (name_used[i])
            {
                names_map[i] = new_names.Length();
                new_names.Next() = grammar -> name[i];
            }
        }

        //
        // Next, save the new map permanently
        //
        grammar -> num_names = new_names.Length() - 1;
        for (int k = 0; k <= grammar -> num_names; k++)
            grammar -> name[k] = new_names[k];

        //
        // Finally, remap each symbol to point to its new name.
        //
        {
            for (int symbol = 1; symbol <= grammar -> num_symbols; symbol++)
                grammar -> symbol_index[symbol].external_name_index = names_map[grammar -> symbol_index[symbol].external_name_index];
        }
    }

    //
    // If the option LIST_BIT is ON, print the name map.
    //
    if (option -> list)
    {
        control -> PrintHeading();
        fprintf(option -> syslis, "\nName map:\n");

        {
            for (int symbol = 1; symbol <= grammar -> num_symbols; symbol++)
                if (grammar -> symbol_index[symbol].external_name_index != grammar -> symbol_index[grammar -> accept_image].external_name_index)
                print_name_map(symbol);
        }

        {
            for (int symbol = 1; symbol <= grammar -> num_symbols; symbol++)
                if (symbol != grammar -> accept_image &&
                    grammar -> symbol_index[symbol].external_name_index == grammar -> symbol_index[grammar -> accept_image].external_name_index)
                    print_name_map(symbol);
        }
    }

    if (option -> scopes)
        process_scopes();

    return;
}


//
// This procedure is used to compute the transitive closure of
// the PRODUCES, LEFT_PRODUCES and RIGHT_PRODUCES maps.
//
void Produce::compute_produces(BoundedArray<BitSetWithOffset> &produces, int symbol)
{
    stack.Push(symbol);
    int index = stack.Length();
    index_of[symbol] = index;

    for (int i = 0; i < direct_produces[symbol].Length(); i++)
    {
        int new_symbol = direct_produces[symbol][i];
        if (index_of[new_symbol] == Util::OMEGA)  // first time seen?
            compute_produces(produces, new_symbol);
        index_of[symbol] = Util::Min(index_of[symbol], index_of[new_symbol]);
        produces[symbol] += produces[new_symbol];
    }

    if (index_of[symbol] == index)  // symbol is SCC root
    {
        for (int new_symbol = stack.Top(); new_symbol != symbol; new_symbol = stack.Top())
        {
            produces[new_symbol] = produces[symbol];
            index_of[new_symbol] = Util::INFINITY_;
            stack.Pop();
        }

        index_of[symbol] = Util::INFINITY_;
        stack.Pop();
    }

    return;
}



//
// This procedure prints the name associated with a given symbol.
// The same format that was used in the procedure DISPLAY_INPUT
// to print aliases is used to print name mappings.
//
void Produce::print_name_map(int symbol)
{
    char line[Control::PRINT_LINE_SIZE],
         tok[Control::SYMBOL_SIZE + 1];

    grammar -> RestoreSymbol(tok, grammar -> RetrieveString(symbol));

    int len = Control::PRINT_LINE_SIZE - 5;
    grammar -> PrintLargeToken(line, tok, "", len);
    strcat(line, " ::= ");
    grammar -> RestoreSymbol(tok, grammar -> RetrieveName(grammar -> symbol_index[symbol].external_name_index));
    if (strlen(line) + strlen(tok) > Control::PRINT_LINE_SIZE - 1)
    {
        fprintf(option -> syslis, "\n%s", line);
        len = Control::PRINT_LINE_SIZE - 4;
        grammar -> PrintLargeToken(line, tok, "    ", len);
    }
    else strcat(line, tok);

    fprintf(option -> syslis, "\n%s", line);

    return;
}


//
// Compute set of "scopes" and use it to construct SCOPE map.
//
void Produce::process_scopes(void)
{
    //
    // Each element of state_index points to the start of a list in scope_state. Element zero of 
    // the scope_state array is initialized to 0, indicating a null list. Initially, all elements
    // of state_index point to this null list. Later, all reachable nonterminals will be reassigned
    // to their respective list in scope_state. However, if the grammar contains unreachable nonterminals
    // they will not be reassigned to a new list and will instead continue to point to the default null list.
    //
    BoundedArray<int> state_index(grammar -> num_terminals + 1, grammar -> num_symbols, 0);

    BoundedArray<Tuple <int> > states_of(grammar -> num_terminals + 1, grammar -> num_symbols);
    Array<int> prefix_index(grammar -> num_items + 1),
               suffix_index(grammar -> num_items + 1);

    next_item.Resize(grammar -> num_items + 1);
    symbol_seen.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);

    //
    // Make the RIGHT_PRODUCES map reflexive.  Recall that
    // RIGHT_PRODUCES is a mapping from each nonterminal B into the set
    // of nonterminals A such that:
    //
    //    A =>rm* B
    //
    // Next, initialize and construct the LEFT_PRODUCES map. Initially,
    // all the sets in LEFT_PRODUCES are set to the empty map.
    // LEFT_PRODUCES is a mapping  from each nonterminal A into the set
    // of nonterminals B such that:
    //
    //    A =>lm* B x
    //
    // for some arbitrary string x.
    //
    // Since A ->* A for all A,  we insert A in PRODUCES(A)  (but not
    // in the linked list).
    //
    //
    left_produces.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
    for (int nt = grammar -> FirstNonTerminal(); nt <= grammar -> LastNonTerminal(); nt++)
    {
        right_produces[nt].AddElement(nt); // make reflexive

        left_produces[nt].Initialize(grammar -> num_nonterminals + 1, grammar -> num_terminals);
        left_produces[nt].AddElement(nt); // reflexive
        direct_produces[nt].Reset(); // reset the list (tuple) to empty

        for (int k = 0; k < base -> clitems[nt].Length(); k++)
        {
            for (int item_no = base -> clitems[nt][k];
                 grammar -> IsNonTerminal(base -> item_table[item_no].symbol);
                 item_no++)
            {
                int symbol = base -> item_table[item_no].symbol;
                if (! left_produces[nt][symbol])
                {
                    left_produces[nt].AddElement(symbol);
                    direct_produces[nt].Next() = symbol;
                }
                if (! base -> IsNullable(symbol))
                    break;
            }
        }
    }

    //
    // Complete the construction of the LEFT_produces map for
    // non_terminals using the digraph algorithm.
    //
    index_of.Initialize(Util::OMEGA);
    {
        for (int nt = grammar -> FirstNonTerminal(); nt <= grammar -> LastNonTerminal(); nt++)
            if (index_of[nt] == Util::OMEGA)
                compute_produces(left_produces, nt);
    }

    //
    // Allocate and initialize the PRODUCES array to construct the
    // PRODUCES map.  After allocation, CALLOC sets all sets to empty.
    // Since A ->* A for all A,  we insert A in PRODUCES(A)  (but not
    // in the linked list).
    //
    produces.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
    {
        for (int nt = grammar -> FirstNonTerminal(); nt <= grammar -> LastNonTerminal(); nt++)
        {
            produces[nt].Initialize(grammar -> num_nonterminals + 1, grammar -> num_terminals);
            produces[nt].AddElement(nt); // reflexive
            direct_produces[nt].Reset(); // reset the list (tuple) to empty

            for (int k = 0; k < base -> clitems[nt].Length(); k++)
            {
                for (int item_no = base -> clitems[nt][k];  base -> item_table[item_no].symbol != grammar -> empty; item_no++)
                {
                    int symbol = base -> item_table[item_no].symbol;
                    if (grammar -> IsNonTerminal(symbol))
                    {
                        if (! produces[nt][symbol])
                        {
                            produces[nt].AddElement(symbol);
                            direct_produces[nt].Next() = symbol;
                        }
                    }
                }
            }
        }
    }

    //
    // Complete the construction of the PRODUCES map for
    // non_terminals using the digraph algorithm.
    //
    // Since $ACC =>* x A y for all nonterminal A in the grammar, a
    // single call to COMPUTE_PRODUCES does the trick.
    //
    index_of.Initialize(Util::OMEGA);
    compute_produces(produces, grammar -> accept_image);

    //
    // Construct a mapping from each non_terminal A into the set of
    // items of the form [B  ->  x . A y].
    //
    item_of.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
    item_of.Initialize(Util::NIL);
    {
        for (int item_no = 1; item_no <= grammar -> num_items; item_no++)
        {
            int dot_symbol = base -> item_table[item_no].symbol;
            if (grammar -> IsNonTerminal(dot_symbol))
            {
                next_item[item_no] = item_of[dot_symbol];
                item_of[dot_symbol] = item_no;
            }
        }
    }

    //
    // Construct a list of scoped items in ITEM_LIST.
    // Scoped items are derived from rules of the form  A -> x B y such
    // that B =>* w A z, %empty not in FIRST(y), and it is not the case
    // that x = %empty and B ->* A v.
    // Scoped items may also be identified by the user, using a "recovery"
    // symbol as a marker in a production.
    // As scoped items are added to the list, we keep track of the
    // longest prefix encountered.  This is subsequently used to
    // bucket sort the scoped items in descending order of the length
    // of their prefixes.
    //
    item_list.Resize(grammar -> num_items + 1);
    item_list.Initialize(Util::OMEGA);
    int item_root = Util::NIL,
        max_prefix_length = 0;
    for (int item_no = 1; item_no <= grammar -> num_items; item_no++)
    {
        int dot_symbol = base -> item_table[item_no].symbol;

        //
        // A user-specified recovery item is automatically recognized as a
        // scope. Note that if the item is improper (in that its prefix is
        // nullable, we've already issued a warning for it.
        //
        if (grammar -> IsRecover(dot_symbol))
        {
            if (! is_item_prefix_nullable(item_no))
            {
                if (item_list[item_no] == Util::OMEGA)
                {
                    item_list[item_no] = item_root;
                    item_root = item_no;
                    max_prefix_length = Util::Max(max_prefix_length,
                                                  base -> item_table[item_no].dot);
                }
            }
            else
            {
                char tok[Control::SYMBOL_SIZE + 1];
                grammar -> RestoreSymbol(tok, grammar -> RetrieveString(dot_symbol));

                int rule_no = base -> item_table[item_no].rule_number,
                    error_location = control -> lex_stream -> StartLocation(grammar -> rules[rule_no].first_token_index);
                InputFileSymbol *input_file_symbol = control -> lex_stream -> GetFileSymbol(grammar -> rules[rule_no].first_token_index);
                int error_token_index = control -> lex_stream -> GetNextToken(input_file_symbol, error_location);
                Token *current_token = control -> lex_stream -> GetTokenReference(error_token_index);
                current_token -> SetEndLocation(control -> lex_stream -> EndLocation(grammar -> rules[rule_no].last_token_index));
                current_token -> SetKind(0);

                Tuple<const char *> msg;
                msg.Next() = "This rule is not processed as a scope because the prefix preceding the recovery symbol \"";
                msg.Next() = tok;
                msg.Next() = "\" is nullable";
                option -> EmitInformative(error_token_index, msg);
            }
        }
        else if (grammar -> IsNonTerminal(dot_symbol))
        {
            //
            // Check whether or not item_no is a scope. If so, add it to the
            // item_list of scopes.
            //
            int symbol = grammar -> rules[base -> item_table[item_no].rule_number].lhs;
            if (! base -> First(base -> item_table[item_no].suffix_index)[grammar -> empty] &&
                  produces[dot_symbol][symbol])
            {
                if (is_scope(item_no))
                {
                    int i;
                    for (i = item_no + 1; ;i++)
                    {
                        symbol = base -> item_table[i].symbol;
                        if (grammar -> IsTerminal(symbol))
                            break;
                        if (! base -> IsNullable(symbol))
                            break;
                    }
                    if (grammar -> IsNonTerminal(symbol))
                    {
                        symbol_seen.MemReset();
                        symbol = get_shift_symbol(symbol);
                    }

                    if (symbol != grammar -> empty && item_list[i] == Util::OMEGA)
                    {
                        item_list[i] = item_root;
                        item_root = i;
                        max_prefix_length = Util::Max(max_prefix_length,
                                                      base -> item_table[i].dot);
                    }
                }
            }
        }
    }

    //
    // In this loop, the prefix and suffix string for each scope in
    // entered into a table.  We also use the SYMBOL_SEEN array to
    // identify the set of left-hand side symbols associated with the
    // scopes.
    //
    scope_table.Resize(SCOPE_SIZE);
    scope_table.Initialize(Util::NIL);
    symbol_seen.MemReset();
    int num_scopes = 0;
    {
        for (int item_no = item_root; item_no != Util::NIL; item_no = item_list[item_no])
        {
            int rule_no = base -> item_table[item_no].rule_number,
                symbol = grammar -> rules[rule_no].lhs;
            num_scopes = num_scopes + 1;

            symbol_seen[symbol] = true;

            prefix_index[item_no] = insert_prefix(item_no);
            suffix_index[item_no] = insert_suffix(item_no);
        }
    }

    //
    // We now construct a mapping from each nonterminal symbol that is
    // the left-hand side of a rule containing scopes into the set of
    // states that has a transition on the nonterminal in question.
    //
    BoundedArray<int> nt_list(grammar -> num_terminals + 1, grammar -> num_symbols);
    int nt_root = Util::NIL;
    int num_state_sets = 0;
    for (int state_no = 1; state_no <= num_states; state_no++)
    {
        Dfa::GotoHeader &go_to = statset[state_no].go_to;
        for (int i = 0; i < go_to.Length(); i++)
        {
            int symbol = go_to[i].Symbol();
            if (symbol_seen[symbol])
            {
                if (states_of[symbol].Length() == 0)
                {
                    nt_list[symbol] = nt_root;
                    nt_root = symbol;
                    num_state_sets = num_state_sets + 1;
                }
                states_of[symbol].Next() = state_no;
            }
        }
    }

    //
    // Next, we used the optimal partition procedure to compress the
    // space used by the sets of states, allocate the SCOPE structure
    // and store the compressed sets of states in it.
    // We also sort the list of items by the length of their prefixes in
    // descending order.  This is done primarily as an optimization.
    // If a longer prefix matches prior to a shorter one, the parsing
    // will terminate quicker.
    //
    // process_scope_states:
    //
    {
        Array<BitSet> collection(num_states + 1);
        {
            for (int i = 1; i <= num_states; i++)
                collection[i].Initialize(num_states + 1);
        }

        Array<int> element_size(num_state_sets + 1),
                   list(num_state_sets + 1),
                   start(num_state_sets + 2),
                   stack(num_state_sets + 1),
                   ordered_symbol(num_state_sets + 1),
                   bucket(max_prefix_length + 1);

        for (int symbol = nt_root, j = 1;
             symbol != Util::NIL; symbol = nt_list[symbol], j++)
        {
            list[j] = j;
            ordered_symbol[j] = symbol;
            collection[j].SetEmpty();
            element_size[j] = 0;
            for (int k = 0; k < states_of[symbol].Length(); k++)
            {
                element_size[j]++;
                collection[j].AddElement(states_of[symbol][k]);
            }
        }

        Partition<BitSet>::partset(collection,
                                   num_state_sets,
                                   element_size,
                                   list,
                                   start,
                                   stack,
                                   num_states);

        {
            for (int i = 1; i <= num_state_sets; i++)
                state_index[ordered_symbol[i]] = Util::Abs(start[i]);
        }

        scope_state.Resize(num_state_sets == 0 ? 1 : start[num_state_sets + 1]);
        scope_state[0] = 0;
        Array<int> state_list(num_states + 1, Util::OMEGA);
        int k = 0;
        for (int i = 1; i <= num_state_sets; i++)
        {
            if (start[i] > 0)
            {
                int state_root = 0;
                state_list[state_root] = Util::NIL;
                int j;
                for (bool end_node = ((j = i) == Util::NIL);
                     ! end_node; end_node = (j == i))
                {
                    j = stack[j];
                    int symbol = ordered_symbol[j];
                    for (int l = 0; l < states_of[symbol].Length(); l++)
                    {
                        int state_no = states_of[symbol][l];
                        if (state_list[state_no] == Util::OMEGA)
                        {
                            state_list[state_no] = state_root;
                            state_root = state_no;
                        }
                    }
                }

                for (int state_no = state_root;
                     state_no != Util::NIL; state_no = state_root)
                {
                    state_root = state_list[state_no];
                    state_list[state_no] = Util::OMEGA;
                    k++;
                    scope_state[k] = state_no;
                }
            }
        }

        //
        // Use the BUCKET array as a base to partition the scoped items
        // based on the length of their prefixes.  The list of items in each
        // bucket is kept in the NEXT_ITEM array sorted in descending order
        // of the length of the right-hand side of the item.
        // Items are kept sorted in that fashion because when two items have
        // the same prefix, we want the one with the shortest suffix to be
        // chosen. In other words, if we have two scoped items, say:
        //
        //    A ::= x . y       and      B ::= x . z     where |y| < |z|
        //
        // and both of them are applicable in a given context with similar
        // result, then we always want A ::= x . y to be used.
        //
        bucket.Initialize(Util::NIL);
        for (int item_no = item_root; item_no != Util::NIL; item_no = item_list[item_no])
        {
            int tail = Util::NIL;

            k = base -> item_table[item_no].dot;
            int i;
            for (i = bucket[k]; i != Util::NIL; tail = i, i = next_item[i])
            {
                if (grammar -> RhsSize(base -> item_table[item_no].rule_number) >=
                    grammar -> RhsSize(base -> item_table[i].rule_number))
                   break;
            }

            next_item[item_no] = i;
            if (i == bucket[k])
                 bucket[k] = item_no;       // insert at the beginning
            else next_item[tail] = item_no; // insert in middle or end
        }

        //
        // Reconstruct list of scoped items in sorted order. Since we want
        // the items in descending order, we start with the smallest bucket
        // proceeding to the largest one and insert the items from each
        // bucket in LIFO order in ITEM_LIST.
        //
        item_root = Util::NIL;
        {
            for (int k = 1; k <= max_prefix_length; k++)
                for (int item_no = bucket[k]; item_no != Util::NIL; item_no = next_item[item_no])
                {
                    item_list[item_no] = item_root;
                    item_root = item_no;
                }
        }
    } // End PROCESS_SCOPE_STATES

    //
    // Next, we initialize the remaining fields of the SCOPE structure.
    //
    scope_prefix.Resize(num_scopes);
    scope_suffix.Resize(num_scopes);
    scope_lhs_symbol.Resize(num_scopes);
    scope_look_ahead.Resize(num_scopes);
    scope_state_set.Resize(num_scopes);
    {
        for (int i = 0, item_no = item_root; item_no != Util::NIL; i++, item_no = item_list[item_no])
        {
            scope_prefix[i] = prefix_index[item_no];
            scope_suffix[i] = suffix_index[item_no];
            int rule_no = base -> item_table[item_no].rule_number;
            scope_lhs_symbol[i] = grammar -> rules[rule_no].lhs;
            int symbol = grammar -> rhs_sym[grammar -> rules[rule_no].rhs_index + base -> item_table[item_no].dot];
            if (grammar -> IsTerminal(symbol))
                scope_look_ahead[i] = symbol;
            else
            {
                symbol_seen.MemReset();
                scope_look_ahead[i] = get_shift_symbol(symbol);
            }
            scope_state_set[i] = state_index[scope_lhs_symbol[i]];
        }
    }

    scope_right_side.Resize(scope_element.Length() == 0
                                 ? 1
                                 : scope_element[scope_element.Length() - 1].index +
                                   scope_element[scope_element.Length() - 1].size);
    scope_right_side[0] = 0;
    int scope_index = 1;
    for (int j = 0; j < scope_element.Length(); j++)
    {
        assert(scope_index == scope_element[j].index);

        if (scope_element[j].item < 0)
        {
            int item_no = -scope_element[j].item,
                rule_no = base -> item_table[item_no].rule_number;
            for (int k = grammar -> rules[rule_no].rhs_index+base -> item_table[item_no].dot - 1;
                 k >= grammar -> rules[rule_no].rhs_index; // symbols before dot
                 k--)
                scope_right_side[scope_index++] = grammar -> rhs_sym[k];
        }
        else
        {
            int item_no = scope_element[j].item,
                rule_no = base -> item_table[item_no].rule_number;
            for (int k = grammar -> rules[rule_no].rhs_index + base -> item_table[item_no].dot;
                 k < grammar -> rules[rule_no + 1].rhs_index; // symbols after dot
                 k++)
            {
                scope_right_side[scope_index++] = grammar -> rhs_sym[k];
                //
                // TODO(1): REMOVE THIS!
                //
                // This space optimization has been removed because although
                // it helps provide clearer diagnosis, it prevents recovery
                // when Ast nodes must be constructed during recovery.
                //
                // int symbol = grammar -> rhs_sym[k];
                // if (grammar -> IsNonTerminal(symbol))
                // {
                //     if (! base -> IsNullable(symbol))
                //         scope_right_side[scope_index++] = grammar -> rhs_sym[k];
                // }
                // else if (symbol != grammar -> error_image)
                //     scope_right_side[scope_index++] = grammar -> rhs_sym[k];
                //
            }
        }
        scope_right_side[scope_index++] = 0;
    }

    if (option -> list)
        print_scopes();

    return;
}


//
// Given an item of the form [A  ->  w B x]. This function
// computes whether or not the prefix w is nullable.
//
bool Produce::is_item_prefix_nullable(int item_no)
{
    for (int i = item_no - base -> item_table[item_no].dot; i < item_no; i++)
    {
        int symbol = base -> item_table[i].symbol;
        if (grammar -> IsTerminal(symbol))
            return false;
        if (! base -> IsNullable(symbol))
            return false;
    }
    return true;
}

//
// This procedure checks whether or not an item [A  ->  w B x] is a
// valid scope. The item in question is known to satisfy the following
// conditions: 
//     . B ->* y A z
//     . it is not the case that x =>* %empty
//
// Such an item is a valid scope if, in addition, it satisfies  the
// following conditions:
//
// 1) either it is not the case that w =>* %empty or it is not the
//    case that B =>lm* A.
// 2) it is not the case that whenever A is introduced through
//    closure, it is introduced by a nonterminal C where C =>rm* A
//    and C =>rm+ B.
//
bool Produce::is_scope(int item_no)
{
    // 
    // If the prefix of item_no is not nullable
    // 
    if (! is_item_prefix_nullable(item_no))
        return true;

    int lhs_symbol = grammar -> rules[base -> item_table[item_no].rule_number].lhs,
        target = base -> item_table[item_no].symbol;
    if (left_produces[target][lhs_symbol])
        return(false);

    if (base -> item_table[item_no].dot > 0)
        return(true);

    symbol_seen.MemReset();

    return(scope_check(lhs_symbol, target, lhs_symbol));
}


//
// Given a nonterminal LHS_SYMBOL and a nonterminal TARGET where,
//
//                     LHS_SYMBOL ::= TARGET x
//
// find out if whenever LHS_SYMBOL is introduced through closure, it
// is introduced by a nonterminal SOURCE such that
//
//                     SOURCE ->rm* LHS_SYMBOL
//
//                               and
//
//                     SOURCE ->rm+ TARGET
//
//
bool Produce::scope_check(int lhs_symbol, int target, int source)
{
    symbol_seen[source] = true;

    if (right_produces[target][source] &&
        right_produces[lhs_symbol][source])
        return(false);

    for (int item_no = item_of[source];
         item_no != Util::NIL;
         item_no = next_item[item_no])
    {
        if (base -> item_table[item_no].dot != 0)
            return(true);

        int rule_no = base -> item_table[item_no].rule_number,
            symbol = grammar -> rules[rule_no].lhs;
        if (! symbol_seen[symbol])        // not yet processed
        {
            if (scope_check(lhs_symbol, target, symbol))
                return(true);
        }
    }

    return(false);
}


//
// This procedure takes as argument an item and inserts the string
// prefix of the item preceeding the "dot" into the scope table, if
// that string is not already there.  In any case, the index  number
// associated with the prefix in question is returned.
// NOTE that since both prefixes and suffixes are entered in the
// table, the prefix of a given item, ITEM_NO, is encoded as
// -ITEM_NO, whereas the suffix of that item is encoded as +ITEM_NO.
//
int Produce::insert_prefix(int item_no)
{
    unsigned hash_address = 0;

    int rule_no = base -> item_table[item_no].rule_number;
    for (int i = grammar -> rules[rule_no].rhs_index;    // symbols before dot
         i < grammar -> rules[rule_no].rhs_index + base -> item_table[item_no].dot;
         i++)
        hash_address += grammar -> rhs_sym[i];

    int k = hash_address % SCOPE_SIZE;

    for (int j = scope_table[k]; j != Util::NIL; j = scope_element[j].link)
    {
        if (is_prefix_equal(scope_element[j].item, item_no))
            return(scope_element[j].index);
    }
    int scope_index = scope_element.NextIndex();
    scope_element[scope_index].item = -item_no;
    scope_element[scope_index].index =
         (scope_index == 0
                       ? 1
                       : scope_element[scope_index - 1].index +
                         scope_element[scope_index - 1].size);
    scope_element[scope_index].size = (base -> item_table[item_no].dot + 1);
    scope_element[scope_index].link = scope_table[k];
    scope_table[k] = scope_index;

    return(scope_element[scope_index].index);
}


//
// This boolean function takes two items as arguments and checks
// whether or not they have the same prefix.
//
bool Produce::is_prefix_equal(int item_no, int item_no2)
{
    if (item_no > 0)    // a suffix
        return(false);

    int item_no1 = -item_no;
    if (base -> item_table[item_no1].dot != base -> item_table[item_no2].dot)
        return(false);

    int j = grammar -> rules[base -> item_table[item_no1].rule_number].rhs_index,
        start = grammar -> rules[base -> item_table[item_no2].rule_number].rhs_index,
        dot = start + base -> item_table[item_no2].dot - 1;
    for (int i = start; i <= dot; i++) // symbols before dot
    {
        if (grammar -> rhs_sym[i] != grammar -> rhs_sym[j])
            return(false);
        j++;
    }

    return(true);
}


//
// This procedure is analoguous to INSERT_PREFIX.  It takes as
// argument an item, and inserts the suffix string following the dot
// in the item into the scope table, if it is not already there.
// In any case, it returns the index associated with the suffix.
// When inserting a suffix into the table, all nullable nonterminals
// in the suffix are disregarded.
//
int Produce::insert_suffix(int item_no)
{
    unsigned hash_address = 0;

    int rule_no = base -> item_table[item_no].rule_number,
        num_elements = 0;
    for (int i = grammar -> rules[rule_no].rhs_index + base -> item_table[item_no].dot;
         i < grammar -> rules[rule_no + 1].rhs_index; // symbols after dot
         i++)
    {
        hash_address += grammar -> rhs_sym[i];
        num_elements++;
        //
        // TODO(1): REMOVE THIS!
        //
        // This space optimization has been removed because although
        // it helps provide clearer diagnosis, it prevents recovery
        // when Ast nodes must be constructed during recovery.
        //
        // if (grammar -> IsNonTerminal(grammar -> rhs_sym[i]))
        // {
        //     if (! base -> IsNullable(grammar -> rhs_sym[i]))
        //     {
        //         hash_address += grammar -> rhs_sym[i];
        //         num_elements++;
        //     }
        // }
        // else if (grammar -> rhs_sym[i] != grammar -> error_image)
        // {
        //     hash_address += grammar -> rhs_sym[i];
        //     num_elements++;
        // }
        //
    }

    int k = hash_address % SCOPE_SIZE;

    for (int j = scope_table[k]; j != Util::NIL; j = scope_element[j].link)
    {
        if (is_suffix_equal(scope_element[j].item, item_no))
            return(scope_element[j].index);
    }

    int scope_index = scope_element.NextIndex();
    scope_element[scope_index].item = item_no;
    scope_element[scope_index].index =
         (scope_index == 0
                       ? 1
                       : scope_element[scope_index - 1].index +
                         scope_element[scope_index - 1].size);
    scope_element[scope_index].size = num_elements + 1;
    scope_element[scope_index].link = scope_table[k];
    scope_table[k] = scope_index;

    return(scope_element[scope_index].index);
}


//
// This boolean function takes two items as arguments and checks
// whether or not they have the same suffix.
//
bool Produce::is_suffix_equal(int item_no1, int item_no2)
{
    if (item_no1 < 0) // a prefix
        return(false);

    int rule_no1 = base -> item_table[item_no1].rule_number,
        i = grammar -> rules[rule_no1].rhs_index + base -> item_table[item_no1].dot,
        dot1 = grammar -> rules[rule_no1 + 1].rhs_index - 1,
        rule_no2 = base -> item_table[item_no2].rule_number,
        j = grammar -> rules[rule_no2].rhs_index + base -> item_table[item_no2].dot,
        dot2 = grammar -> rules[rule_no2 + 1].rhs_index - 1;
    while (i <= dot1 && j <= dot2) // non-nullable syms before dot
    {
        //
        // TODO(1): REMOVE THIS!
        //
        // This space optimization has been removed because although
        // it helps provide clearer diagnosis, it prevents recovery
        // when Ast nodes must be constructed during recovery.
        //
        // if (grammar -> IsNonTerminal(grammar -> rhs_sym[i]))
        // {
        //     if (base -> IsNullable(grammar -> rhs_sym[i]))
        //     {
        //         i++;
        //         continue;
        //     }
        // }
        // else if (grammar -> rhs_sym[i] == grammar -> error_image)
        // {
        //     i++;
        //     continue;
        // }
        //
        // if (grammar -> IsNonTerminal(grammar -> rhs_sym[j]))
        // {
        //     if (base -> IsNullable(grammar -> rhs_sym[j]))
        //     {
        //         j++;
        //         continue;
        //     }
        // }
        // else if (grammar -> rhs_sym[j] == grammar -> error_image)
        // {
        //     j++;
        //     continue;
        // }
        //
        //  if (grammar -> rhs_sym[i] != grammar -> rhs_sym[j])
        //     return(false);
        //
        // j++;
        // i++;
        //
        if (grammar -> rhs_sym[i] != grammar -> rhs_sym[j])
            return(false);

        j++;
        i++;
    }

    //
    // TODO(1): REMOVE THIS!
    //
    // This space optimization has been removed because although
    // it helps provide clearer diagnosis, it prevents recovery
    // when Ast nodes must be constructed during recovery.
    //
    // for (; i <= dot1; i++)
    // {
    //     if (grammar -> IsNonTerminal(grammar -> rhs_sym[i]))
    //     {
    //         if (! base -> IsNullable(grammar -> rhs_sym[i]))
    //             return(false);
    //     }
    //     else if (grammar -> rhs_sym[i] != grammar -> error_image)
    //         return(false);
    // }
    //
    // for (; j <= dot2; j++)
    // {
    //     if (grammar -> IsNonTerminal(grammar -> rhs_sym[j]))
    //     {
    //         if (! base -> IsNullable(grammar -> rhs_sym[j]))
    //             return(false);
    //     }
    //     else if (grammar -> rhs_sym[j] != grammar -> error_image)
    //         return(false);
    // }
    //  return(true);
    //

    return (i > dot1 && j > dot2);
}


//
// This procedure is similar to the global procedure PrintItem.
//
void Produce::print_scopes(void)
{
    char line[Control::PRINT_LINE_SIZE + 1],
         tok[Control::SYMBOL_SIZE + 1],
         tmp[Control::PRINT_LINE_SIZE];

    control -> PrintHeading();
    fprintf(option -> syslis, "\nScopes:\n");

    for (int k = 0; k < scope_lhs_symbol.Size(); k++)
    {
        int symbol = scope_lhs_symbol[k];
        grammar -> RestoreSymbol(tok, grammar -> RetrieveString(symbol));
        int len = Control::PRINT_LINE_SIZE - 5;
        grammar -> PrintLargeToken(line, tok, "", len);
        strcat(line, " ::= ");
        int i = (Control::PRINT_LINE_SIZE / 2) - 1,
            offset = Util::Min(strlen(line) - 1, i);
        len = Control::PRINT_LINE_SIZE - (offset + 4);

        // locate end of list
        for (i = scope_prefix[k]; scope_right_side[i] != 0; i++)
            ;

        for (i = i - 1; i >= scope_prefix[k]; i--) // symbols before dot
        {
            int symbol = scope_right_side[i];
            grammar -> RestoreSymbol(tok, grammar -> RetrieveString(symbol));
            if (strlen(line) + strlen(tok) > Control::PRINT_LINE_SIZE - 4)
            {
                fprintf(option -> syslis, "\n%s", line);
                FillIn(tmp, offset, ' ');
                grammar -> PrintLargeToken(line, tok, tmp, len);
            }
            else
                strcat(line, tok);
            strcat(line, " ");
        }

        //
        // We now add a dot "." to the output line, and print the remaining
        // symbols in the right hand side.
        //
        strcat(line, " .");
        len = Control::PRINT_LINE_SIZE - (offset + 1);

        for (i = scope_suffix[k]; scope_right_side[i] != 0; i++)
        {
            int symbol = scope_right_side[i];
            grammar -> RestoreSymbol(tok, grammar -> RetrieveString(symbol));
            if (strlen(line) + strlen(tok) > Control::PRINT_LINE_SIZE - 1)
            {
                fprintf(option -> syslis, "\n%s", line);
                FillIn(tmp, offset, ' ');
                grammar -> PrintLargeToken(line, tok, tmp, len);
            }
            else
                strcat(line, tok);
            strcat(line, " ");
        }
        fprintf(option -> syslis, "\n%s", line);
    }

    return;
}


//
// This procedure takes as parameter a nonterminal, LHS_SYMBOL, and
// determines whether or not there is a terminal symbol t such that
// LHS_SYMBOL can rightmost produce a string tX.  If so, t is
// returned, otherwise EMPTY is returned.
//
int Produce::get_shift_symbol(int lhs_symbol)
{
    if (! symbol_seen[lhs_symbol])
    {
        symbol_seen[lhs_symbol] = true;

        for (int k = 0; k < base -> clitems[lhs_symbol].Length(); k++)
        {
            int item_no = base -> clitems[lhs_symbol][k],
                rule_no = base -> item_table[item_no].rule_number;
            if (grammar -> RhsSize(rule_no) > 0)
            {
                int symbol = grammar -> rhs_sym[grammar -> rules[rule_no].rhs_index];
                if (grammar -> IsTerminal(symbol))
                    return(symbol);
                else
                {
                    symbol = get_shift_symbol(symbol);
                    if (symbol != grammar -> empty)
                        return(symbol);
                }
            }
        }
    }

    return(grammar -> empty);
}
