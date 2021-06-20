#include "control.h"
#include "util.h"

Dfa::Dfa(Control *control_, Base *base_) : control(control_),
                                           option(control_ -> option),
                                           grammar(control_ -> grammar),
                                           node_pool(control_ -> node_pool),
                                           base(base_),

                                           num_states(0),
                                           num_shift_maps(0),
                                           num_shifts(0),
                                           num_shift_reduces(0),
                                           num_gotos(0),
                                           num_goto_reduces(0),

                                           shift_table(SHIFT_TABLE_SIZE, Util::NIL),
                                           conflict_table(CONFLICT_TABLE_SIZE, Util::NIL)
{
    (void) shift.NextIndex();
    (void) next_shift.Next();
    conflicts.Next() = 0;
    assert(shift.Length() == 1 && next_shift.Length() == 1 && conflicts.Length() == 1);

    return;
}

void Dfa::Process(void)
{
    MakeLr0();
    BuildPredecessorsMap();
    return;
}


//
// This procedure constructs an LR(0) automaton.
//
void Dfa::MakeLr0(void)
{
    (void) statset.Next(); // skip the zeroth element !!!

    //
    // Kernel of the first state consists of the first item of the
    // rule produced by the Accept non-terminal.
    //
    assert(base -> clitems[grammar -> accept_image].Length() == 1);
    Node *q = node_pool -> AllocateNode();
    q -> value = base -> clitems[grammar -> accept_image][0];
    q -> next = NULL;

    //
    // STATE_TABLE is the array used to hash the states. States are
    // identified by their Kernel set of items. Hash locations are
    // computed for the states. As states are inserted in the table,
    // they are threaded together via the QUEUE component of
    // STATE_ELEMENT. The variable STATE_ROOT points to the root of
    // the thread, and the variable STATE_TAIL points to the tail.
    //
    //   After constructing a state, Shift and Goto actions are
    // defined on that state based on a partition of the set of items
    // in that state. The partitioning is based on the symbols
    // following the dot in the items. The array PARTITION is used
    // for that partitioning. LIST and ROOT are used to construct
    // temporary lists of symbols in a state on which Shift or Goto
    // actions are defined.
    //   NT_LIST and NT_ROOT are used to build temporary lists of
    // non-terminals.
    //
    Array<int> state_table(STATE_TABLE_SIZE, Util::NIL),
               shift_action(grammar -> num_terminals + 1, Util::OMEGA);
    int goto_size = 0,
        nt_root = Util::NIL;;
    BoundedArray<int> nt_list(grammar -> num_terminals + 1, grammar -> num_symbols, Util::OMEGA);
    Array<Node *> partition(grammar -> num_symbols + 1);
    partition.MemReset();
    Tuple<int> shift_list,
               list,
               next_state;
    (void) next_state.Next();

    //
    // Insert first state in STATE_TABLE and keep constructing states
    // until we no longer can.
    //
    for (int state_index = lr0_state_map(state_table, next_state, q); // insert initial state
         state_index < statset.Length(); // and process next state until no more
         state_index++)
    {
        //
        // Now we construct a list of all non-terminals that can be
        // introduced in this state through closure.  The CLOSURE of each
        // non-terminal has been previously computed in MKFIRST.
        //
        for (q = statset[state_index].kernel_items;
             q != NULL; // iterate over kernel set of items
             q = q -> next)
        {
            int item_no = q -> value,
                symbol = base -> item_table[item_no].symbol;  // symbol after dot
            if (grammar -> IsNonTerminal(symbol)) // Dot symbol
            {
                if (nt_list[symbol] == Util::OMEGA) // not yet seen
                {
                    nt_list[symbol] = nt_root;
                    nt_root = symbol;

                    for (int k = 0; k < base -> closure[symbol].Length(); k++)
                    {
                        int element = base -> closure[symbol][k];

                        if (nt_list[element] == Util::OMEGA)
                        {
                            nt_list[element] = nt_root;
                            nt_root = element;
                        }
                    }
                }
            }
        }

        //
        // We now construct lists of all start items that the closure
        // non-terminals produce.  A map from each non-terminal to its set
        // start items has previously been computed in MKFIRST. (CLITEMS)
        // Empty items are placed directly in the state, whereas non_empty
        // items are placed in a temporary list rooted at CLOSURE_ROOT.
        //
        Node *closure_root = NULL, // used to construct list of closure items
             *closure_tail = NULL;
        for (int symbol = nt_root; symbol != Util::NIL; nt_list[symbol] = Util::OMEGA, symbol = nt_root)
        {
            nt_root = nt_list[symbol];

            for (int k = 0; k < base -> clitems[symbol].Length(); k++)
            {
                int item_no = base -> clitems[symbol][k];

                Node *new_item = node_pool -> AllocateNode();
                new_item -> value = item_no;

                if (base -> item_table[item_no].symbol == grammar -> empty) // complete item
                {
                    new_item -> next = statset[state_index].complete_items;
                    statset[state_index].complete_items = new_item;
                }
                else // closure item, add to closure list
                {
                    if (closure_root == NULL)
                         closure_root = new_item;
                    else closure_tail -> next = new_item;
                    closure_tail = new_item;
                }
            }
        }

        Node *item_ptr;
        if  (closure_root != NULL) // any non-complete closure items?
        {
            closure_tail -> next = statset[state_index].kernel_items;
            item_ptr = closure_root;
        }
        else  // else just consider kernel items
            item_ptr = statset[state_index].kernel_items;

        //
        //   In this loop, the PARTITION map is constructed. At this point,
        // ITEM_PTR points to all the non_complete items in the closure of
        // the state, plus all the kernel items.  We note that the kernel
        // items may still contain complete-items, and if any is found, the
        // COMPLETE_ITEMS list is updated.
        //
        list.Reset();
        for (; item_ptr != NULL; item_ptr = item_ptr -> next)
        {
            int item_no = item_ptr -> value,
                symbol = base -> item_table[item_no].symbol;
            if (symbol != grammar -> empty)  // incomplete item
            {
                int next_item_no = item_no + 1;

                if (partition[symbol] == NULL) // PARTITION not defined on symbol
                {
                    list.Next() = symbol;  // add to list
                    if (grammar -> IsNonTerminal(symbol))
                        goto_size++;
                }
                Node *p,
                     *tail = NULL;
                for (p = partition[symbol];
                     p != NULL;
                     tail = p, p = p -> next)
                {
                    if (p -> value > next_item_no)
                        break;
                }

                Node *r = node_pool -> AllocateNode();
                r -> value = next_item_no;
                r -> next = p;

                if (p == partition[symbol]) // Insert at beginning
                     partition[symbol] = r;
                else tail -> next = r;
            }
            else // Update complete item set with item from kernel
            {
                 Node *p = node_pool -> AllocateNode();
                 p -> value = item_no;
                 p -> next = statset[state_index].complete_items;
                 statset[state_index].complete_items = p;
            }
        }
        if (closure_root != NULL)
            node_pool -> FreeNodes(closure_root, closure_tail);

        //
        // We now iterate over the set of partitions and update the state
        // automaton and the transition maps: SHIFT and GOTO. Each
        // partition represents the kernel of a state.
        //
        for (int i = list.Length() - 1; i >= 0; i--)
        {
            int symbol = list[i],
                action = Util::OMEGA;

            //
            // If the partition contains only one item, and it is adequate
            // (i.e. the dot immediately follows the last symbol), and
            // READ-REDUCE is requested, a new state is not created, and the
            // action is marked as a Shift-reduce or a Goto-reduce. Otherwise
            // if a state with that kernel set does not yet exist, we create
            // it.
            //
            Node *q = partition[symbol]; // kernel of a new state
            if (option -> read_reduce && q -> next == NULL)
            {
                int item_no = q -> value;
                if (base -> item_table[item_no].symbol == grammar -> empty)
                {
                    int rule_no = base -> item_table[item_no].rule_number;
                    if (grammar -> rules[rule_no].lhs != grammar -> accept_image)
                    {
                        action = -rule_no;
                        node_pool -> FreeNodes(q, q);
                    }
                }
            }

            if (action == Util::OMEGA) // Not a Read-Reduce action
                action = lr0_state_map(state_table, next_state, q);

            //
            // At this stage, the partition list has been freed (for an old
            // state or an ADEQUATE item), or used (for a new state).  The
            // PARTITION field involved should be reset.
            //
            partition[symbol] = NULL;           // To be reused

            //
            // At this point, ACTION contains the value of the state to Shift
            // to, or rule to Read-Reduce on. If the symbol involved is a
            // terminal, we update the Shift map; else, it is a non-terminal
            // and we update the Goto map.
            // Shift maps are constructed temporarily in SHIFT_ACTION.
            // Later, they are inserted into a map of unique Shift maps, and
            // shared by states that contain identical shifts.
            // Since the lookahead set computation is based on the GOTO maps,
            // all these maps and their element maps should be kept as
            // separate entities.
            //
            if (grammar -> IsTerminal(symbol)) // terminal? add to SHIFT map
            {
                shift_action[symbol] = action;
                shift_list.Next() = symbol;
                if (action > 0)
                     num_shifts++;
                else num_shift_reduces++;
            }
            //
            // NOTE that for Goto's we update the field LA_PTR of GOTO. This
            // field will be used later in the routine MKRDCTS to point to a
            // look-ahead set.
            //
            else
            {
                int index = statset[state_index].go_to.NextIndex();

                statset[state_index].go_to[index].SetSymbol(symbol); // symbol field
                statset[state_index].go_to[index].SetAction(action); // state field
                statset[state_index].go_to[index].SetLaptr(Util::OMEGA); // la_ptr field

                if (action > 0)
                     num_gotos++;
                else num_goto_reduces++;
            }
        }

        statset[state_index].shift_number = UpdateShiftMaps(shift_list, shift_action);

        //
        // Reset shift_list and shift_action for reuse later...
        //
        for (int j = 0; j < shift_list.Length(); j++)
        {
            int symbol = shift_list[j];
            shift_action[symbol] = Util::OMEGA;
        }
        shift_list.Reset();
    }

    //
    // Construct STATSET, a "compact" and final representation of
    // State table, and SHIFT which is a set of all shift maps needed.
    // NOTE that assignments to elements of SHIFT may occur more than
    // once, but that's ok. It is probably faster to  do that than to
    // set up an elaborate scheme to avoid the multiple assignment which
    // may in fact cost more.  Look at it this way: it is only a pointer
    // assignment, namely a Load and a Store.
    // Release all NODEs used by  the maps CLITEMS and CLOSURE.
    //
    // If the grammar is LALR(k), k > 1, more states may be added and
    // the size of the shift map increased.
    //
    num_states = statset.Length() - 1; // recall that the 0th element is not used!

    assert(num_shift_maps == shift.Length() - 1);

    return;
}


