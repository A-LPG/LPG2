#include "partition.h"
#include "CppTable2.h"

#include <iostream>
using namespace std;

//
//
//
void CppTable2::PrintHeader(const char *type, const char *name, const char *initial_elements)
{

    prs_buffer.Put("inline static ");          
    prs_buffer.Put(type);
    prs_buffer.Put(" _");
    prs_buffer.Put(name);
    prs_buffer.Put("[] = {");
    prs_buffer.Put(initial_elements);
    prs_buffer.Put('\n');

}


//
//
//
void CppTable2::PrintTrailer()
{
    prs_buffer.Put("        };\n"
                   "    };\n");
    return;
}



//
//
//
void CppTable2::PrintIntsSubrange(int init, int gate, Array<int> &array)
{
    prs_buffer.Pad();
    int k = 0;
    for (int i = init; i < gate; i++)
    {
        prs_buffer.Put(array[i]);
        prs_buffer.Put(',');
        k++;
        if (k == 10 && i != gate - 1)
        {
            prs_buffer.Put('\n');
            prs_buffer.Pad();
            k = 0;
        }
    }
    if (k != 0)
    {
        prs_buffer.UnputChar(); // remove last comma, if possible
        prs_buffer.Put('\n');
    }

    return;
}

//
//
//
void CppTable2::Print(IntArrayInfo &array_info)
{
    const char *type = type_name[array_info.type_id];
    const char *name = array_name[array_info.name_id];
    Array<int> &array = array_info.array;

    //
    // If the first element is 0, write it out on the initial line.
    //
    int init = (array[0] == 0 ? 1 : 0);
    const char *initial_elements = (init == 1 ? "0," : "");

    PrintHeader(type, name, initial_elements);
    PrintIntsSubrange(init, array.Size(), array);
    prs_buffer.Put("        };\n");

    
 
    prs_buffer.Put(type);
    prs_buffer.Put(" * get_");
    prs_buffer.Put(name);
    prs_buffer.Put("_data(){ return ");
    prs_buffer.Put(" _");
    prs_buffer.Put(name);
    prs_buffer.Put(";}\n");

    //
    // Generate a function with the same name as the array.
    // The function takes an integer argument and returns
    // the corresponding element in the array.
    //
    prs_buffer.Put("      ");
    prs_buffer.Put(array_info.type_id == Table::B ? "bool " : "int ");
    prs_buffer.Put(name);
    prs_buffer.Put("(int index) { return _");
    prs_buffer.Put(name);
    prs_buffer.Put("[index]");
    if (array_info.type_id == Table::B)
        prs_buffer.Put(" != 0");
    prs_buffer.Put("; }\n");

    return;
}


//
//
//
void CppTable2::PrintNames()
{
    PrintHeader("std::wstring", "name");
    char tok[Control::SYMBOL_SIZE + 1];
    for (int i = 0; i < name_info.Size(); i++)
    {
        strcpy(tok, name_info[i]);
        prs_buffer.Pad();
        prs_buffer.Put('L');
        prs_buffer.Put('\"');
        int k = 0,
            len = Length(name_start, i);
        for (int j = 0; j < len; j++)
        {
           
            if (tok[j] == '\"' || tok[j] == '\\')
                prs_buffer.Put('\\');

            if (tok[j] == '\n')
                 prs_buffer.Put(option -> escape);
            else prs_buffer.Put(tok[j]);
            k++;
            //if (k == 30 && (! (j == len - 1)))
            //{
            //    k = 0;
            //    prs_buffer.Put('\"');
            //    prs_buffer.Put(' ');
            //    prs_buffer.Put('+');
            //    prs_buffer.Put('\n');
            //    prs_buffer.Pad();
            //    prs_buffer.Put('\"');
            //}
        }
        prs_buffer.Put('\"');
        if (i < name_info.Size() - 1)
            prs_buffer.Put(',');
        prs_buffer.Put('\n');
    }
    prs_buffer.Put("        };\n");
    prs_buffer.Put("      std::wstring name(int index) { return _name[index]; }\n\n");

    return;
}


void CppTable2::WriteInteger(int num)
{
    unsigned char c1 = (num >> 24),
                  c2 = (num >> 16) & 0xFF,
                  c3 = (num >> 8) & 0xFF,
                  c4 = (num & 0xFF);
    data_buffer.Put(c1);
    data_buffer.Put(c2);
    data_buffer.Put(c3);
    data_buffer.Put(c4);
}

void CppTable2::WriteData(TypeId type_id, Array<int> &array)
{
    WriteInteger(array.Size()); // write the size of the array
    switch(type_id)  // write the largest value that the array can contain
    {
        case B:
        case I8:
             WriteInteger((1 << 7) - 1);
             break;
        case U8:
             WriteInteger((1 << 8) - 1);
             break;
        case I16:
             WriteInteger((1 << 15) - 1);
             break;
        case U16:
             WriteInteger((1 << 16) - 1);
             break;
        case I32:
	  WriteInteger((unsigned (1 << 31)) - 1);
             break;
        default:
             assert(false);
    }

    if (type_id == B || type_id == I8)
    {
        for (int i = 0; i < array.Size(); i++)
        {
            unsigned char c = array[i] & 0xFF;
            data_buffer.Put(c);
        }
    }
    else if (type_id == I16 || type_id == U16)
    {
        for (int i = 0; i < array.Size(); i++)
        {
            int num = array[i];
            unsigned char c1 = (num >> 8) & 0xFF,
                          c2 = (num & 0xFF);
            data_buffer.Put(c1);
            data_buffer.Put(c2);
        }
    }
    else
    {
        assert(type_id == I32);
        for (int i = 0; i < array.Size(); i++)
            WriteInteger(array[i]);
    }

    return;
}

