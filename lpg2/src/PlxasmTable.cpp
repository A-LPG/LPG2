#include "tuple.h"
#include "PlxasmTable.h"

#include <iostream>
using namespace std;

//
// Print address declaration
//
void PlxasmTable::PrintAddress(const char *name)
{
    char *ptr_name = new char[strlen(name) + 4]; // +3 for "PTR" +1 for gate
    strcpy(ptr_name, name);
    strcat(ptr_name, "PTR");

    dcl_buffer.Field(ptr_name, 8);
    dcl_buffer.Put(" DC    A(");
    dcl_buffer.Put(name);
    dcl_buffer.Put(")\n");
}


//
// Print array declaration
//
void PlxasmTable::Print(const char *name, const char *plx_name, int type_id, Array<int> &array, int lbound, int ubound)
{
    const char *type = type_name[type_id],
               *plx_type = PlxTable::type_name[type_id];

    dcl_buffer.Put("*\n");
    dcl_buffer.Put("* DCL ");
    dcl_buffer.Put(plx_name);
    dcl_buffer.Put('(');
    if (lbound != 1)
    {
        dcl_buffer.Put(lbound);
        dcl_buffer.Put(':');
    }
    dcl_buffer.Put(ubound);
    dcl_buffer.Put(") ");
    dcl_buffer.Put(plx_type);
    dcl_buffer.Put(" BASED");
    dcl_buffer.Put("(");
    dcl_buffer.Put(name);
    dcl_buffer.Put("PTR)");
    dcl_buffer.Put(";\n");
    dcl_buffer.Put("*\n");

    dcl_buffer.Field(name, 9);
    dcl_buffer.Field("DS", 6);
    dcl_buffer.Put('0');
    dcl_buffer.Put(type);
    dcl_buffer.Put('\n');
    dcl_buffer.Pad(9);
    dcl_buffer.Field("DC", 6);
    dcl_buffer.Put(type);
    dcl_buffer.Put('\'');
    int k = 0;
    for (int i = lbound; i <= ubound; i++)
    {
        dcl_buffer.Put(array[i]);
        dcl_buffer.Put(',');
        k++;
        if (k == 5 && i != ubound)
        {
            dcl_buffer.UnputChar(); // remove last comma, if possible
            dcl_buffer.Put("\'\n");
            dcl_buffer.Pad(9);
            dcl_buffer.Field("DC", 6);
            dcl_buffer.Put(type);
            dcl_buffer.Put('\'');
            k = 0;
        }
    }
    if (k != 0)
    {
        dcl_buffer.UnputChar(); // remove last comma, if possible
        dcl_buffer.Put("\'\n");
    }

    return;
}