//
// LR0_STATE_MAP takes as an argument a pointer to a kernel set of items. If
// no state based on that kernel set already exists, then a new one is
// created and added to STATE_TABLE. In any case, a pointer to the STATE of
// the KERNEL is returned.
//
int Dfa::lr0_state_map(Array<int> &state_table, Tuple<int> &next_state, Node *kernel)
{
    unsigned hash_address = 0;

    //
    //       Compute the hash address.
    //
    for (Node *p = kernel; p != NULL; p = p -> next)
         hash_address += p -> value;
    hash_address %= STATE_TABLE_SIZE;

    //
    // Check whether a state is already defined by the KERNEL set.
    //
    for (int i = state_table[hash_address]; i != Util::NIL; i = next_state[i])
    {
        Node *p,
             *q,
             *tail = NULL;

        for (p = statset[i].kernel_items, q = kernel;
             p != NULL && q != NULL;
             p = p -> next, tail = q, q = q -> next)
        {
            if (p -> value != q -> value)
                break;
        }

        if (p == q) // Both P and Q are NULL?
        {
            node_pool -> FreeNodes(kernel, tail);
            return i;
        }
    }

    //
    // Add a new state based on the KERNEL set.
    //
    int state_index = statset.NextIndex();

    assert(state_index == next_state.Length()); // sanity check !!!

    statset[state_index].kernel_items = kernel;
    statset[state_index].complete_items = NULL;
    statset[state_index].single_production_items = NULL;
    statset[state_index].transition_symbol = (base -> item_table[kernel -> value].dot == 0
                                                    ? grammar -> empty
                                                    : base -> item_table[kernel -> value - 1].symbol);
    next_state.Next() = state_table[hash_address];
    state_table[hash_address] = state_index;

    return state_index;
}


