#include "control.h"
#include <string.h>
#include <iostream>
using namespace std;

int Base::MAX_LENGTH = Control::PRINT_LINE_SIZE - 4;

//
// Constructor
//
Base::Base(Control *control_) : control(control_),
                                option(control_ -> option),
                                
                                grammar(control_ -> grammar),
                                node_pool(control_ -> node_pool)
{}


//
// MKFIRST constructs the FIRST and FOLLOW maps, the CLOSURE map,
// ADEQUATE_ITEM and ITEM_TABLE maps and all other basic maps.
//
void Base::Process(void)
{
    //
    // NT_FIRST is used to construct a mapping from non-terminals to the
    // set of terminals that may appear first in a string derived from
    // the non-terminal.
    //
    nt_first.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
    for (int i = grammar -> FirstNonTerminal(); i <= grammar -> LastNonTerminal(); i++)
        nt_first[i].Initialize(grammar -> num_terminals + 1);

    //
    // In this loop, we construct the LHS_RULE map which maps
    // each non-terminal symbol into the set of rules it produces
    //
    lhs_rule.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
    for (int rule_no = grammar -> FirstRule(); rule_no <= grammar -> LastRule(); rule_no++)
    {
        int lhs_symbol = grammar -> rules[rule_no].lhs;
        lhs_rule[lhs_symbol].Next() = rule_no;
    }

    //
    // Check if there are any non-terminals that do not produce
    // any rules.
    //
    NoRulesProduced();

    //
    // Construct the CLOSURE map of non-terminals.
    //
    closure.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
    index_of.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
    index_of.Initialize(OMEGA);
    {
        for (int nt = grammar -> FirstNonTerminal(); nt <= grammar -> LastNonTerminal(); nt++)
            if (index_of[nt] == OMEGA)
                ComputeClosure(nt);
    }

    //
    //
    //
    generates_null.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
    index_of.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
    index_of.Initialize(OMEGA);
    {
        for (int nt = grammar -> FirstNonTerminal(); nt <= grammar -> LastNonTerminal(); nt++)
            if (index_of[nt] == OMEGA)
                CheckCanGenerateNull(nt);
    }

    //
    // Construct the FIRST map for non-terminals and also a list
    // of non-terminals whose first set is empty.
    //
    index_of.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
    index_of.Initialize(OMEGA);
    {
        for (int nt = grammar -> FirstNonTerminal(); nt <= grammar -> LastNonTerminal(); nt++)
            if (index_of[nt] == OMEGA)
                ComputeFirst(nt);
    }

    //
    //  Since every input source will be followed by the EOFT
    //  symbol, FIRST[accept_image] cannot contain empty but
    //  instead must contain the EOFT symbol.
    //
    if (generates_null[grammar -> accept_image])
    {
        generates_null[grammar -> accept_image] = false;
        nt_first[grammar -> accept_image].RemoveElement(grammar -> empty);
        nt_first[grammar -> accept_image].AddElement(grammar -> eof_image);
    }

    //
    // Construct the ITEM_TABLE, FIRST_ITEM_OF, and NT_ITEMS maps.
    //
    item_table.Resize(grammar -> num_items + 1);
    first_item_of.Resize(grammar -> num_rules + 1);
    next_item.Resize(grammar -> num_items + 1);
    nt_items.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
    nt_items.Initialize(NIL);
    first_table.Resize(grammar -> num_symbols + 1);
    first_table.Initialize(NIL);

    assert(first.Length() == 0);
    first.Next(); // skip zeroth element

    int first_of_empty = first.NextIndex(); // use 1st slot for empty
    first[first_of_empty].suffix_root = 1;
    first[first_of_empty].suffix_tail = 0;

    int item_no = 0;
    item_table[item_no].rule_number = 0;
    item_table[item_no].symbol = grammar -> empty;
    item_table[item_no].dot = 0;
    item_table[item_no].suffix_index = NIL;

    {
        for (int rule_no = grammar -> FirstRule(); rule_no <= grammar -> LastRule(); rule_no++)
        {
            first_item_of[rule_no] = item_no + 1;
            int j = 0,
                k = grammar -> rules[rule_no + 1].rhs_index - 1;
            for (int i = grammar -> FirstRhsIndex(rule_no); i < grammar -> EndRhsIndex(rule_no); i++)
            {
                item_no++;
                int symbol = grammar -> rhs_sym[i];
                item_table[item_no].rule_number = rule_no;
                item_table[item_no].symbol = symbol;
                item_table[item_no].dot = j;

                if (option -> lalr_level > 1 ||
                    grammar -> IsNonTerminal(symbol) ||
                    symbol == grammar -> error_image)
                {
                    if (i == k)
                        item_table[item_no].suffix_index = first_of_empty;
                    else
                        item_table[item_no].suffix_index = FirstMap(i + 1, k);
                }
                else
                    item_table[item_no].suffix_index = NIL;
    
                if (grammar -> IsNonTerminal(symbol))
                {
                    next_item[item_no] = nt_items[symbol];
                    nt_items[symbol] = item_no;
                }
                j++;
            }

            item_table[++item_no].rule_number = rule_no;
            item_table[item_no].symbol = grammar -> empty;
            item_table[item_no].dot = j;
            item_table[item_no].suffix_index = NIL;
        }
    }


    //
    // We now compute the first set for all suffixes that were
    // inserted in the FIRST map.
    // Extra space is also allocated to compute the first set for
    // suffixes whose left-hand side is the ACCEPT non-terminal.
    // The first set for these suffixes are the sets needed to
    // construct the FOLLOW map and compute look-ahead sets.  They
    // are placed in the FIRST table in the range 1..NUM_FIRST_SETS
    // The first element in the FIRST table contains the first sets
    // for the empty sequence.
    //
    {
        for (int i = 1; i < first.Length(); i++)
        {
            first[i].set.Initialize(grammar -> num_terminals + 1);
            SuffixFirst(i);
        }
    }

    //
    // There must be only one rule whose left-hand side is accep_image.
    //
    int index = first.NextIndex();
    item_no = first_item_of[lhs_rule[grammar -> accept_image][0]];
    item_table[item_no].suffix_index = index;
    first[index].set.Initialize(grammar -> num_terminals + 1);
    first[index].set.AddElement(grammar -> eof_image);

    //
    // If the READ/REDUCE option is on, we precalculate the kernel
    // of the final states which simply consists of the last item
    // in  the corresponding rule.  Rules with the ACCEPT
    // non-terminal as their left-hand side are not considered so
    // as to let the Accpet action remain as a Reduce action
    // instead of a Goto/Reduce action.
    //
    if (option -> read_reduce)
    {
        adequate_item.Resize(grammar -> num_rules + 1);
        adequate_item.MemReset();
        for (int rule_no = grammar -> FirstRule(); rule_no <= grammar -> LastRule(); rule_no++)
        {
            int rhs_size = grammar -> RhsSize(rule_no);
            if (grammar -> rules[rule_no].lhs != grammar -> accept_image)
            {
                int item_no = first_item_of[rule_no] + rhs_size;
                Node *p = node_pool -> AllocateNode();
                p -> value = item_no;
                p -> next = NULL;
                adequate_item[rule_no] = p;
            }
        }
    }


    //
    // Construct the CLITEMS map. Each element of CLITEMS points
    // to a circular linked list of items.
    //
    clitems.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
    for (int nt = grammar -> FirstNonTerminal(); nt <= grammar -> LastNonTerminal(); nt++)
    {
        assert(clitems[nt].Length() == 0);
        for (int i = 0; i < lhs_rule[nt].Length(); i++)
        {
            int rule_no = lhs_rule[nt][i];
            clitems[nt].Next() = first_item_of[rule_no];
        }
    }

    //
    // Construct the GENERATES_STRING map for non-terminals.
    //
    {
        Tuple<int> bad_list,
                   depend_list;
        generates_string.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
        is_cyclic.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
        index_of.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
        index_of.Initialize(OMEGA);
        for (int nt = grammar -> FirstNonTerminal(); nt <= grammar -> LastNonTerminal(); nt++)
        {
            if (index_of[nt] == OMEGA)
                CheckCanGenerateString(nt);
            if (! generates_string[nt]) {
                if (is_cyclic[nt])
                     bad_list.Next() = nt;
                else depend_list.Next() = nt;
            }
        }

        if (bad_list.Length() > 0 || depend_list.Length())
        {
            for (int i = 0; i < bad_list.Length(); i++)
            {
                int symbol = bad_list[i];
                char tok[Control::SYMBOL_SIZE + 1];
                grammar -> RestoreSymbol(tok, grammar -> RetrieveString(symbol));

                Tuple<const char *> msg;
                msg.Next() = "The nonterminal ";
                msg.Next() = tok;
                msg.Next() = " cannot generate complete strings of terminals";
                option -> EmitError(grammar -> RetrieveTokenLocation(symbol), msg);
            }

            /*
            for (int i = 0; i < depend_list.Length(); i++)
            {
                int symbol = depend_list[i];
                char tok[Control::SYMBOL_SIZE + 1];
                grammar -> RestoreSymbol(tok, grammar -> RetrieveString(symbol));

                int symbol = bad_list[i];
                char tok[Control::SYMBOL_SIZE + 1];
                grammar -> RestoreSymbol(tok, grammar -> RetrieveString(symbol));

                Tuple<const char *> msg;
                msg.Next() = "The nonterminal ";
                msg.Next() = tok;
                msg.Next() = " depends on nonterminals that cannot generate complete strings of terminals";
                option -> EmitError(grammar -> RetrieveTokenLocation(symbol), msg);
            }
            */

            control -> Exit(12);
        }
    }

    //
    // If LALR_LEVEL > 1, we need to calculate RMPSELF, a set that
    // identifies the nonterminals that can right-most produce
    // themselves. In order to compute RMPSELF, the map PRODUCES
    // must be constructed which identifies for each nonterminal
    // the set of nonterminals that it can right-most produce.
    //
    if (option -> lalr_level > 1)
    {
        direct_produces.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
        produces.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
        {
            for (int nt = grammar -> FirstNonTerminal(); nt <= grammar -> LastNonTerminal(); nt++)
                produces[nt].Initialize(grammar -> num_nonterminals + 1, grammar -> num_terminals);
        }

        for (int nt = grammar -> FirstNonTerminal(); nt <= grammar -> LastNonTerminal(); nt++)
        {
            for (int k = 0; k < clitems[nt].Length(); k++)
            {
                int item_no = clitems[nt][k],
                    symbol = item_table[item_no].symbol;
                if (grammar -> IsNonTerminal(symbol))
                {
                    int i = item_table[item_no].suffix_index;
                    if (first[i].set[grammar -> empty] && (! produces[nt][symbol]))
                    {
                        produces[nt].AddElement(symbol);
                        direct_produces[nt].Next() = symbol;
                    }
                }
            }
        }

        //
        // Complete the construction of the RIGHT_MOST_PRODUCES map
        // for non-terminals using the digraph algorithm.
        //
        index_of.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
        index_of.Initialize(OMEGA);
        {
            for (int nt = grammar -> FirstNonTerminal(); nt <= grammar -> LastNonTerminal(); nt++)
                if (index_of[nt] == OMEGA)
                    ComputeProduces(nt);
        }

        InitRmpself(produces);
    }

    //
    // Construct the FOLLOW map if
    //   1) an SLR table is requested
    //   2) if we have to print the FOLLOW map
    //   3) Error-maps are requested
    //   4) There are more than one starting symbol.
    //
    if (option -> slr || option -> follow || option -> error_maps || lhs_rule[grammar -> accept_image].Length() > 1)
    {
        follow.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
        for (int i = grammar -> FirstNonTerminal(); i <= grammar -> LastNonTerminal(); i++)
            follow[i].Initialize(grammar -> num_terminals + 1);
        follow[grammar -> accept_image].AddElement(grammar -> eof_image);


        index_of.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
        index_of.Initialize(OMEGA);
        index_of[grammar -> accept_image] = INFINITY_;  // mark computed
        for (int nt = grammar -> FirstNonTerminal(); nt <= grammar -> LastNonTerminal(); nt++)
        {
            if (index_of[nt] == OMEGA) // not yet computed ?
                ComputeFollow(nt);
        }

        //
        // Initialize FIRST for suffixes that can follow each starting
        // non-terminal ( except the main symbol) with the FOLLOW set
        // of the non-terminal in question.
        //
        if (lhs_rule[grammar -> accept_image].Length() > 1)
        {
            int rule_no = lhs_rule[grammar -> accept_image][0],
                top = item_table[first_item_of[rule_no]].suffix_index;
            for (int i = top + 1, k = 0; i < first.Length(); i++, k++)
            {
                int rule_no = lhs_rule[grammar -> accept_image][k],
                    item_no = first_item_of[rule_no],
                    symbol = item_table[item_no].symbol;
                if (grammar -> IsNonTerminal(symbol))
                    first[i].set = follow[symbol];
            }
        }
    }

    //
    // If we are constructing a backtracking parser, we need to order
    // the rules based on the relation:
    //
    //    (A, B) is in the relation iff A ::=* B xxx
    //
    // Note that this implies that (A, B) if A ::= yyy B xxx if
    // yyy =>* empty.
    //
    if (option -> backtrack)
    {
        rank.Resize(grammar -> num_rules + 1);

        lhs_ranked_rules.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);

        //
        // In this loop, we construct the LHS_RANKED_RULES map which maps
        // each non-terminal symbol into the set of rules it produces.
        // When we encounter a rule where the priority is explicitly set,
        // we process it right away.
        //
        for (int rule_no = grammar -> FirstRule(); rule_no <= grammar -> LastRule(); rule_no++)
        {
            if (option -> priority && grammar -> rules[rule_no].IsPriorityProduction())
                ordered_rules.Next() = rule_no;
            else
            {
                int symbol = grammar -> rules[rule_no].lhs;
                lhs_ranked_rules[symbol].Next() = rule_no;
            }
        }

        //
        // Now assign priorities to rules based on partial order of the
        // nonterminals (left-hand sides). See ComputeRank for the
        // relation on which the partial order is constructed.
        //
        index_of.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);
        index_of.Initialize(OMEGA);
        for (int nt = grammar -> FirstNonTerminal(); nt <= grammar -> LastNonTerminal(); nt++)
        {
            if (index_of[nt] == OMEGA)
                ComputeRank(nt);
        }

        for (int i = 0; i < ordered_rules.Length(); i++)
            rank[ordered_rules[i]] = i + 1;
        lhs_ranked_rules.Resize(0, -1); // free space

        //
        // Report any problem found in computing the rank of the rules.
        //
        if (option -> priority)
        {
            //
            // First, flush any data left in the report buffer.
            //
            option -> FlushReport();

            if (SCC_in_ranks.Length() > 0)
            {
                fprintf(option -> syslis, "\n\nStrongly-connected components in ranks:\n\n");
                for (int i = 0; i < SCC_in_ranks.Length(); i++)
                {
                    for (int k = 0; k < SCC_in_ranks[i].Length(); k++)
                        fprintf(option -> syslis, "    %s\n", grammar -> RetrieveString(SCC_in_ranks[i][k]));
                    fprintf(option -> syslis, "\n");
                }
            }

            fprintf(option -> syslis, "\n\n" "RANK RULE\n" "==== ====\n");
            for (int i = 0; i < ordered_rules.Length(); i++)
            {
                int rule_no = ordered_rules[i];
                fprintf(option -> syslis, "%-4d %-4d  ", rank[rule_no], rule_no);
                fprintf(option -> syslis, "%s ::=", grammar -> RetrieveString(grammar -> rules[rule_no].lhs));
                for (int k = grammar -> FirstRhsIndex(rule_no); k < grammar -> EndRhsIndex(rule_no); k++)
                {
                    int symbol = grammar -> rhs_sym[k];
                    fprintf(option -> syslis, " %s", grammar -> RetrieveString(symbol));
                }
                fprintf(option -> syslis, "\n");
            }
        }

        SCC_in_ranks.Resize(0); // free space
    }

    //
    // Construct the LAST map from each non-terminal A into a set
    // of terminals that may appear as the last symbol in a string
    // derived from A. I.e., if (A =>* X t) where X is an arbitrary
    // string then t is in LAST(A).
    //
    if (grammar -> check_predecessor_sets_for.Length() > 0)
    {
        last.Resize(grammar -> num_symbols + 1);
        last[0].Initialize(grammar -> num_terminals + 1);
        index_of.Resize(0, last.Size());
        index_of[0] = INFINITY_;
        for (int i = 1; i < last.Size(); i++)
        {
            last[i].Initialize(grammar -> num_terminals + 1);
            if (grammar -> IsTerminal(i))
            {
                 last[i].AddElement(i);
                 index_of[i] = INFINITY_;
            }
            else index_of[i] = OMEGA;
        }

        for (int nt = grammar -> FirstNonTerminal(); nt <= grammar -> LastNonTerminal(); nt++)
        {
            if (index_of[nt] == OMEGA)
                ComputeLast(nt);
        }

        //
        //
        //
        item_of.Resize(grammar -> num_symbols + 1);
        for (int item_no = 0; item_no < item_table.Size(); item_no++)
        {
            int symbol = item_table[item_no].symbol;
            item_of[symbol].Next() = item_no;
        }

        predecessor.Resize(grammar -> num_symbols + 1);
        {
            for (int i = 0; i < predecessor.Size(); i++)
                predecessor[i].Initialize(grammar -> num_terminals + 1);
        }

        index_of.Resize(0, predecessor.Size());
        index_of.Initialize(OMEGA);
        {
            for (int i = grammar -> FirstTerminal(); i <= grammar -> LastNonTerminal(); i++)
                if (index_of[i] == OMEGA)
                    ComputePredecessor(i);
        }

        //
        // Check the predecessor sets of each pairs of symbols.
        // If their intersection is not empty then issue an error message.
        //
        {
            for (int i = 0; i < grammar -> check_predecessor_sets_for.Length(); i++)
            {
                int left_symbol = grammar -> check_predecessor_sets_for[i].left_symbol,
                    right_symbol = grammar -> check_predecessor_sets_for[i].right_symbol;
                BitSet inter(predecessor[left_symbol]);
                inter *= predecessor[right_symbol];
                for (int k = 0; k < inter.Size(); k++)
                {
                    if (inter[k])
                    {
                        char left_tok[Control::SYMBOL_SIZE + 1],
                             right_tok[Control::SYMBOL_SIZE + 1];
                        grammar -> RestoreSymbol(left_tok, grammar -> RetrieveString(left_symbol));
                        grammar -> RestoreSymbol(right_tok, grammar -> RetrieveString(right_symbol));
    
                        Tuple<const char *> msg;
                        msg.Next() = "This symbol is a predecessor of both \"";
                        msg.Next() = left_tok;
                        msg.Next() = "\"  and \"";
                        msg.Next() = right_tok;
                        msg.Next() = "\"";
                        option -> EmitError(grammar -> RetrieveTokenLocation(k), msg);
                    }
                }
            }
        }
    }

    //
    // If WARNINGS option is turned on, the unreachable symbols in
    // the grammar and duplicate rules are reported.
    //
    if (option -> warnings)
    {
        CheckDuplicateRules();
        PrintUnreachables();
    }

    //
    // If a Cross_Reference listing is requested, it is generated
    // here.
    //
    if (option -> xref)
        PrintXref();

    //
    // If a listing of the FIRST map is requested, it is generated
    // here.
    //
    if (option -> first)
        PrintNonTerminalFirst();

    //
    // If a listing of the PREDECESSOR map is requested, it is generated
    // here.
    //
    if (grammar -> check_predecessor_sets_for.Length() > 0)
    {
        PrintSymbolMap("Last map", last);
        PrintSymbolMap("Predecessor map", predecessor);
    }

    //
    // If a listing of the FOLLOW map is requested, it is generated
    // here.
    //
    if (option -> follow)
        PrintFollowMap();

    option -> FlushReport();

    return;
}