//
//
//
void CppTable2::Declare(int name_id, int type_id)
{
    prs_buffer.Put("     static ");
    prs_buffer.Put(type_name[type_id]);
    prs_buffer.Put(' ');
    prs_buffer.Put(array_name[name_id]);
    prs_buffer.Put("[];\n");
    prs_buffer.Put("      ");
    prs_buffer.Put(type_id == Table::B ? "bool " : "int ");
    prs_buffer.Put(array_name[name_id]);
    prs_buffer.Put("(int index) { return ");
    prs_buffer.Put(array_name[name_id]);
    prs_buffer.Put("[index]");
    if (type_id == Table::B)
        prs_buffer.Put(" != 0");
    prs_buffer.Put("; }\n");
}


//
//
//
void CppTable2::Serialize(const char *variable, const char *method, int value)
{
    //
    // serialize
    //
    WriteInteger(value);

    //
    // deserialize
    //
    des_buffer.Put("            "); 
    des_buffer.Put(variable);
    des_buffer.Put(" = readInt(buffer);\n");

    //
    // declare
    //
    prs_buffer.Put("     static int ");
    prs_buffer.Put(variable);
    prs_buffer.Put(";\n");
    prs_buffer.Put("     int ");
    prs_buffer.Put(method);
    prs_buffer.Put("() { return ");
    prs_buffer.Put(variable);
    prs_buffer.Put("; }\n\n");
}


//
//
//
void CppTable2::Serialize(const char *variable, const char *method, bool value)
{
    //
    // serialize
    //
    WriteInteger(value ? 1 : 0);

    //
    // deserialize
    //
    des_buffer.Put("            "); 
    des_buffer.Put(variable);
    des_buffer.Put(" = readInt(buffer) > 0;\n");

    //
    // declare
    //
    prs_buffer.Put("     static bool ");
    prs_buffer.Put(variable);
    prs_buffer.Put(";\n");
    prs_buffer.Put("      bool ");
    prs_buffer.Put(method);
    prs_buffer.Put("() { return ");
    prs_buffer.Put(variable);
    prs_buffer.Put("; }\n\n");
}


//
//
//
void CppTable2::ConditionalDeserialize(const char *indentation, int name_id, int type_id)
{
    Declare(name_id, type_id);

    //
    // deserialize
    //
    des_buffer.Put("                ");
    des_buffer.Put(indentation);
    des_buffer.Put(array_name[name_id]);
    des_buffer.Put(" = read");
    if (type_id == I32)
         des_buffer.Put("Int");
    else if (type_id == U16)
         des_buffer.Put("Char");
    else if (type_id == I16)
         des_buffer.Put("Short");
    else des_buffer.Put("Byte");
    des_buffer.Put("Array(buffer);\n");

    return;
}


//
//
//
void CppTable2::ConditionalSerialize(const char *indentation, IntArrayInfo &array_info)
{
    //
    // serialize
    //
    WriteData(array_info.type_id, array_info.array);

    ConditionalDeserialize(indentation, array_info.name_id, array_info.type_id);

    return;
}


//
//
//
void CppTable2::Serialize(IntArrayInfo &array_info)
{
    Declare(array_info.name_id, array_info.type_id);

    //
    // serialize
    //
    WriteData(array_info.type_id, array_info.array);

    //
    // deserialize
    //
    des_buffer.Put("            ");
    des_buffer.Put(array_name[array_info.name_id]);
    des_buffer.Put(" = read");
    if (array_info.type_id == I32)
         des_buffer.Put("Int");
    else if (array_info.type_id == U16)
         des_buffer.Put("Char");
    else if (array_info.type_id == I16)
         des_buffer.Put("Short");
    else des_buffer.Put("Byte");
    des_buffer.Put("Array(buffer);\n");

    return;
}


//
//
//
void CppTable2::Serialize(const char *name, int max_length, IntArrayInfo &start, Array<const char *> &info)
{
    //
    // declare
    //
    prs_buffer.Put("     static std::wstring ");
    prs_buffer.Put(name);
    prs_buffer.Put("[];\n");
    prs_buffer.Put("      std::wstring ");
    prs_buffer.Put(name);
    prs_buffer.Put("(int index) { return ");
    prs_buffer.Put(name);
    prs_buffer.Put("[index]; }\n\n");

    //
    // serialize
    //
    Array<int> length(info.Size());
    for (int i = 0; i < length.Size(); i++)
        length[i] = Length(start, i);

    WriteData(Type(0, max_length), length);

    for (int k = 0; k < info.Size(); k++)
    {
        for (const char *p = info[k]; *p; p++)
            data_buffer.Put(*p);
    }

    //
    // deserialize
    //
    des_buffer.Put("            ");
    des_buffer.Put(name);
    des_buffer.Put(" = readStringArray(buffer);\n"); 

    return;
}


//
//
//
void CppTable2::non_terminal_action(void)
{
    prs_buffer.Put("    /**\n"
                   "     * assert(goto_default);\n"
                   "     */\n"
                   "     int ntAction(int state, int sym) {\n"
                   "        return (baseCheck[state + sym] == sym)\n"
                   "                             ? baseAction[state + sym]\n"
                   "                             : defaultGoto[sym];\n"
                   "    }\n\n");
    return;
}


//
//
//
void CppTable2::non_terminal_no_goto_default_action(void)
{
    prs_buffer.Put("    /**\n"
                   "     * assert(! goto_default);\n"
                   "     */\n"
                   "     int ntAction(int state, int sym) {\n"
                   "        return _baseAction[state + sym];\n"
                   "    }\n\n");

    return;
}