//
// We are now going to update the set of Shift-maps. Ths idea is
// to do a look-up in a hash table based on SHIFT_TABLE to see if
// the Shift map associated with the current state has already been
// computed. If it has, we simply update the SHIFT_NUMBER and the
// SHIFT field of the current state. Otherwise, we allocate and
// construct a SHIFT_ELEMENT map, update the current state, and
// add it to the set of Shift maps in the hash table.
//   Note that the SHIFT_NUMBER field in the STATE_ELEMENTs could
// have been factored out and associated instead with the
// SHIFT_ELEMENTs. That would have saved some space, but only in
// the short run. This field was purposely stored in the
// STATE_ELEMENTs, because once the states have been constructed,
// they are not kept, whereas the SHIFT_ELEMENTs are kept.
//    One could have also threaded through the states that contain
// original shift maps so as to avoid duplicate assignments in
// creating the SHIFT map later. However, this would have
// increased the storage requirement, and would probably have saved
// (at most) a totally insignificant amount of time.
//
int Dfa::UpdateShiftMaps(Tuple<int> &shift_list, Array<int> &shift_action)
{
    if (shift_list.Length() == 0)
        return 0;

    unsigned hash_address = shift_list.Length();
    for (int j = 0; j < shift_list.Length(); j++)
    {
        int symbol = shift_list[j];
        hash_address += Util::Abs(shift_action[symbol]);
    }
    hash_address %= SHIFT_TABLE_SIZE;

    for (int k = shift_table[hash_address];
         k != Util::NIL; // Search has table for shift map
         k = next_shift[k])
    {
        ShiftHeader &sh = shift[k];
        if (sh.Length() == shift_list.Length())
        {
            int i;
            for (i = 0; i < shift_list.Length(); i++) // Compare shift maps
            {
                if (sh[i].Action() != shift_action[sh[i].Symbol()])
                    break;
            }

            if (i == shift_list.Length())  // Are they equal ?
                return k;
        }
    }

    num_shift_maps++;
    int shift_index = shift.NextIndex();

    assert(shift_index == num_shift_maps);
    assert(shift_index == next_shift.Length());

    next_shift.Next() = shift_table[hash_address];
    shift_table[hash_address] = shift_index;

    ResetShiftMap(shift_index, shift_list, shift_action);

    return shift_index;
}


