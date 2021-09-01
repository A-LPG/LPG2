#include "partition.h"
#include "GoTable.h"

#include <iostream>
using namespace std;

//
//
//
void GoTable::PrintHeader(const char *type, const char *name, const char *initial_elements)
{

    prs_buffer + "\nvar  "+ option->prs_type+ "_"+ name + " []" + type + "=[]" + type + "{";
   
    prs_buffer.Put(initial_elements);
    prs_buffer.Put('\n');

    return;
}


//
//
//
void GoTable::PrintTrailer()
{
    prs_buffer.Put("}\n");
}

//
//
//
void GoTable::PrintTrailerAndVariable(const char *type, const char *name)
{
    PrintTrailer();
    return;
}


//
//
//
void GoTable::PrintIntsSubrange(int init, int gate, Array<int> &array)
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
        //prs_buffer.UnputChar(); // remove last comma, if possible
        prs_buffer.Put('\n');
    }

    return;
}


//
//
//
void GoTable::Print(IntArrayInfo &array_info)
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
    PrintTrailerAndVariable(type, name);
    

    //
    // Generate a function with the same name as the array.
    // The function takes an integer argument and returns
    // the corresponding element in the array.
    //
    prs_buffer + prs_def_prefix + " " + name+"(index int)";
    prs_buffer.Put(array_info.type_id == Table::B ? "bool" : "int");
    prs_buffer.Put("{\n");
    prs_buffer+ "    return "+ option->prs_type+"_"+name+ "[index]";

    if (array_info.type_id == Table::B)
        prs_buffer.Put(" != 0");

    prs_buffer.Put("\n}\n");

    return;
}


//
//
//
void GoTable::PrintNames()
{
    PrintHeader("string", "Name");
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
                 prs_buffer.Put(option -> macro_prefix);
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
        //if (i < Name_info.Size() - 1)
            prs_buffer.Put(',');
        prs_buffer.Put('\n');
    }
    PrintTrailerAndVariable("string", "Name");

    prs_buffer + prs_def_prefix + " Name(index int) string {\n" ;
    prs_buffer + "    return " + option->prs_type + "_Name[index] \n}\n" ;

    return;
}




//
//
//
void GoTable::non_terminal_action(void)
{
    char  temp[1024] = {};
    sprintf(temp,"    /**\n"
			"     * assert(goto_default);\n"
			"     */\n"
            "    %s NtAction(state int, sym int) int{\n"
			"        if %s_baseCheck[state + sym] == sym {\n"
            "           return %s_BaseAction[state + sym]\n"
			"        }else{\n"
            "              return %s_defaultGoto[sym]\n"
            "        }\n"
            "    }\n\n",prs_def_prefix, option->prs_type, option->prs_type, option->prs_type);

	prs_buffer.Put(temp);
}


//
//
//
void GoTable::non_terminal_no_goto_default_action(void)
{
    char  temp[1024] = {};
    sprintf(temp, "    /**\n"
                   "     * assert(! goto_default);\n"
                   "     */\n"
                   "    %s NtAction(state int,  sym int) int{\n"
                   "        return %s_BaseAction[state + sym]\n"
                   "    }\n\n", prs_def_prefix, option->prs_type);
    prs_buffer.Put(temp);
    return;
}


//
//
//
void GoTable::terminal_action(void)
{
    char  temp[1024] = {};
    sprintf(temp, "    /**\n"
					"     * assert(! shift_default);\n"
					"     */\n"
					"    %s TAction(state int,  sym int)int{\n"
					"        var i = %s_BaseAction[state]\n"
					"        var k = i + sym\n"
					"        var index int\n"
					"        if %s_TermCheck[k] == sym {\n"
					"           index = k\n"
					"        }else{\n"
					"           index = i\n"
					"        }\n"
					"        return %s_TermAction[index]\n"
					"    }\n"
					"    %s LookAhead(la_state int , sym int)int{\n"
					"        var k = la_state + sym\n"
			        "        var index int\n"
			        "        if %s_TermCheck[k] == sym {\n"
			        "           index = k\n"
			        "        }else{\n"
			        "           index = la_state\n"
			        "        }\n"
                   "        return %s_TermAction[ index]\n"
                   "    }\n", prs_def_prefix, option->prs_type, option->prs_type, option->prs_type, prs_def_prefix, option->prs_type, option->prs_type);

    prs_buffer.Put(temp);
    return;
}