//
//
//
void CppTable2::terminal_action(void)
{
    prs_buffer.Put("    /**\n"
                   "     * assert(! shift_default);\n"
                   "     */\n"
                   "     int tAction(int state, int sym) {\n"
                   "        int i = _baseAction[state],\n"
                   "            k = i + sym;\n"
                   "        return _termAction[_termCheck[k] == sym ? k : i];\n"
                   "    }\n"
                   "     int lookAhead(int la_state, int sym) {\n"
                   "        int k = la_state + sym;\n"
                   "        return _termAction[_termCheck[k] == sym ? k : la_state];\n"
                   "    }\n");

    return;
}


//
//
//
void CppTable2::terminal_shift_default_action(void)
{
    prs_buffer.Put("    /**\n"
                   "     * assert(shift_default);\n"
                   "     */\n"
                   "     int tAction(int state, int sym) {\n"
                   "        if (sym == 0)\n"
                   "            return ERROR_ACTION;\n"
                   "        int i = _baseAction[state],\n"
                   "            k = i + sym;\n"
                   "        if (_termCheck[k] == sym)\n"
                   "            return termAction[k];\n"
                   "        i = _termAction[i];\n"
                   "        return (_shiftCheck[_shiftState[i] + sym] == sym\n"
                   "                                ? _defaultShift[sym]\n"
                   "                                : _defaultReduce[i]);\n"
                   "    }\n"
                   "     int lookAhead(int la_state, int sym) {\n"
                   "        int k = la_state + sym;\n"
                   "        if (_termCheck[k] == sym)\n"
                   "            return _termAction[k];\n"
                   "        int i = _termAction[la_state];\n"
                   "        return (_shiftCheck[shiftState[i] + sym] == sym\n"
                   "                                ? _defaultShift[sym]\n"
                   "                                : _defaultReduce[i]);\n"
                   "    }\n");
    return;
}


//
//
//
void CppTable2::init_file(FILE **file, const char *file_name)
{
    if ((*file = fopen(file_name, "wb")) == NULL)
    {
        Tuple<const char *> msg;
        msg.Next() = "Output file \"";
        msg.Next() = file_name;
        msg.Next() = "\" could not be opened";

        option -> EmitError(0, msg);
        Table::Exit(12);
    }

    grammar -> NoticeBuffer().Print(*file);

    return;
}


//
//
//
void CppTable2::init_parser_files(void)
{
    init_file(&sysprs, option -> prs_file);
    init_file(&syssym, option -> sym_file);
    if (grammar -> exported_symbols.Length() > 0)
        init_file(&sysexp, option -> exp_file);

    if (option -> serialize)
        init_file(&sysdat, option -> dat_file);

    return;
}


//
//
//
void CppTable2::exit_parser_files(void)
{
    fclose(sysprs); sysprs = NULL;
    fclose(syssym); syssym = NULL;
    if (grammar -> exported_symbols.Length() > 0) {
	fclose(sysexp);
	sysexp = NULL;
    }
}


//
//
//
void CppTable2::print_symbols(void)
{
    Array<const char *> symbol_name(grammar -> num_terminals + 1);
    int symbol;
    char sym_line[Control::SYMBOL_SIZE +       /* max length of a token symbol  */
                  2 * MAX_PARM_SIZE + /* max length of prefix + suffix */
                  64];                /* +64 for error messages lines  */
                                  /* or other fillers(blank, =,...)*/

    strcpy(sym_line, "");

    strcat(sym_line, "#pragma once\n #include <vector>\n#include<string>\n ");
    strcat(sym_line, " struct ");
    strcat(sym_line, option -> sym_type);
	
    strcat(sym_line, " {\n     typedef  unsigned char byte;\n");
    strcat(sym_line, "      static constexpr int\n");


    //
    // We write the terminal symbols map.
    //
    symbol_name[0] = "";
    for (symbol = grammar -> FirstTerminal(); symbol <= grammar -> LastTerminal(); symbol++)
    {
        char *tok = grammar -> RetrieveString(symbol);

        fprintf(syssym, "%s", sym_line);

        if (tok[0] == '\n' || tok[0] == option -> escape)
        {
            tok[0] = option -> escape;

            Tuple<const char *> msg;
            msg.Next() = "Escaped symbol ";
            msg.Next() = tok;
            msg.Next() = " may be an invalid  variable.";
            option -> EmitWarning(grammar -> RetrieveTokenLocation(symbol), msg);
        }
        else if (strpbrk(tok, "!%^&*()-+={}[];:\"`~|\\,.<>/?\'") != NULL)
        {
            Tuple<const char *> msg;
            msg.Next() = tok;
            msg.Next() = " is an invalid  variable name.";
            option -> EmitError(grammar -> RetrieveTokenLocation(symbol), msg);
        }

        strcpy(sym_line, "      ");
        strcat(sym_line, option -> prefix);
        strcat(sym_line, tok);
        strcat(sym_line, option -> suffix);
        strcat(sym_line, " = ");
        IntToString num(symbol_map[symbol]);
        strcat(sym_line, num.String());
        strcat(sym_line, (symbol < grammar -> LastTerminal() ? ",\n" : ";\n"));

        symbol_name[symbol_map[symbol]] = tok;
    }

    fprintf(syssym, "%s", sym_line);
    fprintf(syssym, "\n      inline const static std::vector<std::wstring> orderedTerminalSymbols");
   // fprintf(syssym, "[%d]", grammar->num_terminals+1);
    fprintf(syssym, " = {\n");
    //                    "                 \"\",\n");
    for (int i = 0; i < grammar -> num_terminals; i++)
        fprintf(syssym, "                 L\"%s\",\n", symbol_name[i]);
    fprintf(syssym, "                 L\"%s\"\n             };\n",
            symbol_name[grammar -> num_terminals]);
    fprintf(syssym, "\n     static constexpr  int numTokenKinds = %d;", grammar->num_terminals+1);
    fprintf(syssym, "\n     static constexpr  bool isValidForParser = true;\n};\n");

    if (option -> serialize)
        Table::initialize(symbol_name, Table::SYMBOL_START, Table::symbol_start, Table::symbol_info, Table::max_symbol_length);

    return;
}