//
//
//
void Dfa::ResetShiftMap(int shift_index, Tuple<int> &shift_list, Array<int> &shift_action)
{
    ShiftHeader &sh = shift[shift_index];
    sh.Reset();
    for (int i = 0; i < shift_list.Length(); i++)
    {
        int symbol = shift_list[i],
            index = sh.NextIndex();
        sh[index].SetSymbol(symbol);
        sh[index].SetAction(shift_action[symbol]);
    }

    assert(sh.Length() == shift_list.Length());

    return;
}


//
//
//
void Dfa::QuickSort(Tuple<ConflictCell> &array)
{
    Stack<int> lostack,
               histack;
    int low = 0,
        high = array.Length() - 1;
    lostack.Push(low);
    histack.Push(high);
    while (! lostack.IsEmpty())
    {
        int lower = lostack.Pop(),
            upper = histack.Pop();

        while (upper > lower)
        {
            int i = lower;
            ConflictCell pivot = array[lower];
            for (int j = lower + 1; j <= upper; j++)
            {
                if (array[j].priority < pivot.priority)
                {
                    array[i] = array[j];
                    i++;
                    array[j] = array[i];
                }
            }
            array[i] = pivot;

            if (i - lower < upper - i)
            {
                lostack.Push(i + 1);
                histack.Push(upper);
                upper = i - 1;
            }
            else
            {
                histack.Push(i - 1);
                lostack.Push(lower);
                lower = i + 1;
            }
        }
    }

    return;
}