//
//
//
void GoTable::terminal_shift_default_action(void)
{
    char  temp[1024] = {};
    sprintf(temp, "    /**\n"
					"     * assert(shift_default);\n"
					"     */\n"
					"    %s TAction(state int, sym int) int{\n"
					"        if sym == 0 {\n"
					"            return ERROR_ACTION\n"
					"        }\n"
					"        var i = %s_BaseAction[state]\n"
					"        var k = i + sym\n"
					"        if %s_TermCheck[k] == sym{\n"
					"            return %s_TermAction[k]\n"
					"        }\n"
					"        i = %s_TermAction[i]\n"
			        "        if %s_shiftCheck[%s_shiftState[i] + sym] == sym{\n"
			        "           return %s_defaultShift[sym]\n"
					"        }else{\n"
			        "             return %s_defaultReduce[i]\n"
					"        }\n"
			        "    }\n"
					"    %s LookAhead(la_state int, sym int) int{\n"
					"        var k = la_state + sym\n"
					"        if %s_TermCheck[k] == sym {\n"
					"            return %s_TermAction[k]\n"
					"        }\n"
					"        var i = %s_TermAction[la_state]\n"
			        "        if %s_shiftCheck[%s_shiftState[i] + sym] == sym{\n"
			        "           return %s_defaultShift[sym]\n"
					"        }else{\n"
			        "             return %s_defaultReduce[i]\n"
					"        }\n"
                    "    }\n",   prs_def_prefix,  option->prs_type, option->prs_type, option->prs_type, 
						         option->prs_type, option->prs_type, option->prs_type, option->prs_type, option->prs_type,

								 prs_def_prefix ,option->prs_type, option->prs_type, option->prs_type,
						         option->prs_type, option->prs_type, option->prs_type, option->prs_type);

    prs_buffer.Put(temp);
}


//
//
//
void GoTable::init_file(FILE **file, const char *file_name)
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
void GoTable::init_parser_files(void)
{
    init_file(&sysprs, option -> prs_file);
    init_file(&syssym, option -> sym_file);
    if (grammar -> exported_symbols.Length() > 0)
        init_file(&sysexp, option -> exp_file);

  
    return;
}


