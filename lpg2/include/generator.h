#ifndef GENERATOR_INCLUDED
#define GENERATOR_INCLUDED

#include "tuple.h"
#include "control.h"

class Base;
class Table;

class Generator
{
protected:

    enum {
             SHIFT_TABLE_SIZE = 401 // 401 is a prime
         };

    class ActionElement
    {
    public:
        int count,
            action;
    };

    struct NewStateElement
    {
        Dfa::ReduceHeader reduce;
        Tuple<int> reduce_range;
        int shift_number,
            link,
            thread,
            image;
    };

    class NewStateType : public Tuple<NewStateElement>
    {
    public:
        inline int NextIndex()
        {
            return Tuple<NewStateElement>::NextIndex();
        }
        inline NewStateElement& Next() { int i = NextIndex(); return base[i >> log_blksize][i]; }

        NewStateType(int size) : Tuple<NewStateElement>(size)
        {}
    } new_state_element;

    Array<int> shift_image,
               real_shift_number,
               term_state_index,
               shift_check_index;

    int shift_domain_count,
        num_terminal_states,
        check_size,
        term_check_size,
        term_action_size,
        shift_check_size;

    BoundedArray<int> default_goto;
    Array<int> default_shift;

    int top,
        empty_root,
        single_root,
        multi_root;

    Array<int> terminal_row_size,
               terminal_frequency_symbol,
               terminal_frequency_count;

    Array<bool> shift_on_error_symbol;

    Tuple<int> nullable_nonterminals;

    Tuple<int> next,
               previous;

    Array<int> symbol_map,
               state_index,
               ordered_state,
               state_list;

    int num_entries,
        table_size,
        action_size,
        minimum_increment,
        first_index,
        last_index,
        last_symbol,
        accept_act,
        error_act;

    Array<BitSetWithOffset> naction_symbols;
    Array<BitSet> action_symbols;

    static const int MAX_TABLE_SIZE;

    void process_shift_actions(Array< Tuple<ActionElement> > &, int);
    void compute_default_shift(void);
    void compute_default_goto(void);

    void compute_action_symbols_range(Array<int> &, Array<int> &, Array<int> &, Array<int> &);
    void compute_naction_symbols_range(Array<int> &, Array<int> &, Array<int> &, Array<int> &);

    void remap_non_terminals(void);
    void overlap_nt_rows(void);
    void merge_similar_t_rows(void);
    void merge_shift_domains(void);
    int overlay_sim_t_rows(void);
    void overlap_t_rows(int);

    void InitializeTables(int, int);
    void Reallocate(int);
    int InsertRow(Tuple<int> &, int, int);

    Control *control;
    Pda *pda;
    Option *option;
    LexStream *lex_stream;
    Grammar *grammar;
    Base *base;

public:

    void Process(void);
    void Generate(Table *);

    Generator(Control *control_, Pda *pda_) : new_state_element(pda_ -> max_la_state + 1),
                                              shift_on_error_symbol(pda_ -> max_la_state + 1, false),
                                              next(12, 256),
                                              previous(12, 256),
                                              control(control_),
                                              pda(pda_),
                                              option(control_ -> option),
                                              lex_stream(control_ -> lex_stream),
                                              grammar(control_ -> grammar),
                                              base(pda_ -> base)
    {
        next.Resize(32768);
        previous.Resize(32768);

        shift_check_size = 0;

        //
        // Construct list of nullable terminals
        //
        for (int symbol = grammar -> FirstNonTerminal(); symbol <= grammar -> LastNonTerminal(); symbol++)
            if (base -> IsNullable(symbol)) nullable_nonterminals.Next() = symbol;
    }
};

#endif
