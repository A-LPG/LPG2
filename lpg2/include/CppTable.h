#ifndef CppTable_INCLUDED
#define CppTable_INCLUDED

#include "CTable.h"

class CppTable : public CTable
{
protected:
    void non_terminal_action(void);
    void non_terminal_no_goto_default_action(void);
    void terminal_action(void);
    void terminal_shift_default_action(void);
    void print_externs_and_definitions(void);
    void print_definitions(void);

public:

    CppTable(Control *control_, Pda *pda_) : CTable(control_, pda_)
    {}

    virtual ~CppTable() {}

    virtual void PrintTables(void);
};

#endif /* CppTable_INCLUDED */