//
//
//
void GoTable::exit_parser_files(void)
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
void GoTable::print_symbols(void)
{
     Array<const char *> symbol_name(grammar -> num_terminals + 1);
	 int symbol;

	 std::string line;
	 if (strlen(option->package) > 0)
	 {
	     line+= "package ";
	     line += option->package;
 		 line += "\n";
	 }
 
    line+= "type __";
    line += option -> sym_type;
    line += "__ struct{\n";
   

    //
    // We write the terminal symbols map.
    //
    symbol_name[0] = "";
    for (symbol = grammar -> FirstTerminal(); symbol <= grammar -> LastTerminal(); symbol++)
    {
        char *tok = grammar -> RetrieveString(symbol);

        fprintf(syssym, "%s", line.c_str());
        line.clear();

        if (tok[0] == '\n' || tok[0] == option -> macro_prefix)
        {
            tok[0] = option ->macro_prefix;

            Tuple<const char *> msg;
            msg.Next() = "Escaped symbol ";
            msg.Next() = tok;
            msg.Next() = " may be an invalid variable.";
            option -> EmitWarning(grammar -> RetrieveTokenLocation(symbol), msg);
        }
        else if (strpbrk(tok, "!%^&*()-+={}[];:\"`~|\\,.<>/?\'") != NULL)
        {
            Tuple<const char *> msg;
            msg.Next() = tok;
            msg.Next() = " is an invalid variable name.";
            option -> EmitError(grammar -> RetrieveTokenLocation(symbol), msg);
        }
       
        line += "   ";
        line += option -> prefix;
        line += tok;
        line += option -> suffix;
        line += " int\n";


        symbol_name[symbol_map[symbol]] = tok;
    }

    fprintf(syssym, "%s", line.c_str());
    line.clear();

    fprintf(syssym, "\n   OrderedTerminalSymbols []string\n");

    fprintf(syssym, "\n   NumTokenKinds int\n");

	fprintf(syssym, "\n   IsValidForParser  bool\n}\n");


    std::string new_func_name;
    new_func_name = "New__";
    new_func_name += option->sym_type;
    new_func_name += "__()";

    line += "func ";
    line += new_func_name;
    line += " *__";
    line += option->sym_type;
    line += "__{\n";
    line += "    self := new(__";
    line += option->sym_type;
    line += "__)\n";
    //
    // We write the terminal symbols map.
    //
    symbol_name[0] = "";
    for (symbol = grammar->FirstTerminal(); symbol <= grammar->LastTerminal(); symbol++)
    {
        char* tok = grammar->RetrieveString(symbol);

        fprintf(syssym, "%s", line.c_str());
        line.clear();

        if (tok[0] == '\n' || tok[0] == option->macro_prefix)
        {
            tok[0] = option->macro_prefix;

            Tuple<const char*> msg;
            msg.Next() = "Escaped symbol ";
            msg.Next() = tok;
            msg.Next() = " may be an invalid variable.";
            option->EmitWarning(grammar->RetrieveTokenLocation(symbol), msg);
        }
        else if (strpbrk(tok, "!%^&*()-+={}[];:\"`~|\\,.<>/?\'") != NULL)
        {
            Tuple<const char*> msg;
            msg.Next() = tok;
            msg.Next() = " is an invalid variable name.";
            option->EmitError(grammar->RetrieveTokenLocation(symbol), msg);
        }

        line += "   self.";
        line += option->prefix;
        line += tok;
        line += option->suffix;

        line += " = ";
        IntToString num(symbol_map[symbol]);
        line += num.String();
        line += "\n";

        symbol_name[symbol_map[symbol]] = tok;
    }

    fprintf(syssym, "%s", line.c_str());
    line.clear();

    fprintf(syssym, "\n   self.OrderedTerminalSymbols = []string{\n");
    //                    "                 \"\",\n");
    for (int i = 0; i < grammar->num_terminals; i++)
        fprintf(syssym, "                 \"%s\",\n", symbol_name[i]);
    fprintf(syssym, "                 \"%s\",\n             }\n",symbol_name[grammar->num_terminals]);
    fprintf(syssym, "\n   self.NumTokenKinds = %d", grammar->num_terminals);

    fprintf(syssym, "\n   self.IsValidForParser = true");
    fprintf(syssym, "\n   return self\n}\n");



    fprintf(syssym, "var %s = %s\n", option->sym_type, new_func_name.c_str());


    return;
}


