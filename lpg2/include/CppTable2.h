#ifndef CppTable2_INCLUDED
#define CppTable2_INCLUDED

#include "table.h"

class CppTable2 : public Table
{
private:
    char parse_table_class_name[Control::SYMBOL_SIZE + sizeof("_table")];
	
    

    Array<const char *> type_name,
                        array_name;

    virtual TypeId Type(int min, int max)
    {
        if (min >= SCHAR_MIN && max <= SCHAR_MAX)
            return I8;
        if (min >= 0 && max <= USHRT_MAX)
            return U16;
        if (min >= SHRT_MIN && max <= SHRT_MAX)
            return I16;
        return I32;
    }

    UnbufferedTextFile prs_buffer;
    UnbufferedBinaryFile data_buffer;

    void PrintHeader(const char *, const char *, const char * = "");
    void PrintTrailer();
   
    void PrintIntsSubrange(int, int, Array<int> &);
    void Print(IntArrayInfo &);
    void PrintNames(void);
   
   
    void non_terminal_action(void);
    void non_terminal_no_goto_default_action(void);
    void terminal_action(void);
    void terminal_shift_default_action(void);
    void init_file(FILE **, const char *);
    void init_parser_files(void);
    void exit_parser_files(void);
 
    void print_symbols(void);
    void print_exports(void);
    void print_definition(const char *, const char *, int);
    void print_definition(const char *, const char *, bool);
    void print_definitions(void);
    void print_externs(void);
    void print_source_tables(void);

public:

    CppTable2(Control *control_, Pda *pda_) : Table(control_, pda_),
                                              prs_buffer(&sysprs),
                                              data_buffer(&sysdat)
    {
        type_name.Resize(num_type_ids);
        array_name.Resize(num_name_ids);

     /*   type_name[B] = type_name[I8] = "unsigned char";
        type_name[I16] = "short";
        type_name[U8] = type_name[U16] = "wchar_t";
        type_name[I32] = "int";*/

        //type_name[B] = "char"; // recall that "bool" doesn't work in C
        //type_name[I8] = "signed char";
        //type_name[U8] = "unsigned char";
        //type_name[I16] = "signed short";
        //type_name[U16] = "unsigned short";
        //type_name[I32] = "signed int";
        type_name[B] = "char"; // recall that "bool" doesn't work in C
        type_name[I8] = "signed int";
        type_name[U8] = "signed int";
        type_name[I16] = "signed int";
        type_name[U16] = "signed int";
        type_name[I32] = "signed int";

    	
        array_name[NULLABLES] = "isNullable";
        array_name[PROSTHESES_INDEX] = "prosthesesIndex";
        array_name[KEYWORDS] = "isKeyword";
        array_name[BASE_CHECK] = "baseCheck";
        array_name[BASE_ACTION] = "baseAction";
        array_name[TERM_CHECK] = "termCheck";
        array_name[TERM_ACTION] = "termAction";
        array_name[DEFAULT_GOTO] = "defaultGoto";
        array_name[DEFAULT_REDUCE] = "defaultReduce";
        array_name[SHIFT_STATE] = "shiftState";
        array_name[SHIFT_CHECK] = "shiftCheck";
        array_name[DEFAULT_SHIFT] = "defaultShift";
        array_name[ACTION_SYMBOLS_BASE] = "asb";
        array_name[ACTION_SYMBOLS_RANGE] = "asr";
        array_name[NACTION_SYMBOLS_BASE] = "nasb";
        array_name[NACTION_SYMBOLS_RANGE] = "nasr";
        array_name[TERMINAL_INDEX] = "terminalIndex";
        array_name[NONTERMINAL_INDEX] = "nonterminalIndex";
        array_name[SCOPE_PREFIX] = "scopePrefix";
        array_name[SCOPE_SUFFIX] = "scopeSuffix";
        array_name[SCOPE_LHS_SYMBOL] = "scopeLhs";
        array_name[SCOPE_LOOK_AHEAD] = "scopeLa";
        array_name[SCOPE_STATE_SET] = "scopeStateSet";
        array_name[SCOPE_RIGHT_SIDE] = "scopeRhs";
        array_name[SCOPE_STATE] = "scopeState";
        array_name[IN_SYMB] = "inSymb";
        array_name[NAME_START] = "!?";
    }

    virtual ~CppTable2() {}

    virtual void PrintTables(void);
};

#endif /* CppTable2_INCLUDED */
