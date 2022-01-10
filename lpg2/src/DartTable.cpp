#include "partition.h"
#include "DartTable.h"

#include <algorithm>
#include <stdio.h>
#include <set>
using namespace std;

//
//
//
void DartTable::PrintHeader(const char *type, const char *name, const char *initial_elements)
{

    prs_buffer.Put("\n     static  const");
    prs_buffer.Put(" List<").Put(type).Put("> _");
    prs_buffer.Put(name);
    prs_buffer.Put(" = [");
    prs_buffer.Put(initial_elements);
    prs_buffer.Put('\n');

    return;
}


//
//
//
void DartTable::PrintTrailer()
{
    prs_buffer.Put("        ];\n");
}

//
//
//
void DartTable::PrintTrailerAndVariable(const char *, const char *)
{
    PrintTrailer();
    return;
}


//
//
//
void DartTable::PrintIntsSubrange(int init, int gate, Array<int> &array)
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
void DartTable::Print(IntArrayInfo &array_info)
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
    prs_buffer.Put("       ");
    prs_buffer.Put(array_info.type_id == Table::B ? "bool " : "int ");
    prs_buffer.Put(name);
    prs_buffer.Put("(int index)");

    prs_buffer.Put("{ return  ");
    prs_buffer.Put(option->prs_type);
    prs_buffer.Put("._");
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
void DartTable::PrintNames()
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
                 prs_buffer.Put(option -> macro_prefix);
            else if (tok[j] == '$') {
                prs_buffer.Put(R"(\$)");
            }
            else 
                prs_buffer.Put(tok[j]);
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
    prs_buffer.Put("    String   name(int index){ return ");
    prs_buffer.Put(option->prs_type);
    prs_buffer.Put("._name[index]; }\n\n");

    return;
}




//
//
//
void DartTable::non_terminal_action(void)
{
    char  temp[1024] = {};
    sprintf(temp, "    /**\n"
            "     * assert(goto_default);\n"
            "     */\n"
            "    int ntAction(int state,  int sym){\n"
            "        return (%s._baseCheck[state + sym] == sym)\n"
            "                             ? %s._baseAction[state + sym]\n"
            "                             : %s._defaultGoto[sym];\n"
            "    }\n\n", option->prs_type, option->prs_type, option->prs_type);

	prs_buffer.Put(temp);
    return;
}


//
//
//
void DartTable::non_terminal_no_goto_default_action(void)
{
    char  temp[1024] = {};
    sprintf(temp, "    /**\n"
                   "     * assert(! goto_default);\n"
                   "     */\n"
                   "    int ntAction(int state,  int sym){\n"
                   "        return %s._baseAction[state + sym];\n"
                   "    }\n\n", option->prs_type);
    prs_buffer.Put(temp);
    return;
}


//
//
//
void DartTable::terminal_action(void)
{
    char  temp[1024] = {};
    sprintf(temp, "    /**\n"
                   "     * assert(! shift_default);\n"
                   "     */\n"
                   "     int  tAction(int state,  int sym){\n"
                   "        var i = %s._baseAction[state],\n"
                   "            k = i + sym;\n"
                   "        return %s._termAction[ %s._termCheck[k] == sym ? k : i];\n"
                   "    }\n"
                   "    int   lookAhead( int la_state ,  int sym){\n"
                   "        var k = la_state + sym;\n"
                   "        return %s._termAction[ %s._termCheck[k] == sym ? k : la_state];\n"
                   "    }\n", option->prs_type, option->prs_type, option->prs_type, option->prs_type, option->prs_type);

    prs_buffer.Put(temp);
    return;
}


//
//
//
void DartTable::terminal_shift_default_action(void)
{
    char  temp[1024] = {};
    sprintf(temp, "    /**\n"
                   "     * assert(shift_default);\n"
                   "     */\n"
                   "     int  tAction(int state,  int sym){\n"
                   "        if (sym == 0)\n"
                   "            return ERROR_ACTION;\n"
                   "        var i = %s._baseAction[state],\n"
                   "            k = i + sym;\n"
                   "        if (%s._termCheck[k] == sym)\n"
                   "            return %s._termAction[k];\n"
                   "        i = %s._termAction[i];\n"
                   "        return (%s._shiftCheck[%s._shiftState[i] + sym] == sym\n"
                   "                                ? %s._defaultShift[sym]\n"
                   "                                : %s._defaultReduce[i]);\n"
                   "    }\n"
                   "    int    lookAhead( int la_state,  int sym)  {\n"
                   "        var k = la_state + sym;\n"
                   "        if (%s._termCheck[k] == sym)\n"
                   "            return %s._termAction[k];\n"
                   "        var i = %s._termAction[la_state];\n"
                   "        return (%s._shiftCheck[%s._shiftState[i] + sym] == sym\n"
                   "                                ? %s._defaultShift[sym]\n"
                   "                                : %s._defaultReduce[i]);\n"
                   "    }\n",   option->prs_type, option->prs_type, option->prs_type, 
						        option->prs_type, option->prs_type, option->prs_type, option->prs_type, 
						        option->prs_type, option->prs_type, option->prs_type, option->prs_type,
						        option->prs_type, option->prs_type, option->prs_type, option->prs_type);

    prs_buffer.Put(temp);
}