//
//
//
void GoTable::print_exports(void)
{
    Array<const char *> symbol_name(grammar -> exported_symbols.Length() + 1);
 
    std::string line;
    if (strlen(option->package) > 0)
    {
        line += "package ";
        line += option->package;
        line += "\n";
    }

    line += "type __";
    line += option->exp_type;
    line += "__ struct{\n";


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

        fprintf(sysexp, "%s", line.c_str());
        line.clear();
        if (tok[0] == '\n' || tok[0] == option -> macro_prefix)
        {
            tok[0] = option ->macro_prefix;

            Tuple<const char *> msg;
            msg.Next() = "Escaped exported symbol ";
            msg.Next() = tok;
            msg.Next() = " may be an invalid variable.";
            option -> EmitWarning(variable_symbol -> Location(), msg);
        }
        else if (strpbrk(tok, "!%^&*()-+={}[];:\"`~|\\,.<>/?\'") != NULL)
        {
            Tuple<const char *> msg;
            msg.Next() = "Exported symbol \"";
            msg.Next() = tok;
            msg.Next() = "\" is an invalid variable name.";
            option -> EmitError(variable_symbol -> Location(), msg);
        }

        line += "   ";
        line += option->exp_prefix;
        line += tok;
        line += option->exp_suffix;
        line += " int\n";



        symbol_name[i] = tok;
    }

    fprintf(sysexp, "%s", line.c_str());
    line.clear();
    fprintf(sysexp, "\n   OrderedTerminalSymbols []string\n");

    fprintf(sysexp, "\n   NumTokenKinds int\n");

    fprintf(sysexp, "\n   IsValidForParser  bool\n}\n");

    std::string new_func_name;
    new_func_name = "New__";
    new_func_name += option->exp_type;
    new_func_name += "__()";

    line += "func ";
    line += new_func_name;
    line += " *__";
    line += option->exp_type;
    line += "__{\n";
    line += "    self := new(__";
    line += option->exp_type;
    line += "__)\n";


    symbol_name[0] = temp;
    for (int i = 1; i <= grammar->exported_symbols.Length(); i++)
    {
        VariableSymbol* variable_symbol = grammar->exported_symbols[i - 1];
        char* tok = new char[variable_symbol->NameLength() + 1];
        strcpy(tok, variable_symbol->Name());

        fprintf(sysexp, "%s", line.c_str());
        line.clear();
        if (tok[0] == '\n' || tok[0] == option->macro_prefix)
        {
            tok[0] = option->macro_prefix;

            Tuple<const char*> msg;
            msg.Next() = "Escaped exported symbol ";
            msg.Next() = tok;
            msg.Next() = " may be an invalid variable.";
            option->EmitWarning(variable_symbol->Location(), msg);
        }
        else if (strpbrk(tok, "!%^&*()-+={}[];:\"`~|\\,.<>/?\'") != NULL)
        {
            Tuple<const char*> msg;
            msg.Next() = "Exported symbol \"";
            msg.Next() = tok;
            msg.Next() = "\" is an invalid variable name.";
            option->EmitError(variable_symbol->Location(), msg);
        }

        line += "   self.";
        line += option->exp_prefix;
        line += tok;
        line += option->exp_suffix;

        line += " = ";
        IntToString num(i);
        line+= num.String();
        line += "\n";


        symbol_name[i] = tok;
    }
    fprintf(sysexp, "%s", line.c_str());
    line.clear();

    fprintf(sysexp, "\n   self.OrderedTerminalSymbols = []string{\n");
    //                    "                 \"\",\n");
    {
        for (int i = 0; i < grammar -> exported_symbols.Length(); i++)
        {
            fprintf(sysexp, "                 \"%s\",\n", symbol_name[i]);
            delete [] symbol_name[i];
        }
    }
    fprintf(sysexp, "                 \"%s\",\n             }\n",
            symbol_name[grammar -> exported_symbols.Length()]);
    delete [] symbol_name[grammar -> exported_symbols.Length()];

  
    fprintf(sysexp, "\n   self.NumTokenKinds int = %d", grammar->exported_symbols.Length());

	fprintf(sysexp, "\n   self.IsValidForParser = false\n\n");
    fprintf(sysexp, "\n   return self\n}\n");
    fprintf(syssym, "var %s = %s\n", option->exp_type, new_func_name.c_str());
    return;
}