//
// DEPRECATED: TODO: Note that since the new input format does
// not require terminal symbols to be declared, all non-declared
// grammar symbols are assumed to be terminals. Therefore, it
// will never be the case that we detect a nonterminal that does not
// produce any rule.
//
void Base::NoRulesProduced(void)
{
    //
    // Build a list of all non-terminals that do not produce any
    // rules.
    //
    Tuple<int> list;
    for (int symbol = grammar -> FirstNonTerminal(); symbol <= grammar -> LastNonTerminal(); symbol++)
    {
        if (lhs_rule[symbol].Length() == 0)
            list.Next() = symbol;
    }

    //
    // If the list of non-terminals that do not produce any rules
    // is not empty, signal error and stop.
    //
    if (list.Length() > 0)
    {
        for (int i = 0; i < list.Length(); i++)
        {
            int symbol = list[i];
            char tok[Control::SYMBOL_SIZE + 1];
            grammar -> RestoreSymbol(tok, grammar -> RetrieveString(symbol));

            Tuple<const char *> msg;
            msg.Next() = "The nonterminal ";
            msg.Next() = tok;
            msg.Next() = " does not produce any rule.";
            option -> EmitError(grammar -> RetrieveTokenLocation(symbol), msg);
        }

        control -> Exit(12);
    }

    return;
}


