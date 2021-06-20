#ifndef PlxTable_INCLUDED
#define PlxTable_INCLUDED

#include "table.h"

class PlxTable : public Table
{
protected:

    enum
    {
        PARSER_LINE_SIZE = 80
    };

    Array<const char *> type_name,
                        array_name;

    virtual TypeId Type(int min, int max)
    {
        if (min >= SHRT_MIN && max <= SHRT_MAX)
            return I16;
        return I32;
    }

    UnbufferedTextFile dcl_buffer,
                       imp_buffer;

    void Print(const char *, int, Array<int> &, int, int);
    void non_terminal_action(void);
    void non_terminal_no_goto_default_action(void);
    void terminal_action(void);
    void terminal_shift_default_action(void);
    void init_file(FILE **, const char *);
    void init_parser_files(void);
    void exit_parser_files(void);
    void PrintSemanticFunctionsMap(void);
    void PrintNames(void);
    void print_symbols(void);
    void print_exports(void);
    void print_definitions(void);
    void print_externs(void);
    void print_tables(void);

public:

    PlxTable(Control *control_, Pda *pda_) : Table(control_, pda_),
                                             type_name(num_type_ids),
                                             array_name(num_name_ids),
                                             dcl_buffer(&sysdcl),
                                             imp_buffer(&sysimp)
    {
        type_name[B] = "BIT(1)";
        type_name[U8] = type_name[I8] = type_name[I16] = "FIXED BIN(15) SIGNED";
        type_name[U16] = type_name[I32] = "FIXED BIN(31) SIGNED";

        array_name[NULLABLES] = "is_nullable";
        array_name[PROSTHESES_INDEX] = "prostheses_index";
        array_name[KEYWORDS] = "is_keyword";
        array_name[BASE_CHECK] = "base_check";
        array_name[BASE_ACTION] = "base_action";
        array_name[TERM_CHECK] = "term_check";
        array_name[TERM_ACTION] = "term_action";
        array_name[DEFAULT_GOTO] = "default_goto";
        array_name[DEFAULT_REDUCE] = "default_reduce";
        array_name[SHIFT_STATE] = "shift_state";
        array_name[SHIFT_CHECK] = "shift_check";
        array_name[DEFAULT_SHIFT] = "default_shift";
        array_name[ACTION_SYMBOLS_BASE] = "asb";
        array_name[ACTION_SYMBOLS_RANGE] = "asr";
        array_name[NACTION_SYMBOLS_BASE] = "nasb";
        array_name[NACTION_SYMBOLS_RANGE] = "nasr";
        array_name[TERMINAL_INDEX] = "terminal_index";
        array_name[NONTERMINAL_INDEX] = "non_terminal_index";
        array_name[SCOPE_PREFIX] = "scope_prefix";
        array_name[SCOPE_SUFFIX] = "scope_suffix";
        array_name[SCOPE_LHS_SYMBOL] = "scope_lhs";
        array_name[SCOPE_LOOK_AHEAD] = "scope_la";
        array_name[SCOPE_STATE_SET] = "scope_state_set";
        array_name[SCOPE_RIGHT_SIDE] = "scope_rhs";
        array_name[SCOPE_STATE] = "scope_state";
        array_name[IN_SYMB] = "in_symb";
        array_name[NAME_START] = "name_start";
   }

    virtual ~PlxTable() {}

    virtual void PrintTables(void);
};

#endif /* PlxTable_INCLUDED */
