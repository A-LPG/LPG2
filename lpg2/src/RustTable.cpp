#include "partition.h"
#include "RustTable.h"
#include <set>
#include <iostream>
using namespace std;

bool RustTable::IsTraitAccessor(int name_id)
{
    switch (name_id)
    {
    case NULLABLES:
    case BASE_CHECK:
    case BASE_ACTION:
    case TERM_CHECK:
    case TERM_ACTION:
    case ACTION_SYMBOLS_BASE:
    case ACTION_SYMBOLS_RANGE:
    case NACTION_SYMBOLS_BASE:
    case NACTION_SYMBOLS_RANGE:
    case TERMINAL_INDEX:
    case NONTERMINAL_INDEX:
    case SCOPE_PREFIX:
    case SCOPE_SUFFIX:
    case SCOPE_LHS_SYMBOL:
    case SCOPE_LOOK_AHEAD:
    case SCOPE_STATE_SET:
    case SCOPE_RIGHT_SIDE:
    case SCOPE_STATE:
    case IN_SYMB:
        return true;
    default:
        return false;
    }
}

void RustTable::PrintHeader(const char *type, const char *name, const char *initial_elements)
{
    prs_buffer + "\nstatic " + option->prs_type + "_" + name + ": &[" + type + "] = &[";
    prs_buffer.Put(initial_elements);
    prs_buffer.Put('\n');
}

void RustTable::PrintTrailer()
{
    prs_buffer.Put("];\n");
}

void RustTable::PrintTrailerAndVariable(const char *, const char *)
{
    PrintTrailer();
}

void RustTable::PrintIntsSubrange(int init, int gate, Array<int> &array)
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
        prs_buffer.Put('\n');
}

void RustTable::Print(IntArrayInfo &array_info)
{
    const char *type = type_name[array_info.type_id];
    const char *name = array_name[array_info.name_id];
    Array<int> &array = array_info.array;

    int init = (array[0] == 0 ? 1 : 0);
    const char *initial_elements = (init == 1 ? "0," : "");

    PrintHeader(type, name, initial_elements);
    PrintIntsSubrange(init, array.Size(), array);
    PrintTrailerAndVariable(type, name);

    if (!IsTraitAccessor(array_info.name_id))
        return;

    if (array_info.name_id == NULLABLES)
    {
        method_code += "    fn is_nullable(&self, symbol: i32) -> bool {\n";
        method_code += "        ";
        method_code += option->prs_type;
        method_code += "_is_nullable[symbol as usize] != 0\n    }\n";
        return;
    }

    method_code += "    fn ";
    method_code += name;
    method_code += "(&self, index: i32) -> i32 {\n        ";
    method_code += option->prs_type;
    method_code += "_";
    method_code += name;
    method_code += "[index as usize]\n    }\n";
}

void RustTable::PrintNames()
{
    PrintHeader("&str", "name");
    char tok[Control::SYMBOL_SIZE + 1];
    for (int i = 0; i < name_info.Size(); i++)
    {
        strcpy(tok, name_info[i]);
        prs_buffer.Pad();
        prs_buffer.Put('\"');
        int len = Length(name_start, i);
        for (int j = 0; j < len; j++)
        {
            if (tok[j] == '\"' || tok[j] == '\\')
                prs_buffer.Put('\\');
            if (tok[j] == '\n')
                prs_buffer.Put(option->macro_prefix);
            else
                prs_buffer.Put(tok[j]);
        }
        prs_buffer.Put('\"');
        prs_buffer.Put(',');
        prs_buffer.Put('\n');
    }
    PrintTrailerAndVariable("&str", "name");

    method_code += "    fn name(&self, index: i32) -> String {\n        ";
    method_code += option->prs_type;
    method_code += "_name[index as usize].to_string()\n    }\n";
}

void RustTable::non_terminal_action(void)
{
    char temp[1024] = {};
    sprintf(temp,
            "    fn nt_action(&self, state: i32, sym: i32) -> i32 {\n"
            "        if %s_base_check[(state + sym) as usize] == sym {\n"
            "            %s_base_action[(state + sym) as usize]\n"
            "        } else {\n"
            "            %s_default_goto[sym as usize]\n"
            "        }\n"
            "    }\n\n",
            option->prs_type, option->prs_type, option->prs_type);
    method_code += temp;
}