//
//
//
void GoTable::print_definition(const char *variable, const char *method, int value)
{

    prs_buffer+"const " + option->prs_type + "_" + variable + " int = " + value + "\n";
    prs_buffer + prs_def_prefix + " " + method + "() int {\n";
    prs_buffer + "     return " + option->prs_type + "_" + variable + "\n}\n";
    
}


//
//
//
void GoTable::print_definition(const char *variable, const char *method, bool value)
{

    prs_buffer + "const " + option->prs_type + "_" + variable + " bool = ";
    prs_buffer.Put(value ? "true" : "false").Put("\n" );

    prs_buffer + prs_def_prefix + " " + method + "() bool {\n";
    prs_buffer + "     return " + option->prs_type + "_" + variable + "\n}\n";

}


//
//
//
void GoTable::print_definitions(void)
{
   

    print_definition("ERROR_SYMBOL", "GetErrorSymbol", option -> error_maps ? grammar -> error_image : 0);
    print_definition("SCOPE_UBOUND", "GetScopeUbound", option -> error_maps ? pda -> scope_prefix.Size() - 1 : 0);
    print_definition("SCOPE_SIZE", "GetScopeSize", option -> error_maps ? pda -> scope_prefix.Size() : 0);
    print_definition("MAX_NAME_LENGTH", "GetMaxNameLength", option -> error_maps ? max_name_length : 0);
    print_definition("NUM_STATES", "GetNumStates", pda -> num_states);
    print_definition("NT_OFFSET", "GetNtOffset", grammar -> num_terminals);
    print_definition("LA_STATE_OFFSET", "GetLaStateOffset", option -> read_reduce ? error_act + grammar -> num_rules : error_act);
    print_definition("MAX_LA", "GetMaxLa", pda -> highest_level);
    print_definition("NUM_RULES", "GetNumRules", grammar -> num_rules);
    print_definition("NUM_NONTERMINALS", "GetNumNonterminals", grammar -> num_nonterminals);
    print_definition("NUM_SYMBOLS", "GetNumSymbols", grammar -> num_symbols);
  
    print_definition("START_STATE", "GetStartState", start_state);
    print_definition("IDENTIFIER_SYMBOL", "getIdentifier_SYMBOL", grammar -> identifier_image);
    print_definition("EOFT_SYMBOL", "GetEoftSymbol", grammar -> eof_image);
    print_definition("EOLT_SYMBOL", "GetEoltSymbol", grammar -> eol_image);
    print_definition("ACCEPT_ACTION", "GetAcceptAction", accept_act);
    print_definition("ERROR_ACTION", "GetErrorAction", error_act);
    print_definition("BACKTRACK", "GetBacktrack", option -> backtrack);

    {
        char  temp[1024] = {};
        sprintf(temp, "%s GetStartSymbol() int{\n    return self.Lhs(0)\n}\n"
            "%s IsValidForParser() bool{\n    return ", prs_def_prefix,prs_def_prefix);
        prs_buffer.Put(temp);
    }

   
    {
        prs_buffer.Put(option -> sym_type);
        prs_buffer.Put(".IsValidForParser");
    }
    prs_buffer.Put("\n}\n");

    return;
}


