#include "partition.h"
#include "CppTable2.h"
#include <set>
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
                 prs_buffer.Put(option ->macro_prefix);
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
    fprintf(syssym, "");

    fprintf(syssym, "#pragma once\n #include <vector>\n#include<string>\n ");
    fprintf(syssym, " struct ");
    fprintf(syssym,"%s", option -> sym_type);

    fprintf(syssym, " {\n     typedef  unsigned char byte;\n");

    {
        fprintf(syssym, "      static constexpr int\n");
        Array<const char *> symbol_name(grammar->num_terminals + 1);
        int symbol;
        char sym_line[Control::SYMBOL_SIZE +       /* max length of a token symbol  */
                      2 * MAX_PARM_SIZE + /* max length of prefix + suffix */
                      64];                /* +64 for error messages lines  */
        memset(sym_line,0x00,sizeof(sym_line));
        /* or other fillers(blank, =,...)*/

        //
        // We write the terminal symbols map.
        //
        symbol_name[0] = "";
        for (symbol = grammar->FirstTerminal(); symbol <= grammar->LastTerminal(); symbol++) {
            char *tok = grammar->RetrieveString(symbol);

            fprintf(syssym, "%s", sym_line);

            if (tok[0] == '\n' || tok[0] == option->macro_prefix) {
                tok[0] = option->macro_prefix;

                Tuple<const char *> msg;
                msg.Next() = "Escaped symbol ";
                msg.Next() = tok;
                msg.Next() = " may be an invalid  variable.";
                option->EmitWarning(grammar->RetrieveTokenLocation(symbol), msg);
            } else if (strpbrk(tok, "!%^&*()-+={}[];:\"`~|\\,.<>/?\'") != NULL) {
                Tuple<const char *> msg;
                msg.Next() = tok;
                msg.Next() = " is an invalid  variable name.";
                option->EmitError(grammar->RetrieveTokenLocation(symbol), msg);
            }

            strcpy(sym_line, "      ");
            strcat(sym_line, option->prefix);
            strcat(sym_line, tok);
            strcat(sym_line, option->suffix);
            strcat(sym_line, " = ");
            IntToString num(symbol_map[symbol]);
            strcat(sym_line, num.String());
            strcat(sym_line, (symbol < grammar->LastTerminal() ? ",\n" : ";\n"));

            symbol_name[symbol_map[symbol]] = tok;
        }

        fprintf(syssym, "%s", sym_line);
        fprintf(syssym, "\n      inline const static std::vector<std::wstring> orderedTerminalSymbols");
        fprintf(syssym, " = {\n");
        //                    "                 \"\",\n");
        for (int i = 0; i < grammar->num_terminals; i++)
            fprintf(syssym, "                 L\"%s\",\n", symbol_name[i]);
        fprintf(syssym, "                 L\"%s\"\n             };\n",
                symbol_name[grammar->num_terminals]);
        fprintf(syssym, "\n     static constexpr  int numTokenKinds = %d;\n", grammar->num_terminals);
    }
    if(option->automatic_ast != Option::NONE)
    {
        Array<const char *> symbol_name(grammar->LastRule() + 1);

        char sym_line[Control::SYMBOL_SIZE +       /* max length of a token symbol  */
                      2 * MAX_PARM_SIZE + /* max length of prefix + suffix */
                      64];                /* +64 for error messages lines  */
        memset(sym_line,0x00,sizeof(sym_line));

        std::set<std::string> ruleNames;

        symbol_name[0] = "";
        for (int rule_no = 1; rule_no <= grammar->LastRule(); rule_no++) {
            int lhs = grammar->rules[rule_no].lhs;
            char *tok = grammar->RetrieveString(lhs);

            fprintf(syssym, "%s", sym_line);
            memset(sym_line,0x00,sizeof(sym_line));
            symbol_name[rule_no] = tok;

            // Filter duplicate values
            if(ruleNames.find(tok) != ruleNames.end()){
                continue;
            }
            ruleNames.insert(tok);

            strcpy(sym_line, "    static constexpr int ");
            strcat(sym_line, "RULE_");
            strcat(sym_line, tok);
            strcat(sym_line, " = ");
            IntToString num(rule_no);
            strcat(sym_line, num.String());
            strcat(sym_line, ";\n");
        }

        fprintf(syssym, "%s", sym_line);
        fprintf(syssym, "\n    inline const static std::vector<std::wstring> orderedRuleNames");
        fprintf(syssym, " = {\n");
        //                    "                 \"\",\n");
        for (int i = 0; i < grammar->LastRule(); i++)
            fprintf(syssym, "                 L\"%s\",\n", symbol_name[i]);

        fprintf(syssym, "                 L\"%s\"\n             };\n",
                symbol_name[grammar->LastRule()]);

        fprintf(syssym, "\n     static constexpr  int numRuleNames = %d;\n", grammar->LastRule());
    }

    fprintf(syssym, "\n     static constexpr  bool isValidForParser = true;\n};\n");

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

        if (tok[0] == '\n' || tok[0] == option ->macro_prefix)
        {
            tok[0] = option ->macro_prefix;

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

    fprintf(sysexp, "\n\n    constexpr   static int numTokenKinds = %d;", grammar->exported_symbols.Length());
    fprintf(sysexp, "\n    constexpr   static bool isValidForParser = false;\n};\n");

    return;
}

//
//
//
void CppTable2::print_definition(const char *variable, const char *method, int value)
{
  
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

    // Now process the parse file

    prs_buffer.Put("#pragma once\n");

    prs_buffer.Put(" #include <string>\n ");

    prs_buffer.Put(" #include \"lpg2/ParseTable.h\"\n");

    prs_buffer.Put(" #include \"");
    prs_buffer.Put(option->sym_type);
    prs_buffer.Put(".h\"\n");


    prs_buffer.Put(" struct ");
    prs_buffer.Put(option -> prs_type);
    prs_buffer.Put(" :");
    bool need_colon = false;
    if (option -> extends_parsetable)
    {
        prs_buffer.Put(" public ");
        prs_buffer.Put(option -> extends_parsetable);
        need_colon = true;
    }
   
    if (option -> parsetable_interfaces)
    {
    	if(need_colon)
    	{
            prs_buffer.Put(",");
    	}
        prs_buffer.Put("public ");
        prs_buffer.Put(option -> parsetable_interfaces);
        
        need_colon = true;
    }
	if(need_colon)
	{
        prs_buffer.Put(",");
	}
    prs_buffer.Put("public ");
    prs_buffer.Put(option -> sym_type);
    prs_buffer.Put(" {\n");
    prs_buffer.Put("             typedef  unsigned char byte;\n");
 

    print_definitions();

    print_source_tables();

    print_externs();

    prs_buffer.Put("};\n");
    prs_buffer.Flush();

    exit_parser_files();

    return;
}