void RustTable::non_terminal_no_goto_default_action(void)
{
    char temp[1024] = {};
    sprintf(temp,
            "    fn nt_action(&self, state: i32, sym: i32) -> i32 {\n"
            "        %s_base_action[(state + sym) as usize]\n"
            "    }\n\n",
            option->prs_type);
    method_code += temp;
}

void RustTable::terminal_action(void)
{
    char temp[2048] = {};
    sprintf(temp,
            "    fn t_action(&self, act: i32, sym: i32) -> i32 {\n"
            "        let i = %s_base_action[act as usize];\n"
            "        let k = i + sym;\n"
            "        let index = if %s_term_check[k as usize] == sym { k } else { i };\n"
            "        %s_term_action[index as usize]\n"
            "    }\n\n"
            "    fn look_ahead(&self, la_state: i32, sym: i32) -> i32 {\n"
            "        let k = la_state + sym;\n"
            "        let index = if %s_term_check[k as usize] == sym { k } else { la_state };\n"
            "        %s_term_action[index as usize]\n"
            "    }\n\n",
            option->prs_type, option->prs_type, option->prs_type,
            option->prs_type, option->prs_type);
    method_code += temp;
}

void RustTable::terminal_shift_default_action(void)
{
    char temp[4096] = {};
    sprintf(temp,
            "    fn t_action(&self, act: i32, sym: i32) -> i32 {\n"
            "        if sym == 0 {\n"
            "            return %s_ERROR_ACTION;\n"
            "        }\n"
            "        let mut i = %s_base_action[act as usize];\n"
            "        let k = i + sym;\n"
            "        if %s_term_check[k as usize] == sym {\n"
            "            return %s_term_action[k as usize];\n"
            "        }\n"
            "        i = %s_term_action[i as usize];\n"
            "        if %s_shift_check[(%s_shift_state[i as usize] + sym) as usize] == sym {\n"
            "            %s_default_shift[sym as usize]\n"
            "        } else {\n"
            "            %s_default_reduce[i as usize]\n"
            "        }\n"
            "    }\n\n"
            "    fn look_ahead(&self, la_state: i32, sym: i32) -> i32 {\n"
            "        let k = la_state + sym;\n"
            "        if %s_term_check[k as usize] == sym {\n"
            "            return %s_term_action[k as usize];\n"
            "        }\n"
            "        let i = %s_term_action[la_state as usize];\n"
            "        if %s_shift_check[(%s_shift_state[i as usize] + sym) as usize] == sym {\n"
            "            %s_default_shift[sym as usize]\n"
            "        } else {\n"
            "            %s_default_reduce[i as usize]\n"
            "        }\n"
            "    }\n\n",
            option->prs_type, option->prs_type, option->prs_type, option->prs_type,
            option->prs_type, option->prs_type, option->prs_type, option->prs_type,
            option->prs_type, option->prs_type, option->prs_type, option->prs_type,
            option->prs_type, option->prs_type, option->prs_type, option->prs_type);
    method_code += temp;
}

void RustTable::init_file(FILE **file, const char *file_name)
{
    if (! OpenOutput(file, file_name))
    {
        Tuple<const char *> msg;
        msg.Next() = "Output file \"";
        msg.Next() = file_name;
        msg.Next() = "\" could not be opened";
        option->EmitError(0, msg);
        Table::Exit(12);
    }

    fprintf(*file,
            "// Generated by LPG2 %s; runtime ABI: lpg-rust-runtime-v1\n",
            Control::VERSION);
    grammar->NoticeBuffer().Print(*file);
}

void RustTable::init_parser_files(void)
{
    init_file(&sysprs, option->prs_file);
    init_file(&syssym, option->sym_file);
    if (grammar->exported_symbols.Length() > 0)
        init_file(&sysexp, option->exp_file);
}

