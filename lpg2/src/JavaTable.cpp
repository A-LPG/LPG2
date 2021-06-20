#include "partition.h"
#include "JavaTable.h"

#include <iostream>
using namespace std;

//
//
//
void JavaTable::PrintHeader(const char *type, const char *name, const char *initial_elements)
{
    prs_buffer.Put("\n    public interface ");
    prs_buffer.Put(Code::ToUpper(*name)); // capitalize the first letter
    prs_buffer.Put(name + 1);
    prs_buffer.Put(" {\n"
                   "        public final static ");
    prs_buffer.Put(type);
    prs_buffer.Put(' ');
    prs_buffer.Put(name);
    prs_buffer.Put("[] = {");
    prs_buffer.Put(initial_elements);
    prs_buffer.Put('\n');

    return;
}


//
//
//
void JavaTable::PrintTrailer()
{
    prs_buffer.Put("        };\n"
                   "    };\n");
    return;
}

//
//
//
void JavaTable::PrintTrailerAndVariable(const char *type, const char *name)
{
    PrintTrailer();
    prs_buffer.Put("    public final static ");
    prs_buffer.Put(type);
    prs_buffer.Put(' ');
    prs_buffer.Put(name);
    prs_buffer.Put("[] = ");
    prs_buffer.Put(Code::ToUpper(*name));
    prs_buffer.Put(name + 1);
    prs_buffer.Put('.');
    prs_buffer.Put(name);
    prs_buffer.Put(";\n");

    return;
}


