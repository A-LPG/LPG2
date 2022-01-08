#ifndef TTC_INCLUDED
#define TTC_INCLUDED

#include "tuple.h"
#include "grammar.h"

//
// TTC (Terminal transitive closure)
//
// A symbol is identified as terminal if either:
//     1. It is a terminal symbol
//     2. It is a nonterminal that only produces a single terminal 
//        in each of its rules.
//
class TTC
{
    Array<bool> is_terminal;
    Array<int> index_of;
    Stack<int> stack;

    BoundedArray< Tuple<int> > &global_map;
    Tuple< Tuple<ProcessedRuleElement> > &processed_rule_map;

    //
    // ComputeClosure computes closure[nt] for some non-terminal NT using the
    // digraph algorithm.
    //
    void ComputeClosure(int nt)
    {
        stack.Push(nt);
        int indx = stack.Length();
        index_of[nt] = indx;

        is_terminal[nt] = true;

        for (int i = 0; i < global_map[nt].Length(); i++)
        {
            int rule_no = global_map[nt][i];
            if (processed_rule_map[rule_no].Length() != 1)
                is_terminal[nt] = false;
            else
            {
                int dependent = processed_rule_map[rule_no][0].image;

                if (index_of[dependent] == Util::OMEGA)
                    ComputeClosure(dependent);
                is_terminal[nt] = is_terminal[nt] && is_terminal[dependent];
                index_of[nt] = Util::Min(index_of[nt], index_of[dependent]);
            }
        }

        if (index_of[nt] == indx)
        {
            for (int dependent = stack.Top(); dependent != nt; dependent = stack.Top())
            {
                is_terminal[dependent] = is_terminal[nt];
                index_of[dependent] = Util::INFINITY_;
                stack.Pop();
            }

            index_of[nt] = Util::INFINITY_;
            stack.Pop();
        }

        return;
    }

public:

    TTC(BoundedArray< Tuple<int> > &global_map_, Tuple< Tuple<ProcessedRuleElement> > &processed_rule_map_)
        : is_terminal(global_map_.Ubound() + 1),
          index_of(global_map_.Ubound() + 1, Util::OMEGA),
          global_map(global_map_),
          processed_rule_map(processed_rule_map_)
    {
        //
        // Symbol 0 is associated with the root type and is never
        // considered to be a terminal
        //
        is_terminal[0] = false;
        index_of[0] = Util::INFINITY_;

        //
        // Identify each terminal symbol as a terminal.
        //
        for (int t = 1; t < global_map_.Lbound(); t++)
        {
            is_terminal[t] = true;
            index_of[t] = Util::INFINITY_;
        }

        //
        // Calculate which nonterminal symbols only produce terminal singletons.
        //
        for (int nt = global_map.Lbound(); nt <= global_map.Ubound(); nt++)
        {
            if (index_of[nt] == Util::OMEGA)
                ComputeClosure(nt);
        }
    }

    bool IsTerminalSymbol(int i) { return is_terminal[i]; }
};


#endif /* TTC_INCLUDED */
