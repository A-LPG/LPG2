#include "XmlTable.h"
#include "tuple.h"

void XmlTable::PrintTable(Array<int> &array, int init, int field_size, int perline)
{
    int k = 0;
    for (int i = init; i < array.Size(); i++)
    {
        tab_buffer.Put(array[i], field_size);
        k++;
        if (k == perline)
        {
            tab_buffer.Put('\n');
            k = 0;
        }
    }

    if (k != 0)
        tab_buffer.Put('\n');

    return;
}

void XmlTable::PrintSymbol(int image, const char *name)
{
    int length = strlen(name);

    tab_buffer.Put(image, 4);
    tab_buffer.Put(length, 4);

    //
    // if we are dealing with a special symbol,
    // replace its initial marker with escape.
    //
    tab_buffer.Put(name[0] == '\n'
                            ? option -> macro_prefix
                            : name[0]);
    if (length <= 64)
    {
        tab_buffer.Put(&name[1], length - 1);
        tab_buffer.Put('\n');
    }
    else
    {
        tab_buffer.Put(&name[1], 63);
        tab_buffer.Put('\n');
        for (int i = 64; i < length; i += 72)
        {
            tab_buffer.Put(&name[i], Util::Min(length - i, 72));
            tab_buffer.Put('\n');
        }
    }

    return;
}


