#include "partition.h"
#include "MlTable.h"

#include <iostream>
using namespace std;

//
//
//
void MlTable::Print(IntArrayInfo &array_info)
{
    const char *name = array_name[array_info.name_id];
    Array<int> &array = array_info.array;

    int init = (array[0] == 0 ? 1 : 0);

    dcl_buffer.Put("\nlet ");
    dcl_buffer.Put(name);
    dcl_buffer.Put(" = [|");
    dcl_buffer.Put(init == 1 ? "0;\n" : "\n");

    dcl_buffer.Pad();
    int k = 0;
    for (int i = init; i < array.Size(); i++)
    {
        dcl_buffer.Put(array[i]);
        dcl_buffer.Put(';');
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

    dcl_buffer.Put("    |]\n");
}


//
//
//
void MlTable::non_terminal_action(void)
{
    fprintf(sysprs,
            "let nt_action state sym =\n"
            "    let k = state + sym in\n"
            "        if (base_check.(k) = sym) then\n"
            "             base_action.(k)\n"
            "        else default_goto.(sym)\n\n");

    return;
}


void MlTable::non_terminal_no_goto_default_action(void)
{
    fprintf(sysprs,
            "let nt_action state sym = base_action.(state + sym)\n\n");

    return;
}


void MlTable::terminal_action(void)
{
    fprintf(sysprs,
            "let t_action state sym lex_stream =\n"
            "    let k = base_action.(state) + sym in\n"
            "        if (term_check.(k) = sym)then\n"
            "             term_action.(k)\n"
            "        else term_action.(base_action.(state))\n\n");

    return;
}


void MlTable::terminal_shift_default_action(void)
{
    fprintf(sysprs,
            "let t_action state sym lex_stream =\n"
            "    if (sym = 0)\n"
            "         then ERROR_ACTION\n"
            "    else let i = base_action.(state) in\n"
            "         ("
            "         if (term_check.(i + sym) = sym)\n"
            "             then term_action.(i + sym);\n"
            "         let j = term_action.(i) in\n"
            "             if (shift_check.(shift_state.(j) + sym) = sym)\n"
            "                  then default_shift.(sym)"
            "             else default_reduce.(j)\n\n"
            "         )\n");

    return;
}


void MlTable::terminal_lalr_k(void)
{
    fprintf(sysprs,
        "let regular_action state sym =\n"
        "    let k = state + sym in\n"
        "        if (term_check.(k) = sym) then\n"
        "             term_action.(k)\n"
        "        else term_action.(state)\n"
        "\n"
        "let rec lookahead_action act lex_stream =\n"
        "    if (act > lA_STATE_OFFSET) then\n"
        "        let state = act - lA_STATE_OFFSET in\n"
        "            let (sym, tail) = match lex_stream with\n"
        "                                    [] -> (eOFT_SYMBOL, [])\n"
        "                                  | (kind,_) :: tail -> (kind, tail) in\n"
        "                lookahead_action (regular_action state sym) tail\n"
        "    else act\n"
        "\n"
        "let t_action state sym lex_stream =\n"
        "    lookahead_action (regular_action base_action.(state) sym) lex_stream\n");

    return;
}


void MlTable::terminal_shift_default_lalr_k(void)
{
    fprintf(sysprs,
        "let regular_action state sym =\n"
        "    let k = state + sym in\n"
        "        if (term_check.(k) = sym) then\n"
        "             term_action.(k)\n"
        "        else\n"
        "        (\n"
        "            let state = term_action.(state) in\n"
        "                let i = shift_state.(state) + sym in\n"
        "                    if (shift_check.(i) = sym) then\n"
        "                         default_shift.(sym)\n"
        "                    else default_reduce.(state)\n"
        "        )\n"
        "\n"
        "let rec lookahead_action act lex_stream =\n"
        "    if (act > lA_STATE_OFFSET) then\n"
        "        let state = act - lA_STATE_OFFSET in\n"
        "            let (sym, tail) = match lex_stream with\n"
        "                                    [] -> (eOFT_SYMBOL, [])\n"
        "                                  | (kind,_) :: tail -> (kind, tail) in\n"
        "                lookahead_action (regular_action state sym) tail\n"
        "    else act\n"
        "\n"
        "let t_action state sym lex_stream =\n"
        "    if (sym = 0) then\n"
        "         eRROR_ACTION\n"
        "    else lookahead_action (regular_action base_action.(state) sym) lex_stream\n");
    return;
}


void MlTable::init_file(FILE **file, const char *file_name)
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


void MlTable::init_parser_files(void)
{
    init_file(&sysdcl, option -> dcl_file);
    init_file(&syssym, option -> sym_file);
    init_file(&sysprs, option -> prs_file);
    init_file(&sysimp, option -> imp_file);
    if (grammar -> exported_symbols.Length() > 0)
    {
        init_file(&sysexp, option -> exp_file);
        fprintf(sysexp, "type tokenVal = int\n");
    }

    fprintf(sysprs, "open %c%s\n"
                    "open %c%s\n\n",
                    (islower(*option -> def_type) ? toupper(*option -> def_type) : *option -> def_type), option -> def_type + 1,
                    (islower(*option -> dcl_type) ? toupper(*option -> dcl_type) : *option -> dcl_type), option -> dcl_type + 1);
    fprintf(sysdcl, "open %c%s\n\n", (islower(*option -> def_type) ? toupper(*option -> def_type) : *option -> def_type), option -> def_type + 1);
    fprintf(syssym, "module type Abstrtoken = sig\n"
                    "    type tokenVal = int\n");
    fprintf(sysimp, "type tokenVal = int\n");

    return;
}


void MlTable::exit_parser_files(void)
{
    fclose(sysdcl);
    fclose(syssym);
    fclose(sysprs);
    fclose(sysimp);
    if (grammar -> exported_symbols.Length() > 0)
        fclose(sysexp);
}


void MlTable::PrintNames(void)
{
    dcl_buffer.Put("let name = [|\n");
    for (int i = 0; i < name_info.Size(); i++)
    {
        dcl_buffer.Pad();
        dcl_buffer.Put('\"');
        const char *name = name_info[i];
        int length = Length(name_start, i);
        int k = 0;
        for (int j = 0; j < length; j++)
        {
            if (name[j] == '\"' || name[j] == '\\')
                dcl_buffer.Put('\\');

            if (name[j] == '\n')
                 dcl_buffer.Put(option -> escape);
            else dcl_buffer.Put(name[j]);

            k++;
            if (k == 30 && (! (j == length - 1)))
            {
                k = 0;
                dcl_buffer.Put('\"');
                dcl_buffer.Put(' ');
                dcl_buffer.Put('+');
                dcl_buffer.Put('\n');
                dcl_buffer.Pad();
                dcl_buffer.Put('\"');
            }
        }
        dcl_buffer.Put('\"');
        if (i < grammar -> num_names)
            dcl_buffer.Put(';');
        dcl_buffer.Put('\n');
    }
    dcl_buffer.UnputChar();
    dcl_buffer.Put('\n');  // overwrite last comma
    dcl_buffer.Put("    |]\n");

    return;
}


void MlTable::print_symbols(void)
{
    int symbol;
    char imp_line[Control::SYMBOL_SIZE +       // max length of a token symbol
                  2 * MAX_PARM_SIZE + // max length of prefix + suffix
                  64];                // +64 for error messages lines
                                  // or other fillers(blank, =,...)
    char sym_line[Control::SYMBOL_SIZE +       // max length of a token symbol
                  2 * MAX_PARM_SIZE + // max length of prefix + suffix
                  64];                // +64 for error messages lines
                                  // or other fillers(blank, =,...)

    strcpy(imp_line, "type token = (tokenVal * TokenContents.tokenContents)\n\n");
    strcpy(sym_line, "    ");
    strcat(sym_line, imp_line);

    //
    // We write the terminal symbols map.
    //
    for (symbol = grammar -> FirstTerminal(); symbol <= grammar -> LastTerminal(); symbol++)
    {
        char *tok = grammar -> RetrieveString(symbol);

        fprintf(syssym, "%s", sym_line);
        fprintf(sysimp, "%s", imp_line);

        if (tok[0] == '\n' || tok[0] == option -> escape)
        {
            sym_line[0] = '\0';
            tok[0] = option -> escape;

            Tuple<const char *> msg;
            msg.Next() = "Escaped symbol ";
            msg.Next() = tok;
            msg.Next() = " may be an invalid ML variable.";
            option -> EmitWarning(grammar -> RetrieveTokenLocation(symbol), msg);
        }
        else if (strpbrk(tok, "!%^&*()-+={}[];:\"`~|\\,.<>/?\'") != NULL)
        {
            sym_line[0] = '\0';

            Tuple<const char *> msg;
            msg.Next() = tok;
            msg.Next() = " is an invalid ML variable name.";
            option -> EmitError(grammar -> RetrieveTokenLocation(symbol), msg);
        }

        strcpy(imp_line, "let ");
        strcat(imp_line, option -> prefix);
        strcat(imp_line, tok);
        strcat(imp_line, option -> suffix);
        strcat(imp_line, " = ");
        IntToString num(symbol_map[symbol]);
        strcat(imp_line, num.String());
        strcat(imp_line, " \n");

        strcat(sym_line, "    val ");
        strcat(sym_line, option -> prefix);
        strcat(sym_line, tok);
        strcat(sym_line, option -> suffix);
        strcat(sym_line, ": tokenVal\n");
    }

    imp_line[strlen(imp_line) - 2] = '\0'; // remove the string ",\n" from last imp_line

    fprintf(syssym, "%s\n    val numTokens: tokenVal\n", sym_line);
    fprintf(syssym, "end;;\n");
    fprintf(sysimp, "%s\n\nlet numTokens = %d\nlet isValidForParser = 1\n", imp_line, grammar -> num_terminals);

    return;
}


void MlTable::print_exports(void)
{
    char exp_line[Control::SYMBOL_SIZE + 64];  /* max length of a token symbol  */
                                               /* +64 for error messages lines  */
                                               /* or other fillers(blank, =,...)*/

    strcpy(exp_line, "type token = (tokenVal * TokenContents.tokenContents)\n\n");

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

        if (tok[0] == '\n' || tok[0] == option -> escape)
        {
            tok[0] = option -> escape;

            Tuple<const char *> msg;
            msg.Next() = "Escaped exported symbol ";
            msg.Next() = tok;
            msg.Next() = " may be an invalid ML variable.";
            option -> EmitWarning(variable_symbol -> Location(), msg);
        }
        else if (strpbrk(tok, "!%^&*()-+={}[];:\"`~|\\,.<>/?\'") != NULL)
        {
            Tuple<const char *> msg;
            msg.Next() = "Exported symbol \"";
            msg.Next() = tok;
            msg.Next() = "\" is an invalid ML variable name.";
            option -> EmitError(variable_symbol -> Location(), msg);
        }

        strcpy(exp_line, "let ");
        strcat(exp_line, option -> exp_prefix);
        strcat(exp_line, tok);
        strcat(exp_line, option -> exp_suffix);
        strcat(exp_line, " = ");
        IntToString num(i);
        strcat(exp_line, num.String());
        strcat(exp_line, " \n");

        delete [] tok;
    }

    fprintf(sysexp, "%s\nlet numTokens = %d\nlet isValidForParser = 0\n", exp_line, grammar -> exported_symbols.Length());

    return;
}