void RustTable::exit_parser_files(void)
{
    fclose(sysprs);
    sysprs = NULL;
    fclose(syssym);
    syssym = NULL;
    if (grammar->exported_symbols.Length() > 0)
    {
        fclose(sysexp);
        sysexp = NULL;
    }
}

void RustTable::print_symbols(void)
{
    std::string line;
    if (strlen(option->package) > 0)
    {
        line += "// mod ";
        line += option->package;
        line += "\n";
    }

    line += "pub struct ";
    line += option->sym_type;
    line += ";\n\nimpl ";
    line += option->sym_type;
    line += " {\n";
    fprintf(syssym, "%s", line.c_str());
    line.clear();

    for (int symbol = grammar->FirstTerminal(); symbol <= grammar->LastTerminal(); symbol++)
    {
        char *tok = grammar->RetrieveString(symbol);
        if (tok[0] == '\n' || tok[0] == option->macro_prefix)
        {
            tok[0] = option->macro_prefix;
            Tuple<const char *> msg;
            msg.Next() = "Escaped symbol ";
            msg.Next() = tok;
            msg.Next() = " may be an invalid variable.";
            option->EmitWarning(grammar->RetrieveTokenLocation(symbol), msg);
        }
        else if (strpbrk(tok, "!%^&*()-+={}[];:\"`~|\\,.<>/?\'") != NULL)
        {
            Tuple<const char *> msg;
            msg.Next() = tok;
            msg.Next() = " is an invalid variable name.";
            option->EmitError(grammar->RetrieveTokenLocation(symbol), msg);
        }
    }

    {
        Array<const char *> symbol_name(grammar->num_terminals + 1);
        symbol_name[0] = "";
        for (int symbol = grammar->FirstTerminal(); symbol <= grammar->LastTerminal(); symbol++)
        {
            char *tok = grammar->RetrieveString(symbol);
            line += "    pub const ";
            line += option->prefix;
            line += tok;
            line += option->suffix;
            line += ": i32 = ";
            IntToString num(symbol_map[symbol]);
            line += num.String();
            line += ";\n";
            symbol_name[symbol_map[symbol]] = tok;
            fprintf(syssym, "%s", line.c_str());
            line.clear();
        }

        fprintf(syssym, "\n    pub const ORDERED_TERMINAL_SYMBOLS: &[&str] = &[\n");
        for (int i = 0; i < grammar->num_terminals; i++)
            fprintf(syssym, "        \"%s\",\n", symbol_name[i]);
        fprintf(syssym, "        \"%s\",\n    ];\n\n", symbol_name[grammar->num_terminals]);
        fprintf(syssym, "    pub const NUM_TOKEN_KINDS: i32 = %d;\n", grammar->num_terminals);
    }

    if (option->automatic_ast != Option::NONE)
    {
        Array<const char *> symbol_name(grammar->LastRule() + 1);
        std::set<std::string> ruleNames;
        symbol_name[0] = "";
        for (int rule_no = 1; rule_no <= grammar->LastRule(); rule_no++)
        {
            int lhs = grammar->rules[rule_no].lhs;
            char *tok = grammar->RetrieveString(lhs);
            symbol_name[rule_no] = tok;
            if (ruleNames.find(tok) != ruleNames.end())
                continue;
            ruleNames.insert(tok);
            line += "    pub const RULE_";
            line += tok;
            line += ": i32 = ";
            IntToString num(rule_no);
            line += num.String();
            line += ";\n";
            fprintf(syssym, "%s", line.c_str());
            line.clear();
        }

        fprintf(syssym, "\n    pub const ORDERED_RULE_NAMES: &[&str] = &[\n");
        for (int i = 0; i < grammar->LastRule(); i++)
            fprintf(syssym, "        \"%s\",\n", symbol_name[i]);
        fprintf(syssym, "        \"%s\",\n    ];\n\n", symbol_name[grammar->LastRule()]);
        fprintf(syssym, "    pub const NUM_RULE_NAMES: i32 = %d;\n", grammar->LastRule());
    }

    fprintf(syssym, "\n    pub const IS_VALID_FOR_PARSER: bool = true;\n}\n");
}

