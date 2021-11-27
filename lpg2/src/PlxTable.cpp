#include "tuple.h"
#include "PlxTable.h"

#include <iostream>
using namespace std;

//
//
//
void PlxTable::Print(const char *name, int type_id, Array<int> &array, int lbound, int ubound)
{
    const char *type = type_name[type_id];

    dcl_buffer.Put(" dcl ");
    dcl_buffer.Put(name);
    dcl_buffer.Put('(');
    if (lbound != 1)
    {
        dcl_buffer.Put(lbound);
        dcl_buffer.Put(':');
    }
    dcl_buffer.Put(ubound);
    dcl_buffer.Put(") ");
    dcl_buffer.Put(type);
    dcl_buffer.Put(" STATIC INITIAL(\n");

    dcl_buffer.Pad();
    int k = 0;
    for (int i = lbound; i <= ubound; i++)
    {
        dcl_buffer.Put(array[i]);
        dcl_buffer.Put(',');
        k++;
        if (k == 10 && i != ubound)
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

    dcl_buffer.Put("        );\n");
}


//
//
//
void PlxTable::non_terminal_action(void)
{
    fprintf(sysprs,
            " nt_action:Procedure(state, sym) returns (FIXED(31));\n"
            "     dcl state fixed(31),\n"
            "         sym fixed(31),\n"
            "         act fixed(31);\n"
            "     if (base_check(state + sym) = sym) then\n"
            "          act = base_action(state + sym);\n"
            "     else act = default_goto(sym);\n"
            "     return (act);\n"
            " end;\n\n");
    return;
}


//
//
//
void PlxTable::non_terminal_no_goto_default_action(void)
{
    fprintf(sysprs,
            " nt_action:Procedure(state, sym) returns (FIXED(31));\n"
            "     dcl state fixed(31),\n"
            "         sym fixed(31);\n"
            "     return (base_action(state + sym));\n"
            " end;\n\n");
    return;
}


//
//
//
void PlxTable::terminal_action(void)
{
    fprintf(sysprs,
            " t_action:Procedure(state, sym) returns (FIXED(31));\n"
            "     dcl state fixed(31),\n"
            "         sym fixed(31),\n"
            "         act fixed(31),\n"
            "         i fixed(31);\n"
            "     act = base_action(state);\n"
            "     i = act + sym;\n"
            "     if (term_check(i) = sym) then\n"
            "          act = term_action(i);\n"
            "     else act = term_action(act);\n"
            "     return (act);\n"
            " end;\n\n"
            " look_ahead:Procedure(la_state, sym) returns (FIXED(31));\n"
            "     dcl la_state fixed(31),\n"
            "         sym fixed(31),\n"
            "         act fixed(31),\n"
            "         i fixed(31);\n"
            "     i = la_state + sym;\n"
            "     if (term_check(i) = sym) then\n"
            "          act = term_action(i);\n"
            "     else act = term_action(la_state);\n"
            "     return (act);\n"
            " end;\n\n");

    return;
}


//
//
//
void PlxTable::terminal_shift_default_action(void)
{
    fprintf(sysprs,
            " t_action:Procedure(state, sym) returns (FIXED(31));\n"
            "     dcl state fixed(31),\n"
            "         sym fixed(31),\n"
            "         i fixed(31);\n"
            "     if (sym = 0) then\n"
            "         return (ERROR_ACTION);\n"
            "     i = base_action(state);\n"
            "     if (term_check(i + sym) = sym) then\n"
            "         return (term_action(i + sym));\n"
            "     i = term_action(i);\n"
            "     if (shift_check(shift_state(i) + sym) = sym) then\n"
            "          return (default_shift(sym));\n"
            "     else return (default_reduce(i));\n"
            " end;\n\n"
            " look_ahead:Procedure(la_state, sym) returns (FIXED(31));\n"
            "     dcl la_state fixed(31),\n"
            "         sym fixed(31),\n"
            "         i fixed(31);\n"
            "     i = la_state + sym;\n"
            "     if (term_check(i) = sym) then\n"
            "         return (term_action(i));\n"
            "     i = term_action(la_state);\n"
            "     if (shift_check(shift_state(i) + sym) = sym) then\n"
            "          return (default_shift(sym));\n"
            "     else return (default_reduce(i));\n"
            " end;\n\n");

    return;
}


//
//
//
void PlxTable::init_file(FILE **file, const char *file_name)
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
void PlxTable::init_parser_files(void)
{
    init_file(&sysdcl, option -> dcl_file);
    init_file(&sysdef, option -> def_file);
    init_file(&syssym, option -> sym_file);
    init_file(&sysprs, option -> prs_file);
    init_file(&sysimp, option -> imp_file);
    if (grammar -> exported_symbols.Length() > 0)
        init_file(&sysexp, option -> exp_file);

    return;
}


//
//
//
void PlxTable::exit_parser_files(void)
{
    fclose(sysdcl);sysdcl=NULL;
    fclose(sysdef);sysdef=NULL;
    fclose(syssym);syssym=NULL;
    fclose(sysprs);sysprs=NULL;
    fclose(sysimp);sysimp=NULL;
    if (grammar -> exported_symbols.Length() > 0) {
        fclose(sysexp);sysexp=NULL;
    }
}


//
// This code is used to mimic the mapping of semantic action functions
// to rules as is done in the LALONDE parser generator.
//
void PlxTable::PrintSemanticFunctionsMap(void)
{
    BoundedArray<int> count(grammar -> num_terminals + 1, grammar -> num_symbols);
    count.MemReset();
    for (int i = 1; i <= grammar -> num_rules; i++)
    {
        char *name = grammar -> RetrieveString(grammar -> rules[i].lhs);
/************************** Unneeded Test? ****************************
        if (count[grammar -> rules[i].lhs] == 99)
        {
            Tuple<const char *> msg;
            msg.Next() = "Nonterminal \"";
            msg.Next() = name;
            msg.Next() = "\" produces more than 99 rules.";
            option -> EmitError(0, msg);

            Table::Exit(12);
        }
*****************************************************************************/        
        imp_buffer.Put("     %dcl ");
        imp_buffer.Put(name);
        imp_buffer.Field(++count[grammar -> rules[i].lhs], 2);
        imp_buffer.Put(" FIXED; %");
        imp_buffer.Put(name);
        imp_buffer.Field(count[grammar -> rules[i].lhs], 2);
        imp_buffer.Put(" = ");
        imp_buffer.Put(i);
        imp_buffer.Put(";\n");
    }

    imp_buffer.Put("     %dcl PNAM_MAX FIXED; %PNAM_MAX = ");
    imp_buffer.Put(grammar -> num_rules);
    imp_buffer.Put(";\n");
    imp_buffer.Flush();

    return;
}

//
//
//
void PlxTable::PrintNames(void)
{
    dcl_buffer.Put(" dcl name(0:");
    dcl_buffer.Put(name_info.Size() - 1);
    dcl_buffer.Put(") CHAR(");
    dcl_buffer.Put(max_name_length);
    dcl_buffer.Put(") VARYING constant(\n");
    char tok[Control::SYMBOL_SIZE + 1];
    for (int i = 0; i < name_info.Size(); i++)
    {
        strcpy(tok, name_info[i]);
        dcl_buffer.Pad();
        dcl_buffer.Put('\"');
        int len = Length(name_start, i);
        for (int j = 0; j < len; j++)
        {
            if (tok[j] == '\"')
                dcl_buffer.Put('\"');

            if (tok[j] == '\n')
                 dcl_buffer.Put(option -> macro_prefix);
            else dcl_buffer.Put(tok[j]);
        }
        dcl_buffer.Put('\"');
        if (i < name_info.Size() - 1)
            dcl_buffer.Put(',');
        dcl_buffer.Put('\n');
    }

    dcl_buffer.Put("    );\n");

    return;
}


//
//
//
void PlxTable::print_symbols(void)
{
    int symbol;
    char sym_line[Control::SYMBOL_SIZE + /* max length of a token symbol  */
                  2 * MAX_PARM_SIZE +    /* max length of prefix + suffix */
                  64];                   /* +64 for error messages lines  */
                                         /* or other fillers(blank, =,...)*/

    strcpy(sym_line, " dcl ");

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
            msg.Next() = " may be an invalid PLX variable.";
            option -> EmitWarning(grammar -> RetrieveTokenLocation(symbol), msg);
        }
        else if (strpbrk(tok, "!%^&*()-+={}[];:\"`~|\\,.<>/?\'") != NULL)
        {
            Tuple<const char *> msg;
            msg.Next() = tok;
            msg.Next() = " is an invalid PLX variable name.";
            option -> EmitError(grammar -> RetrieveTokenLocation(symbol), msg);
        }

        strcpy(sym_line, option -> prefix);
        strcat(sym_line, tok);
        strcat(sym_line, option -> suffix);
        strcat(sym_line, " FIXED BIN(15) SIGNED CONSTANT(");
        IntToString num(symbol_map[symbol]);
        strcat(sym_line, num.String());
        strcat(sym_line, "),\n     ");

        while(strlen(sym_line) > PARSER_LINE_SIZE)
        {
            fwrite(sym_line, sizeof(char), PARSER_LINE_SIZE - 2, syssym);
            fprintf(syssym, "\\\n");
            strcpy(sym_line, &sym_line[PARSER_LINE_SIZE - 2]);
        }
    }

    fprintf(syssym, "%s\n     "
                    "NUMTOKS FIXED BIN(15) SIGNED CONSTANT(%i),\n     "
                    "ISVALID BIT CONSTANT('1'b);\n",
                    sym_line, grammar -> num_terminals);

    return;
}