//
//
//
int Dfa::MapConflict(Tuple<ConflictCell> &cell)
{
    assert(cell.Length() > 1);
    QuickSort(cell);
    unsigned hash_address = 0;
    for (int j = 0; j < cell.Length(); j++)
        hash_address += ((unsigned) cell[j].action);
    hash_address %= CONFLICT_TABLE_SIZE;

    for (int k = conflict_table[hash_address];
         k != Util::NIL; // Search has table for conflict map
         k = conflict_element[k].next)
    {
        int i = conflict_element[k].index,
            j;
        for (j = 0; j < cell.Length(); j++) // Compare action list
        {
            if (conflicts[i + j] != cell[j].action)
                break;
        }

        if ((i + j >= conflicts.Length() || conflicts[i + j] == 0) && (j == cell.Length()))  // Are they equal ?
            return i;
    }

    int conflict_index = conflict_element.NextIndex();

    conflict_element[conflict_index].next = conflict_table[hash_address];
    conflict_table[hash_address] = conflict_index;

    conflict_element[conflict_index].index = conflicts.Length();

    for (int i = 0; i < cell.Length(); i++)
        conflicts.Next() = cell[i].action;
    conflicts.Next() = 0;

    return conflict_element[conflict_index].index;
}


//
// Replace a conflict list with new list.
//
void Dfa::RemapConflict(int index, Tuple<ConflictCell> &cell)
{
    assert(conflicts[index + cell.Length()] == 0);
    for (int i = 0; i < cell.Length(); i++)
        conflicts[index + i] = cell[i].action;

    return;
}

//
// We construct the IN_STAT map which is the inverse of the transition
// map formed by GOTO and SHIFT maps.
// This map is implemented as a table of pointers that can be indexed
// by the states to a circular list of integers representing other
// states that contain transitions to the state in question.
//
void Dfa::BuildPredecessorsMap(void)
{
    //
    //
    //
    for (int k = 1; k <= num_states; k++)
        statset[k].predecessors.Reset();

    //
    // Construct map.
    //
    for (int state_no = 1; state_no <= num_states; state_no++)
    {
        Dfa::ShiftHeader &sh = Shift(state_no);
        for (int i = 0; i < sh.Length(); ++i)
        {
            int action = sh[i].Action();
            if (action > 0 && action <= num_states) // A shift action?
                AddPredecessor(action, state_no);
        }

        Dfa::GotoHeader &go_to = statset[state_no].go_to;
        for (int j = 0; j < go_to.Length(); j++)
        {
            int action = go_to[j].Action();
            if (action > 0) // A goto action
                AddPredecessor(action, state_no);
        }
    }

    return;
}


//
// Given a STATE_NO and an ITEM_NO, ACCESS computes the set of states where
// the rule from which ITEM_NO is derived was introduced through closure.
//
void Dfa::Access(Tuple<int> &state_list, int state_no, int item_no)
{
    //
    // Build a list pointed to by ACCESS_ROOT originally consisting
    // only of STATE_NO.
    //
    Tuple<int> temp_list;
    int first_index = temp_list.NextIndex();
    temp_list[first_index] = state_no;
    for (int i = base -> item_table[item_no].dot; i > 0; i--) // distance to travel is DOT
    {
        int n = temp_list.Length();
        for (int j = first_index; j < n; j++)
        {
            int state_number = temp_list[j];
            //
            // Compute set of states with transition into p->value.
            //
            for (int k = 0; k < statset[state_number].predecessors.Length(); k++)
                temp_list.Next() = statset[state_number].predecessors[k];
        }

        first_index = n;
    }

    state_list.Reset(); // clear the list out
    for (int k = first_index; k < temp_list.Length(); k++)
        state_list.Next() = temp_list[k];

    return;
}