//
//
//
void DartTable::init_file(FILE **file, const char *file_name)
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
void DartTable::init_parser_files(void)
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
void DartTable::exit_parser_files(void)
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
void DartTable::print_symbols(void) {

    fprintf(syssym, "");
    fprintf(syssym, "class ");
    fprintf(syssym,"%s", option->sym_type);
    fprintf(syssym, " {\n");

    {
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
            memset(sym_line,0x00,sizeof(sym_line));
            if (tok[0] == '\n' || tok[0] == option->macro_prefix) {
                tok[0] = option->macro_prefix;

                Tuple<const char *> msg;
                msg.Next() = "Escaped symbol ";
                msg.Next() = tok;
                msg.Next() = " may be an invalid variable.";
                option->EmitWarning(grammar->RetrieveTokenLocation(symbol), msg);
            } else if (strpbrk(tok, "!%^&*()-+={}[];:\"`~|\\,.<>/?\'") != NULL) {
                Tuple<const char *> msg;
                msg.Next() = tok;
                msg.Next() = " is an invalid variable name.";
                option->EmitError(grammar->RetrieveTokenLocation(symbol), msg);
            }

            strcpy(sym_line, "static const int ");
            strcat(sym_line, option->prefix);
            strcat(sym_line, tok);
            strcat(sym_line, option->suffix);
            strcat(sym_line, " = ");
            IntToString num(symbol_map[symbol]);
            strcat(sym_line, num.String());
            strcat(sym_line, ";\n");

            symbol_name[symbol_map[symbol]] = tok;
        }

        fprintf(syssym, "%s", sym_line);

        fprintf(syssym, "\nstatic const  List<String> orderedTerminalSymbols = [\n");
        //                    "                 \"\",\n");
        for (int i = 0; i < grammar->num_terminals; i++) {
            std::string name = Util::ReplaceAll(symbol_name[i], "$", R"(\$)");

            fprintf(syssym, "    '%s',\n", name.c_str());
        }
        {
            std::string name = Util::ReplaceAll(symbol_name[grammar->num_terminals], "$", R"(\$)");
            fprintf(syssym, "    '%s'\n     ];\n",
                    name.c_str());
        }

        fprintf(syssym, "\nstatic const int numTokenKinds  = %d;\n\n", grammar->num_terminals);
    }

    if(option->automatic_ast != Option::NONE)
    {
        Array<const char *> symbol_name(grammar->LastRule()+1);

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

            strcpy(sym_line, "static const int ");
            strcat(sym_line, "RULE_");
            strcat(sym_line, tok);
            strcat(sym_line, " = ");
            IntToString num(rule_no);
            strcat(sym_line, num.String());
            strcat(sym_line, ";\n");

        }

        fprintf(syssym, "%s", sym_line);

        fprintf(syssym, "\nstatic const  List<String> orderedRuleNames = [\n");
        //                    "                 \"\",\n");
        for (int i = 0; i < grammar->LastRule(); i++) {
            std::string name = Util::ReplaceAll(symbol_name[i], "$", R"(\$)");

            fprintf(syssym, "    '%s',\n", name.c_str());
        }
        {
            std::string name = Util::ReplaceAll(symbol_name[grammar->LastRule()], "$", R"(\$)");
            fprintf(syssym, "    '%s'\n     ];\n",
                    name.c_str());
        }

        fprintf(syssym, "\nstatic const int numRuleNames  = %d;\n\n", grammar->LastRule());
    }


	fprintf(syssym, "static const bool isValidForParser = true;\n}\n");
    
    return;
}