//
//
//
void JavaTable::PrintIntsSubrange(int init, int gate, Array<int> &array)
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
void JavaTable::Print(IntArrayInfo &array_info)
{
    const char *type = type_name[array_info.type_id];
    const char *name = array_name[array_info.name_id];
    Array<int> &array = array_info.array;

    //
    // If the first element is 0, write it out on the initial line.
    //
    int init = (array[0] == 0 ? 1 : 0);
    const char *initial_elements = (init == 1 ? "0," : "");

    if (NeedsSegmentation(array))
    {
        int length = strlen(name);
        IntToString suffix(0);
        char *subname = new char[length + 3];
        strcpy(subname, name);
        strcat(subname, suffix.String());

        PrintHeader(type, subname, initial_elements);
        PrintIntsSubrange(init, MAX_ARRAY_SIZE, array);
        PrintTrailer();

        int num_segments = array.Size() / MAX_ARRAY_SIZE + (array.Size() % MAX_ARRAY_SIZE ? 1 : 0);
        for (int i = 1; i < num_segments; i++)
        {
            IntToString suffix(i);
            strcpy(subname, name);
            strcat(subname, suffix.String());

            PrintHeader(type, subname);
            init = MAX_ARRAY_SIZE * i;
            PrintIntsSubrange(init, Util::Min(init + MAX_ARRAY_SIZE, array.Size()), array);
            PrintTrailer();
        }

        delete [] subname;

        prs_buffer.Put("\n    public final static ");
        prs_buffer.Put(type);
        prs_buffer.Put(' ');
        prs_buffer.Put(name);
        prs_buffer.Put("[] = new ");
        prs_buffer.Put(type);
        prs_buffer.Put('[');
        {
            for (int k = 0; k < num_segments; k++)
            {
                IntToString suffix(k);
                prs_buffer.Put(Code::ToUpper(*name));
                prs_buffer.Put(name + 1);
                prs_buffer.Put(suffix.String());
                prs_buffer.Put('.');
                prs_buffer.Put(name);
                prs_buffer.Put(suffix.String());
                prs_buffer.Put(".length");
                if (k < num_segments - 1)
                    prs_buffer.Put(" + ");
            }
        }
        prs_buffer.Put("];");
        prs_buffer.Put("\n    {");
        prs_buffer.Put("\n        int index = 0;");
        for (int k = 0; k < num_segments; k++)
        {
            prs_buffer.Put("\n        System.arraycopy(");
            IntToString suffix(k);
            prs_buffer.Put(Code::ToUpper(*name));
            prs_buffer.Put(name + 1);
            prs_buffer.Put(suffix.String());
            prs_buffer.Put('.');
            prs_buffer.Put(name);
            prs_buffer.Put(suffix.String());
            prs_buffer.Put(", 0, ");
            prs_buffer.Put(name);
            prs_buffer.Put(", index, ");
            prs_buffer.Put(Code::ToUpper(*name));
            prs_buffer.Put(name + 1);
            prs_buffer.Put(suffix.String());
            prs_buffer.Put('.');
            prs_buffer.Put(name);
            prs_buffer.Put(suffix.String());
            prs_buffer.Put(".length);");
            if (k < num_segments - 1)
            {
                prs_buffer.Put("\n        index += ");
                prs_buffer.Put(Code::ToUpper(*name));
                prs_buffer.Put(name + 1);
                prs_buffer.Put(suffix.String());
                prs_buffer.Put('.');
                prs_buffer.Put(name);
                prs_buffer.Put(suffix.String());
                prs_buffer.Put(".length;");
            }
        }
        prs_buffer.Put("\n    };\n");
    }
    else
    {
        PrintHeader(type, name, initial_elements);
        PrintIntsSubrange(init, array.Size(), array);
        PrintTrailerAndVariable(type, name);
    }

    //
    // Generate a function with the same name as the array.
    // The function takes an integer argument and returns
    // the corresponding element in the array.
    //
    prs_buffer.Put("    public final ");
    prs_buffer.Put(array_info.type_id == Table::B ? "boolean " : "int ");
    prs_buffer.Put(name);
    prs_buffer.Put("(int index) { return ");
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
void JavaTable::PrintNames()
{
    PrintHeader("String", "name");
    char tok[Control::SYMBOL_SIZE + 1];
    for (int i = 0; i < name_info.Size(); i++)
    {
        strcpy(tok, name_info[i]);
        prs_buffer.Pad();
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
            if (k == 30 && (! (j == len - 1)))
            {
                k = 0;
                prs_buffer.Put('\"');
                prs_buffer.Put(' ');
                prs_buffer.Put('+');
                prs_buffer.Put('\n');
                prs_buffer.Pad();
                prs_buffer.Put('\"');
            }
        }
        prs_buffer.Put('\"');
        if (i < name_info.Size() - 1)
            prs_buffer.Put(',');
        prs_buffer.Put('\n');
    }
    PrintTrailerAndVariable("String", "name");
    prs_buffer.Put("    public final String name(int index) { return name[index]; }\n\n");

    return;
}


void JavaTable::WriteInteger(int num)
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

void JavaTable::WriteData(TypeId type_id, Array<int> &array)
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
void JavaTable::Declare(int name_id, int type_id)
{
    prs_buffer.Put("    private static ");
    prs_buffer.Put(type_name[type_id]);
    prs_buffer.Put(' ');
    prs_buffer.Put(array_name[name_id]);
    prs_buffer.Put("[];\n");
    prs_buffer.Put("    public final ");
    prs_buffer.Put(type_id == Table::B ? "boolean " : "int ");
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
void JavaTable::Serialize(const char *variable, const char *method, int value)
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
    prs_buffer.Put("    private static int ");
    prs_buffer.Put(variable);
    prs_buffer.Put(";\n");
    prs_buffer.Put("    public final int ");
    prs_buffer.Put(method);
    prs_buffer.Put("() { return ");
    prs_buffer.Put(variable);
    prs_buffer.Put("; }\n\n");
}


//
//
//
void JavaTable::Serialize(const char *variable, const char *method, bool value)
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
    prs_buffer.Put("    private static boolean ");
    prs_buffer.Put(variable);
    prs_buffer.Put(";\n");
    prs_buffer.Put("    public final boolean ");
    prs_buffer.Put(method);
    prs_buffer.Put("() { return ");
    prs_buffer.Put(variable);
    prs_buffer.Put("; }\n\n");
}