//
//  This function computes the closure of a non-terminal LHS_SYMBOL passed
// to it as an argument using the digraph algorithm.
// The closure of a non-terminal A is the set of all non-terminals Bi that
// can directly or indirectly start a string generated by A.
// I.e., A *::= Bi X where X is an arbitrary string.
//
void Base::ComputeClosure(int lhs_symbol)
{
    BoundedArray<int> nont_list(grammar -> num_terminals + 1, grammar -> num_symbols, OMEGA);
    nont_list[lhs_symbol] = NIL;
    int nt_root = lhs_symbol;

    stack.Push(lhs_symbol);
    int indx = stack.Length();
    index_of[lhs_symbol] = indx;

    assert(closure[lhs_symbol].Length() == 0);

    for (int i = 0; i < lhs_rule[lhs_symbol].Length(); i++)
    {
        int rule_no = lhs_rule[lhs_symbol][i],
            symbol = (grammar -> RhsSize(rule_no) == 0
                               ? grammar -> empty
                               : grammar -> rhs_sym[grammar -> rules[rule_no].rhs_index]);

        if (grammar -> IsNonTerminal(symbol))
        {
            if (nont_list[symbol] == OMEGA)
            {
                if (index_of[symbol] == OMEGA) // if first time seen
                    ComputeClosure(symbol);

                index_of[lhs_symbol] = Util::Min(index_of[lhs_symbol],
                                                    index_of[symbol]);
                nont_list[symbol] = nt_root;
                nt_root = symbol;

                //
                // add closure[symbol] to closure of LHS_SYMBOL.
                //
                for (int k = 0; k < closure[symbol].Length(); k++)
                {
                    int element = closure[symbol][k];
                    if (nont_list[element] == OMEGA)
                    {
                        nont_list[element] = nt_root;
                        nt_root = element;
                    }
                }
            }
        }
    }

    assert(closure[lhs_symbol].Length() == 0);

    for (; nt_root != lhs_symbol; nt_root = nont_list[nt_root])
        closure[lhs_symbol].Next() = nt_root;

    if (index_of[lhs_symbol] == indx)
    {
        for (int symbol = stack.Top(); symbol != lhs_symbol; symbol = stack.Top())
        {
            closure[symbol].Next() = lhs_symbol;

            for (int k = 0; k < closure[lhs_symbol].Length(); k++)
            {
                int element = closure[lhs_symbol][k];
                if (element != symbol)
                    closure[symbol].Next() = element;
            }

            index_of[symbol] = INFINITY_;
            stack.Pop();
        }

        index_of[lhs_symbol] = INFINITY_;
        stack.Pop();
    }

    return;
}