//
//
//
void CppTable2::print_exports(void)
{
    Array<const char *> symbol_name(grammar -> exported_symbols.Length() + 1);
    char exp_line[Control::SYMBOL_SIZE + 64];  /* max length of a token symbol  */
                                               /* +64 for error messages lines  */
                                               /* or other fillers(blank, =,...)*/

    strcpy(exp_line, "");
    strcat(exp_line, "#pragma once\n #include <vector>\n#include<string>\n ");
    strcat(exp_line, " struct ");
    strcat(exp_line, option->exp_type);
    strcat(exp_line, " {\n     typedef  unsigned char byte;\n");
    strcat(exp_line, "      static constexpr int\n");
    //
    // We write the exported terminal symbols and map
    // them according to the order in which they were specified.
    //
    char *temp = new char[1];
    *temp = '\0';
    symbol_name[0] = temp;
    for (int i = 1; i <= grammar -> exported_symbols.Length(); i++)
    {
        VariableSymbol *variable_symbol = grammar -> exported_symbols[i - 1];
        char *tok = new char[variable_symbol -> NameLength() + 1];
        strcpy(tok, variable_symbol -> Name());

        fprintf(sysexp, "%s", exp_line);

        if (tok[0] == '\n' || tok[0] == option -> escape)
        {
            tok[0] = option -> escape;

            Tuple<const char *> msg;
            msg.Next() = "Escaped exported symbol ";
            msg.Next() = tok;
            msg.Next() = " may be an invalid  variable.";
            option -> EmitWarning(variable_symbol -> Location(), msg);
        }
        else if (strpbrk(tok, "!%^&*()-+={}[];:\"`~|\\,.<>/?\'") != NULL)
        {
            Tuple<const char *> msg;
            msg.Next() = "Exported symbol \"";
            msg.Next() = tok;
            msg.Next() = "\" is an invalid  variable name.";
            option -> EmitError(variable_symbol -> Location(), msg);
        }

        strcpy(exp_line, "      ");
        strcat(exp_line, option -> exp_prefix);
        strcat(exp_line, tok);
        strcat(exp_line, option -> exp_suffix);
        strcat(exp_line, " = ");
        IntToString num(i);
        strcat(exp_line, num.String());
        strcat(exp_line, (i < grammar -> exported_symbols.Length() ? ",\n" : ";\n"));
                          
        symbol_name[i] = tok;
    }


    fprintf(sysexp, "%s", exp_line);
    fprintf(sysexp, "\n      inline const static std::vector<std::wstring> orderedTerminalSymbols");
   // fprintf(sysexp, "[%d]", grammar->exported_symbols.Length()+1);
    fprintf(sysexp, " = {\n");

    {
        for (int i = 0; i < grammar -> exported_symbols.Length(); i++)
        {
            fprintf(sysexp, "                 L\"%s\",\n", symbol_name[i]);
            delete [] symbol_name[i];
        }
    }
    fprintf(sysexp, "                 \"%s\"\n             };\n",
            symbol_name[grammar -> exported_symbols.Length()]);
    delete [] symbol_name[grammar -> exported_symbols.Length()];

    fprintf(sysexp, "\n\n    constexpr   static int numTokenKinds = %d;", grammar->exported_symbols.Length()+1);
    fprintf(sysexp, "\n    constexpr   static bool isValidForParser = false;\n};\n");

    return;
}

//
//
//
void CppTable2::print_definition(const char *variable, const char *method, int value)
{
    if (option -> serialize)
        Serialize(variable, method, value);
    else
    {
        prs_buffer.Put("     constexpr   static int ");
        prs_buffer.Put(variable);
        prs_buffer.Put(" = ");
        prs_buffer.Put(value);
        prs_buffer.Put(";\n");
        prs_buffer.Put("     int ");
        prs_buffer.Put(method);
        prs_buffer.Put("() { return ");
        prs_buffer.Put(variable);
        prs_buffer.Put("; }\n\n");
    }
}


//
//
//
void CppTable2::print_definition(const char *variable, const char *method, bool value)
{
    if (option -> serialize)
        Serialize(variable, method, value);
    else
    {
        prs_buffer.Put("   constexpr   static bool ");
        prs_buffer.Put(variable);
        prs_buffer.Put(" = ");
        prs_buffer.Put(value ? "true" : "false");
        prs_buffer.Put(";\n");
        prs_buffer.Put("      bool ");
        prs_buffer.Put(method);
        prs_buffer.Put("() { return ");
        prs_buffer.Put(variable);
        prs_buffer.Put("; }\n\n");
    }
}


