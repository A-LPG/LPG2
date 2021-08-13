#include "partition.h"
#include "CTable.h"

#include <iostream>
using namespace std;

//
//
//
void CTable::Print(const char *name, int type_id, Array<int> &array)

{
    const char *type = type_name[type_id];

    int init = (array[0] == 0 ? 1 : 0);

    dcl_buffer.Put("\nconst ");
    dcl_buffer.Put(type);
    dcl_buffer.Put(' ');
    dcl_buffer.Put(parse_table_class_name_prefix);
    dcl_buffer.Put(name);
    dcl_buffer.Put("[] = {");
    dcl_buffer.Put(init == 1 ? "0,\n" : "\n");

    dcl_buffer.Pad();
    int k = 0;
    for (int i = init; i < array.Size(); i++)
    {
        dcl_buffer.Put(array[i]);
        dcl_buffer.Put(',');
        k++;
        if (k == 10 && i != array.Size() - 1)
        {
            dcl_buffer.Put('\n');
            dcl_buffer.Pad();
            k = 0;
        }
    }
    if (k != 0)
    {
        dcl_buffer.UnputChar(); // remove last comma, if possible
        dcl_buffer.Put('\n');
    }

    dcl_buffer.Put("        };\n");
}


//
//
//
void CTable::non_terminal_action(void)
{
    fprintf(sysprs,
            "#define nt_action(state, sym) \\\n"
            "           ((base_check[state + sym] == sym) ? \\\n"
            "               base_action[state + sym] : "
                               "default_goto[sym])\n\n");
    return;
}


//
//
//
void CTable::non_terminal_no_goto_default_action(void)
{
    fprintf(sysprs,
            "#define nt_action(state, sym) base_action[state + sym]\n\n");
    return;
}


//
//
//
void CTable::terminal_action(void)
{
    fprintf(sysprs,
            "#define t_action(state, sym) \\\n"
            "  term_action[term_check[base_action[state]+sym] == sym ? \\\n"
            "          base_action[state] + sym : base_action[state]]\n\n"
            "#define look_ahead(la_state, sym) \\\n"
            "  term_action[term_check[la_state+sym] == sym ? la_state+sym : la_state]\n");
    return;
}


//
//
//
void CTable::terminal_shift_default_action(void)
{
    fprintf(sysprs,
            "static int t_action(int state, int sym)\n"
            "{\n"
            "    int i;\n\n"
            "    if (sym == 0)\n"
            "        return ERROR_ACTION;\n"
            "    i = base_action[state];\n"
            "    if (term_check[i + sym] == sym)\n"
            "        return term_action[i + sym];\n"
            "    i = term_action[i];\n"
            "    return ((shift_check[shift_state[i] + sym] == sym) ?\n"
            "                 default_shift[sym] : default_reduce[i]);\n"
            "}\n\n"
            "static int look_ahead(int la_state, int sym)\n"
            "{\n"
            "    int i;\n\n"
            "    i = la_state + sym;\n"
            "    if (term_check[i] == sym)\n"
            "        return term_action[i];\n"
            "    i = term_action[la_state];\n"
            "    return (shift_check[shift_state[i] + sym] == sym\n"
            "                               ? default_shift[sym]\n"
            "                               : default_reduce[i]);\n"
            "    }\n"
            "    return;\n"
            "}\n\n"
            "#define t_action(state, sym, next_tok) t_action1(state, sym)\n\n");

    return;
}


//
//
//
void CTable::init_file(FILE **file, const char *file_name, const char *file_type, bool header_file)
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

    if (header_file)
    {
        fprintf(*file, "#ifndef %s_INCLUDED\n", file_type);
        fprintf(*file, "#define %s_INCLUDED\n\n", file_type);
    }

    return;
}


//
//
//
void CTable::init_parser_files(void)
{
    init_file(&sysdcl, option -> dcl_file, option -> dcl_type, false);
    init_file(&syssym, option -> sym_file, option -> sym_type);
    init_file(&sysprs, option -> prs_file, option -> prs_type);
    if (grammar -> exported_symbols.Length() > 0)
        init_file(&sysexp, option -> exp_file, option -> exp_type);

    return;
}


//
//
//
void CTable::exit_parser_files(void)
{
    fprintf(syssym, "\n#endif /* %s_INCLUDED */\n", option -> sym_type);
    fprintf(sysprs, "\n#endif /* %s_INCLUDED */\n", option -> prs_type);
    if (grammar -> exported_symbols.Length() > 0)
        fprintf(sysexp, "\n#endif /* %s_INCLUDED */\n", option -> exp_type);

    fclose(sysdcl);
    fclose(syssym);
    fclose(sysprs);
    if (grammar -> exported_symbols.Length() > 0)
        fclose(sysexp);

    return;
}


