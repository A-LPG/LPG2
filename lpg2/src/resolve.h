#ifndef RESOLVE_INCLUDED
#define RESOLVE_INCLUDED

#include "tuple.h"
#include "control.h"

class Resolve : public Util
{
    //
    // VISITED is a structure used to mark state-symbol pairs that have
    // been visited in the process of computing follow-sources for a
    // given action in conflict.
    // The field MAP is an array indexable by the states 1..NUM_STATES;
    // each element of which points to a set (list) of symbols. Thus, for
    // a given state S, and for all symbol X in the list MAP[S], [S,X]
    // has been visited. For efficiency, the fields LIST and ROOT are used
    // to store the set (list) of indexed elements of MAP that are not
    // NULL.
    // See routines MARK_VISITED, WAS_VISITED, CLEAR_VISITED,
    //              INIT_LALRK_PROCESS, EXIT_PROCESS
    //
    class VisitedElement
    {
    public:
        Array<Tuple <int> > map;
        Array<int> list;
        int root;

        VisitedElement(int num_states) : map(num_states + 1),
                                         list(num_states + 1),
                                         root(Util::NIL)
        {}
    };
    VisitedElement visited;

    //
    // Given a set of actions that are in conflict on a given symbol, the
    // structure SOURCES_ELEMENT is used to store a mapping from each
    // such action into a set of configurations that can be reached
    // following execution of the action in question up to the point where
    // the automaton is about to shift the conflict symbol.
    // The field CONFIGS is an array indexable by actions which are
    // encoded as follows:
    //    1. shift-reduce  [-NUM_RULES..-1]
    //    2. reduce        [0..NUM_RULES]
    //    3. shift         [NUM_RULES+1..NUM_STATES+1].
    // Each element of CONFIGS points to a set (sorted list) of
    // configurations.  For efficiency, the fields LIST and ROOT are used
    // to store the set (list) of indexed elements of CONFIGS that are not
    // NULL.
    // See routines ADD_CONFIGS, UNION_CONFIG_SETS.
    // See STATE_TO_RESOLVE_CONFLICTS for an explanation of STACK_SEEN.
    //
    // A configuration is a stack of states that represents a certain path
    // in the automaton. The stack is implemented as a list of
    // STACK_ELEMENT nodes linked through the field PREVIOUS.
    // A set/list of configurations is linked through the field NEXT.
    // When attempting to resolve conflicts we try to make sure that the
    // set of configurations associated with each action is unique. This
    // is achieved by throwing these configurations into a set and making
    // sure that there are no duplicates. The field LINK is used for that
    // purpose (see routine STACK_WAS_SEEN). The field STATE_NUMBER is
    // obviously used to store the number of a state in the automaton. The
    // field SIZE holds index of the node within the stack. Thus, for the
    // first element of the stack this field represents the number of
    // elements in the stack; for the last element, this field holds the
    // value 1.
    // See routines ALLOCATE_STACK_ELEMENT, FREE_STACK_ELEMENT,
    //              ADD_DANGLING_STACK, FREE_DANGLING_STACK.
    //
    class SourcesElement;
public: // Because of bug in AIX IBM compiler, we have to make this class public
    struct StackElement
    {
        StackElement *previous,
                     *next,
                     *link;
        int state_number,
            size;
        friend class SourcesElement;
    };
private:
    Tuple<StackElement> stack_element_pool;

    class SourcesElement
    {
    public:
        enum { STATE_TABLE_SIZE = 1021 /* 1021 is a prime */ };

        BoundedArray<StackElement *> configs;
        Array<StackElement *> stack_seen;
        BoundedArray<int> list;
        int root;

        SourcesElement(int num_rules, int num_states)
        {
            configs.Resize(-num_rules, num_rules + num_states + 1);
            configs.MemReset();

            stack_seen.Resize(STATE_TABLE_SIZE);
            stack_seen.MemReset();

            list.Resize(-num_rules, num_rules + num_states + 1);
            root = Util::NIL;
        }

        void Clear()
        {
            for (int act = root; act != Util::NIL; act = list[act])
                configs[act] = NULL;
            root = Util::NIL;
            return;
        }
    };
    SourcesElement sources;

    //
    // The structures SR_CONFLICT_ELEMENT and RR_CONFLICT_ELEMENT are used
    // th store conflict information. CONFLICT_ELEMENT_POOL is used to
    // keep track of a pool conflict element structures (SR or RR) that
    // are available for allocation.
    // See routines ALLOCATE_CONFLICT_ELEMENT and FREE_CONFLICT_ELEMENTS.
    //
    struct SrConflictElement
    {
        int state_number,
            item,
            symbol;
    };
    Tuple<SrConflictElement> sr_conflicts,
                             soft_sr_conflicts;