//
// We now write out the tables to the SYSTAB file.
//
void XmlTable::PrintTables(void)
{
    IntArrayInfo *base_check_info = NULL,
                 *base_action_info = NULL,
                 *term_check_info = NULL,
                 *term_action_info = NULL,
                 *default_goto_info = NULL,
                 *default_reduce_info = NULL,
                 *shift_state_info = NULL,
                 *shift_check_info = NULL,
                 *default_shift_info = NULL,
                 *action_symbols_base_info = NULL,
                 *action_symbols_range_info = NULL,
                 *naction_symbols_base_info = NULL,
                 *naction_symbols_range_info = NULL,
                 *terminal_index_info = NULL,
                 *nonterminal_index_info = NULL,
                 *scope_prefix_info = NULL,
                 *scope_suffix_info = NULL,
                 *scope_lhs_symbol_info = NULL,
                 *scope_look_ahead_info = NULL,
                 *scope_state_set_info = NULL,
                 *scope_right_side_info = NULL,
                 *scope_state_info = NULL,
                 *in_symb_info = NULL;

    for (int i = 0; i < data.Length(); i++)
    {
        IntArrayInfo &array_info = data[i];
        switch(array_info.name_id)
        {
            case BASE_CHECK:
                 base_check_info = &array_info;
                 break;
            case BASE_ACTION:
                 base_action_info = &array_info;
                 break;
            case TERM_CHECK:
                 term_check_info = &array_info;
                 break;
            case TERM_ACTION:
                 term_action_info = &array_info;
                 break;
            case DEFAULT_GOTO:
                 default_goto_info = &array_info;
                 break;
            case DEFAULT_REDUCE:
                 default_reduce_info = &array_info;
                 break;
            case SHIFT_STATE:
                 shift_state_info = &array_info;
                 break;
            case SHIFT_CHECK:
                 shift_check_info = &array_info;
                 break;
            case DEFAULT_SHIFT:
                 default_shift_info = &array_info;
                 break;
            case ACTION_SYMBOLS_BASE:
                 action_symbols_base_info = &array_info;
                 break;
            case ACTION_SYMBOLS_RANGE:
                 action_symbols_range_info = &array_info;
                 break;
            case NACTION_SYMBOLS_BASE:
                 naction_symbols_base_info = &array_info;
                 break;
            case NACTION_SYMBOLS_RANGE:
                 naction_symbols_range_info = &array_info;
                 break;
            case TERMINAL_INDEX:
                 terminal_index_info = &array_info;
                 break;
            case NONTERMINAL_INDEX:
                 nonterminal_index_info = &array_info;
                 break;
            case SCOPE_PREFIX:
                 scope_prefix_info = &array_info;
                 break;
            case SCOPE_SUFFIX:
                 scope_suffix_info = &array_info;
                 break;
            case SCOPE_LHS_SYMBOL:
                 scope_lhs_symbol_info = &array_info;
                 break;
            case SCOPE_LOOK_AHEAD:
                 scope_look_ahead_info = &array_info;
                 break;
            case SCOPE_STATE_SET:
                 scope_state_set_info = &array_info;
                 break;
            case SCOPE_RIGHT_SIDE:
                 scope_right_side_info = &array_info;
                 break;
            case SCOPE_STATE:
                 scope_state_info = &array_info;
                 break;
            case IN_SYMB:
                 in_symb_info = &array_info;
                 break;
            default:
                 break;
        }
    }

    if ((systab = fopen(option -> tab_file, "wb")) == NULL)
    {
        fprintf(stderr,
                "***ERROR: Table file \"%s\" cannot be opened\n",
                option -> tab_file);
        Table::Exit(12);
    }

    std::cout << "***Xml tables not yet implemented\n";
    Table::Exit(12);

    tab_buffer.Put('S');
    tab_buffer.Put(option -> goto_default ? '1' : '0');
    tab_buffer.Put(option -> nt_check ? '1' : '0');
    tab_buffer.Put(option -> read_reduce ? '1' : '0');
    tab_buffer.Put(option -> single_productions ? '1' : '0');
    tab_buffer.Put(option -> shift_default ? '1' : '0');
    tab_buffer.Put(grammar -> rules[1].lhs == grammar -> accept_image ? '1' : '0'); // are there more than 1 start symbols?
    tab_buffer.Put(option -> error_maps ? '1' : '0');
    tab_buffer.Put(option -> byte && last_symbol <= 255 ? '1' : '0');
    tab_buffer.Put(option -> escape);
    tab_buffer.Put(grammar -> num_terminals, 5);
    tab_buffer.Put(grammar -> num_nonterminals, 5);
    tab_buffer.Put(grammar -> num_rules, 5);
    tab_buffer.Put(pda -> num_states, 5);
    tab_buffer.Put(base_check_info -> array.Size() - grammar -> num_rules - 1, 5);
    tab_buffer.Put(base_action_info -> array.Size() - grammar -> num_rules - 1, 5);
    tab_buffer.Put(term_check_info -> array.Size() - 1, 5);
    tab_buffer.Put(term_action_info -> array.Size() - 1, 5);
    tab_buffer.Put(start_state, 5);
    tab_buffer.Put(grammar -> eof_image, 5);
    tab_buffer.Put(accept_act, 5);
    tab_buffer.Put(error_act, 5);
    tab_buffer.Put((option -> read_reduce
                            ? error_act + grammar -> num_rules
                            : error_act),
                   5);
    tab_buffer.Put(pda -> highest_level, 5);
    tab_buffer.Put('\n');

    //
    // We write the terminal symbols map.
    //
    {
        for (int symbol = grammar -> FirstTerminal(); symbol <= grammar -> LastTerminal(); symbol++)
            PrintSymbol(symbol_map[symbol], grammar -> RetrieveString(symbol));
    }

    //
    // We write the non-terminal symbols map.
    //
    {
        for (int symbol = grammar -> FirstNonTerminal(); symbol <= grammar -> LastNonTerminal(); symbol++)
            PrintSymbol(symbol_map[symbol] - grammar -> num_terminals, grammar -> RetrieveString(symbol));
    }

    //
    // Write size of right hand side of rules followed by CHECK table.
    //
    PrintTable(base_check_info -> array, 1, 4, 18);

    //
    // Write left hand side symbol of rules followed by ACTION table.
    //
    PrintTable(base_action_info -> array, 1, 6, 12);

    //
    // Write Terminal Check Table.
    //
    PrintTable(term_check_info -> array, 1, 4, 18);

    //
    // Write Terminal Action Table.
    //
    PrintTable(term_action_info -> array, 1, 6, 12);

    //
    // If GOTO_DEFAULT is requested, we print out the GOTO_DEFAULT vector.
    //
    if (option -> goto_default)
        PrintTable(default_goto_info -> array, 1, 6, 12);

    //
    // If SHIFT_DEFAULT is requested, we print out the Default Reduce
    // map, the Shift State map, the Shift Check vector, and the SHIFT_DEFAULT
    // vector.
    //
    if (option -> shift_default)
    {
        // Print out header
        tab_buffer.Put(default_reduce_info -> array.Size() - 1, 5);
        tab_buffer.Put(shift_check_info -> array.Size() - 1, 5);
        tab_buffer.Put('\n');

        PrintTable(default_reduce_info -> array, 1, 4, 18);

        //
        // First, check whether or not maximum value in SHIFT_STATE
        // table exceeds 9999. If so, stop. Otherwise, write out
        // SHIFT_STATE table.
        //
        if ((shift_check_info -> array.Size() - 1 - grammar -> num_terminals) > 9999)
        {
            fprintf(option -> syslis,
                    "***ERROR: %s\n",
                    "SHIFT_STATE map contains > 9999 elements");
            return;
        }

        PrintTable(shift_state_info -> array, 1, 4, 18);

        PrintTable(shift_check_info -> array, 1, 4, 18);

        PrintTable(default_shift_info -> array, 1, 6, 12);
    }

    if (option -> error_maps)
    {
        //
        // Print the FOLLOW map.
        //
        PrintTable(follow_base, 0, 6, 12);
        PrintTable(follow_range, 1, 4, 18);

        PrintTable(sorted_state, 1, 6, 12);

        PrintTable(original_state, 1, 6, 12);

        //
        // Print the ACTION_SYMBOLS map.
        //
        PrintTable(action_symbols_base_info -> array, 0, 6, 12);
        PrintTable(action_symbols_range_info -> array, 1, 4, 18);

        //
        // Print the NACTION_SYMBOLS map.
        //
        PrintTable(naction_symbols_base_info -> array, 0, 6, 12);
        PrintTable(naction_symbols_range_info -> array, 1, 4, 18);

        //
        // Print the SHIFT_STATES map.
        //
        PrintTable(shift_states_base, 0, 6, 12);
        PrintTable(shift_states_range, 1, 6, 12);

        //
        // Print the GOTO_STATES map.
        //
        PrintTable(goto_states_base, 0, 6, 12);
        PrintTable(goto_states_range, 1, 6, 12);

        //
        // Write the number associated with the ERROR symbol.
        //
        tab_buffer.Put(grammar -> error_image, 4);
        tab_buffer.Put(grammar -> eol_image, 4);
        tab_buffer.Put(grammar -> num_names, 4);
        tab_buffer.Put(pda -> scope_prefix.Size(), 4);
        tab_buffer.Put(pda -> scope_right_side.Size() - 1, 4);
        tab_buffer.Put(pda -> scope_state.Size() - 1, 4);
        tab_buffer.Put(pda -> num_error_rules, 4);
        tab_buffer.Put('\n');

        //
        // We write out the names map.
        //
        for (int i = 1; i <= grammar -> num_names; i++)
        {
            int length = Length(name_start, i);
            tab_buffer.Put(length, 4);

            const char *name = name_info[i];
            if (length <= 68)
            {
                tab_buffer.Put(name, length);
                tab_buffer.Put('\n');
            }
            else
            {
                tab_buffer.Put(name, 68);
                tab_buffer.Put('\n');
                for (int i = 68; i < length; i += 72)
                {
                    tab_buffer.Put(&name[i], Util::Min(length - i, 72));
                    tab_buffer.Put('\n');
                }
            }
        }

        //
        // We write the name_index of each terminal symbol.
        //
        PrintTable(terminal_index_info -> array, 1, 4, 18);

        //
        // We write the name_index of each non_terminal symbol.
        //
        PrintTable(nonterminal_index_info -> array, 1, 4, 18);

        //
        //
        //
        if (option -> scopes)
        {
            PrintTable(scope_prefix_info -> array, 0, 4, 18);
            PrintTable(scope_suffix_info -> array, 0, 4, 18);
            PrintTable(scope_lhs_symbol_info -> array, 0, 4, 18);
            PrintTable(scope_look_ahead_info -> array, 0, 4, 18);
            PrintTable(scope_state_set_info -> array, 0, 4, 18);
            PrintTable(scope_right_side_info -> array, 1, 4, 18);
            PrintTable(scope_state_info -> array, 1, 6, 12);
        }
    }

    tab_buffer.Flush();

    return;
}