//
//
//
void CTable::PrintNames(void)
{
    dcl_buffer.Put("\nconst char ");
    dcl_buffer.Put(parse_table_class_name_prefix);
    dcl_buffer.Put("string_buffer[] = {0,\n");
    dcl_buffer.Pad();
    int n = 0;
    for (int i = 0; i < name_info.Size(); i++)
    {
        int k = 0;
        const char *name = name_info[i];
        int length = Length(name_start, i);
        for (int j = 0; j < length; j++)
        {
            dcl_buffer.Put('\'');
            if (name[k] == '\'' || name[k] == '\\')
                 dcl_buffer.Put('\\');
            if (name[k] == '\n')
                 dcl_buffer.Put(option ->macro_prefix);
            else dcl_buffer.Put(name[k]);
            k++;
            dcl_buffer.Put('\'');
            dcl_buffer.Put(',');
            n++;
            if (n == 10 && ! (i == (name_info.Size() - 1) && j == length - 1))
            {
                n = 0;
                dcl_buffer.Put('\n');
                dcl_buffer.Pad();
            }
        }
    }
    dcl_buffer.UnputChar();
    dcl_buffer.Put('\n');  // overwrite last comma
    dcl_buffer.Put("        };\n");

    Print(name_start);

    return;
}


//
//
//
void CTable::print_symbols(void)
{
    int symbol;
    char sym_line[Control::SYMBOL_SIZE +       /* max length of a token symbol  */
                  2 * MAX_PARM_SIZE + /* max length of prefix + suffix */
                  64];                /* +64 for error messages lines  */
                                  /* or other fillers(blank, =,...)*/

    strcpy(sym_line, "    enum\n    {\n");

    //
    // we write the terminal symbols map.
    //
    for (symbol = grammar -> FirstTerminal(); symbol <= grammar -> LastTerminal(); symbol++)
    {
        char *tok = grammar -> RetrieveString(symbol);

        fprintf(syssym, "%s", sym_line);

        if (tok[0] == '\n' || tok[0] == option ->macro_prefix)
        {
            tok[0] = option ->macro_prefix;

            Tuple<const char *> msg;
            msg.Next() = "Escaped symbol ";
            msg.Next() = tok;
            msg.Next() = " may be an invalid C/C++ variable.";
            option -> EmitWarning(grammar -> RetrieveTokenLocation(symbol), msg);
        }
        else if (strpbrk(tok, "!%^&*()-+={}[];:\"`~|\\,.<>/?\'") != NULL)
        {
            Tuple<const char *> msg;
            msg.Next() = tok;
            msg.Next() = " is an invalid C/C++ variable name.";
            option -> EmitError(grammar -> RetrieveTokenLocation(symbol), msg);
        }

        strcpy(sym_line, "        ");
        strcat(sym_line, option -> prefix);
        strcat(sym_line, tok);
        strcat(sym_line, option -> suffix);
        strcat(sym_line, " = ");
        IntToString num(symbol_map[symbol]);
        strcat(sym_line, num.String());
        strcat(sym_line, ",\n");

        while(strlen(sym_line) > PARSER_LINE_SIZE)
        {
            fwrite(sym_line, sizeof(char), PARSER_LINE_SIZE - 2, syssym);
            fprintf(syssym, "\\\n");
            strcpy(sym_line, &sym_line[PARSER_LINE_SIZE - 2]);
        }
    }

    fprintf(syssym, "%s", sym_line);
    fprintf(syssym, "\n        NUM_TOKENS = %d,\n", grammar -> num_terminals);
    fprintf(syssym, "\n        IS_VALID_FOR_PARSER = 1\n    };\n");

    return;
}