void MlTable::print_definitions(void)
{
    if (option -> error_maps)
    {
         fprintf(sysprs,
                 "let eRROR_SYMBOL      = %d\n"
                 "let sCOPE_UBOUND      = %d\n"
                 "let sCOPE_SIZE        = %d\n"
                 "let mAX_NAME_LENGTH   = %d\n"
                 "let nUM_STATES        = %d\n\n",
                 grammar -> error_image,
                 pda -> scope_prefix.Size() - 1,
                 pda -> scope_prefix.Size(),
                 max_name_length,
                 pda -> num_states);
    }

    fprintf(sysprs,
                 "let nT_OFFSET         = %d\n"
                 "let lA_STATE_OFFSET   = %d\n"
                 "let mAX_LA            = %d\n"
                 "let nUM_RULES         = %d\n"
                 "let nUM_TERMINALS     = %d\n"
                 "let nUM_NONTERMINALS  = %d\n"
                 "let nUM_SYMBOLS       = %d\n"
                 "let sTART_STATE       = %d\n"
                 "let iDENTIFIER_SYMBOL = %d\n"
                 "let eOFT_SYMBOL       = %d\n"
                 "let eOLT_SYMBOL       = %d\n"
                 "let aCCEPT_ACTION     = %d\n"
                 "let eRROR_ACTION      = %d\n"
                 "let bACKTRACK         = %s\n",

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
                 (option -> backtrack ? "true" : "false"));

    return;
}


