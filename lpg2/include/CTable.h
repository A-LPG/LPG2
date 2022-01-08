#ifndef CTable_INCLUDED
#define CTable_INCLUDED

#include "table.h"

class CTable : public Table
{
protected:

    enum
    {
        PARSER_LINE_SIZE = 80
    };

    char parse_table_class_name[Control::SYMBOL_SIZE + sizeof("_table")],
         parse_table_class_name_prefix[Control::SYMBOL_SIZE + sizeof("_table::")];

    Array<const char *> type_name,
                        array_name;

    UnbufferedTextFile dcl_buffer;

    inline void Print(IntArrayInfo &array_info)
    {
        const char *name = array_name[array_info.name_id];
        Print(name, array_info.type_id, array_info.array);
    }
    void Print(const char *, int, Array<int> &);
    void non_terminal_action(void);
    void non_terminal_no_goto_default_action(void);
    void terminal_action(void);
    void terminal_shift_default_action(void);
    void init_file(FILE **, const char *, const char *, bool = true);
    void init_parser_files(void);
    void exit_parser_files(void);
    void PrintNames(void);
    void print_symbols(void);
    void print_exports(void);
    void print_definitions(void);
    void print_externs_and_definitions(void);
    void print_tables(void);

public:

    CTable(Control *control_, Pda *pda_) : Table(control_, pda_),
                                           type_name(num_type_ids),
                                           array_name(num_name_ids),
                                           dcl_buffer(&sysdcl)
    {
        type_name[B] = "char"; // recall that "bool" doesn't work in C
        type_name[I8] = "signed char";
        type_name[U8] = "unsigned char";
        type_name[I16] = "signed short";
        type_name[U16] = "unsigned short";
        type_name[I32] = "signed int";

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

    virtual ~CTable() {}

    virtual void PrintTables(void);
};

#endif /* CTable_INCLUDED */