//
// DEPRECATED:
//
// This procedure computes the set of non-terminal symbols that can
// generate the empty string.  Such non-terminals are said to be nullable.
//
// A non-terminal "A" can generate empty if the grammar in question contains
// a rule:
//
//          A ::= B1 B2 ... Bn     n >= 0,  1 <= i <= n
// and Bi, for all i, is a nullable non-terminal.
//
/*
void Base::NullablesComputation(void)
{
    //
    // First, initialize
    // RHS_START. RHS_START is a mapping from each rule in the grammar
    // into the next symbol in its right-hand side that has not yet
    // proven to be nullable.
    //
    Array<int> rhs_start(grammar -> num_rules + 1);
    for (int rule_no = grammar -> FirstRule(); rule_no <= grammar -> LastRule(); rule_no++)
        rhs_start[rule_no] = grammar -> rules[rule_no].rhs_index;

    //
    // We now iterate over the rules and try to advance the RHS_START
    // pointer thru each right-hand side as far as we can.  If one or
    // more non-terminals are found to be nullable, they are marked
    // as such and the process is repeated.
    //
    // If we go through all the rules and no new non-terminal is found
    // to be nullable then we stop and return.
    //
    // Note that for each iteration, only rules associated with
    // non-terminals that are non-nullable are considered.  Further,
    // as soon as a non-terminal is found to be nullable, the
    // remaining rules associated with it are not considered.  I.e.,
    // we quit the inner loop.
    //
    bool changed = true;
    while(changed)
    {
        changed = false;

        for (int nt = grammar -> FirstNonTerminal(); nt <= grammar -> LastNonTerminal(); nt++)
        {
            for (int i = 0; i < lhs_rule[nt].Length() && (! null_nt[nt]); i++)
            {
                int rule_no = lhs_rule[nt][i];
                if (IsNullableRhs(rhs_start, rule_no))
                {
                    changed = true;
                    null_nt[nt] = true;
                }
            }
        }
    }

    return;
}

//
// DEPRECATED:
//
// This procedure tries to advance the RHS_START pointer.  If the current
// symbol identified by the RHS_START element is a terminal it returns false
// to indicate that it cannot go any further.  If it encounters a  non-null-
// lable non-terminal, it also returns false. Otherwise, the whole right-hand
// side is consumed, and it returns the value true.
//
bool Base::IsNullableRhs(Array<int> &rhs_start, int rule_no)
{
    for(rhs_start[rule_no] = rhs_start[rule_no];
        rhs_start[rule_no] <= grammar -> rules[rule_no + 1].rhs_index - 1;
        rhs_start[rule_no]++)
    {
        int symbol = grammar -> rhs_sym[rhs_start[rule_no]];
        if (grammar -> IsTerminal(symbol))
             return(false);
        else if (! null_nt[symbol]) // symbol is a non-terminal
             return(false);
    }

    return(true);
}
*/


//
// This subroutine computes FIRST(NT) for some non-terminal NT using the
// digraph algorithm.
// FIRST(NT) is the set of all terminals Ti that may start a string generated
// by NT. That is, NT *::= Ti X where X is an arbitrary string.
//
void Base::ComputeFirst(int nt)
{
    stack.Push(nt);
    int indx = stack.Length();
    index_of[nt] = indx;

    //
    // Iterate over all rules generated by non-terminal NT...
    // In this application of the transitive closure algorithm,
    //
    //  G(A) := { t | A ::= t X for a terminal t and a string X }
    //
    // The relation R is defined as follows:
    //
    //    R(A, B) iff A ::= B1 B2 ... Bk B X
    //
    // where Bi is nullable for 1 <= i <= k
    //
    for (int k = 0; k < lhs_rule[nt].Length(); k++)
    {
        int rule_no = lhs_rule[nt][k];

        bool blocked = false;
        for (int i = grammar -> FirstRhsIndex(rule_no); i < grammar -> EndRhsIndex(rule_no); i++)
        {
            int symbol = grammar -> rhs_sym[i];
            if (grammar -> IsNonTerminal(symbol))
            {
                if (index_of[symbol] == OMEGA)
                    ComputeFirst(symbol);
                index_of[nt] = Util::Min( index_of[nt], index_of[symbol]);

                BitSet temp_set(nt_first[symbol]);
                temp_set.RemoveElement(grammar -> empty);
                nt_first[nt] += temp_set;
                blocked = ! IsNullable(symbol);
            }
            else
            {
                nt_first[nt].AddElement(symbol);
                blocked = true;
            }

            if (blocked)
                break;
        }

        if (! blocked)
        {
            nt_first[nt].AddElement(grammar -> empty);
        }
    }

    if (index_of[nt] == indx)
    {
        for (int symbol = stack.Top(); symbol != nt; symbol = stack.Top())
        {
            nt_first[symbol] = nt_first[nt];
            index_of[symbol] = INFINITY_;
            stack.Pop();
        }
        index_of[nt] = INFINITY_;
        stack.Pop();
    }

    return;
}


//
// This subroutine computes LAST(NT) for some non-terminal NT using the
// digraph algorithm.
// LAST(NT) is the set of all terminals Ti that may appear last in a string
// generated by NT. That is, NT *::= X Ti where X is an arbitrary string.
//
void Base::ComputeLast(int nt)
{
    stack.Push(nt);
    int indx = stack.Length();
    index_of[nt] = indx;

    //
    //
    //
    for (int k = 0; k < clitems[nt].Length(); k++)
    {
        int item_no = clitems[nt][k],
            rule_no = item_table[item_no].rule_number,
            last_item_no = item_no + grammar -> RhsSize(rule_no) - 1;
        int i;
        for (i = last_item_no; i >= item_no; i--)
        {
            int symbol = item_table[i].symbol;
            if (grammar -> IsNonTerminal(symbol))
            {
                if (index_of[symbol] == OMEGA)
                    ComputeLast(symbol);
            }
            index_of[nt] = Util::Min(index_of[nt], index_of[symbol]);
            last[nt] += last[item_table[i].symbol];
            if (grammar -> IsTerminal(symbol) || (! IsNullable(symbol)))
                break;
        }

        if (i < item_no)
            last[nt].AddElement(grammar -> empty);
    }

    if (index_of[nt] == indx)
    {
        for (int symbol = stack.Top(); symbol != nt; symbol = stack.Top())
        {
            last[symbol] = last[nt];
            index_of[symbol] = INFINITY_;
            stack.Pop();
        }
        index_of[nt] = INFINITY_;
        stack.Pop();
    }

    return;
}