//
//
//
void JavaTable::ConditionalDeserialize(const char *indentation, int name_id, int type_id)
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
void JavaTable::ConditionalSerialize(const char *indentation, IntArrayInfo &array_info)
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
void JavaTable::Serialize(IntArrayInfo &array_info)
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
void JavaTable::Serialize(const char *name, int max_length, IntArrayInfo &start, Array<const char *> &info)
{
    //
    // declare
    //
    prs_buffer.Put("    private static String ");
    prs_buffer.Put(name);
    prs_buffer.Put("[];\n");
    prs_buffer.Put("    public final String ");
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
void JavaTable::non_terminal_action(void)
{
    prs_buffer.Put("    /**\n"
                   "     * assert(goto_default);\n"
                   "     */\n"
                   "    public final int ntAction(int state, int sym) {\n"
                   "        return (baseCheck[state + sym] == sym)\n"
                   "                             ? baseAction[state + sym]\n"
                   "                             : defaultGoto[sym];\n"
                   "    }\n\n");
    return;
}


//
//
//
void JavaTable::non_terminal_no_goto_default_action(void)
{
    prs_buffer.Put("    /**\n"
                   "     * assert(! goto_default);\n"
                   "     */\n"
                   "    public final int ntAction(int state, int sym) {\n"
                   "        return baseAction[state + sym];\n"
                   "    }\n\n");

    return;
}


//
//
//
void JavaTable::terminal_action(void)
{
    prs_buffer.Put("    /**\n"
                   "     * assert(! shift_default);\n"
                   "     */\n"
                   "    public final int tAction(int state, int sym) {\n"
                   "        int i = baseAction[state],\n"
                   "            k = i + sym;\n"
                   "        return termAction[termCheck[k] == sym ? k : i];\n"
                   "    }\n"
                   "    public final int lookAhead(int la_state, int sym) {\n"
                   "        int k = la_state + sym;\n"
                   "        return termAction[termCheck[k] == sym ? k : la_state];\n"
                   "    }\n");

    return;
}


//
//
//
void JavaTable::terminal_shift_default_action(void)
{
    prs_buffer.Put("    /**\n"
                   "     * assert(shift_default);\n"
                   "     */\n"
                   "    public final int tAction(int state, int sym) {\n"
                   "        if (sym == 0)\n"
                   "            return ERROR_ACTION;\n"
                   "        int i = baseAction[state],\n"
                   "            k = i + sym;\n"
                   "        if (termCheck[k] == sym)\n"
                   "            return termAction[k];\n"
                   "        i = termAction[i];\n"
                   "        return (shiftCheck[shiftState[i] + sym] == sym\n"
                   "                                ? defaultShift[sym]\n"
                   "                                : defaultReduce[i]);\n"
                   "    }\n"
                   "    public final int lookAhead(int la_state, int sym) {\n"
                   "        int k = la_state + sym;\n"
                   "        if (termCheck[k] == sym)\n"
                   "            return termAction[k];\n"
                   "        int i = termAction[la_state];\n"
                   "        return (shiftCheck[shiftState[i] + sym] == sym\n"
                   "                                ? defaultShift[sym]\n"
                   "                                : defaultReduce[i]);\n"
                   "    }\n");
    return;
}


//
//
//
void JavaTable::init_file(FILE **file, const char *file_name)
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
void JavaTable::init_parser_files(void)
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
void JavaTable::exit_parser_files(void)
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
void JavaTable::print_symbols(void)
{
    Array<const char *> symbol_name(grammar -> num_terminals + 1);
    int symbol;
    char sym_line[Control::SYMBOL_SIZE +       /* max length of a token symbol  */
                  2 * MAX_PARM_SIZE + /* max length of prefix + suffix */
                  64];                /* +64 for error messages lines  */
                                  /* or other fillers(blank, =,...)*/

    strcpy(sym_line, "");
    if (strlen(option -> package) > 0)
    {
        strcat(sym_line, "package ");
        strcat(sym_line, option -> package);
        strcat(sym_line, ";\n\n");
    }
    strcat(sym_line, "public interface ");
    strcat(sym_line, option -> sym_type);
    strcat(sym_line, " {\n    public final static int\n");

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
            msg.Next() = " may be an invalid Java variable.";
            option -> EmitWarning(grammar -> RetrieveTokenLocation(symbol), msg);
        }
        else if (strpbrk(tok, "!%^&*()-+={}[];:\"`~|\\,.<>/?\'") != NULL)
        {
            Tuple<const char *> msg;
            msg.Next() = tok;
            msg.Next() = " is an invalid Java variable name.";
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

    fprintf(syssym, "\n    public final static String orderedTerminalSymbols[] = {\n");
    //                    "                 \"\",\n");
    for (int i = 0; i < grammar -> num_terminals; i++)
        fprintf(syssym, "                 \"%s\",\n", symbol_name[i]);
    fprintf(syssym, "                 \"%s\"\n             };\n",
            symbol_name[grammar -> num_terminals]);
    fprintf(syssym, "\n    public final static int numTokenKinds = orderedTerminalSymbols.length;");
    fprintf(syssym, "\n    public final static boolean isValidForParser = true;\n}\n");

    if (option -> serialize)
        Table::initialize(symbol_name, Table::SYMBOL_START, Table::symbol_start, Table::symbol_info, Table::max_symbol_length);

    return;
}


//
//
//
void JavaTable::print_exports(void)
{
    Array<const char *> symbol_name(grammar -> exported_symbols.Length() + 1);
    char exp_line[Control::SYMBOL_SIZE + 64];  /* max length of a token symbol  */
                                               /* +64 for error messages lines  */
                                               /* or other fillers(blank, =,...)*/

    strcpy(exp_line, "");
    if (strlen(option -> package) > 0)
    {
        strcat(exp_line, "package ");
        strcat(exp_line, option -> package);
        strcat(exp_line, ";\n\n");
    }
    strcat(exp_line, "public interface ");
    strcat(exp_line, option -> exp_type);
    strcat(exp_line, " {\n    public final static int\n");

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
            msg.Next() = " may be an invalid Java variable.";
            option -> EmitWarning(variable_symbol -> Location(), msg);
        }
        else if (strpbrk(tok, "!%^&*()-+={}[];:\"`~|\\,.<>/?\'") != NULL)
        {
            Tuple<const char *> msg;
            msg.Next() = "Exported symbol \"";
            msg.Next() = tok;
            msg.Next() = "\" is an invalid Java variable name.";
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
    fprintf(sysexp, "\n    public final static String orderedTerminalSymbols[] = {\n");
    //                    "                 \"\",\n");
    {
        for (int i = 0; i < grammar -> exported_symbols.Length(); i++)
        {
            fprintf(sysexp, "                 \"%s\",\n", symbol_name[i]);
            delete [] symbol_name[i];
        }
    }
    fprintf(sysexp, "                 \"%s\"\n             };\n",
            symbol_name[grammar -> exported_symbols.Length()]);
    delete [] symbol_name[grammar -> exported_symbols.Length()];

    fprintf(sysexp, "\n\n    public final static int numTokenKinds = orderedTerminalSymbols.length;");
    fprintf(sysexp, "\n    public final static boolean isValidForParser = false;\n}\n");

    return;
}

//
//
//
void JavaTable::print_definition(const char *variable, const char *method, int value)
{
    if (option -> serialize)
        Serialize(variable, method, value);
    else
    {
        prs_buffer.Put("    public final static int ");
        prs_buffer.Put(variable);
        prs_buffer.Put(" = ");
        prs_buffer.Put(value);
        prs_buffer.Put(";\n");
        prs_buffer.Put("    public final int ");
        prs_buffer.Put(method);
        prs_buffer.Put("() { return ");
        prs_buffer.Put(variable);
        prs_buffer.Put("; }\n\n");
    }
}


//
//
//
void JavaTable::print_definition(const char *variable, const char *method, bool value)
{
    if (option -> serialize)
        Serialize(variable, method, value);
    else
    {
        prs_buffer.Put("    public final static boolean ");
        prs_buffer.Put(variable);
        prs_buffer.Put(" = ");
        prs_buffer.Put(value ? "true" : "false");
        prs_buffer.Put(";\n");
        prs_buffer.Put("    public final boolean ");
        prs_buffer.Put(method);
        prs_buffer.Put("() { return ");
        prs_buffer.Put(variable);
        prs_buffer.Put("; }\n\n");
    }
}


//
//
//
void JavaTable::print_definitions(void)
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
    print_definition("SEGMENT_SIZE", "getSegmentSize", MAX_ARRAY_SIZE);
    print_definition("START_STATE", "getStartState", start_state);
    print_definition("IDENTIFIER_SYMBOL", "getIdentifier_SYMBOL", grammar -> identifier_image);
    print_definition("EOFT_SYMBOL", "getEoftSymbol", grammar -> eof_image);
    print_definition("EOLT_SYMBOL", "getEoltSymbol", grammar -> eol_image);
    print_definition("ACCEPT_ACTION", "getAcceptAction", accept_act);
    print_definition("ERROR_ACTION", "getErrorAction", error_act);
    print_definition("BACKTRACK", "getBacktrack", (bool) option -> backtrack);

    prs_buffer.Put("    public final int getStartSymbol() { return lhs(0); }\n"
                   "    public final boolean isValidForParser() { return ");
    if (option -> serialize)
        prs_buffer.Put("true");
    else
    {
        prs_buffer.Put(option -> sym_type);
        prs_buffer.Put(".isValidForParser");
    }
    prs_buffer.Put("; }\n\n");

    return;
}