//
//
//
void CTable::print_exports(void)
{
    char exp_line[Control::SYMBOL_SIZE + 64]; /* max length of a token symbol  */
                                              /* +64 for error messages lines  */
                                              /* or other fillers(blank, =,...)*/

    strcpy(exp_line, "    enum\n    {\n");

    //
    // We write the exported terminal symbols and map
    // them according to the order in which they were specified.
    //
    for (int i = 1; i <= grammar -> exported_symbols.Length(); i++)
    {
        VariableSymbol *variable_symbol = grammar -> exported_symbols[i - 1];
        char *tok = new char[variable_symbol -> NameLength() + 1];
        strcpy(tok, variable_symbol -> Name());

        fprintf(sysexp, "%s", exp_line);

        if (tok[0] == '\n' || tok[0] == option ->macro_prefix)
        {
            tok[0] = option ->macro_prefix;

            Tuple<const char *> msg;
            msg.Next() = "Escaped exported symbol ";
            msg.Next() = tok;
            msg.Next() = " may be an invalid C/C++ variable.";
            option -> EmitWarning(variable_symbol -> Location(), msg);
        }
        else if (strpbrk(tok, "!%^&*()-+={}[];:\"`~|\\,.<>/?\'") != NULL)
        {
            Tuple<const char *> msg;
            msg.Next() = "Exported symbol \"";
            msg.Next() = tok;
            msg.Next() = "\" is an invalid C/C++ variable name.";
            option -> EmitError(variable_symbol -> Location(), msg);
        }

        strcpy(exp_line, "        ");
        strcat(exp_line, option -> exp_prefix);
        strcat(exp_line, tok);
        strcat(exp_line, option -> exp_suffix);
        strcat(exp_line, " = ");
        IntToString num(i);
        strcat(exp_line, num.String());
        strcat(exp_line, ",\n");
                          
        while(strlen(exp_line) > PARSER_LINE_SIZE)
        {
            fwrite(exp_line, sizeof(char), PARSER_LINE_SIZE - 2, sysexp);
            fprintf(sysexp, "\\\n");
            strcpy(exp_line, &exp_line[PARSER_LINE_SIZE - 2]);
        }

        delete [] tok;
    }

    fprintf(sysexp, "%s", exp_line);
    fprintf(sysexp, "\n        NUM_TOKENS = %i,\n", grammar -> exported_symbols.Length());
    fprintf(sysexp, "        IS_VALID_FOR_PARSER = 0\n    };\n");

    return;
}


//
//
//
void CTable::print_definitions(void)
{
    fprintf(sysprs, "enum {\n");

    if (option -> error_maps)
    {
        fprintf(sysprs, "      ERROR_SYMBOL      = %d,\n"
                        "      SCOPE_UBOUND      = %d,\n"
                        "      SCOPE_SIZE        = %d,\n"
                        "      MAX_NAME_LENGTH   = %d,\n"
                        "      MAX_TERM_LENGTH   = %d,\n"
                        "      NUM_STATES        = %d,\n\n",

                        grammar -> error_image,
                        pda -> scope_prefix.Size() - 1,
                        pda -> scope_prefix.Size(),
                        max_name_length,
                        max_name_length,
                        pda -> num_states);
    }

    fprintf(sysprs,
                 "      NT_OFFSET         = %d,\n"
                 "      LA_STATE_OFFSET   = %d,\n"
                 "      MAX_LA            = %d,\n"
                 "      NUM_RULES         = %d,\n"
                 "      NUM_NONTERMINALS  = %d,\n"
                 "      NUM_SYMBOLS       = %d,\n"
                 "      START_STATE       = %d,\n"
                 "      IDENTIFIER_SYMBOL = %d,\n"
                 "      EOFT_SYMBOL       = %d,\n"
                 "      EOLT_SYMBOL       = %d,\n"
                 "      ACCEPT_ACTION     = %d,\n"
                 "      ERROR_ACTION      = %d,\n"
                 "      BACKTRACK         = %d\n"
                 "     };\n\n",


                 grammar -> num_terminals,
                 (option -> read_reduce ? error_act + grammar -> num_rules : error_act),
                 pda -> highest_level,
                 grammar -> num_rules,
                 grammar -> num_nonterminals,
                 grammar -> num_symbols,
                 start_state,
                 grammar -> identifier_image,
                 grammar -> eof_image,
                 grammar -> eol_image,
                 accept_act,
                 error_act,
                 (option -> backtrack ? 1 : 0));

    return;
}