//
//
//
void PlxTable::print_exports(void)
{
    char exp_line[Control::SYMBOL_SIZE + 64];  /* max length of a token symbol  */
                                               /* +64 for error messages lines  */
                                               /* or other fillers(blank, =,...)*/

    strcpy(exp_line, " dcl ");

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
            msg.Next() = " may be an invalid PLX variable.";
            option -> EmitWarning(variable_symbol -> Location(), msg);
        }
        else if (strpbrk(tok, "!%^&*()-+={}[];:\"`~|\\,.<>/?\'") != NULL)
        {
            Tuple<const char *> msg;
            msg.Next() = "Exported symbol \"";
            msg.Next() = tok;
            msg.Next() = "\" is an invalid PLX variable name.";
            option -> EmitError(variable_symbol -> Location(), msg);
        }

        strcpy(exp_line, option -> exp_prefix);
        strcat(exp_line, tok);
        strcat(exp_line, option -> exp_suffix);
        strcat(exp_line, " FIXED BIN(15) SIGNED CONSTANT(");
        IntToString num(i);
        strcat(exp_line, num.String());
        strcat(exp_line, "),\n     ");

        while(strlen(exp_line) > PARSER_LINE_SIZE)
        {
            fwrite(exp_line, sizeof(char), PARSER_LINE_SIZE - 2, sysexp);
            fprintf(sysexp, "\\\n");
            strcpy(exp_line, &exp_line[PARSER_LINE_SIZE - 2]);
        }

        delete [] tok;
    }

    fprintf(sysexp, "%s\n     "
                    "NUMTOKS FIXED BIN(15) SIGNED CONSTANT(%i),\n     "
                    "ISVALID BIT CONSTANT('0'b);\n",
                    exp_line, grammar -> exported_symbols.Length());


    return;
}