//
//
//
void CppTable2::print_definitions(void)
{
    if (option -> serialize)
    {
        Serialize("goto_default", "getGotoDefault", (bool) option -> goto_default);
        Serialize("shift_default", "getShiftDefault", (bool) option -> shift_default);
        Serialize("error_maps", "getErrorMaps", (bool) option -> error_maps);
        Serialize("scopes", "getScopes", (bool) pda -> scope_prefix.Size());
    }

    print_definition("ERROR_SYMBOL", "getErrorSymbol", option -> error_maps ? grammar -> error_image : 0);
    print_definition("SCOPE_UBOUND", "getScopeUbound", option -> error_maps ? pda -> scope_prefix.Size() - 1 : 0);
    print_definition("SCOPE_SIZE", "getScopeSize", option -> error_maps ? pda -> scope_prefix.Size() : 0);
    print_definition("MAX_NAME_LENGTH", "getMaxNameLength", option -> error_maps ? max_name_length : 0);
    print_definition("NUM_STATES", "getNumStates", pda -> num_states);
    print_definition("NT_OFFSET", "getNtOffset", grammar -> num_terminals);
    print_definition("LA_STATE_OFFSET", "getLaStateOffset", option -> read_reduce ? error_act + grammar -> num_rules : error_act);
    print_definition("MAX_LA", "getMaxLa", pda -> highest_level);
    print_definition("NUM_RULES", "getNumRules", grammar -> num_rules);
    print_definition("NUM_NONTERMINALS", "getNumNonterminals", grammar -> num_nonterminals);
    print_definition("NUM_SYMBOLS", "getNumSymbols", grammar -> num_symbols);
    print_definition("START_STATE", "getStartState", start_state);
    print_definition("IDENTIFIER_SYMBOL", "getIdentifier_SYMBOL", grammar -> identifier_image);
    print_definition("EOFT_SYMBOL", "getEoftSymbol", grammar -> eof_image);
    print_definition("EOLT_SYMBOL", "getEoltSymbol", grammar -> eol_image);
    print_definition("ACCEPT_ACTION", "getAcceptAction", accept_act);
    print_definition("ERROR_ACTION", "getErrorAction", error_act);
    print_definition("BACKTRACK", "getBacktrack", (bool) option -> backtrack);

    prs_buffer.Put("     int getStartSymbol() { return lhs(0); }\n"
                   "      bool isValidForParser() { return ");
    if (option -> serialize)
        prs_buffer.Put("true");
    else
    {
        prs_buffer.Put(option -> sym_type);
        prs_buffer.Put("::isValidForParser");
    }
    prs_buffer.Put("; }\n\n");

    return;
}


//
//
//
void CppTable2::print_externs(void)
{
    if (option -> serialize || option -> error_maps || option -> debug)
    {
        prs_buffer.Put("     int originalState(int state) {\n");
        prs_buffer.Put("        return - _baseCheck[state];\n");
        prs_buffer.Put("    }\n");
    }
    else
    {
        prs_buffer.Put("     int originalState(int state) { return 0; }\n");
    }

    if (option -> serialize || option -> error_maps)
    {
        prs_buffer.Put("     int asi(int state) {\n"
                       "        return _asb[originalState(state)];\n"
                       "    }\n"
                       "     int nasi(int state) {\n"
                       "        return _nasb[originalState(state)];\n"
                       "    }\n"
                       "     int inSymbol(int state) {\n"
                       "        return _inSymb[originalState(state)];\n"
                       "    }\n");
    }
    else
    {
        prs_buffer.Put("     int asi(int state) { return 0; }\n"
                       "     int nasi(int state) { return 0; }\n"
                       "     int inSymbol(int state) { return 0; }\n");
    }

    prs_buffer.Put("\n");

    if (option -> goto_default)
         non_terminal_action();
    else non_terminal_no_goto_default_action();

    if (option -> shift_default)
         terminal_shift_default_action();
    else terminal_action();

    return;
}