//
//
//
void GoTable::print_externs(void)
{
    if (option -> serialize || option -> error_maps || option -> debug)
    {
        char  temp[1024] = {};
        sprintf(temp,
       "%s OriginalState(state int) int{\n"
    	    "        return - %s_baseCheck[state]\n"
            "}\n", prs_def_prefix,option->prs_type);
        prs_buffer.Put(temp);
    }
    else
    {
        prs_buffer.Put(prs_def_prefix).Put(" OriginalState(state int) int{\n    return 0\n}\n");
    }

    if (option -> serialize || option -> error_maps)
    {
        char  temp[1024] = {};
        sprintf(temp, "%s Asi(state int) int{\n"
                       "        return %s_Asb[self.OriginalState(state)]\n"
                       "}\n"
                       "%s Nasi(state int ) int{\n"
                       "        return %s_Nasb[self.OriginalState(state)]\n"
                       "}\n"
                       "%s InSymbol(state int) int{\n"
                       "        return %s_InSymb[self.OriginalState(state)]\n"
                       "}\n", prs_def_prefix, option->prs_type, prs_def_prefix,option->prs_type, prs_def_prefix, option->prs_type);
        prs_buffer.Put(temp);
    }
    else
    {
        char  temp[1024] = {};
        sprintf(temp, "%s Asi(state int) int{\n    return 0\n}\n"
                       "%s Nasi(state int ) int{\n    return 0\n}\n"
                       "%s InSymbol(state int) int{\n    return 0\n}\n", prs_def_prefix, prs_def_prefix, prs_def_prefix);
        prs_buffer.Put(temp);
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
void GoTable::print_source_tables(void)
{
    for (int i = 0; i < data.Length(); i++)
    {
        IntArrayInfo &array_info = data[i];
        Print(array_info);
        switch(array_info.name_id)
        {
        case BASE_CHECK:
        {
            prs_buffer+"var "+ option->prs_type+"_Rhs  = " + option->prs_type + "_" + array_name[array_info.name_id];
            char  temp[1024] = {};
            sprintf(temp,
            "\n"
                "%s Rhs(index int) int{ return %s_Rhs[index] }\n",prs_def_prefix,option->prs_type);
            prs_buffer.Put(temp);
        }

                break;
        case BASE_ACTION: 
        {
            prs_buffer + "var " + option->prs_type + "_Lhs  = " + option->prs_type + "_" + array_name[array_info.name_id];
          
            char  temp[1024] = {};
            sprintf(temp, "\n"
                "%s Lhs(index int) int{ return %s_Lhs[index] }\n", prs_def_prefix, option->prs_type);
            prs_buffer.Put(temp);
        }
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
            char  temp[1024] = {};
            sprintf(temp, "var  %s_ScopePrefix  []int\n"
                           "%s ScopePrefix(index int) int{ return 0 }\n\n"
                           "var  %s_ScopeSuffix []int\n"
                           "%s ScopeSuffix(index int)int{ return 0 }\n\n"
                           "var  %s_ScopeLhs []int\n"
                           "%s ScopeLhs(index int)int{ return 0 }\n\n"
                           "var  %s_ScopeLa []int\n"
                           "%s ScopeLa(index int)int{ return 0 }\n\n"
                           "var  %s_ScopeStateSet []int \n"
                           "%s ScopeStateSet(index int) int{ return 0 }\n\n"
                           "var  %s_ScopeRhs []int \n"
                           "%s ScopeRhs(index int)int{ return 0 }\n\n"
                           "var  %s_scopeState []int \n"
                           "%s ScopeState(index int)int{ return 0 }\n\n"
                           "var  %s_InSymb []int \n"
                           "%s InSymb(index int) int{ return 0 }\n\n",
                 option->prs_type,prs_def_prefix,
                 option->prs_type, prs_def_prefix, 
                 option->prs_type, prs_def_prefix, 
                 option->prs_type, prs_def_prefix, 
                 option->prs_type, prs_def_prefix,
                option->prs_type, prs_def_prefix,
                option->prs_type, prs_def_prefix,
                option->prs_type, prs_def_prefix
            );
            prs_buffer.Put(temp);
        }

        PrintNames();
    }
    else
    {
        prs_buffer.Put("%s Asb(index int) int{ return 0 }\n"
                       "%s Asr(index int) int{ return 0 }\n"
                       "%s Nasb(index int) int{ return 0 }\n"
                       "%s Nasr(index int) int{ return 0 }\n"
                       "%s TerminalIndex(index int) int{ return 0 }\n"
                       "%s NonterminalIndex(index int) int{ return 0 }\n"
                       "%s ScopePrefix(index int) int{ return 0 }\n"
                       "%s ScopeSuffix(index int) int{ return 0 }\n"
                       "%s ScopeLhs(index int) int{ return 0 }\n"
                       "%s ScopeLa(index int) int{ return 0 }\n"
                       "%s ScopeStateSet(index int) int{ return 0 }\n"
                       "%s ScopeRhs(index int) int{ return 0 }\n"
                       "%s ScopeState(index int) int{ return 0 }\n"
                       "%s InSymb(index int) int{ return 0 }\n"
                       "%s Name(index int)   string{ return \"\" }\n");
    }

    return;
}

GoTable::GoTable(Control* control_, Pda* pda_): Table(control_, pda_),
                                                prs_buffer(&sysprs),
                                                data_buffer(&sysdat)
{
    std::string temp = "func (self * ";
    temp += option->prs_type;
	temp+=")";
    prs_def_prefix = new char[temp.size() + 1];
    memcpy(prs_def_prefix, temp.data(), temp.size());
    prs_def_prefix[temp.size()] = 0;

	type_name.Resize(num_type_ids);
	array_name.Resize(num_name_ids);

	type_name[B] = type_name[I8] = "int";
	type_name[I16] = "int";
	type_name[U8]  = "int";
	type_name[U16] = "int";
	type_name[I32] = "int";

	array_name[NULLABLES] = "IsNullable";
	array_name[PROSTHESES_INDEX] = "prosthesesIndex";
	array_name[KEYWORDS] = "isKeyword";
	array_name[BASE_CHECK] = "BaseCheck";
	array_name[BASE_ACTION] = "BaseAction";
	array_name[TERM_CHECK] = "TermCheck";
	array_name[TERM_ACTION] = "TermAction";
	array_name[DEFAULT_GOTO] = "defaultGoto";
	array_name[DEFAULT_REDUCE] = "defaultReduce";
	array_name[SHIFT_STATE] = "shiftState";
	array_name[SHIFT_CHECK] = "shiftCheck";
	array_name[DEFAULT_SHIFT] = "defaultShift";
	array_name[ACTION_SYMBOLS_BASE] = "Asb";
	array_name[ACTION_SYMBOLS_RANGE] = "Asr";
	array_name[NACTION_SYMBOLS_BASE] = "Nasb";
	array_name[NACTION_SYMBOLS_RANGE] = "Nasr";
	array_name[TERMINAL_INDEX] = "TerminalIndex";
	array_name[NONTERMINAL_INDEX] = "NonterminalIndex";
	array_name[SCOPE_PREFIX] = "ScopePrefix";
	array_name[SCOPE_SUFFIX] = "ScopeSuffix";
	array_name[SCOPE_LHS_SYMBOL] = "ScopeLhs";
	array_name[SCOPE_LOOK_AHEAD] = "ScopeLa";
	array_name[SCOPE_STATE_SET] = "ScopeStateSet";
	array_name[SCOPE_RIGHT_SIDE] = "ScopeRhs";
	array_name[SCOPE_STATE] = "ScopeState";
	array_name[IN_SYMB] = "InSymb";
	array_name[NAME_START] = "!?";
}

GoTable::~GoTable()
{
    delete[]prs_def_prefix;
}


//
//
//
void GoTable::PrintTables(void)
{
    init_parser_files();

    print_symbols();

    if (grammar -> exported_symbols.Length() > 0)
        print_exports();

    //
    // Now process the parse file
    //
    
    if (strlen(option->package) > 0)
    {
        std::string line;
        line += "package ";
        line += option->package;
        line += "\n";
        prs_buffer.Put(line.data());
    }
    prs_buffer.Put("type ");
    prs_buffer.Put(option -> prs_type);
    prs_buffer.Put(" struct{}\n");

    print_definitions();


    print_source_tables();

    print_externs();


    
	prs_buffer.Put("\n");
    

    prs_buffer.Flush();

    exit_parser_files();

    return;
}
