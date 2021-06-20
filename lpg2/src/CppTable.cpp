#include "partition.h"
#include "CppTable.h"

#include <iostream>
using namespace std;

void CppTable::non_terminal_action(void)
{
    fprintf(sysprs,
            "    static inline int nt_action(int state, int sym)\n"
            "    {\n"
            "        return (base_check[state + sym] == sym)\n"
            "                             ? base_action[state + sym]\n"
            "                             : default_goto[sym];\n"
            "    }\n\n");
    return;
}


void CppTable::non_terminal_no_goto_default_action(void)
{
    fprintf(sysprs,
            "    static inline int nt_action(int state, int sym)\n"
            "    {\n        return base_action[state + sym];\n    }\n\n");

    return;
}


void CppTable::terminal_action(void)
{
    fprintf(sysprs,
            "    static inline int t_action(int state, int sym)\n"
            "    {\n"
            "        int i = base_action[state],\n"
            "            k = i + sym;\n"
            "        return term_action[term_check[k] == sym ? k : i];\n"
            "    }\n\n"
            "    static inline int look_ahead(int la_state, int sym)\n"
            "    {\n"
            "        int k = la_state + sym;\n"
            "        return term_action[term_check[k] == sym ? k : la_state];\n"
            "    }\n");
    return;
}


void CppTable::terminal_shift_default_action(void)
{
    fprintf(sysprs,
            "    static inline int t_action(int state, int sym)\n"
            "    {\n"
            "        if (sym == 0)\n"
            "            return ERROR_ACTION;\n"
            "        int i = base_action[state],\n"
            "            k = i + sym;\n"
            "        if (term_check[k] == sym)\n"
            "            return term_action[k];\n"
            "        i = term_action[i];\n"
            "        return (shift_check[shift_state[i] + sym] == sym\n"
            "                     ? default_shift[sym]\n"
            "                     : default_reduce[i]);\n"
            "    }\n\n"
            "    static inline int look_ahead(int la_state, int sym)\n"
            "    {\n"
            "        int k = la_state + sym;\n"
            "        if (term_check[k] == sym)\n"
            "            return term_action[k];\n"
            "        int i = term_action[la_state];\n"
            "        return (shift_check[shift_state[i] + sym] == sym\n"
            "                     ? default_shift[sym]\n"
            "                     : default_reduce[i]);\n"
            "    }\n");

    return;
}

//
//
//
void CppTable::print_definitions(void)
{
    fprintf(sysprs, "\n    enum {\n");

    if (option -> error_maps)
    {
        fprintf(sysprs, "          ERROR_SYMBOL      = %d,\n"
                        "          SCOPE_UBOUND      = %d,\n"
                        "          SCOPE_SIZE        = %d,\n"
                        "          MAX_NAME_LENGTH   = %d,\n"
                        "          MAX_TERM_LENGTH   = %d,\n"
                        "          NUM_STATES        = %d,\n\n",

                        grammar -> error_image,
                        pda -> scope_prefix.Size() - 1,
                        pda -> scope_prefix.Size(),
                        max_name_length,
                        max_name_length,
                        pda -> num_states);
    }

    fprintf(sysprs,
                 "          NT_OFFSET         = %d,\n"
                 "          LA_STATE_OFFSET   = %d,\n"
                 "          MAX_LA            = %d,\n"
                 "          NUM_RULES         = %d,\n"
                 "          NUM_TERMINALS     = %d,\n"
                 "          NUM_NONTERMINALS  = %d,\n"
                 "          NUM_SYMBOLS       = %d,\n"
                 "          START_STATE       = %d,\n"
                 "          IDENTIFIER_SYMBOL = %d,\n"
                 "          EOFT_SYMBOL       = %d,\n"
                 "          EOLT_SYMBOL       = %d,\n"
                 "          ACCEPT_ACTION     = %d,\n"
                 "          ERROR_ACTION      = %d,\n"
                 "          BACKTRACK         = %d\n"
                 "         };\n\n",


                 grammar -> num_terminals,
                 (option -> read_reduce ? error_act + grammar -> num_rules : error_act),
                 pda -> highest_level,
                 grammar -> num_rules,
                 grammar -> num_terminals,
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


void CppTable::print_externs_and_definitions(void)
{
    fprintf(sysprs,
            "#include \"%s\"\n\n"
            "#include <stdio.h> // Need definition for NULL\n\n",

            option -> GetFilename(option -> sym_file));

    fprintf(sysprs,
            "class %s\n"
            "{\n"
            "public:\n",

            parse_table_class_name);

    print_definitions();

    if (option -> error_maps || option -> debug)
        fprintf(sysprs,
                "    static inline int original_state(int state) "
                "{ return -%s[state]; }\n",

                "base_check");

    if (option -> error_maps)
    {
        fprintf(sysprs,
                "    static inline int asi(int state) { return asb[original_state(state)]; }\n"
                "    static inline int nasi(int state) { return nasb[original_state(state)]; }\n"
                "    static inline int in_symbol(int state) { return in_symb[original_state(state)]; }\n");
    }

    fprintf(sysprs, "\n");

    for (int i = 0; i < data.Length(); i++)
    {
        IntArrayInfo &array_info = data[i];
        const char *name = array_name[array_info.name_id];
        const char *type = type_name[array_info.type_id];
        switch(array_info.name_id)
        {
            case BASE_CHECK:
                 fprintf(sysprs, "    static const %s base_check[];\n"
                                 "    static const %s *rhs;\n",
                                 type, type);
                 break;
            case BASE_ACTION:
                 fprintf(sysprs, "    static const %s base_action[];\n"
                                 "    static const %s *lhs;\n",
                                 type, type);
                 break;
            default:
                 fprintf(sysprs, "    static const %s %s[];\n",
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
            fprintf(sysprs, "    static const unsigned char scope_prefix[];\n"
                            "    static const unsigned char scope_suffix[];\n"
                            "    static const unsigned char scope_lhs[];\n"
                            "    static const unsigned char scope_la[];\n"
                            "    static const unsigned char scope_state_set[];\n"
                            "    static const unsigned char scope_rhs[];\n"
                            "    static const unsigned char scope_state[];\n"
                            "    static const unsigned char in_symb[];\n");
        }

        fprintf(sysprs, "    static const %s name_start[];\n"
                        "    static const char string_buffer[];\n"
                        "    static inline int name_length(int i) { return name_start[i + 1] - name_start[i]; }\n",
                        type_name[name_start.type_id]);
        fprintf(sysprs, "\n");
    }

    if (option -> goto_default)
         non_terminal_action();
    else non_terminal_no_goto_default_action();

    if (option -> shift_default)
         terminal_shift_default_action();
    else terminal_action();

    fprintf(sysprs, "};\n");

    return;
}


void CppTable::PrintTables(void)
{
    init_parser_files();

    strcpy(parse_table_class_name, option -> prs_type);

    strcpy(parse_table_class_name_prefix, parse_table_class_name);
    strcat(parse_table_class_name_prefix, "::");

    print_tables();

    print_symbols();

    if (grammar -> exported_symbols.Length() > 0)
        print_exports();

    print_externs_and_definitions();

    exit_parser_files();

    return;
}