void MlTable::print_externs_and_definitions(void)
{
    print_definitions();

    if (option -> goto_default)
         non_terminal_action();
    else non_terminal_no_goto_default_action();

    if (pda -> highest_level > 1)
    {
        if (option -> shift_default)
             terminal_shift_default_lalr_k();
        else terminal_lalr_k();
    }
    else
    {
        if (option -> shift_default)
             terminal_shift_default_action();
        else terminal_action();
    }

    return;
}


void MlTable::print_tables(void)
{
    for (int i = 0; i < data.Length(); i++)
    {
        IntArrayInfo &array_info = data[i];
        Print(array_info);
        switch(array_info.name_id)
        {
            case BASE_CHECK:
                dcl_buffer.Put("\nlet rhs = ");
                dcl_buffer.Put(array_name[array_info.name_id]);
                dcl_buffer.Put("\n");
                break;
            case BASE_ACTION:
                dcl_buffer.Put("\nlet lhs = ");
                dcl_buffer.Put(array_name[array_info.name_id]);
                dcl_buffer.Put("\n");
                break;
            default:
                break;
        }
    }

    if (option -> error_maps)
        PrintNames();

    dcl_buffer.Flush();

    return;
}


void MlTable::PrintTables(void)
{
    init_parser_files();

    print_tables();

    print_symbols();
    if (grammar -> exported_symbols.Length() > 0)
        print_exports();
    print_externs_and_definitions();

    exit_parser_files();

    return;
}
