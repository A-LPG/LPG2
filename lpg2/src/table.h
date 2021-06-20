#ifndef TABLE_INCLUDED
#define TABLE_INCLUDED

#include "tuple.h"
#include "set.h"
#include "buffer.h"
#include "control.h"

class Table
{
protected:
    friend class Generator;

    enum TypeId
    {
        B,
        I8,
        U8,
        I16,
        U16,
        I32,

        num_type_ids
    };

    enum NameId
    {
        NULLABLES,
        PROSTHESES_INDEX,
        KEYWORDS,
        BASE_CHECK,
        BASE_ACTION,
        TERM_CHECK,
        TERM_ACTION,
        DEFAULT_GOTO,
        DEFAULT_REDUCE,
        SHIFT_STATE,
        SHIFT_CHECK,
        DEFAULT_SHIFT,
        ACTION_SYMBOLS_BASE,
        ACTION_SYMBOLS_RANGE,
        NACTION_SYMBOLS_BASE,
        NACTION_SYMBOLS_RANGE,
        TERMINAL_INDEX,
        NONTERMINAL_INDEX,
        SCOPE_PREFIX,
        SCOPE_SUFFIX,
        SCOPE_LHS_SYMBOL,
        SCOPE_LOOK_AHEAD,
        SCOPE_STATE_SET,
        SCOPE_RIGHT_SIDE,
        SCOPE_STATE,
        IN_SYMB,
        NAME_START,
        SYMBOL_START,

        num_name_ids
    };

    enum { MAX_PARM_SIZE = 22 };

    Array<int> symbol_map;

    class IntArrayInfo
    {
    public:
        NameId name_id;
        TypeId type_id;
        int num_bytes;
        Array<int> array;
    };
    Tuple<IntArrayInfo> data;

    Array<int> follow_base,
               follow_range,
               shift_states_base,
               shift_states_range,
               goto_states_base,
               goto_states_range,
               sorted_state,
               original_state;

    IntArrayInfo name_start;
    Array<const char *> name_info;

    IntArrayInfo symbol_start;
    Array<const char *> symbol_info;

    void initialize(Array<const char *> &input, NameId id, IntArrayInfo &start, Array<const char *> & info, int &max_length)
    {
        start.array.Resize(input.Size() + 1);
        info.Resize(input.Size());
        start.array[0] = 1;
        max_length = 0;
        for (int k = 0; k < info.Size(); k++)
        {
            info[k] = input[k];
            int length = strlen(info[k]);
            start.array[k + 1] = start.array[k] + length;
            if (max_length < length)
                max_length = length;
        }
        start.name_id = id;
        start.type_id = Type(0, start.array[start.array.Size() - 1]);
    }

    inline int Length(IntArrayInfo &start, int i) { return start.array[i + 1] - start.array[i]; }

    virtual TypeId Type(int min, int max)
    {
        if (min < 0 && min >= SCHAR_MIN && max <= SCHAR_MAX)
            return I8;
        if (min >= 0 && max <= UCHAR_MAX)
           return U8;
        if (min < 0 && min >= SHRT_MIN && max <= SHRT_MAX)
            return I16;
        if (min >= 0 && max <= USHRT_MAX)
            return U16;
        return I32;
    }

    TypeId Type(Array<int> &array)
    {
        int min = 0,
            max = min;
        if (array.Size() > 0)
        {
            min = array[0],
            max = min;
            for (int i = 1; i < array.Size(); i++)
            {
                if (array[i] > max)
                    max = array[i];
                else if (array[i] < min)
                     min = array[i];
            }
        }

        return Type(min, max);
    }

    Control *control;
    Option *option;
    LexStream *lex_stream;
    Grammar *grammar;
    Pda *pda;

    FILE *systab,
         *sysdat,
         *syssym,
         *sysimp,
         *sysdcl,
         *sysdef,
         *sysexp,
         *sysprs;

    int start_state,
        accept_act,
        error_act,
        max_name_length,
        max_symbol_length,
        last_symbol,

        conflict_count,
        default_count,
        goto_count,
        goto_reduce_count,
        reduce_count,
        la_shift_count,
        shift_count,
        shift_reduce_count;

public:
    Table(Control *control_, Pda *pda_) : control(control_),
                                          option(control_ -> option),
                                          lex_stream(control_ -> lex_stream),
                                          grammar(control_ -> grammar),
                                          pda(pda_),

                                          systab(NULL),
                                          sysdat(NULL),
                                          syssym(NULL),
                                          sysimp(NULL),
                                          sysdcl(NULL),
                                          sysdef(NULL),
                                          sysexp(NULL),
                                          sysprs(NULL),

                                          conflict_count(0),
                                          default_count(0),
                                          goto_count(0),
                                          goto_reduce_count(0),
                                          reduce_count(0),
                                          la_shift_count(0),
                                          shift_count(0),
                                          shift_reduce_count(0)
    {}

    virtual ~Table() { CloseFiles(); }

    void CloseFiles()
    {
        if (systab != NULL) fclose(systab);
        if (sysdat != NULL) fclose(sysdat);
        if (syssym != NULL) fclose(syssym);
        if (sysimp != NULL) fclose(sysimp);
        if (sysdcl != NULL) fclose(sysdcl);
        if (sysdef != NULL) fclose(sysdef);
        if (sysexp != NULL) fclose(sysexp);
        if (sysprs != NULL) fclose(sysprs);
    }

    void Exit(int code)
    {
        CloseFiles();       
        control -> Exit(code);
    }

    virtual void PrintTables(void) = 0;
    void PrintReport(void);
    void PrintStateMaps();
};

#endif