//
// This subroutine computes PREDECESSOR(x) for some symbol x using the
// digraph algorithm.
// PREDECESSOR(x) is the set of all terminals Ti that may immediately
// prior to x.
//
void Base::ComputePredecessor(int symbol)
{
    stack.Push(symbol);
    int indx = stack.Length();
    index_of[symbol] = indx;

    //
    //
    //
    for (int k = 0; k < item_of[symbol].Length(); k++)
    {
        int item_no;
        for (item_no = item_of[symbol][k]; item_table[item_no].dot > 0; item_no--)
        {
            int preceding_symbol = item_table[item_no - 1].symbol; // symbol before the dot
            predecessor[symbol] += last[preceding_symbol];
            if (! last[preceding_symbol][grammar -> empty]) // is empty contained in the last set of symbol?
                break;
        }

        if (item_table[item_no].dot == 0)
        {
            int rule_no = item_table[item_no].rule_number,
                lhs_symbol = grammar -> rules[rule_no].lhs;
            if (index_of[lhs_symbol] == OMEGA)
                ComputePredecessor(lhs_symbol);
            index_of[symbol] = Util::Min(index_of[symbol], index_of[lhs_symbol]);
            predecessor[symbol] += predecessor[lhs_symbol];
        }
    }

    if (index_of[symbol] == indx)
    {
        predecessor[symbol].RemoveElement(grammar -> empty);
        for (int other = stack.Top(); other != symbol; other = stack.Top())
        {
            predecessor[other] = predecessor[symbol];
            index_of[other] = INFINITY_;
            stack.Pop();
        }
        index_of[symbol] = INFINITY_;
        stack.Pop();
    }

    return;
}


//
// This subroutine computes GENERATES_STRING(x) for some nonterminal x using the
// digraph algorithm.
// GENERATES_STRING(x) is a boolean indicating whether or not x can generate 
// a string of terminals or the empty string.
//
void Base::CheckCanGenerateString(int nt)
{
    stack.Push(nt);
    int indx = stack.Length();
    index_of[nt] = indx;
    generates_string[nt] = false;
    is_cyclic[nt] = false;

    //
    // A non-terminal "A" can generate a terminal string if the grammar in
    // question contains a rule of the form:
    //
    //         A ::= X1 X2 ... Xn           n >= 0,  1 <= i <= n
    //
    // and Xi, for all i, is either a terminal or a non-terminal that can
    // generate a string of terminals.
    //
    for (int k = 0; k < clitems[nt].Length() && (! generates_string[nt]); k++)
    {
        int rule_no = lhs_rule[nt][k];
        bool rule_generates_string = true; // assume this is true and see if we can prove it!
        for (int i = grammar -> FirstRhsIndex(rule_no); i < grammar -> EndRhsIndex(rule_no) && rule_generates_string; i++)
        {
            int symbol = grammar -> rhs_sym[i];
            if (grammar -> IsNonTerminal(symbol))
            {
                if (index_of[symbol] == OMEGA)
                     CheckCanGenerateString(symbol);
                else is_cyclic[nt] = is_cyclic[nt] || (symbol == nt); // a trivial cycle? record it!
                index_of[nt] = Util::Min(index_of[nt], index_of[symbol]);
                rule_generates_string = generates_string[symbol];
            }
        }

        generates_string[nt] = rule_generates_string;
    }

    if (index_of[nt] == indx)
    {
        int symbol = stack.Top();
        is_cyclic[nt] = is_cyclic[nt] || (symbol != nt); // more than one symbol in SCC?
        index_of[nt] = INFINITY_;
        for ( ; symbol != nt; symbol = stack.Top())
        {
            generates_string[symbol] = generates_string[nt];
            is_cyclic[symbol] = is_cyclic[nt]; // obviously true!!!
            index_of[symbol] = INFINITY_;
            stack.Pop();
        }
        stack.Pop();
    }

    return;
}


//
// This subroutine computes generates_null(x) for some nonterminal x using the
// digraph algorithm.
// generates_null(x) is a boolean indicating whether or not x can generate 
// the empty string.
//
void Base::CheckCanGenerateNull(int nt)
{
    stack.Push(nt);
    int indx = stack.Length();
    index_of[nt] = indx;
    generates_null[nt] = false;

    //
    // A non-terminal A can generate empty iff the grammar in question contains
    // a rule:
    //
    //          A ::= B1 B2 ... Bn     n >= 0,  1 <= i <= n
    //
    // and Bi, for all i, is a nullable non-terminal.
    //
    for (int k = 0; k < lhs_rule[nt].Length() && (! generates_null[nt]); k++)
    {
        int rule_no = lhs_rule[nt][k];
        bool rule_generates_empty = true; // assume this is true and see if we can prove it!
        for (int i = grammar -> FirstRhsIndex(rule_no); i < grammar -> EndRhsIndex(rule_no) && rule_generates_empty; i++)
        {
            int symbol = grammar -> rhs_sym[i];
            if (grammar -> IsNonTerminal(symbol))
            {
                if (index_of[symbol] == OMEGA)
                    CheckCanGenerateNull(symbol);
                index_of[nt] = Util::Min(index_of[nt], index_of[symbol]);
                rule_generates_empty = generates_null[symbol];
            }
            else rule_generates_empty = false; // terminal indicates that this rule does not generate empty
        }

        generates_null[nt] = rule_generates_empty;
    }

    if (index_of[nt] == indx)
    {
        for (int symbol = stack.Top(); symbol != nt; symbol = stack.Top())
        {
            generates_null[symbol] = generates_null[nt];
            index_of[symbol] = INFINITY_;
            stack.Pop();
        }
        index_of[nt] = INFINITY_;
        stack.Pop();
    }

    return;
}


//
// Compute rank for each rule if needed.
// TODO: This is a temporary algorithm. A much deeper
// analysis is required...
//
void Base::ComputeRank(int nt)
{
    stack.Push(nt);
    int indx = stack.Length();
    index_of[nt] = indx;

    //
    // Iterate over all rules generated by non-terminal NT...
    // In this application of the transitive closure algorithm,
    //
    //  G(A) := { t | A ::= t X for a terminal t and a string X }
    //
    // The relation R is defined as follows:
    //
    //    R(A, Bi) and R(A, B) iff A ::= B1 B2 ... Bk B X
    //
    // where Bi is nullable for 0 <= i <= k and B is not nullable
    //
    for (int k = 0; k < lhs_ranked_rules[nt].Length(); k++)
    {
        int rule_no = lhs_ranked_rules[nt][k];
        for (int i = grammar -> FirstRhsIndex(rule_no); i < grammar -> EndRhsIndex(rule_no); i++)
        {
            int symbol = grammar -> rhs_sym[i];
            if (grammar -> IsNonTerminal(symbol))
            {
                if (index_of[symbol] == OMEGA)
                    ComputeRank(symbol);
                index_of[nt] = Util::Min( index_of[nt], index_of[symbol]);

                if (! IsNullable(symbol))
                    break;
            }
            else break;
        }
    }

    if (index_of[nt] == indx)
    {
        int symbol;
        if (nt == stack.Top())
        {
            symbol = stack.Top();
            ProcessRank(symbol);
            index_of[symbol] = INFINITY_;
            stack.Pop();
        }
        else // a non-trivial SCC
        {
            Tuple<int> &scc = SCC_in_ranks.Next();
            do
            {
                symbol = stack.Top();
                scc.Next() = symbol;
                ProcessRank(symbol);
                index_of[symbol] = INFINITY_;
                stack.Pop();
            } while (symbol != nt);
        }
    }

    return;
}

