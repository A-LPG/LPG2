#ifndef PlxasmTable_INCLUDED
#define PlxasmTable_INCLUDED

#include "PlxTable.h"

class PlxasmTable : public PlxTable
{
protected:

    Array<const char *> type_name,
                        array_name;

    void PrintAddress(const char *);
    void Print(const char *, const char *, int, Array<int> &, int, int);
    void PrintNames(void);
    void print_tables(void);

public:

    PlxasmTable(Control *control_, Pda *pda_) : PlxTable(control_, pda_),
                                                type_name(num_type_ids),
                                                array_name(num_name_ids)
    {
        type_name[B] = type_name[U8] = type_name[I8] = type_name[I16] = "H";
        type_name[U16] = type_name[I32] = "F";

        array_name[NULLABLES] = "ISNUL";
        array_name[PROSTHESES_INDEX] = "PRIDX";
        array_name[KEYWORDS] = "ISKEY";
        array_name[BASE_CHECK] = "BSCHK";
        array_name[BASE_ACTION] = "BSACT";
        array_name[TERM_CHECK] = "TMCHK";
        array_name[TERM_ACTION] = "TMACT";
        array_name[DEFAULT_GOTO] = "DFGO";
        array_name[DEFAULT_REDUCE] = "DFRED";
        array_name[SHIFT_STATE] = "SHSTA";
        array_name[SHIFT_CHECK] = "SHCHK";
        array_name[DEFAULT_SHIFT] = "DFSHF";
        array_name[ACTION_SYMBOLS_BASE] = "ASB";
        array_name[ACTION_SYMBOLS_RANGE] = "ASR";
        array_name[NACTION_SYMBOLS_BASE] = "NASB";
        array_name[NACTION_SYMBOLS_RANGE] = "NASR";
        array_name[TERMINAL_INDEX] = "TMIND";
        array_name[NONTERMINAL_INDEX] = "NTIND";
        array_name[SCOPE_PREFIX] = "SCPRF";
        array_name[SCOPE_SUFFIX] = "SCSUF";
        array_name[SCOPE_LHS_SYMBOL] = "SCLHS";
        array_name[SCOPE_LOOK_AHEAD] = "SCLA";
        array_name[SCOPE_STATE_SET] = "SCSST";
        array_name[SCOPE_RIGHT_SIDE] = "SCRHS";
        array_name[SCOPE_STATE] = "SCST";
        array_name[IN_SYMB] = "INSM";
        array_name[NAME_START] = "NMSTR";
    }

    virtual ~PlxasmTable() {}

    virtual void PrintTables(void);
};

#endif /* PlxasmTable_INCLUDED */