//
//
//
void JavaTable::print_externs(void)
{
    if (option -> serialize || option -> error_maps || option -> debug)
    {
        prs_buffer.Put("    public final int originalState(int state) {\n");
        prs_buffer.Put("        return -baseCheck[state];\n");
        prs_buffer.Put("    }\n");
    }
    else
    {
        prs_buffer.Put("    public final int originalState(int state) { return 0; }\n");
    }

    if (option -> serialize || option -> error_maps)
    {
        prs_buffer.Put("    public final int asi(int state) {\n"
                       "        return asb[originalState(state)];\n"
                       "    }\n"
                       "    public final int nasi(int state) {\n"
                       "        return nasb[originalState(state)];\n"
                       "    }\n"
                       "    public final int inSymbol(int state) {\n"
                       "        return inSymb[originalState(state)];\n"
                       "    }\n");
    }
    else
    {
        prs_buffer.Put("    public final int asi(int state) { return 0; }\n"
                       "    public final int nasi(int state) { return 0; }\n"
                       "    public final int inSymbol(int state) { return 0; }\n");
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
void JavaTable::initialize_deserialize_buffer(void)
{
    des_buffer.Put("    private static int offset = 0;\n"
                   "\n"
                   "    static private int readInt(byte buffer[]) {\n"
                   "        return (int) ((buffer[offset++] << 24) +\n"
                   "                      ((buffer[offset++] & 0xFF) << 16) +\n"
                   "                      ((buffer[offset++] & 0xFF) << 8) +\n"
                   "                       (buffer[offset++] & 0xFF));\n"
                   "    }\n"
                   "\n"
                   "    static private short readShort(byte buffer[]) {\n"
                   "        return (short) (((buffer[offset++] & 0xFF) << 8) +\n"
                   "                (buffer[offset++] & 0xFF));\n"
                   "    }\n"
                   "\n"
                   "    static private char readChar(byte buffer[]) {\n"
                   "        return (char) (((buffer[offset++] & 0xFF) << 8) +\n"
                   "                        (buffer[offset++] & 0xFF));\n"
                   "    }\n"
                   "\n"
                   "    static private int[] readIntArray(byte buffer[]) throws java.io.IOException {\n"
                   "        int length = readInt(buffer);\n"
                   "        int type = readInt(buffer); assert(type <= 2147483647);\n"
                   "        int array[] = new int[length];\n"
                   "        for (int i = 0; i < length; i++)\n"
                   "            array[i] = readInt(buffer);\n"
                   "        return array;\n"
                   "    }\n"
                   "\n"
                   "    static private short[] readShortArray(byte buffer[]) throws java.io.IOException {\n"
                   "        int length = readInt(buffer);\n"
                   "        int type = readInt(buffer); assert(type <= 32767);\n"
                   "        short array[] = new short[length];\n"
                   "        for (int i = 0; i < length; i++)\n"
                   "            array[i] = readShort(buffer);\n"
                   "        return array;\n"
                   "    }\n"
                   "\n"
                   "    static private char[] readCharArray(byte buffer[]) throws java.io.IOException {\n"
                   "        int length = readInt(buffer);\n"
                   "        int type = readInt(buffer); assert(type <= 65535);\n"
                   "        char array[] = new char[length];\n"
                   "        for (int i = 0; i < length; i++)\n"
                   "            array[i] = readChar(buffer);\n"
                   "        return array;\n"
                   "    }\n"
                   "\n"
                   "    static private byte[] readByteArray(byte buffer[]) throws java.io.IOException {\n"
                   "        int length = readInt(buffer);\n"
                   "        int type = readInt(buffer); assert(type <= 127);\n"
                   "        byte array[] = new byte[length];\n"
                   "        System.arraycopy(buffer, offset, array, 0, length);\n"
                   "        offset += length;\n"
                   "        return array;\n"
                   "    }\n"
                   "\n"
                   "    static private int[] readArithmeticArray(byte buffer[]) throws java.io.IOException {\n"
                   "        int length = readInt(buffer),\n"
                   "            type = readInt(buffer),\n"
                   "            array[] = new int[length];\n"
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
                   "    static private String[] readStringArray(byte buffer[]) throws java.io.IOException {\n"
                   "        int string_length[] = readArithmeticArray(buffer);\n"
                   "        String array[] = new String[string_length.length];\n"
                   "        for (int i = 0; i < array.length; i++) {\n"
                   "            array[i] = new String(buffer, offset, string_length[i]);\n"
                   "            offset += string_length[i];\n"
                   "        }\n"
                   "        return array;\n"
                   "    }\n\n"
                   "    static {\n"
                   "        initialize(\"");
    des_buffer.PutStringLiteral(option -> dat_file);
    des_buffer.Put("\");\n"
                   "    }\n\n"
                   "    public static void initialize(String filename) {\n"
                   "        initialize(new java.io.File(filename));\n"
                   "    }\n\n"
                   "    public static void initialize(java.io.File file) {\n"
                   "        try {\n"
                   "            java.io.FileInputStream infile = new java.io.FileInputStream(file);\n"
                   "            final byte buffer[] = new byte[(int) file.length()];\n"
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
void JavaTable::print_serialized_tables(void)
{
    Array<IntArrayInfo *> array_info(num_name_ids);
    array_info.MemReset(); // set all elements to NULL
    for (int i = 0; i < data.Length(); i++)
        array_info[data[i].name_id] = &data[i];

    Serialize(*array_info[NULLABLES]);
    Serialize(*array_info[PROSTHESES_INDEX]);
    Serialize(*array_info[KEYWORDS]);
    Serialize(*array_info[BASE_CHECK]);
        prs_buffer.Put("    public final int rhs(int index) { return ");
        prs_buffer.Put(array_name[BASE_CHECK]);
        prs_buffer.Put("[index]; };\n");
    Serialize(*array_info[BASE_ACTION]);
        prs_buffer.Put("    public final int lhs(int index) { return ");
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
void JavaTable::print_source_tables(void)
{
    for (int i = 0; i < data.Length(); i++)
    {
        IntArrayInfo &array_info = data[i];
        Print(array_info);
        switch(array_info.name_id)
        {
            case BASE_CHECK:
                prs_buffer.Put("    public final static ");
                prs_buffer.Put(type_name[array_info.type_id]);
                prs_buffer.Put(" rhs[] = ");
                prs_buffer.Put(array_name[array_info.name_id]);
                prs_buffer.Put(";\n"
                               "    public final int rhs(int index) { return rhs[index]; };\n");

                break;
            case BASE_ACTION:
                prs_buffer.Put("    public final static ");
                prs_buffer.Put(type_name[array_info.type_id]);
                prs_buffer.Put(" lhs[] = ");
                prs_buffer.Put(array_name[array_info.name_id]);
                prs_buffer.Put(";\n"
                               "    public final int lhs(int index) { return lhs[index]; };\n");
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
            prs_buffer.Put("    public final static int scopePrefix[] = null;\n"
                           "    public final int scopePrefix(int index) { return 0;}\n\n"
                           "    public final static int scopeSuffix[] = null;\n"
                           "    public final int scopeSuffix(int index) { return 0;}\n\n"
                           "    public final static int scopeLhs[] = null;\n"
                           "    public final int scopeLhs(int index) { return 0;}\n\n"
                           "    public final static int scopeLa[] = null;\n"
                           "    public final int scopeLa(int index) { return 0;}\n\n"
                           "    public final static int scopeStateSet[] = null;\n"
                           "    public final int scopeStateSet(int index) { return 0;}\n\n"
                           "    public final static int scopeRhs[] = null;\n"
                           "    public final int scopeRhs(int index) { return 0;}\n\n"
                           "    public final static int scopeState[] = null;\n"
                           "    public final int scopeState(int index) { return 0;}\n\n"
                           "    public final static int inSymb[] = null;\n"
                           "    public final int inSymb(int index) { return 0;}\n\n");
        }

        PrintNames();
    }
    else
    {
        prs_buffer.Put("    public final int asb(int index) { return 0; }\n"
                       "    public final int asr(int index) { return 0; }\n"
                       "    public final int nasb(int index) { return 0; }\n"
                       "    public final int nasr(int index) { return 0; }\n"
                       "    public final int terminalIndex(int index) { return 0; }\n"
                       "    public final int nonterminalIndex(int index) { return 0; }\n"
                       "    public final int scopePrefix(int index) { return 0;}\n"
                       "    public final int scopeSuffix(int index) { return 0;}\n"
                       "    public final int scopeLhs(int index) { return 0;}\n"
                       "    public final int scopeLa(int index) { return 0;}\n"
                       "    public final int scopeStateSet(int index) { return 0;}\n"
                       "    public final int scopeRhs(int index) { return 0;}\n"
                       "    public final int scopeState(int index) { return 0;}\n"
                       "    public final int inSymb(int index) { return 0;}\n"
                       "    public final String name(int index) { return null; }\n");
    }

    return;
}


//
//
//
void JavaTable::PrintTables(void)
{
    init_parser_files();

    print_symbols();

    if (grammar -> exported_symbols.Length() > 0)
        print_exports();

    //
    // Now process the parse file
    //
    if (strlen(option -> package) > 0)
    {
        prs_buffer.Put("package ");
        prs_buffer.Put(option -> package);
        prs_buffer.Put(";\n\n");
    }
    prs_buffer.Put("public class ");
    prs_buffer.Put(option -> prs_type);
    if (option -> extends_parsetable)
    {
        prs_buffer.Put(" extends ");
        prs_buffer.Put(option -> extends_parsetable);
    }
    prs_buffer.Put(" implements ");
    if (option -> parsetable_interfaces)
    {
        prs_buffer.Put(option -> parsetable_interfaces);
        prs_buffer.Put(", ");
    }
    prs_buffer.Put(option -> sym_type);
    prs_buffer.Put(" {\n");

    if (option -> serialize)
        initialize_deserialize_buffer();

    print_definitions();

    if (option -> serialize)
         print_serialized_tables();
    else print_source_tables();

    print_externs();

    prs_buffer.Put("}\n");
    prs_buffer.Flush();

    exit_parser_files();

    return;
}