//
//
//
void CTable::print_externs_and_definitions(void)
{
    fprintf(sysprs,
            "#include \"%s\"\n\n",
            option -> GetFilename(option -> sym_file));

    if (option -> error_maps || option -> debug)
    {
        fprintf(sysprs,
                "#define original_state(state) (-%s[state])\n",
                "base_check");
    }
    if (option -> error_maps)
    {
        fprintf(sysprs,
                "#define asi(state)            asb[original_state(state)]\n"
                "#define nasi(state)           nasb[original_state(state)]\n"
                "#define in_symbol(state)      in_symb[original_state(state)]\n\n");
    }

    print_definitions();

    for (int i = 0; i < data.Length(); i++)
    {
        IntArrayInfo &array_info = data[i];
        const char *name = array_name[array_info.name_id];
        const char *type = type_name[array_info.type_id];
        switch(array_info.name_id)
        {
            case BASE_CHECK:
                 fprintf(sysprs, "extern const %s base_check[];\n"
                                 "extern const %s *rhs;\n",
                                 type,
                                 type);
                 break;
            case BASE_ACTION:
                 fprintf(sysprs, "extern const %s base_action[];\n"
                                 "extern const %s *lhs;\n",
                                 type,
                                 type);
                 break;
            default:
                 fprintf(sysprs, "extern const %s %s[];\n",
                                 type, name);
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
            fprintf(sysprs, "extern const unsigned char scope_prefix[];\n"
                            "extern const unsigned char scope_suffix[];\n"
                            "extern const unsigned char scope_lhs[];\n"
                            "extern const unsigned char scope_la[];\n"
                            "extern const unsigned char scope_state_set[];\n"
                            "extern const unsigned char scope_rhs[];\n"
                            "extern const unsigned char scope_state[];\n"
                            "extern const unsigned char in_symb[];\n");
        }

        fprintf(sysprs, "extern const %s name_start[];\n"
                        "extern const char string_buffer[];\n",
                        type_name[name_start.type_id]);
        fprintf(sysprs, "\n");
    }

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
void CTable::print_tables(void)
{
    dcl_buffer.Put("#include \"");
    dcl_buffer.Put(option -> GetFilename(option -> prs_file));
    dcl_buffer.Put("\"\n\n");


    for (int i = 0; i < data.Length(); i++)
    {
        IntArrayInfo &array_info = data[i];
        Print(array_info);
        switch(array_info.name_id)
        {
            case BASE_CHECK:
                dcl_buffer.Put("\nconst ");
                dcl_buffer.Put(type_name[array_info.type_id]);
                dcl_buffer.Put(" *");
                dcl_buffer.Put(parse_table_class_name_prefix);
                dcl_buffer.Put("rhs = ");
                dcl_buffer.Put(array_name[array_info.name_id]);
                dcl_buffer.Put(";\n");
                break;
            case BASE_ACTION:
                dcl_buffer.Put("\nconst ");
                dcl_buffer.Put(type_name[array_info.type_id]);
                dcl_buffer.Put(" *");
                dcl_buffer.Put(parse_table_class_name_prefix);
                dcl_buffer.Put("lhs = ");
                dcl_buffer.Put(array_name[array_info.name_id]);
                dcl_buffer.Put(";\n");
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
            dcl_buffer.Put("\n");

            dcl_buffer.Put("const unsigned char ");
            dcl_buffer.Put(parse_table_class_name_prefix);
            dcl_buffer.Put("scope_prefix[] = {0};\n");

            dcl_buffer.Put("const unsigned char ");
            dcl_buffer.Put(parse_table_class_name_prefix);
            dcl_buffer.Put("scope_suffix[] = {0};\n");

            dcl_buffer.Put("const unsigned char ");
            dcl_buffer.Put(parse_table_class_name_prefix);
            dcl_buffer.Put("scope_lhs[] = {0};\n");

            dcl_buffer.Put("const unsigned char ");
            dcl_buffer.Put(parse_table_class_name_prefix);
            dcl_buffer.Put("scope_la[] = {0};\n");

            dcl_buffer.Put("const unsigned char ");
            dcl_buffer.Put(parse_table_class_name_prefix);
            dcl_buffer.Put("scope_state_set[] = {0};\n");

            dcl_buffer.Put("const unsigned char ");
            dcl_buffer.Put(parse_table_class_name_prefix);
            dcl_buffer.Put("scope_rhs[] = {0};\n");

            dcl_buffer.Put("const unsigned char ");
            dcl_buffer.Put(parse_table_class_name_prefix);
            dcl_buffer.Put("scope_state[] = {0};\n");

            dcl_buffer.Put("const unsigned char ");
            dcl_buffer.Put(parse_table_class_name_prefix);
            dcl_buffer.Put("in_symb[] = {0};\n");
        }

        PrintNames();
    }

    dcl_buffer.Flush();

    return;
}


//
//
//
void CTable::PrintTables(void)
{
    init_parser_files();

    strcpy(parse_table_class_name, "");
    strcpy(parse_table_class_name_prefix, "");

    print_tables();

    print_symbols();
    if (grammar -> exported_symbols.Length() > 0)
        print_exports();
    print_externs_and_definitions();

    exit_parser_files();

    return;
}