//
//
//
void PlxTable::print_definitions(void)
{
    if (option -> error_maps)
    {
        fprintf(sysdef, " %%dcl ERROR_SYMBOL      FIXED; %%ERROR_SYMBOL = %d;\n"
                        " %%dcl SCOPE_UBOUND      FIXED; %%SCOPE_UBOUND = %d;\n"
                        " %%dcl SCOPE_SIZE        FIXED; %%SCOPE_SIZE = %d;\n"
                        " %%dcl MAX_NAME_LENGTH   FIXED; %%MAX_NAME_LENGTH = %d;\n"
                        " %%dcl MAX_TERM_LENGTH   FIXED; %%MAX_TERM_LENGTH = %d;\n"
                        " %%dcl NUM_STATES        FIXED; %%NUM_STATES = %d;\n\n",

                        grammar -> error_image,
                        pda -> scope_prefix.Size() - 1,
                        pda -> scope_prefix.Size(),
                        max_name_length,
                        max_name_length,
                        pda -> num_states);
    }

    fprintf(sysdef, " %%dcl NT_OFFSET         FIXED; %%NT_OFFSET = %d;\n"
                    " %%dcl LA_STATE_OFFSET   FIXED; %%LA_STATE_OFFSET = %d;\n"
                    " %%dcl MAX_LA            FIXED; %%MAX_LA = %d;\n"
                    " %%dcl NUM_RULES         FIXED; %%NUM_RULES = %d;\n"
                    " %%dcl NUM_TERMINALS     FIXED; %%NUM_TERMINALS = %d;\n"
                    " %%dcl NUM_NONTERMINALS  FIXED; %%NUM_NONTERMINALS = %d;\n"
                    " %%dcl NUM_SYMBOLS       FIXED; %%NUM_SYMBOLS = %d;\n"
                    " %%dcl START_STATE       FIXED; %%START_STATE = %d;\n"
                    " %%dcl IDENTIFIER_SYMBOL FIXED; %%IDENTIFIER_SYMBOL = %d;\n"
                    " %%dcl EOFT_SYMBOL       FIXED; %%EOFT_SYMBOL = %d;\n"
                    " %%dcl EOLT_SYMBOL       FIXED; %%EOLT_SYMBOL = %d;\n"
                    " %%dcl ACCEPT_ACTION     FIXED; %%ACCEPT_ACTION = %d;\n"
                    " %%dcl ERROR_ACTION      FIXED; %%ERROR_ACTION = %d;\n\n"
                    " %%dcl BACKTRACK         FIXED; %%BACKTRACK = %d;\n\n",

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
                    (int) (option -> backtrack ? 1 : 0));

    return;
}