//
// PrintItem takes as parameter an ITEM_NO which it prints.
//
void Dfa::PrintItem(int item_no)
{
    //
    // We first print the left hand side of the rule, leaving at least
    // 5 spaces in the output line to accomodate the equivalence symbol
    // "::=" surrounded by blanks on both sides.  Then, we print all the
    // terminal symbols in the right hand side up to but not including
    // the dot symbol.
    //
    int rule_no = base -> item_table[item_no].rule_number,
        symbol = grammar -> rules[rule_no].lhs;
    char tempstr[Control::PRINT_LINE_SIZE + 1],
         line[Control::PRINT_LINE_SIZE + 1],
         tok[Control::SYMBOL_SIZE + 1];
    grammar -> RestoreSymbol(tok, grammar -> RetrieveString(symbol));
    int len = Control::PRINT_LINE_SIZE - 5;
    grammar -> PrintLargeToken(line, tok, "", len);
    strcat(line, grammar -> rules[rule_no].IsArrowProduction() ? " -> " : " ::= ");
    int i = (Control::PRINT_LINE_SIZE / 2) - 1,
        offset = Util::Min(strlen(line)-1, i);
    len = Control::PRINT_LINE_SIZE - (offset + 4);
    i = grammar -> rules[rule_no].rhs_index;  // symbols before dot
    int k = ((grammar -> rules[rule_no].rhs_index + base -> item_table[item_no].dot) - 1);
    for (; i <= k; i++)
    {
        int symbol = grammar -> rhs_sym[i];
        grammar -> RestoreSymbol(tok, grammar -> RetrieveString(symbol));
        if (strlen(tok) + strlen(line) > Control::PRINT_LINE_SIZE - 4)
        {
            fprintf(option -> syslis,"\n%s", line);
            FillIn(tempstr, offset, ' ');
            grammar -> PrintLargeToken(line, tok, tempstr, len);
        }
        else
            strcat(line, tok);
        strcat(line, " ");
    }

    //
    // We now add a DOT "." to the output line and print the remaining
    // symbols in the right hand side.  If ITEM_NO is a complete item,
    // we also print the rule number.
    //
    if (base -> item_table[item_no].dot == 0 || base -> item_table[item_no].symbol == grammar -> empty)
         strcpy(tok, ".");
    else strcpy(tok, " .");
    strcat(line, tok);
    len = Control::PRINT_LINE_SIZE - (offset + 1);
    {
        for (int i = grammar -> rules[rule_no].rhs_index +
                     base -> item_table[item_no].dot;// symbols after dot
             i <= grammar -> rules[rule_no + 1].rhs_index - 1;
             i++)
        {
            int symbol = grammar -> rhs_sym[i];
            grammar -> RestoreSymbol(tok, grammar -> RetrieveString(symbol));
            if (strlen(tok) + strlen(line) > Control::PRINT_LINE_SIZE -1)
            {
                 fprintf(option -> syslis, "\n%s", line);
                 FillIn(tempstr, offset, ' ');
                 grammar -> PrintLargeToken(line, tok, tempstr, len);
            }
            else strcat(line, tok);
            strcat(line, " ");
        }
    }

    if (base -> item_table[item_no].symbol == grammar -> empty)   // complete item
    {
        IntToString num(rule_no);
        if (num.Length() + 3 + strlen(line) > Control::PRINT_LINE_SIZE - 1)
        {
            fprintf(option -> syslis, "\n%s", line);
            FillIn(line,offset, ' ');
        }
        strcat(line, " (");
        strcat(line, num.String());
        strcat(line, ")");
    }
    fprintf(option -> syslis, "\n%s", line);

    return;
}