//
// Assign a rank to all rules whose left-hand side is "symbol" who
// were not explicitly prioritized.
//
void Base::ProcessRank(int symbol)
{
    //
    // List of empty rules associated with symbol.
    // Note that we only consider explicitly specified NULL
    // productions; nullable productions, i.e., a production
    // whose right-hand side consists of a sequence of nullable
    // nonterminals are not considered.
    //
    Tuple<int> empty_rules,
               nullable_rules;

    //
    // In this loop, as we iterate over the non-prioritized rules associated
    // with a nonterminal, we assign ranks to them in the order in which they
    // appear unless they happen to be empty or nullable rules in which case
    // we add them to separate respective lists.
    //
    for (int i = 0; i < lhs_ranked_rules[symbol].Length(); i++)
    {
        int rule_no = lhs_ranked_rules[symbol][i],
            first_item = first_item_of[rule_no],
            first_rhs_symbol = item_table[first_item].symbol;
        if (grammar -> RhsSize(rule_no) == 0)
             empty_rules.Next() = rule_no;
        else if (grammar -> IsNonTerminal(first_rhs_symbol) &&
                 IsNullable(first_rhs_symbol) &&
                 First(item_table[first_item].suffix_index)[grammar -> empty])
             nullable_rules.Next() = rule_no;
        else ordered_rules.Next() = rule_no;
    }

    //
    // We now assign ranks to the nullable rules.
    //
    {
        for (int i = 0; i < nullable_rules.Length(); i++)
            ordered_rules.Next() = nullable_rules[i];
    }

    //
    // We now assign ranks to the empty rules.
    //
    {
        for (int i = 0; i < empty_rules.Length(); i++)
            ordered_rules.Next() = empty_rules[i];
    }

    return;
}


//
//  FIRST_MAP takes as arguments two pointers, ROOT and TAIL, to a sequence
// of symbols in RHS which it inserts in FIRST_TABLE.  The vector FIRST_TABLE
// is used as the base for a hashed table where collisions are resolved by
// links.  Elements added to this hash table are allocated from the vector
// FIRST, with the variable TOP always indicating the position of the
// last element in FIRST that was allocated.
// NOTE: The suffix indentified by ROOT and TAIL is presumed not to be empty.
//       That is, ROOT <= TAIL !!!
//
int Base::FirstMap(int root, int tail)
{
    for (int i = first_table[grammar -> rhs_sym[root]]; i != NIL; i = first[i].link)
    {
        int j,
            k;
        for (j = root + 1,
             k = first[i].suffix_root + 1;
             (j <= tail && k <= first[i].suffix_tail);
             j++, k++)
        {
            if (grammar -> rhs_sym[j] != grammar -> rhs_sym[k])
                break;
        }
        if (j > tail && k > first[i].suffix_tail)
            return i;
    }

    int index = first.NextIndex();
    first[index].suffix_root = root;
    first[index].suffix_tail = tail;
    first[index].link = first_table[grammar -> rhs_sym[root]];
    first_table[grammar -> rhs_sym[root]] = index;

    return(index);
}


//
// SuffixFirst computes the set of all terminals that can appear as the first symbol
// in a rule suffix  and places the result in the FIRST set indexable by INDEX.
//
void Base::SuffixFirst(int index)
{
    int root = first[index].suffix_root,
        tail = first[index].suffix_tail,
        symbol = (root > tail ? grammar -> empty : grammar -> rhs_sym[root]);

    if (grammar -> IsTerminal(symbol))
    {
        first[index].set.SetEmpty();
        first[index].set.AddElement(symbol); // add it to set
    }
    else first[index].set = nt_first[symbol];

    for (int i = root + 1; i <= tail && first[index].set[grammar -> empty]; i++)
    {
        symbol = grammar -> rhs_sym[i];
        first[index].set.RemoveElement(grammar -> empty); // remove EMPTY
        if (grammar -> IsTerminal(symbol))
             first[index].set.AddElement(symbol); // add it to set
        else first[index].set += nt_first[symbol];
    }

    return;
}


//
// For a given symbol, complete the computation of
// PRODUCES[symbol].
//
void Base::ComputeProduces(int symbol)
{
    stack.Push(symbol);
    int indx = stack.Length();
    index_of[symbol] = indx;

    for (int i = 0; i < direct_produces[symbol].Length(); i++)
    {
        int new_symbol = direct_produces[symbol][i];
        if (index_of[new_symbol] == OMEGA)  // first time seen?
            ComputeProduces(new_symbol);
        index_of[symbol] = Util::Min(index_of[symbol], index_of[new_symbol]);
        produces[symbol] += produces[new_symbol];
    }

    if (index_of[symbol] == indx)  //symbol is SCC root
    {
        for (int new_symbol = stack.Top();
             new_symbol != symbol; new_symbol = stack.Top())
        {
            produces[new_symbol] = produces[symbol];
            index_of[new_symbol] = INFINITY_;
            stack.Pop();
        }

        index_of[symbol] = INFINITY_;
        stack.Pop();
    }

    return;
}


//
// COMPUTE_FOLLOW computes FOLLOW[nt] for some non-terminal NT using the
// digraph algorithm.  FOLLOW[NT] is the set of all terminals Ti that
// may immediately follow a string X generated by NT. I.e., if NT *::= X
// then X Ti is a valid substring of a class of strings that may be
// recognized by the language.
//
void Base::ComputeFollow(int nt)
{
    //
    // FOLLOW[NT] was initialized to 0 for all non-terminals.
    //
    stack.Push(nt);
    int indx = stack.Length();
    index_of[nt] = indx;

    //
    // iterate over all items of NT
    //
    for (int item_no = nt_items[nt]; item_no != NIL; item_no = next_item[item_no])
    {
        BitSet temp_set(first[item_table[item_no].suffix_index].set);
        if (temp_set[grammar -> empty])
        {
            temp_set.RemoveElement(grammar -> empty);
            int rule_no = item_table[item_no].rule_number,
                lhs_symbol = grammar -> rules[rule_no].lhs;
            if (index_of[lhs_symbol] == OMEGA)
                ComputeFollow(lhs_symbol);
            follow[nt] += follow[lhs_symbol];
            index_of[nt] = Util::Min( index_of[nt], index_of[lhs_symbol]);
        }
        follow[nt] += temp_set;
    }

    if (index_of[nt] == indx)
    {
        for (int lhs_symbol = stack.Top(); lhs_symbol != nt; lhs_symbol = stack.Top())
        {
            follow[lhs_symbol] = follow[nt];
            index_of[lhs_symbol] = INFINITY_;
            stack.Pop();
        }
        index_of[nt] = INFINITY_;
        stack.Pop();
    }

    return;
}