//
//
//
void PlxTable::print_externs(void)
{
    if (option -> error_maps || option -> debug)
    {
        fprintf(sysprs,
                " original_state:Procedure(state) returns (FIXED(31));\n"
                "     dcl state fixed(31);\n"
                "     return (-base_check(state));\n"
                " end;\n\n");
    }
    if (option -> error_maps)
    {
        fprintf(sysprs,
            " asi:Procedure(state) returns (FIXED(31));\n"
            "     dcl state fixed(31);\n"
            "     return (asb(original_state(state)));\n"
            " end;\n\n"
            " nasi:Procedure(state) returns (FIXED(31));\n"
            "     dcl state fixed(31);\n"
            "     return (nasb(original_state(state)));\n"
            " end;\n\n"
            " in_symbol:Procedure(state) returns (FIXED(31));\n"
            "     dcl state fixed(31);\n"
            "     return (in_symb(original_state(state)));\n"
            " end;\n\n");
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
void PlxTable::print_tables(void)
{
    for (int i = 0; i < data.Length(); i++)
    {
        IntArrayInfo &array_info = data[i];
        const char *name = array_name[array_info.name_id];
        Array<int> &array = array_info.array;
        switch(array_info.name_id)
        {
            case BASE_CHECK:
                Print("rhs", array_info.type_id, array, 1, grammar -> num_rules);
                if (array.Size() - 1 > grammar -> num_rules + 1)
                    Print(name, array_info.type_id, array, grammar -> num_rules + 1, array.Size() - 1);
                break;
            case BASE_ACTION:
                Print("lhs", array_info.type_id, array, 0, grammar -> num_rules);
                Print(name, array_info.type_id, array, grammar -> num_rules + 1, array.Size() - 1);
                break;
            default:
                Print(name, array_info.type_id, array, 0, array.Size() - 1);
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
            dcl_buffer.Put("\n"
                           " dcl scope_prefix(1) FIXED STATIC INITIAL(0);\n"
                           " dcl scope_suffix(1) FIXED STATIC INITIAL(0);\n"
                           " dcl scope_lhs(1) FIXED STATIC INITIAL(0);\n"
                           " dcl scope_la(1) FIXED STATIC INITIAL(0);\n"
                           " dcl scope_state_set(1) FIXED STATIC INITIAL(0);\n"
                           " dcl scope_rhs(1) FIXED STATIC INITIAL(0);\n"
                           " dcl scope_state(1) FIXED STATIC INITIAL(0);\n"
                           " dcl in_symb(1) FIXED STATIC INITIAL(0);\n");
        }

        PrintNames();
    }

    dcl_buffer.Flush();

    return;
}


//
//
//
void PlxTable::PrintTables(void)
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