void RustTable::print_exports(void)
{
    Array<const char *> symbol_name(grammar->exported_symbols.Length() + 1);
    std::string line;
    if (strlen(option->package) > 0)
    {
        line += "// mod ";
        line += option->package;
        line += "\n";
    }

    line += "pub struct ";
    line += option->exp_type;
    line += ";\n\nimpl ";
    line += option->exp_type;
    line += " {\n";

    char *temp = new char[1];
    *temp = '\0';
    symbol_name[0] = temp;
    for (int i = 1; i <= grammar->exported_symbols.Length(); i++)
    {
        VariableSymbol *variable_symbol = grammar->exported_symbols[i - 1];
        char *tok = new char[variable_symbol->NameLength() + 1];
        strcpy(tok, variable_symbol->Name());
        if (tok[0] == '\n' || tok[0] == option->macro_prefix)
        {
            tok[0] = option->macro_prefix;
            Tuple<const char *> msg;
            msg.Next() = "Escaped exported symbol ";
            msg.Next() = tok;
            msg.Next() = " may be an invalid variable.";
            option->EmitWarning(variable_symbol->Location(), msg);
        }
        else if (strpbrk(tok, "!%^&*()-+={}[];:\"`~|\\,.<>/?\'") != NULL)
        {
            Tuple<const char *> msg;
            msg.Next() = "Exported symbol \"";
            msg.Next() = tok;
            msg.Next() = "\" is an invalid variable name.";
            option->EmitError(variable_symbol->Location(), msg);
        }
        symbol_name[i] = tok;
    }

    fprintf(sysexp, "%s", line.c_str());
    line.clear();

    for (int i = 1; i <= grammar->exported_symbols.Length(); i++)
    {
        const char *tok = symbol_name[i];
        line += "    pub const ";
        line += option->exp_prefix;
        line += tok;
        line += option->exp_suffix;
        line += ": i32 = ";
        IntToString num(i);
        line += num.String();
        line += ";\n";
    }
    fprintf(sysexp, "%s", line.c_str());
    line.clear();

    fprintf(sysexp, "\n    pub const ORDERED_TERMINAL_SYMBOLS: &[&str] = &[\n");
    for (int i = 0; i < grammar->exported_symbols.Length(); i++)
        fprintf(sysexp, "        \"%s\",\n", symbol_name[i]);
    fprintf(sysexp, "        \"%s\",\n    ];\n\n", symbol_name[grammar->exported_symbols.Length()]);

    // symbol_name[0..Length] are all heap-allocated (index 0 is the empty string).
    for (int i = 0; i <= grammar->exported_symbols.Length(); i++)
        delete[] symbol_name[i];

    fprintf(sysexp, "    pub const NUM_TOKEN_KINDS: i32 = %d;\n", grammar->exported_symbols.Length());
    fprintf(sysexp, "    pub const IS_VALID_FOR_PARSER: bool = false;\n}\n");
}

void RustTable::print_definition(const char *variable, const char *method, int value)
{
    (void)method;
    IntToString num(value);
    prs_buffer + "pub const " + option->prs_type + "_" + variable + ": i32 = " + num.String() + ";\n";
}

void RustTable::print_definition(const char *variable, const char *method, bool value)
{
    (void)method;
    prs_buffer + "pub const " + option->prs_type + "_" + variable + ": bool = ";
    prs_buffer.Put(value ? "true" : "false").Put(";\n");
}

void RustTable::print_definition_method(const char *variable, const char *method, const char *return_type)
{
    method_code += "    fn ";
    method_code += method;
    method_code += "(&self) -> ";
    method_code += return_type;
    method_code += " {\n        ";
    method_code += option->prs_type;
    method_code += "_";
    method_code += variable;
    method_code += "\n    }\n";
}