//
// NOTE THAT THIS FUNCTION ASSUMES THAT THIS PROGRAM IS COMPILED
// ON AN ASCII MACHINE AND THAT WE WISH TO MAP EACH CHARACTER
// TO ITS RESPECTIVE ASCII CODE!!!
//
void PlxasmTable::PrintNames(void)
{
    dcl_buffer.Put("*\n* DCL name(0:");
    dcl_buffer.Put(name_info.Size() - 1);
    dcl_buffer.Put(") CHAR(");
    dcl_buffer.Put(max_name_length);
    dcl_buffer.Put(") VARYING BASED(TNAMPTR);\n*\n");

    dcl_buffer.Field("TNAM", 9);
    dcl_buffer.Field("DS", 6);
    dcl_buffer.Put("0H\n");

    char tok[Control::SYMBOL_SIZE + 1];
    for (int i = 0; i < name_info.Size(); i++)
    {
        strcpy(tok, name_info[i]);

        //
        // Print the character string representation of the name as a comment
        //
        dcl_buffer.Put("*\n");
        for (int j = 0; j < Length(name_start, i); j += 25)
        {
            dcl_buffer.Put('*');
            dcl_buffer.Pad(8);
            dcl_buffer.Put('\"');

            int length = Util::Min(j + 25, Length(name_start, i));
            for (int k = j; k < length; k++)
            {
                if (tok[k] == '\n') // within a name, the escape character is represented by '\n'
                     dcl_buffer.Put(option -> escape);
                else dcl_buffer.Put(tok[k]);
            }

            dcl_buffer.Put("\"\n");
        }
        dcl_buffer.Put("*\n");

        //
        // Print the HEX representation of the name as data constant.
        //
        dcl_buffer.Pad(9);
        dcl_buffer.Put("DC");
        dcl_buffer.Pad(4);
        dcl_buffer.Put("HL2\'");
        dcl_buffer.Put(Length(name_start, i));
        dcl_buffer.Put("\'\n");
        {
            for (int j = 0; j < Length(name_start, i); j += 25)
            {
                dcl_buffer.Pad(9);
                dcl_buffer.Put("DC");
                dcl_buffer.Pad(4);
                dcl_buffer.Put("XL");
                dcl_buffer.Put(Length(name_start, i));
                dcl_buffer.Put('\'');

                int length = Util::Min(j + 25, Length(name_start, i));
                for (int k = j; k < length; k++)
                {
                    if (tok[k] == '\n') // within a name, the escape character is represented by '\n'
                         dcl_buffer.PutHex(option -> escape);
                    else dcl_buffer.PutHex(tok[k]);
                }

                dcl_buffer.Put("\'\n");
            }
        }
        if (max_name_length > Length(name_start, i))
        {
            dcl_buffer.Pad(9);
            dcl_buffer.Put("DC");
            dcl_buffer.Pad(4);
            int filler_size = max_name_length - Length(name_start, i);
            if (filler_size > 1)
                dcl_buffer.Put(filler_size);
            dcl_buffer.Put("X\'20\'\n");
        }
    }

    return;
}


