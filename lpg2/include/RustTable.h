#pragma once

#include "table.h"
#include <string>

class RustTable : public Table
{
private:

    Array<const char *> type_name,
                        array_name;
    char* prs_def_prefix=nullptr;
    std::string method_code; // trait methods accumulated separately from module-level statics
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

    TextBuffer des_buffer; // deserialize buffer
    UnbufferedTextFile prs_buffer;
    UnbufferedBinaryFile data_buffer;

    void PrintHeader(const char *, const char *, const char * = "");
    void PrintTrailer();
    void PrintTrailerAndVariable(const char *, const char *);
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
    void print_definition_method(const char *, const char *, const char *);
    void print_definitions(void);
    void print_externs(void);
    void print_source_tables(void);
    bool IsTraitAccessor(int name_id);

public:

    RustTable(Control *control_, Pda *pda_);

    virtual ~RustTable();

    virtual void PrintTables(void);
};