void RustTable::print_definitions(void)
{
    print_definition("ERROR_SYMBOL", "get_error_symbol", option->error_maps ? grammar->error_image : 0);
    print_definition("SCOPE_UBOUND", "get_scope_ubound", option->error_maps ? pda->scope_prefix.Size() - 1 : 0);
    print_definition("SCOPE_SIZE", "get_scope_size", option->error_maps ? pda->scope_prefix.Size() : 0);
    print_definition("MAX_NAME_LENGTH", "get_max_name_length", option->error_maps ? max_name_length : 0);
    print_definition("NUM_STATES", "get_num_states", pda->num_states);
    print_definition("NT_OFFSET", "get_nt_offset", grammar->num_terminals);
    print_definition("LA_STATE_OFFSET", "get_la_state_offset", option->read_reduce ? error_act + grammar->num_rules : error_act);
    print_definition("MAX_LA", "get_max_la", pda->highest_level);
    print_definition("NUM_RULES", "get_num_rules", grammar->num_rules);
    print_definition("NUM_NONTERMINALS", "get_num_nonterminals", grammar->num_nonterminals);
    print_definition("NUM_SYMBOLS", "get_num_symbols", grammar->num_symbols);
    print_definition("START_STATE", "get_start_state", start_state);
    print_definition("IDENTIFIER_SYMBOL", "get_identifier_symbol", grammar->identifier_image);
    print_definition("EOFT_SYMBOL", "get_eoft_symbol", grammar->eof_image);
    print_definition("EOLT_SYMBOL", "get_eolt_symbol", grammar->eol_image);
    print_definition("ACCEPT_ACTION", "get_accept_action", accept_act);
    print_definition("ERROR_ACTION", "get_error_action", error_act);
    print_definition("BACKTRACK", "get_backtrack", option->backtrack);

    print_definition_method("ERROR_SYMBOL", "get_error_symbol", "i32");
    print_definition_method("SCOPE_UBOUND", "get_scope_ubound", "i32");
    print_definition_method("SCOPE_SIZE", "get_scope_size", "i32");
    print_definition_method("MAX_NAME_LENGTH", "get_max_name_length", "i32");
    print_definition_method("NUM_STATES", "get_num_states", "i32");
    print_definition_method("NT_OFFSET", "get_nt_offset", "i32");
    print_definition_method("LA_STATE_OFFSET", "get_la_state_offset", "i32");
    print_definition_method("MAX_LA", "get_max_la", "i32");
    print_definition_method("NUM_RULES", "get_num_rules", "i32");
    print_definition_method("NUM_NONTERMINALS", "get_num_nonterminals", "i32");
    print_definition_method("NUM_SYMBOLS", "get_num_symbols", "i32");
    print_definition_method("START_STATE", "get_start_state", "i32");
    print_definition_method("EOFT_SYMBOL", "get_eoft_symbol", "i32");
    print_definition_method("EOLT_SYMBOL", "get_eolt_symbol", "i32");
    print_definition_method("ACCEPT_ACTION", "get_accept_action", "i32");
    print_definition_method("ERROR_ACTION", "get_error_action", "i32");
    print_definition_method("BACKTRACK", "get_backtrack", "bool");

    method_code += "    fn get_start_symbol(&self) -> i32 {\n";
    method_code += "        self.lhs(0)\n    }\n\n";
    // Rust parser templates include the symbol and table files into one
    // module, while table-only users commonly declare them as sibling
    // modules. Avoid baking either module layout into the ParseTable impl.
    method_code += "    fn is_valid_for_parser(&self) -> bool {\n"
                   "        true\n"
                   "    }\n\n";
}

void RustTable::print_externs(void)
{
    if (option->serialize || option->error_maps || option->debug)
    {
        char temp[1024] = {};
        sprintf(temp,
                "    fn original_state(&self, state: i32) -> i32 {\n"
                "        -%s_base_check[state as usize]\n"
                "    }\n\n",
                option->prs_type);
        method_code += temp;
    }
    else
    {
        method_code += "    fn original_state(&self, _state: i32) -> i32 { 0 }\n\n";
    }

    if (option->serialize || option->error_maps)
    {
        char temp[1024] = {};
        sprintf(temp,
                "    fn asi(&self, state: i32) -> i32 {\n"
                "        %s_asb[self.original_state(state) as usize]\n"
                "    }\n\n"
                "    fn nasi(&self, state: i32) -> i32 {\n"
                "        %s_nasb[self.original_state(state) as usize]\n"
                "    }\n\n"
                "    fn in_symbol(&self, state: i32) -> i32 {\n"
                "        %s_in_symb[self.original_state(state) as usize]\n"
                "    }\n\n",
                option->prs_type, option->prs_type, option->prs_type);
        method_code += temp;
    }
    else
    {
        method_code += "    fn asi(&self, _state: i32) -> i32 { 0 }\n\n";
        method_code += "    fn nasi(&self, _state: i32) -> i32 { 0 }\n\n";
        method_code += "    fn in_symbol(&self, _state: i32) -> i32 { 0 }\n\n";
    }

    if (option->goto_default)
        non_terminal_action();
    else
        non_terminal_no_goto_default_action();

    if (option->shift_default)
        terminal_shift_default_action();
    else
        terminal_action();
}