//
//
//
void DartTable::print_exports(void)
{
    Array<const char *> symbol_name(grammar -> exported_symbols.Length() + 1);
    char exp_line[Control::SYMBOL_SIZE + 64];  /* max length of a token symbol  */
                                               /* +64 for error messages lines  */
                                               /* or other fillers(blank, =,...)*/

    strcpy(exp_line, "");
   
    strcat(exp_line, "class ");
    strcat(exp_line, option->exp_type);
    strcat(exp_line, " {\n");
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
        strcpy(exp_line, "static const int ");
        strcat(exp_line, option -> exp_prefix);
        strcat(exp_line, tok);
        strcat(exp_line, option -> exp_suffix);
        strcat(exp_line, " = ");
        IntToString num(i);
        strcat(exp_line, num.String());
        strcat(exp_line,  ";\n");
                          
        symbol_name[i] = tok;
    }

    fprintf(sysexp, "%s", exp_line);
    fprintf(sysexp, "\nstatic const List<String> orderedTerminalSymbols = [\n");
    //                    "                 \"\",\n");
    {
        for (int i = 0; i < grammar -> exported_symbols.Length(); i++)
        {
            const  std::string name = Util::ReplaceAll(symbol_name[i], "$", R"(\$)");
            fprintf(sysexp, "    '%s',\n", name.c_str());
            delete [] symbol_name[i];
        }
    }
    {
       const std::string name = Util::ReplaceAll(symbol_name[grammar->exported_symbols.Length()],
            "$", R"(\$)");
        fprintf(sysexp, "    '%s'\n     ];\n",
            name.c_str());
    }


    delete [] symbol_name[grammar -> exported_symbols.Length()];

  
    fprintf(sysexp, "\nstatic const int numTokenKinds  = %d;", grammar->exported_symbols.Length());
   

	fprintf(sysexp, "\nstatic const  bool isValidForParser  = false;\n}\n");
    
    return;
}

//
//
//
void DartTable::print_definition(const char *variable, const char *method, int value)
{

    {
        prs_buffer.Put("    static const  int ");
        prs_buffer.Put(variable);
        prs_buffer.Put(" = ");
        prs_buffer.Put(value);
        prs_buffer.Put(";\n");
        prs_buffer.Put("    int   ");
        prs_buffer.Put(method);
        prs_buffer.Put("(){ return ");
        prs_buffer.Put(variable);
        prs_buffer.Put("; }\n\n");
    }
}


//
//
//
void DartTable::print_definition(const char *variable, const char *method, bool value)
{

    {
        prs_buffer.Put("    static const bool  ");
        prs_buffer.Put(variable);
        prs_buffer.Put(" = ");
        prs_buffer.Put(value ? "true" : "false");
        prs_buffer.Put(";\n");
        prs_buffer.Put("    bool  ");
        prs_buffer.Put(method);
        prs_buffer.Put("() { return ");
        prs_buffer.Put(variable);
        prs_buffer.Put("; }\n\n");
    }
}