//
//
//
void CppTable2::initialize_deserialize_buffer(void)
{
    des_buffer.Put("     static int offset = 0;\n"
                   "\n"
                   "    static  int readInt(unsigned char* buffer) {\n"
                   "        return (int) ((buffer[offset++] << 24) +\n"
                   "                      ((buffer[offset++] & 0xFF) << 16) +\n"
                   "                      ((buffer[offset++] & 0xFF) << 8) +\n"
                   "                       (buffer[offset++] & 0xFF));\n"
                   "    }\n"
                   "\n"
                   "    static  short readShort(unsigned char* buffer) {\n"
                   "        return (short) (((buffer[offset++] & 0xFF) << 8) +\n"
                   "                (buffer[offset++] & 0xFF));\n"
                   "    }\n"
                   "\n"
                   "    static  char readChar(unsigned char* buffer) {\n"
                   "        return (char) (((buffer[offset++] & 0xFF) << 8) +\n"
                   "                        (buffer[offset++] & 0xFF));\n"
                   "    }\n"
                   "\n"
                   "    static  std::vector< int > readIntArray(unsigned char* buffer){\n"
                   "        int length = readInt(buffer);\n"
                   "        int type = readInt(buffer); assert(type <= 2147483647);\n"
                   "        std::vector< int > array(length,0);\n"
                   "        for (int i = 0; i < length; i++)\n"
                   "            array[i] = readInt(buffer);\n"
                   "        return array;\n"
                   "    }\n"
                   "\n"
                   "    static   std::vector< short >  readShortArray(unsigned char* buffer) {\n"
                   "        int length = readInt(buffer);\n"
                   "        int type = readInt(buffer); assert(type <= 32767);\n"
                   "      std::vector< short > array(length,0);\n"
                   "        for (int i = 0; i < length; i++)\n"
                   "            array[i] = readShort(buffer);\n"
                   "        return array;\n"
                   "    }\n"
                   "\n"
                   "    static   std::vector< wchar_t >  readCharArray(unsigned char*  buffer[]){\n"
                   "        int length = readInt(buffer);\n"
                   "        int type = readInt(buffer); assert(type <= 65535);\n"
                   "        std::vector< wchar_t > array(length,0);\n"
                   "        for (int i = 0; i < length; i++)\n"
                   "            array[i] = readChar(buffer);\n"
                   "        return array;\n"
                   "    }\n"
                   "\n"
                   "    static  std::vector< unsigned char > readByteArray(unsigned char*   buffer) {\n"
                   "        int length = readInt(buffer);\n"
                   "        int type = readInt(buffer); assert(type <= 127);\n"
                   "        std::vector< unsigned char > array(buffer + offset,buffer + offset + length);\n"
                   "        offset += length;\n"
                   "        return array;\n"
                   "    }\n"
                   "\n"
                   "    static  std::vector< int >  readArithmeticArray(unsigned char*   buffer) {\n"
                   "        int length = readInt(buffer),\n"
                   "            type = readInt(buffer);\n"
                   "           std::vector< int > array(length,0);\n"
                   "        if (type <= 127)\n"
                   "             for (int i = 0; i < length; i++)\n"
                   "                 array[i] = buffer[offset++];\n"
                   "        else if (type <= 32767)\n"
                   "             for (int i = 0; i < length; i++)\n"
                   "                 array[i] = readShort(buffer);\n"
                   "        else if (type <= 65535)\n"
                   "             for (int i = 0; i < length; i++)\n"
                   "                 array[i] = readChar(buffer);\n"
                   "        else for (int i = 0; i < length; i++)\n"
                   "                 array[i] = readInt(buffer);\n"
                   "        return array;\n"
                   "    }\n\n"
                   "    static  std::vector<std::wstring> readStringArray(unsigned char*  buffer)  {\n"
                   "        auto string_length = readArithmeticArray(buffer);\n"
                   "       std::vector<std::wstring>array(string_length.size(),"");\n"
                   "        for (int i = 0; i < array.size(); i++) {\n"
                   "            array[i] = std::wstring(buffer + offset,buffer + offset+ string_length[i]);\n"
                   "            offset += string_length[i];\n"
                   "        }\n"
                   "        return array;\n"
                   "    }\n\n"
                   "    static {\n"
                   "        initialize(\"");
    des_buffer.PutStringLiteral(option -> dat_file);
    des_buffer.Put("\");\n"
                   "    }\n\n"
                   "     static void initialize(const std::wstring& filename) {\n"
                   "        initialize(new java.io.File(filename));\n"
                   "    }\n\n"
                   "     static void initialize(File* file) {\n"
                   "        try {\n"
                   "            java.io.FileInputStream infile = new java.io.FileInputStream(file);\n"
                   "             byte buffer[] = new byte[(int) file.length()];\n"
                   "\n"
                   "            //\n"
                   "            // Normally, we should be able to read the content of infile with\n"
                   "            // the single statement: infile.read(buffer);\n"
                   "            // However, there appears to be a problem with this statement\n"
                   "            // when it is used in an eclipse plugin - in that case, only 8192\n"
                   "            // bytes are read, regardless of the length of buffer. Therefore, we\n"
                   "            // have to replace the single statement above with the loop below...\n"
                   "            //\n"
                   "            int current_index = 0;\n"
                   "            do {\n"
                   "                int num_read = infile.read(buffer, current_index, buffer.length - current_index);\n"
                   "                current_index += num_read;\n"
                   "            } while (current_index < buffer.length);\n"
                   "\n");
    return;
}