void RustTable::print_source_tables(void)
{
    for (int i = 0; i < data.Length(); i++)
    {
        IntArrayInfo &array_info = data[i];
        Print(array_info);
        switch (array_info.name_id)
        {
        case BASE_CHECK:
            prs_buffer + "static " + option->prs_type + "_rhs: &[i32] = " + option->prs_type + "_base_check;\n\n";
            method_code += "    fn rhs(&self, index: i32) -> i32 {\n        ";
            method_code += option->prs_type;
            method_code += "_rhs[index as usize]\n    }\n\n";
            break;
        case BASE_ACTION:
            prs_buffer + "static " + option->prs_type + "_lhs: &[i32] = " + option->prs_type + "_base_action;\n\n";
            method_code += "    fn lhs(&self, index: i32) -> i32 {\n        ";
            method_code += option->prs_type;
            method_code += "_lhs[index as usize]\n    }\n\n";
            break;
        default:
            break;
        }
    }

    if (option->error_maps)
    {
        if (pda->scope_prefix.Size() == 0)
        {
            char temp[4096] = {};
            sprintf(temp,
                    "static %s_scope_prefix: &[i32] = &[];\n"
                    "static %s_scope_suffix: &[i32] = &[];\n"
                    "static %s_scope_lhs: &[i32] = &[];\n"
                    "static %s_scope_la: &[i32] = &[];\n"
                    "static %s_scope_state_set: &[i32] = &[];\n"
                    "static %s_scope_rhs: &[i32] = &[];\n"
                    "static %s_scope_state: &[i32] = &[];\n"
                    "static %s_in_symb: &[i32] = &[];\n\n",
                    option->prs_type, option->prs_type, option->prs_type, option->prs_type,
                    option->prs_type, option->prs_type, option->prs_type, option->prs_type);
            prs_buffer.Put(temp);
            method_code +=
                "    fn scope_prefix(&self, _index: i32) -> i32 { 0 }\n\n"
                "    fn scope_suffix(&self, _index: i32) -> i32 { 0 }\n\n"
                "    fn scope_lhs(&self, _index: i32) -> i32 { 0 }\n\n"
                "    fn scope_la(&self, _index: i32) -> i32 { 0 }\n\n"
                "    fn scope_state_set(&self, _index: i32) -> i32 { 0 }\n\n"
                "    fn scope_rhs(&self, _index: i32) -> i32 { 0 }\n\n"
                "    fn scope_state(&self, _index: i32) -> i32 { 0 }\n\n"
                "    fn in_symb(&self, _index: i32) -> i32 { 0 }\n\n";
        }
        PrintNames();
    }
    else
    {
        method_code +=
            "    fn asb(&self, _index: i32) -> i32 { 0 }\n\n"
            "    fn asr(&self, _index: i32) -> i32 { 0 }\n\n"
            "    fn nasb(&self, _index: i32) -> i32 { 0 }\n\n"
            "    fn nasr(&self, _index: i32) -> i32 { 0 }\n\n"
            "    fn terminal_index(&self, _index: i32) -> i32 { 0 }\n\n"
            "    fn nonterminal_index(&self, _index: i32) -> i32 { 0 }\n\n"
            "    fn scope_prefix(&self, _index: i32) -> i32 { 0 }\n\n"
            "    fn scope_suffix(&self, _index: i32) -> i32 { 0 }\n\n"
            "    fn scope_lhs(&self, _index: i32) -> i32 { 0 }\n\n"
            "    fn scope_la(&self, _index: i32) -> i32 { 0 }\n\n"
            "    fn scope_state_set(&self, _index: i32) -> i32 { 0 }\n\n"
            "    fn scope_rhs(&self, _index: i32) -> i32 { 0 }\n\n"
            "    fn scope_state(&self, _index: i32) -> i32 { 0 }\n\n"
            "    fn in_symb(&self, _index: i32) -> i32 { 0 }\n\n"
            "    fn name(&self, _index: i32) -> String { String::new() }\n\n";
    }
}