//
//
//
void PlxasmTable::print_tables(void)
{
    dcl_buffer.Field(option -> dcl_type, 8);
    dcl_buffer.Put(" CSECT\n");
    dcl_buffer.Field(option -> dcl_type, 8);
    dcl_buffer.Put(" AMODE 31\n");
    dcl_buffer.Field(option -> dcl_type, 8);
    dcl_buffer.Put(" RMODE ANY\n");
    dcl_buffer.Pad(9);
    dcl_buffer.Put("USING    *,15\n");
    dcl_buffer.Pad(9);
    dcl_buffer.Put("DSNWDID\n");
    dcl_buffer.Pad(9);
    dcl_buffer.Put("ENTRY PARSTABS\n");
    dcl_buffer.Put("PARSTABS DS    0F\n");

    //
    // Save addresses
    //
    {
        for (int i = 0; i < data.Length(); i++)
        {
            IntArrayInfo &array_info = data[i];
            const char *name = array_name[array_info.name_id];
            Array<int> &array = array_info.array;
            switch(array_info.name_id)
            {
                case BASE_CHECK:
                     PrintAddress("RHS");
                     if (array.Size() - 1 > grammar -> num_rules + 1)
                         PrintAddress(name);
                     break;
                case BASE_ACTION:
                     PrintAddress("LHS");
                     PrintAddress(name);
                     break;
                default:
                     PrintAddress(name);
                     break;
            }
        }
    }

    if (option -> error_maps)
    {
        //
        // If error_maps are requested but not the scope maps, we generate
        // shells for the scope maps to allow an error recovery system that
        // might depend on such maps to compile.
        //
        if (pda -> scope_prefix.Size() == 0)
        {
            PrintAddress(array_name[SCOPE_PREFIX]);
            PrintAddress(array_name[SCOPE_SUFFIX]);
            PrintAddress(array_name[SCOPE_LHS_SYMBOL]);
            PrintAddress(array_name[SCOPE_LOOK_AHEAD]);
            PrintAddress(array_name[SCOPE_STATE_SET]);
            PrintAddress(array_name[SCOPE_RIGHT_SIDE]);
            PrintAddress(array_name[SCOPE_STATE]);
            PrintAddress(array_name[IN_SYMB]);
        }

        PrintAddress("TNAM");
    }

    //
    //
    //
    for (int i = 0; i < data.Length(); i++)
    {
        IntArrayInfo &array_info = data[i];
        const char *name = array_name[array_info.name_id],
                   *plx_name = PlxTable::array_name[array_info.name_id];
        Array<int> &array = array_info.array;
        switch(array_info.name_id)
        {
            case BASE_CHECK:
                 Print("RHS", "rhs", array_info.type_id, array, 1, grammar -> num_rules);
                 if (array.Size() - 1 > grammar -> num_rules + 1)
                 {
                     dcl_buffer.Pad(9);
                     dcl_buffer.Put("EJECT\n");

                     Print(name, plx_name, array_info.type_id, array, grammar -> num_rules + 1, array.Size() - 1);
                 }
                 break;
            case BASE_ACTION:
                 Print("LHS", "lhs", array_info.type_id, array, 0, grammar -> num_rules);
                 dcl_buffer.Pad(9);
                 dcl_buffer.Put("EJECT\n");

                 Print(name, plx_name, array_info.type_id, array, grammar -> num_rules + 1, array.Size() - 1);
                 break;
            default:
                 Print(name, plx_name, array_info.type_id, array, 0, array.Size() - 1);
                 break;
        }

        if (i < data.Length() - 1) // Do not print EJECT directive for last array.
        {
            dcl_buffer.Pad(9);
            dcl_buffer.Put("EJECT\n");
        }
    }

    if (option -> error_maps)
    {
        Array<int> array(1);
        array[0] = 0;

        dcl_buffer.Pad(9);
        dcl_buffer.Put("EJECT\n");

        //
        // If error_maps are requested but not the scope maps, we generate
        // shells for the scope maps to allow an error recovery system that
        // might depend on such maps to compile.
        //
        if (pda -> scope_prefix.Size() == 0)
        {
            Print(array_name[SCOPE_PREFIX], PlxTable::array_name[SCOPE_PREFIX], I16, array, 0, 0);
            dcl_buffer.Pad(9);
            dcl_buffer.Put("EJECT\n");

            Print(array_name[SCOPE_SUFFIX], PlxTable::array_name[SCOPE_SUFFIX], I16, array, 0, 0);
            dcl_buffer.Pad(9);
            dcl_buffer.Put("EJECT\n");

            Print(array_name[SCOPE_LHS_SYMBOL], PlxTable::array_name[SCOPE_LHS_SYMBOL], I16, array, 0, 0);
            dcl_buffer.Pad(9);
            dcl_buffer.Put("EJECT\n");

            Print(array_name[SCOPE_LOOK_AHEAD], PlxTable::array_name[SCOPE_LOOK_AHEAD], I16, array, 0, 0);
            dcl_buffer.Pad(9);
            dcl_buffer.Put("EJECT\n");

            Print(array_name[SCOPE_STATE_SET], PlxTable::array_name[SCOPE_STATE_SET], I16, array, 0, 0);
            dcl_buffer.Pad(9);
            dcl_buffer.Put("EJECT\n");

            Print(array_name[SCOPE_RIGHT_SIDE], PlxTable::array_name[SCOPE_RIGHT_SIDE], I16, array, 0, 0);
            dcl_buffer.Pad(9);
            dcl_buffer.Put("EJECT\n");

            Print(array_name[SCOPE_STATE], PlxTable::array_name[SCOPE_STATE], I16, array, 0, 0);
            dcl_buffer.Pad(9);
            dcl_buffer.Put("EJECT\n");

            Print(array_name[IN_SYMB], PlxTable::array_name[IN_SYMB], I16, array, 0, 0);
            dcl_buffer.Pad(9);
            dcl_buffer.Put("EJECT\n");

        }

        PrintNames();
    }

    dcl_buffer.Pad(9);
    dcl_buffer.Put("SPACE 1\n");
    dcl_buffer.Pad(9);
    dcl_buffer.Put("END\n");

    dcl_buffer.Flush();

    return;
}


//
//
//
void PlxasmTable::PrintTables(void)
{
    init_parser_files();

    PrintSemanticFunctionsMap();

    print_tables();

    print_symbols();
    if (grammar -> exported_symbols.Length() > 0)
        print_exports();
    print_externs();
    print_definitions();

    exit_parser_files();

    return;
}