//
//
//
void CppTable2::print_serialized_tables(void)
{
    Array<IntArrayInfo *> array_info(num_name_ids);
    array_info.MemReset(); // set all elements to NULL
    for (int i = 0; i < data.Length(); i++)
        array_info[data[i].name_id] = &data[i];

    Serialize(*array_info[NULLABLES]);
    Serialize(*array_info[PROSTHESES_INDEX]);
    Serialize(*array_info[KEYWORDS]);
    Serialize(*array_info[BASE_CHECK]);
	
    prs_buffer.Put("     int rhs(int index) { return ");
    prs_buffer.Put(array_name[BASE_CHECK]);
    prs_buffer.Put("[index]; };\n");
    Serialize(*array_info[BASE_ACTION]);
        prs_buffer.Put("     int lhs(int index) { return ");
        prs_buffer.Put(array_name[BASE_ACTION]);
        prs_buffer.Put("[index]; };\n");
    Serialize(*array_info[TERM_CHECK]);
    Serialize(*array_info[TERM_ACTION]);

    des_buffer.Put("            if (goto_default)\n");
    if (option -> goto_default)
        ConditionalSerialize("", *array_info[DEFAULT_GOTO]);
    else
    {
        assert(array_info[DEFAULT_GOTO] == NULL);
        ConditionalDeserialize("", DEFAULT_GOTO, I32);
    }

    des_buffer.Put("            if (shift_default) {\n");
    if (option -> shift_default)
    {
        ConditionalSerialize("", *array_info[DEFAULT_REDUCE]);
        ConditionalSerialize("", *array_info[SHIFT_STATE]);
        ConditionalSerialize("", *array_info[SHIFT_CHECK]);
        ConditionalSerialize("", *array_info[DEFAULT_SHIFT]);
    }
    else
    {
        assert(array_info[DEFAULT_REDUCE] == NULL);
        assert(array_info[SHIFT_STATE] == NULL);
        assert(array_info[SHIFT_CHECK] == NULL);
        assert(array_info[DEFAULT_SHIFT] == NULL);

        ConditionalDeserialize("", DEFAULT_REDUCE, I32);
        ConditionalDeserialize("", SHIFT_STATE, I32);
        ConditionalDeserialize("", SHIFT_CHECK, I32);
        ConditionalDeserialize("", DEFAULT_SHIFT, I32);
    }
    des_buffer.Put("            }\n");

    des_buffer.Put("            if (error_maps) {\n");
    if (option -> error_maps)
    {
        ConditionalSerialize("", *array_info[ACTION_SYMBOLS_BASE]);
        ConditionalSerialize("", *array_info[ACTION_SYMBOLS_RANGE]);
        ConditionalSerialize("", *array_info[NACTION_SYMBOLS_BASE]);
        ConditionalSerialize("", *array_info[NACTION_SYMBOLS_RANGE]);
        ConditionalSerialize("", *array_info[TERMINAL_INDEX]);
        ConditionalSerialize("", *array_info[NONTERMINAL_INDEX]);

        des_buffer.Put("                if (scopes) {\n");
        if (pda -> scope_prefix.Size() > 0)
        {
            ConditionalSerialize("    ", *array_info[SCOPE_PREFIX]);
            ConditionalSerialize("    ", *array_info[SCOPE_SUFFIX]);
            ConditionalSerialize("    ", *array_info[SCOPE_LHS_SYMBOL]);
            ConditionalSerialize("    ", *array_info[SCOPE_LOOK_AHEAD]);
            ConditionalSerialize("    ", *array_info[SCOPE_STATE_SET]);
            ConditionalSerialize("    ", *array_info[SCOPE_RIGHT_SIDE]);
            ConditionalSerialize("    ", *array_info[SCOPE_STATE]);
            ConditionalSerialize("    ", *array_info[IN_SYMB]);
        }
        //
        // If error_maps are requested but not the scope maps, we generate
        // shells for the scope maps to allow an error recovery system that
        // might depend on such maps to compile.
        //
        else
        {
            assert(array_info[SCOPE_PREFIX] == NULL);
            assert(array_info[SCOPE_SUFFIX] == NULL);
            assert(array_info[SCOPE_LHS_SYMBOL] == NULL);
            assert(array_info[SCOPE_LOOK_AHEAD] == NULL);
            assert(array_info[SCOPE_STATE_SET] == NULL);
            assert(array_info[SCOPE_RIGHT_SIDE] == NULL);
            assert(array_info[SCOPE_STATE] == NULL);
            assert(array_info[IN_SYMB] == NULL);

            ConditionalDeserialize("    ", SCOPE_PREFIX, I32);
            ConditionalDeserialize("    ", SCOPE_SUFFIX, I32);
            ConditionalDeserialize("    ", SCOPE_LHS_SYMBOL, I32);
            ConditionalDeserialize("    ", SCOPE_LOOK_AHEAD, I32);
            ConditionalDeserialize("    ", SCOPE_STATE_SET, I32);
            ConditionalDeserialize("    ", SCOPE_RIGHT_SIDE, I32);
            ConditionalDeserialize("    ", SCOPE_STATE, I32);
            ConditionalDeserialize("    ", IN_SYMB, I32);
        }
        des_buffer.Put("                }\n");
    }
    else
    {
        assert(array_info[ACTION_SYMBOLS_BASE] == NULL);
        assert(array_info[ACTION_SYMBOLS_RANGE] == NULL);
        assert(array_info[NACTION_SYMBOLS_BASE] == NULL);
        assert(array_info[NACTION_SYMBOLS_RANGE] == NULL);
        assert(array_info[TERMINAL_INDEX] == NULL);
        assert(array_info[NONTERMINAL_INDEX] == NULL);
        assert(array_info[SCOPE_PREFIX] == NULL);
        assert(array_info[SCOPE_SUFFIX] == NULL);
        assert(array_info[SCOPE_LHS_SYMBOL] == NULL);
        assert(array_info[SCOPE_LOOK_AHEAD] == NULL);
        assert(array_info[SCOPE_STATE_SET] == NULL);
        assert(array_info[SCOPE_RIGHT_SIDE] == NULL);
        assert(array_info[SCOPE_STATE] == NULL);
        assert(array_info[IN_SYMB] == NULL);

        ConditionalDeserialize("", ACTION_SYMBOLS_BASE, I32);
        ConditionalDeserialize("", ACTION_SYMBOLS_RANGE, I32);
        ConditionalDeserialize("", NACTION_SYMBOLS_BASE, I32);
        ConditionalDeserialize("", NACTION_SYMBOLS_RANGE, I32);
        ConditionalDeserialize("", TERMINAL_INDEX, I32);
        ConditionalDeserialize("", NONTERMINAL_INDEX, I32);
        ConditionalDeserialize("", SCOPE_PREFIX, I32);
        ConditionalDeserialize("", SCOPE_SUFFIX, I32);
        ConditionalDeserialize("", SCOPE_LHS_SYMBOL, I32);
        ConditionalDeserialize("", SCOPE_LOOK_AHEAD, I32);
        ConditionalDeserialize("", SCOPE_STATE_SET, I32);
        ConditionalDeserialize("", SCOPE_RIGHT_SIDE, I32);
        ConditionalDeserialize("", SCOPE_STATE, I32);
        ConditionalDeserialize("", IN_SYMB, I32);
    }
    des_buffer.Put("            }\n");

    Serialize("name", max_name_length, name_start, name_info);
    Serialize("orderedTerminalSymbols", max_symbol_length, symbol_start, symbol_info);

    des_buffer.Put("        }\n"
                   "        catch(java.io.IOException e) {\n"
                   "            System.out.println(\"*** Illegal or corrupted LPG data file\");\n"
                   "            System.exit(12);\n"
                   "        }\n"
                   "        catch(Exception e) {\n"
                   "            System.out.println(\"*** Unable to Initialize LPG tables\");\n"
                   "            System.exit(12);\n"
                   "        }\n"
                   "    }\n\n");

    prs_buffer.Put(des_buffer);
    data_buffer.Flush();

    return;
}