//
// Look for duplicate rules and report them.
//
void Base::CheckDuplicateRules(void)
{
    Tuple<int> list;
    for (int symbol = grammar -> FirstNonTerminal(); symbol <= grammar -> LastNonTerminal(); symbol++)
    {
        for (int j = 0; j < lhs_rule[symbol].Length(); j++)
        {
            int this_rule = lhs_rule[symbol][j]; // rule for which we are checking duplication
            for (int k = 0; k < j; k++)
            {
                int rule_no = lhs_rule[symbol][k];
                if (grammar -> RhsSize(rule_no) == grammar -> RhsSize(this_rule))
                {
                    int i1, i2;
                    for (i1 = grammar -> FirstRhsIndex(rule_no),
                         i2 = grammar -> FirstRhsIndex(this_rule);
                         i1 < grammar -> EndRhsIndex(rule_no);
                         i1++,
                         i2++)
                    {
                        if (grammar -> rhs_sym[i1] != grammar -> rhs_sym[i2])
                            break;
                    }

                    if (i1 == grammar -> EndRhsIndex(rule_no))
                    {
                        list.Next() = this_rule;
                        break; // once we've found a duplicate, no need to keep checking
                    }
                }
            }
        }
    }

    if (list.Length() > 0)
    {
        for (int i = 0; i < list.Length(); i++)
        {
            int rule_no = list[i],
                source_index = grammar -> rules[rule_no].source_index;
            option -> EmitError(grammar -> parser.rules[source_index].separator_index,
                                "This duplicate rule will cause reduce-reduce conflicts");
        }
        control -> Exit(12);
    }

    return;
}

void Base::PrintUnreachables(void)
{
    //
    // SYMBOL_LIST is used for two purposes:
    //  1) to mark symbols that are reachable from the Accepting
    //        non-terminal.
    //  2) to construct lists of symbols that are not reachable.
    //
    Array<int> symbol_list(grammar -> num_symbols + 1, Util::OMEGA);
    symbol_list[grammar -> eof_image] = NIL;
    symbol_list[grammar -> empty] = NIL;
    if (option -> error_maps)
        symbol_list[grammar -> error_image] = NIL;

    //
    // Initialize a list consisting only of the Accept non-terminal.
    // This list is a work pile of non-terminals to process as follows:
    // Each non-terminal in the front of the list is removed in turn and
    // 1) All terminal symbols in one of its right-hand sides are
    //     marked reachable.
    // 2) All non-terminals in one of its right-hand sides are placed
    //     in the the work pile of it had not been processed previously
    //
    int nt_root = grammar -> accept_image;
    symbol_list[nt_root] = NIL;
    for (int nt = nt_root; nt != NIL; nt = nt_root)
    {
        nt_root = symbol_list[nt];

        for (int k = 0; k < lhs_rule[nt].Length(); k++)
        {
            int rule_no = lhs_rule[nt][k];
            for (int i = grammar -> FirstRhsIndex(rule_no); i < grammar -> EndRhsIndex(rule_no); i++)
            {
                int symbol = grammar -> rhs_sym[i];
                if (grammar -> IsTerminal(symbol))
                    symbol_list[symbol] = NIL;
                else if (symbol_list[symbol] == OMEGA)
                {
                    symbol_list[symbol] = nt_root;
                    nt_root = symbol;
                }
            }
        }
    }

    //
    // We now iterate (backwards to keep things in order) over the
    // terminal symbols, and place each unreachable terminal in a
    // list. If the list is not empty, we signal that these symbols
    // are unused.
    //
    int t_root = NIL;
    {
        for (int symbol = grammar -> LastTerminal(); symbol >= grammar -> FirstTerminal(); symbol--)
            if (symbol_list[symbol] == OMEGA)
            {
                symbol_list[symbol] = t_root;
                t_root = symbol;
            }
    }

    {
        for (int symbol = t_root; symbol != NIL; symbol = symbol_list[symbol])
        {
            char tok[Control::SYMBOL_SIZE + 1];
            grammar -> RestoreSymbol(tok, grammar -> RetrieveString(symbol));

            Tuple<const char *> msg;
            msg.Next() = "The terminal ";
            msg.Next() = tok;
            msg.Next() = " is useless.";
            option -> EmitInformative(grammar -> RetrieveTokenLocation(symbol), msg);
        }
    }


    //
    // We now iterate (backward to keep things in order) over the
    // non-terminals, and place each unreachable non-terminal in a
    // list.
    //
    nt_root = NIL;
    {
        for (int symbol = grammar -> LastNonTerminal(); symbol >= grammar -> FirstNonTerminal(); symbol--)
            if (symbol_list[symbol] == OMEGA)
            {
                symbol_list[symbol] = nt_root;
                nt_root = symbol;
            }
    }

    //
    // If the list of unreachable nonterminals is not empty, we signal that
    //  each of these symbols is useless.
    //
    for (int symbol = nt_root; symbol != NIL; symbol = symbol_list[symbol])
    {
        char tok[Control::SYMBOL_SIZE + 1];
        grammar -> RestoreSymbol(tok, grammar -> RetrieveString(symbol));

        Tuple<const char *> msg;
        msg.Next() = "The nonterminal ";
        msg.Next() = tok;
        msg.Next() = " is useless.";
        option -> EmitWarning(grammar -> RetrieveTokenLocation(symbol), msg);
    }

    return;
}


//
// PRINT_XREF prints out the Cross-reference map. We build a map from each
// terminal into the set of items whose Dot-symbol (symbol immediately
// following the dot ) is the terminal in question.  Note that we iterate
// backwards over the rules to keep the rules associated with the items
// sorted, since they are inserted in STACK fashion in the lists:  Last-in,
// First out.
//
void Base::PrintXref(void)
{
    //
    // SORT_SYM is used to sort the symbols for cross_reference listing.
    //
    Array<int> sort_sym(grammar -> num_symbols + 1),
               t_items(grammar -> num_terminals + 1);
    t_items.Initialize(NIL);

    for (int rule_no = grammar -> LastRule(); rule_no >= grammar -> FirstRule(); rule_no--)
    {
        int item_no = first_item_of[rule_no];
        for (int i = grammar -> FirstRhsIndex(rule_no); i < grammar -> EndRhsIndex(rule_no); i++)
        {
            int symbol = grammar -> rhs_sym[i];
            if (grammar -> IsTerminal(symbol))
            {
                next_item[item_no] = t_items[symbol];
                t_items[symbol] = item_no;
            }
            item_no++;
        }
    }

    //
    // Sort the grammar symbols.
    //
    {
        for (int i = 1; i <= grammar -> num_symbols; i++)
            sort_sym[i] = i;
        QuickSortSymbols(sort_sym, 1, grammar -> num_symbols);
    }

    char line[Control::PRINT_LINE_SIZE*2 + 1],
         tok[Control::SYMBOL_SIZE*2 + 1];

    //
    // First, flush any data left in the report buffer.
    //
    option -> FlushReport();

    control -> PrintHeading();
    fprintf(option -> syslis, "\n\nCross-reference table:\n");
    for (int i = 1; i <= grammar -> num_symbols; i++)
    {
        int symbol = sort_sym[i];
        if (symbol != grammar -> accept_image && symbol != grammar -> eof_image
                                   && symbol != grammar -> empty)
        {
            fprintf(option -> syslis, "\n");
            grammar -> RestoreSymbol(tok, grammar -> RetrieveString(symbol));
            grammar -> PrintLargeToken(line, tok, "", Control::PRINT_LINE_SIZE-7);
            strcat(line, "  ==>> ");
            int offset = strlen(line) - 1,
                item_no;
            if (grammar -> IsNonTerminal(symbol))
            {
                for (int k = 0; k < lhs_rule[symbol].Length(); k++)
                {
                    int rule_no = lhs_rule[symbol][k];
                    IntToString num(rule_no);
                    if (num.Length() + strlen(line) > Control::PRINT_LINE_SIZE)
                    {
                        fprintf(option -> syslis, "\n%s", line);
                        strcpy(line, " ");
                        for (int j = 1; j <= offset; j++)
                             strcat(line, " ");
                    }
                    strcat(line, num.String());
                    strcat(line, " ");
                }
                fprintf(option -> syslis, "\n%s", line);
                item_no = nt_items[symbol];
            }
            else
            {
                for (item_no = t_items[symbol]; item_no != NIL; item_no = next_item[item_no])
                {
                    int rule_no = item_table[item_no].rule_number;
                    IntToString num(rule_no);
                    if (num.Length() + strlen(line) > Control::PRINT_LINE_SIZE)
                    {
                        fprintf(option -> syslis, "\n%s", line);
                        strcpy(line, " ");
                        for (int j = 1; j <= offset; j++)
                             strcat(line, " ");
                    }
                    strcat(line, num.String());
                    strcat(line, " ");
                }
                fprintf(option -> syslis, "\n%s",line);
            }
        }
    }
    fprintf(option -> syslis, "\n\n");

    return;
}