//
//
//
void DartTable::print_definitions(void)
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

    prs_buffer.Put("    int  getStartSymbol() { return lhs(0); }\n"
                   "    bool  isValidForParser()  { return ");
   
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
void DartTable::print_externs(void)
{
    if (option -> serialize || option -> error_maps || option -> debug)
    {
        char  temp[1024] = {};
        sprintf(temp,
       "     int  originalState(int state){\n"
    	    "        return - %s._baseCheck[state];\n"
            "    }\n",option->prs_type);
        prs_buffer.Put(temp);
    }
    else
    {
        prs_buffer.Put("   int  originalState(int state){ return 0; }\n");
    }

    if (option -> serialize || option -> error_maps)
    {
        char  temp[1024] = {};
        sprintf(temp, "     int   asi(int state) {\n"
                       "        return %s._asb[originalState(state)];\n"
                       "    }\n"
                       "      int  nasi(int state ){\n"
                       "        return %s._nasb[originalState(state)];\n"
                       "    }\n"
                       "    int   inSymbol(int state){\n"
                       "        return %s._inSymb[originalState(state)];\n"
                       "    }\n", option->prs_type, option->prs_type, option->prs_type);
        prs_buffer.Put(temp);
    }
    else
    {
        prs_buffer.Put("     int asi(int state ){ return 0; }\n"
                       "     int   nasi(int state ){ return 0; }\n"
                       "     int   inSymbol(int state){ return 0; }\n");
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
void DartTable::print_source_tables(void)
{
    for (int i = 0; i < data.Length(); i++)
    {
        IntArrayInfo &array_info = data[i];
        Print(array_info);
        switch(array_info.name_id)
        {
        case BASE_CHECK:
        {
            prs_buffer.Put("     static const List<");
            prs_buffer.Put(type_name[array_info.type_id]);
            prs_buffer.Put(">");
            prs_buffer.Put(" _rhs = ");
          
            prs_buffer.Put(option->prs_type);
            prs_buffer.Put("._");
            prs_buffer.Put(array_name[array_info.name_id]);
            char  temp[1024] = {};
            sprintf(temp,
            ";\n"
                "    int    rhs(int index){ return %s._rhs[index]; }\n",option->prs_type);
            prs_buffer.Put(temp);
        }

                break;
        case BASE_ACTION: 
        {
            prs_buffer.Put("     static const List<");
         
            prs_buffer.Put(type_name[array_info.type_id]);
            prs_buffer.Put(">");
            prs_buffer.Put("  _lhs =");
            prs_buffer.Put(option->prs_type);
            prs_buffer.Put("._");
            prs_buffer.Put(array_name[array_info.name_id]);
            char  temp[1024] = {};
            sprintf(temp, ";\n"
                "   int   lhs(int index){ return %s._lhs[index]; }\n", option->prs_type);
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
            prs_buffer.Put("     static const List<int> _scopePrefix=[] ;\n"
                           "     int   scopePrefix(int index)  { return 0;}\n\n"
                           "     static const List<int> _scopeSuffix=[] ;\n"
                           "     int   scopeSuffix(int index) { return 0;}\n\n"
                           "     static const List<int> _scopeLhs=[] ;\n"
                           "     int   scopeLhs(int index) { return 0;}\n\n"
                           "     static const List<int> _scopeLa =[];\n"
                           "     int   scopeLa(int index) { return 0;}\n\n"
                           "     static const List<int> _scopeStateSet=[] ;\n"
                           "     int   scopeStateSet(int index) { return 0;}\n\n"
                           "     static  const List<int> _scopeRhs=[] ;\n"
                           "     int   scopeRhs(int index){ return 0;}\n\n"
                           "     static const List<int> _scopeState =[];\n"
                           "     int   scopeState(int index){ return 0;}\n\n"
                           "     static  const List<int>  _inSymb =[];\n"
                           "     int   inSymb(int index){ return 0;}\n\n");
        }

        PrintNames();
    }
    else
    {
        prs_buffer.Put( "      int  asb(int index) { return 0; }\n"
			            "      int  asr(int index) { return 0; }\n"
			            "      int  nasb(int index)  { return 0; }\n"
			            "      int  nasr(int index)  { return 0; }\n"
			            "      int  terminalIndex(int index)  { return 0; }\n"
			            "      int  nonterminalIndex(int index)  { return 0; }\n"
			            "      int  scopePrefix(int index)  { return 0;}\n"
			            "      int  scopeSuffix(int index)  { return 0;}\n"
			            "      int  scopeLhs(int index)  { return 0;}\n"
			            "      int  scopeLa(int index)  { return 0;}\n"
			            "      int  scopeStateSet(int index)  { return 0;}\n"
			            "      int  scopeRhs(int index)  { return 0;}\n"
			            "      int  scopeState(int index)  { return 0;}\n"
			            "      int  inSymb(int index)  { return 0;}\n"
			            "      String  name(int index) { return \"\"; }\n");
			    }

    return;
}


//
//
//
void DartTable::PrintTables(void)
{
    init_parser_files();

    print_symbols();

    if (grammar -> exported_symbols.Length() > 0)
        print_exports();
    {
        char temp[1024] = {};
        sprintf(temp, R"(import '%s.dart';)", option->sym_type);
    	prs_buffer.Put(temp);
        prs_buffer.Put("\n");
    }
    if (!option->extends_parsetable && option->parsetable_interfaces)
    {
        prs_buffer.Put("import 'package:lpg2/lpg2.dart';\n");
    }
    //
    // Now process the parse file
    //

    prs_buffer.Put(" class ");
    prs_buffer.Put(option -> prs_type);
    if (option -> extends_parsetable)
    {
        prs_buffer.Put(" extends ");
        prs_buffer.Put(option -> extends_parsetable);
        prs_buffer.Put(" ");
    }
    
  
    if (option -> parsetable_interfaces)
    {
    	prs_buffer.Put(" implements ");
        prs_buffer.Put(option -> parsetable_interfaces);
    }
   
    prs_buffer.Put(" {\n");


    print_definitions();


    print_source_tables();

    print_externs();


    
	prs_buffer.Put("}\n");
    

    prs_buffer.Flush();

    exit_parser_files();

    return;
}