//
//
//
void CppTable2::print_source_tables(void)
{
    for (int i = 0; i < data.Length(); i++)
    {
        IntArrayInfo &array_info = data[i];
        Print(array_info);
        switch(array_info.name_id)
        {
            case BASE_CHECK:
                prs_buffer.Put("inline      static ");
                prs_buffer.Put(type_name[array_info.type_id]);
                prs_buffer.Put("*  _rhs = _");
                prs_buffer.Put(array_name[array_info.name_id]);
                prs_buffer.Put(";\n"
                               "     int rhs(int index) { return _rhs[index]; };\n");

                prs_buffer.Put(type_name[array_info.type_id]);
                prs_buffer.Put("*  get_rhs_data(){ return _rhs;}\n");
        	
                break;
            case BASE_ACTION:
                prs_buffer.Put(" inline     static ");
                prs_buffer.Put(type_name[array_info.type_id]);
                prs_buffer.Put(" * _lhs = _");
                prs_buffer.Put(array_name[array_info.name_id]);
                prs_buffer.Put(";\n"
                               "     int lhs(int index) { return _lhs[index]; };\n");

                prs_buffer.Put(type_name[array_info.type_id]);
                prs_buffer.Put("*  get_lhs_data(){ return _lhs;}\n");
                break;
            default:
                break;
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
            prs_buffer.Put("  inline    static int* _scopePrefix = NULL;\n"
                           "     int scopePrefix(int index) { return 0;}\n\n"
                           "   inline   static int* _scopeSuffix = NULL;\n"
                           "     int scopeSuffix(int index) { return 0;}\n\n"
                           "    inline  static int* _scopeLhs = NULL;\n"
                           "     int scopeLhs(int index) { return 0;}\n\n"
                           "    inline  static int* _scopeLa = NULL;\n"
                           "     int scopeLa(int index) { return 0;}\n\n"
                           "    inline  static int* _scopeStateSet = NULL;\n"
                           "     int scopeStateSet(int index) { return 0;}\n\n"
                           "   inline   static int* _scopeRhs = NULL;\n"
                           "     int scopeRhs(int index) { return 0;}\n\n"
                           "   inline   static int* _scopeState = NULL;\n"
                           "     int scopeState(int index) { return 0;}\n\n"
                           "   inline   static int* _inSymb = NULL;\n"
                           "     int inSymb(int index) { return 0;}\n\n");
        }

        PrintNames();
    }
    else
    {
        prs_buffer.Put("     int asb(int index) { return 0; }\n"
                       "     int asr(int index) { return 0; }\n"
                       "     int nasb(int index) { return 0; }\n"
                       "     int nasr(int index) { return 0; }\n"
                       "     int terminalIndex(int index) { return 0; }\n"
                       "     int nonterminalIndex(int index) { return 0; }\n"
                       "     int scopePrefix(int index) { return 0;}\n"
                       "     int scopeSuffix(int index) { return 0;}\n"
                       "     int scopeLhs(int index) { return 0;}\n"
                       "     int scopeLa(int index) { return 0;}\n"
                       "     int scopeStateSet(int index) { return 0;}\n"
                       "     int scopeRhs(int index) { return 0;}\n"
                       "     int scopeState(int index) { return 0;}\n"
                       "     int inSymb(int index) { return 0;}\n"
                       "      std::wstring name(int index) { return {}; }\n");
    }

    return;
}


//
//
//
void CppTable2::PrintTables(void)
{
    init_parser_files();

    print_symbols();

    if (grammar -> exported_symbols.Length() > 0)
        print_exports();

    //
    // Now process the parse file
    //


    prs_buffer.Put("#pragma once\n");
    prs_buffer.Put(" #include <string>\n ");

    if (option->extends_parsetable)
    {
        prs_buffer.Put(" #include \"");
        prs_buffer.Put(option->extends_parsetable);
        prs_buffer.Put(".h\"\n");
    }

    prs_buffer.Put(" #include \"");
    prs_buffer.Put(option->sym_type);
    prs_buffer.Put(".h\"\n");
    if (option->parsetable_interfaces)
    {
        prs_buffer.Put(" #include \"");
        prs_buffer.Put(option->parsetable_interfaces);
        prs_buffer.Put(".h\"\n");
    }
	
    prs_buffer.Put(" struct ");
    prs_buffer.Put(option -> prs_type);
    prs_buffer.Put(" :");
    bool need_conlon = false;
    if (option -> extends_parsetable)
    {
        prs_buffer.Put(" public ");
        prs_buffer.Put(option -> extends_parsetable);
        need_conlon = true;
    }
   
    if (option -> parsetable_interfaces)
    {
    	if(need_conlon)
    	{
            prs_buffer.Put(",");
    	}
        prs_buffer.Put("public ");
        prs_buffer.Put(option -> parsetable_interfaces);
        
        need_conlon = true;
    }
	if(need_conlon)
	{
        prs_buffer.Put(",");
	}
    prs_buffer.Put("public ");
    prs_buffer.Put(option -> sym_type);
    prs_buffer.Put(" {\n");
    prs_buffer.Put("             typedef  unsigned char byte;\n");
    if (option -> serialize)
        initialize_deserialize_buffer();

    print_definitions();

    if (option -> serialize)
         print_serialized_tables();
    else print_source_tables();

    print_externs();

    prs_buffer.Put("};\n");
    prs_buffer.Flush();

    exit_parser_files();

    return;
}