//
// QUICK_SYM takes as arguments an array of pointers whose elements point to
// nodes and two integer arguments: L, H. L and H indicate respectively the
// lower and upper bound of a section in the array.
//
void Base::QuickSortSymbols(Array<int> &array, int low, int high)
{
    Stack<int> lostack,
               histack;

    lostack.Push(low);
    histack.Push(high);

    while(! lostack.IsEmpty())
    {
        int lower = lostack.Pop(),
            upper = histack.Pop();

        while(upper > lower)
        {
            //
            // Split the array section indicated by LOWER and UPPER
            // using ARRAY[LOWER] as the pivot.
            //
            int i = lower,
                pivot = array[lower];
            for (int j = lower + 1; j <= upper; j++)
            {
                if (strcmp(grammar -> RetrieveString(array[j]),
                           grammar -> RetrieveString(pivot)) < 0)
                {
                    int temp = array[++i];
                    array[i] = array[j];
                    array[j] = temp;
                }
            }

            array[lower] = array[i];
            array[i] = pivot;

            if ((i - lower) < (upper - i))
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
// PRINT_NON_TERMINAL_FIRST prints the first set for each non-terminal.
//
void Base::PrintNonTerminalFirst(void)
{
    char line[Control::PRINT_LINE_SIZE + 1],
         tok[Control::SYMBOL_SIZE + 1];

    //
    // First, flush any data left in the report buffer.
    //
    option -> FlushReport();

    control -> PrintHeading();
    fprintf(option -> syslis, "\nFirst map for non-terminals:\n\n");

    for (int nt = grammar -> FirstNonTerminal(); nt <= grammar -> LastNonTerminal(); nt++)
    {
        grammar -> RestoreSymbol(tok, grammar -> RetrieveString(nt));
        grammar -> PrintLargeToken(line, tok, "", Control::PRINT_LINE_SIZE - 7);
        strcat(line, "  ==>> ");
        for (int t = grammar -> FirstTerminal(); t <= grammar -> LastTerminal(); t++)
        {
            if (nt_first[nt][t])
            {
                grammar -> RestoreSymbol(tok, grammar -> RetrieveString(t));
                if (strlen(line) + strlen(tok) > Control::PRINT_LINE_SIZE - 1)
                {
                    fprintf(option -> syslis, "\n%s", line);
                    grammar -> PrintLargeToken(line, tok, "    ", MAX_LENGTH);
                }
                else
                    strcat(line, tok);
                strcat(line, " ");
            }
        }
        fprintf(option -> syslis, "\n%s\n", line);
    }

    return;
}


//
//
//
void Base::PrintSymbolMap(const char *header, Array<BitSet> &map)
{
    char line[Control::PRINT_LINE_SIZE + 1],
         tok[Control::SYMBOL_SIZE + 1];

    //
    // First, flush any data left in the report buffer.
    //
    option -> FlushReport();

    control -> PrintHeading();
    fprintf(option -> syslis, "\n%s:\n\n", header);

    for (int symbol = grammar -> FirstTerminal(); symbol <= grammar -> LastNonTerminal(); symbol++)
    {
        grammar -> RestoreSymbol(tok, grammar -> RetrieveString(symbol));
        grammar -> PrintLargeToken(line, tok, "", Control::PRINT_LINE_SIZE - 7);
        strcat(line, "  ==>> ");
        for (int t = grammar -> FirstTerminal(); t <= grammar -> LastTerminal(); t++)
        {
            if (map[symbol][t])
            {
                grammar -> RestoreSymbol(tok, grammar -> RetrieveString(t));
                if (strlen(line) + strlen(tok) > Control::PRINT_LINE_SIZE - 1)
                {
                    fprintf(option -> syslis, "\n%s", line);
                    grammar -> PrintLargeToken(line, tok, "    ", MAX_LENGTH);
                }
                else
                    strcat(line, tok);
                strcat(line, " ");
            }
        }
        fprintf(option -> syslis, "\n%s\n", line);
    }

    return;
}


//
// PRINT_FOLLOW_MAP prints the follow map.
//
void Base::PrintFollowMap(void)
{
    //
    // First, flush any data left in the report buffer.
    //
    option -> FlushReport();

    control -> PrintHeading();
    fprintf(option -> syslis, "\nFollow Map:\n\n");
    for (int nt = grammar -> FirstNonTerminal(); nt <= grammar -> LastNonTerminal(); nt++)
    {
        char line[Control::PRINT_LINE_SIZE + 1],
             tok[Control::SYMBOL_SIZE + 1];

        grammar -> RestoreSymbol(tok, grammar -> RetrieveString(nt));
        grammar -> PrintLargeToken(line, tok, "", Control::PRINT_LINE_SIZE-7);
        strcat(line, "  ==>> ");
        for (int t = grammar -> FirstTerminal(); t <= grammar -> LastTerminal(); t++)
        {
            if (follow[nt][t])
            {
                grammar -> RestoreSymbol(tok, grammar -> RetrieveString(t));
                if (strlen(line) + strlen(tok) > Control::PRINT_LINE_SIZE-2)
                {
                    fprintf(option -> syslis, "\n%s", line);
                    grammar -> PrintLargeToken(line, tok, "    ", MAX_LENGTH);
                }
                else
                    strcat(line, tok);
                strcat(line, " ");
            }
        }
        fprintf(option -> syslis, "\n%s\n", line);
    }
    return;
}


//
// This procedure is invoked when LALR_LEVEL > 1 to construct the
// RMPSELF set which identifies the nonterminals that can right-most
// produce themselves. It takes as argumen the map PRODUCES which
// identifies for each nonterminal the set of nonterminals that it can
// right-most produce.
//
void Base::InitRmpself(BoundedArray<BitSetWithOffset> &produces)
{
    rmpself.Resize(grammar -> num_terminals + 1, grammar -> num_symbols);

    //
    // Note that each element of the map produces is a boolean vector
    // that is indexable in the range 1..num_nonterminals. Since each
    // nonterminal is offset by the value num_terminals (to distinguish
    // it from the terminals),it must therefore be adjusted accordingly
    // when dereferencing an element in the range of the produces map.
    //
    for (int nt = grammar -> FirstNonTerminal(); nt <= grammar -> LastNonTerminal(); nt++)
        rmpself[nt] = produces[nt][nt];

    return;
}