RustTable::RustTable(Control *control_, Pda *pda_) : Table(control_, pda_),
                                                     prs_buffer(&sysprs),
                                                     data_buffer(&sysdat)
{
    prs_def_prefix = new char[1];
    prs_def_prefix[0] = '\0';

    type_name.Resize(num_type_ids);
    array_name.Resize(num_name_ids);

    type_name[B] = type_name[I8] = "i32";
    type_name[I16] = "i32";
    type_name[U8] = "i32";
    type_name[U16] = "i32";
    type_name[I32] = "i32";

    array_name[NULLABLES] = "is_nullable";
    array_name[PROSTHESES_INDEX] = "prostheses_index";
    array_name[KEYWORDS] = "is_keyword";
    array_name[BASE_CHECK] = "base_check";
    array_name[BASE_ACTION] = "base_action";
    array_name[TERM_CHECK] = "term_check";
    array_name[TERM_ACTION] = "term_action";
    array_name[DEFAULT_GOTO] = "default_goto";
    array_name[DEFAULT_REDUCE] = "default_reduce";
    array_name[SHIFT_STATE] = "shift_state";
    array_name[SHIFT_CHECK] = "shift_check";
    array_name[DEFAULT_SHIFT] = "default_shift";
    array_name[ACTION_SYMBOLS_BASE] = "asb";
    array_name[ACTION_SYMBOLS_RANGE] = "asr";
    array_name[NACTION_SYMBOLS_BASE] = "nasb";
    array_name[NACTION_SYMBOLS_RANGE] = "nasr";
    array_name[TERMINAL_INDEX] = "terminal_index";
    array_name[NONTERMINAL_INDEX] = "nonterminal_index";
    array_name[SCOPE_PREFIX] = "scope_prefix";
    array_name[SCOPE_SUFFIX] = "scope_suffix";
    array_name[SCOPE_LHS_SYMBOL] = "scope_lhs";
    array_name[SCOPE_LOOK_AHEAD] = "scope_la";
    array_name[SCOPE_STATE_SET] = "scope_state_set";
    array_name[SCOPE_RIGHT_SIDE] = "scope_rhs";
    array_name[SCOPE_STATE] = "scope_state";
    array_name[IN_SYMB] = "in_symb";
    array_name[NAME_START] = "name";
}

RustTable::~RustTable()
{
    delete[] prs_def_prefix;
}

void RustTable::PrintTables(void)
{
    method_code.clear();
    init_parser_files();
    print_symbols();

    if (grammar->exported_symbols.Length() > 0)
        print_exports();

    if (strlen(option->package) > 0)
    {
        std::string line;
        line += "// mod ";
        line += option->package;
        line += "\n";
        prs_buffer.Put(line.data());
    }

    prs_buffer + "#[derive(Clone, Copy, Debug, Default)]\n";
    prs_buffer + "pub struct " + option->prs_type + ";\n\n";
    prs_buffer + "impl " + option->prs_type + " {\n";
    prs_buffer + "    pub fn new() -> Self {\n";
    prs_buffer + "        " + option->prs_type + "\n";
    prs_buffer.Put("    }\n}\n\n");

    print_definitions();
    print_source_tables();
    print_externs();

    prs_buffer.Put("\nimpl lpg2::traits::ParseTable for ");
    prs_buffer.Put(option->prs_type);
    prs_buffer.Put(" {\n");
    prs_buffer.Put(method_code.c_str());
    prs_buffer.Put("}\n\n");

    prs_buffer.Flush();
    exit_parser_files();
}
