#ifndef SP_INCLUDED
#define SP_INCLUDED

#include "control.h"

class Sp
{
    enum { STATE_TABLE_SIZE = 1021 /* 1021 is a prime */ };

    struct ConflictLocation
    {
        int state,
            conflict_index;
    };

    struct UpdateActionElement
    {
        int symbol,
            action,
            state;
    };

    Array< Tuple <UpdateActionElement> > update_action;

    struct ActionElement
    {
        int symbol,
            action;
    };

    Array< Tuple<ActionElement> > new_action;

    struct SpStateElement
    {
        int link;
        Tuple<ActionElement> action;
        Tuple<int> rule;
        Node *complete_items;
    };
    Tuple<SpStateElement> sp_state;

    Array< Tuple<int> > sp_action;

    Stack<int> stack;
    Array<int> index_of,
               sp_rules,
               next_rule,
               sp_table;

    Tuple<int> rule_list;

    BitSet is_sp_rule,
           is_conflict_symbol;

    Control *control;
    Option *option;
    Grammar *grammar;
    NodePool *node_pool;
    Pda *pda;
    Base *base;

    bool IsSingleProductionRhs(int symbol)   { return sp_rules[symbol] != Util::NIL; }
    bool IsSingleProductionRule(int rule_no) { return is_sp_rule[rule_no]; }

    int SourceState(int state_no)
    {
        if (state_no <= pda -> num_states)
            return state_no;
        Dfa::StateElement &state_element = pda -> statset[state_no];
        return (state_element.predecessors.Length() == 0 // an unreachable look-ahead state
                                                     ? Util::OMEGA
                                                     : SourceState(state_element.predecessors[0])); 
    }

    void ComputeSpMap(int);
    void ComputeSingleProductionAction(int, int, int);
    int DefaultAction(int, int);
    int NonterminalAction(int, int, int);
    int GreatestCommonAncestor(Dfa::Reduce &, int, int, UpdateActionElement &);
    void ComputeUpdateActions(int, int, int);
    int StateMap(int, int, int, int, int);
    void AddSingleProductionStates();
    void UpdateBaseAutomaton();
    void UpdateConflictActions();
    void UpdateLookaheadAutomata();
    void MoveLaStates();

public:

    Sp(Control *control_, Pda *pda_) : control(control_),
                                       option(control_ -> option),
                                       grammar(control_ -> grammar),
                                       node_pool(control_ -> node_pool),
                                       pda(pda_),
                                       base(pda_ -> base)
    {}

    void RemoveSingleProductions(void);
};
#endif