    struct RrConflictElement
    {
        int symbol,
            item1,
            item2;
    };
    Tuple<RrConflictElement> rr_conflicts,
                             soft_rr_conflicts;

    struct SsConflictElement
    {
        int state_number,
            symbol;
    };
    Tuple<SsConflictElement> soft_ss_conflicts;

    //
    // NT_ITEMS and ITEM_LIST are used to construct a mapping from each
    // nonterminal into the set of items of which the nonterminal in
    // question is the dot symbol. See CONFLICTS_INITIALIZATION.
    //
    BoundedArray<int> nt_items;
    Array<int> item_list;

    //
    // LALR_VISITED is used to keep track of (state, nonterminal) pairs
    // that are visited in tracing the path of a lalr conflict. SLR_VISITED
    // is similarly used to keep track of nonterminal symbols that are
    // visited in tracing the path of an slr conflict. SYMBOL_SEEN is used
    // to keep track of nonterminal symbols that are visited in tracing a
    // path to the start state (root).
    //
    Array<bool> lalr_visited;
    BoundedArray<bool> slr_visited,
                       symbol_seen;

    Stack<int> stack;
    Array<int> index_of;

    StackElement *allocate_stack_element(void);
    StackElement *union_config_sets(StackElement *, StackElement *);
    void add_configs(SourcesElement &, int, StackElement *);
    void clear_visited(void);
    bool was_visited(int, int);
    void mark_visited(int, int);
    void compute_cyclic(int);
    bool trace_root(int);
    void print_root_path(int);
    bool lalr_path_retraced(int, int, int);
    void print_relevant_lalr_items(int, int, int);
    bool slr_trace(int, int);
    void print_relevant_slr_items(int, int);
    void ConflictsInitialization(void);
    void process_conflicts(int);
    StackElement *follow_sources(StackElement *, int, int);
    void next_la(StackElement *, int, BitSet &);
    bool stack_was_seen(Array<StackElement *> &, StackElement *);
    int state_to_resolve_conflicts(SourcesElement &, int, int);
    void clear_action(Array<Node *> &, Tuple<int> &);

    Control *control;
    Option *option;
    Grammar *grammar;
    NodePool *node_pool;
    Pda *pda;
    Base *base;

public:

    Resolve(Control *control_, Pda *pda_) : visited(pda_ -> num_states),
                                            sources(control_ -> grammar -> num_rules, pda_ -> num_states),
                                            control(control_),
                                            option(control_ -> option),
                                            grammar(control_ -> grammar),
                                            node_pool(control_ -> node_pool),
                                            pda(pda_),
                                            base(pda_ -> base)
    {
        //
        // Allocate and compute the CYCLIC vector which identifies
        // states that can enter a cycle via transitions on nullable
        // nonterminals. If such a cyle exists then the grammar is
        // not LR(k) for any k.
        //
        cyclic.Resize(pda -> num_states + 1);
        index_of.Resize(pda -> num_states + 1);
        index_of.Initialize(Util::OMEGA);
        for (int state_no = 1; state_no <= pda -> num_states; state_no++)
        {
            if (index_of[state_no] == Util::OMEGA)
                compute_cyclic(state_no);
        }

        return;
    }

    //
    // lalr_priority is used in a similar fashion as lalr_visited 
    // to keep track of priorities for conflicts.
    //
    Array<int> lalr_priority;

    void PriorityRuleTrace(int, int, int);
    int ComputeReducePriority(int, int, int);
    int ComputeShiftPriority(int, int, int);

    //
    // CYCLIC is a boolean vector used to identify states that can enter
    // a cycle of transitions on nullable nonterminals.
    // As the computation of CYCLIC requires a modified version of the
    // digraph algorithm, the variables STACK and INDEX_OF are used
    // for that algorithm.
    //
    Array<bool> cyclic;

    void ResolveConflicts(int, Array< Tuple<int> > &, Array<int> &, int);
    void ResolveConflictsAndAddSoftActions(int, Array< Tuple<int> > &, Array<int> &, int, Array<int> &);
    int  ResolveIdentifierConflicts(int, Array<int> &, Array< Tuple<int> > &);
    void ResolveShiftReduceConflicts(int, int, int, Array< Tuple<int> > &);
    void ResolveReduceReduceConflicts(int, int, Array< Tuple<int> > &);
    void ResolveKeywordIdentifierShiftReduceConflicts(int, int, int, int, Array< Tuple<int> > &);
    void ResolveKeywordIdentifierReduceReduceConflicts(int, int, int, Array< Tuple<int> > &);
    void CreateLastats(void);
};
#endif
