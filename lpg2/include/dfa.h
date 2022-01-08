#ifndef DFA_INCLUDED
#define DFA_INCLUDED

#include "node.h"
#include "tuple.h"
#include "util.h"

class Control;
class Base;
class Dfa : public Util
{
public:
    class ShiftPair
    {
        int symbol,
            action;
    public:
        int Symbol() { return symbol; }
        int Action() { return action; }

        void SetSymbol(int symbol_) { symbol = symbol_; }
        void SetAction(int action_) { action = action_; }
    };
    class ShiftHeader : public Tuple<ShiftPair>
    {
    public:
        int Index(int symbol)
        {
            for (int i = 0; i < Length(); i++)
            {
                if ((*this)[i].Symbol() == symbol)
                    return i;
            }
            return Util::OMEGA;
        }
        int Action(int symbol)
        {
            for (int i = 0; i < Length(); i++)
            {
                if ((*this)[i].Symbol() == symbol)
                    return (*this)[i].Action();
            }
            return Util::OMEGA;
        }
    };

    class GotoTriplet
    {
        int laptr,
            symbol,
            action;
    public:
        int Symbol() { return symbol; }
        int Action() { return action; }
        int Laptr()  { return laptr; }

        void SetSymbol(int symbol_) { symbol = symbol_; }
        void SetAction(int action_) { action = action_; }
        void SetLaptr(int laptr_)   { laptr = laptr_; }
    };
    class GotoHeader : public Tuple<GotoTriplet>
    {
    public:
        int Index(int symbol)
        {
            for (int i = 0; i < Length(); i++)
            {
                if ((*this)[i].Symbol() == symbol)
                    return i;
            }
            return Util::OMEGA;
        }
        int Action(int symbol)
        {
            for (int i = 0; i < Length(); i++)
            {
                if ((*this)[i].Symbol() == symbol)
                    return (*this)[i].Action();
            }
            return Util::OMEGA;
        }
        void RemoveDefaults(BoundedArray<int> &goto_default)
        {
            int last_index = this -> Length() - 1;
            for (int i = last_index; i >= 0; i--)
            {
                //
                // If the action matches the default, remove it
                //
                if (goto_default[(*this)[i].Symbol()] == (*this)[i].Action())
                {
                    (*this)[i].SetSymbol((*this)[last_index].Symbol());
                    (*this)[i].SetAction((*this)[last_index].Action());
                    Reset(last_index);
                    last_index--;
                }
            }

            return;
        }
    };

    class Reduce
    {
        int symbol,
            rule_number;
    public:
        int Symbol() { return symbol; }
        int RuleNumber() { return rule_number; }

        void SetSymbol(int symbol_) { symbol = symbol_; }
        void SetRuleNumber(int rule_number_) { rule_number = rule_number_; }
    };
    class ReduceHeader : public Tuple<Reduce>
    {
    public:
        int Index(int symbol)
        {
            for (int i = 0; i < Length(); i++)
            {
                if ((*this)[i].Symbol() == symbol)
                    return i;
            }
            return Util::OMEGA;
        }
        int RuleNumber(int symbol)
        {
            for (int i = 0; i < Length(); i++)
            {
                if ((*this)[i].Symbol() == symbol)
                    return (*this)[i].RuleNumber();
            }
            return Util::OMEGA;
        }
    };

    class Conflict
    {
        int symbol,
            conflict_index;
    public:
        int Symbol() { return symbol; }
        int ConflictIndex() { return conflict_index; }

        void SetSymbol(int symbol_) { symbol = symbol_; }
        void SetConflictIndex(int conflict_index_) { conflict_index = conflict_index_; }
    };
    class ConflictHeader : public Tuple<Conflict>
    {
    public:
        int Index(int symbol)
        {
            for (int i = 0; i < Length(); i++)
            {
                if ((*this)[i].Symbol() == symbol)
                    return i;
            }
            return Util::OMEGA;
        }
        int ConflictIndex(int symbol)
        {
            for (int i = 0; i < Length(); i++)
            {
                if ((*this)[i].Symbol() == symbol)
                    return (*this)[i].ConflictIndex();
            }
            return Util::OMEGA;
        }
    };

    struct ConflictCell
    {
        int action,
            priority;
    };

    struct StateElement
    {
        Node *kernel_items,
             *complete_items,
             *single_production_items;
        Tuple<int> predecessors;
        GotoHeader   go_to;
        ReduceHeader reduce;
        ConflictHeader conflict;
        Tuple<int> soft_keywords;
        int shift_number,
            transition_symbol;
    };

    Tuple<StateElement> statset;
    Tuple<ShiftHeader> shift;
    Tuple<int> conflicts;

    Control *control;
    Option *option;
    Grammar *grammar;
    NodePool *node_pool;
    Base *base;

    int num_states,
        num_shift_maps,
        num_shifts,
        num_shift_reduces,
        num_gotos,
        num_goto_reduces;

    Dfa(Control *, Base *);

    void Process(void);

    ShiftHeader &Shift(int state_no) { return shift[statset[state_no].shift_number]; }
    int UpdateShiftMaps(Tuple<int> &, Array<int> &);
    void ResetShiftMap(int, Tuple<int> &, Array<int> &);
    void Access(Tuple<int> &, int, int);

    void PrintItem(int);

    //
    //
    //
    void AddPredecessor(int next_state_no, int state_no)
    {
        statset[next_state_no].predecessors.Next() = state_no;
    }

    void QuickSort(Tuple<ConflictCell> &);

    int MapConflict(Tuple <ConflictCell> &);
    void RemapConflict(int, Tuple<ConflictCell> &);

protected:

    void BuildPredecessorsMap(void);

private:

    enum {
             SHIFT_TABLE_SIZE = 401, /* 401 is a prime  */
             STATE_TABLE_SIZE = 1021, /* 1021 is a prime */
             CONFLICT_TABLE_SIZE = 1021 /* 1021 is a prime */
         };

    Array<int> shift_table;
    Tuple<int> next_shift;

    class ConflictListElement
    {
    public:
        int index,
            next;
    };
    Array<int> conflict_table;
    Tuple<ConflictListElement> conflict_element;

    void MakeLr0(void);
    int lr0_state_map(Array<int> &, Tuple<int> &, Node *);
};

#endif
