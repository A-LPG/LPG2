#include "partition.h"
#include "TypeScriptTable.h"

#include <iostream>
using namespace std;

//
//
//
void TypeScriptTable::PrintHeader(const char *type, const char *name, const char *initial_elements)
{

    prs_buffer.Put("\n    public static  _");
    prs_buffer.Put(name);
    prs_buffer.Put(" : ");
    prs_buffer.Put(type);
    prs_buffer.Put("[]");
    prs_buffer.Put(" = [");
    prs_buffer.Put(initial_elements);
    prs_buffer.Put('\n');

    return;
}


//
//
//
void TypeScriptTable::PrintTrailer()
{
    prs_buffer.Put("        ];\n");
}

//
//
//
void TypeScriptTable::PrintTrailerAndVariable(const char *type, const char *name)
{
    PrintTrailer();
    return;
}


//
//
//
void TypeScriptTable::PrintIntsSubrange(int init, int gate, Array<int> &array)
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
void TypeScriptTable::Print(IntArrayInfo &array_info)
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
    prs_buffer.Put("    public   ");
    prs_buffer.Put(name);
    prs_buffer.Put("(index : number) : ");
    prs_buffer.Put(array_info.type_id == Table::B ? "boolean " : "number ");
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
void TypeScriptTable::PrintNames()
{
    PrintHeader("string", "name");
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
    PrintTrailerAndVariable("string", "name");
    prs_buffer.Put("    public   name(index : number):string { return ");
    prs_buffer.Put(option->prs_type);
    prs_buffer.Put("._name[index]; }\n\n");

    return;
}




//
//
//
void TypeScriptTable::non_terminal_action(void)
{
    char  temp[1024] = {};
    sprintf(temp, "    /**\n"
            "     * assert(goto_default);\n"
            "     */\n"
            "    public    ntAction(state : number,  sym : number) : number {\n"
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
void TypeScriptTable::non_terminal_no_goto_default_action(void)
{
    char  temp[1024] = {};
    sprintf(temp, "    /**\n"
                   "     * assert(! goto_default);\n"
                   "     */\n"
                   "    public    ntAction(state : number,  sym : number) : number {\n"
                   "        return %s._baseAction[state + sym];\n"
                   "    }\n\n", option->prs_type);
    prs_buffer.Put(temp);
    return;
}


//
//
//
void TypeScriptTable::terminal_action(void)
{
    char  temp[1024] = {};
    sprintf(temp, "    /**\n"
                   "     * assert(! shift_default);\n"
                   "     */\n"
                   "    public    tAction(state : number,  sym : number): number {\n"
                   "        let i = %s._baseAction[state],\n"
                   "            k = i + sym;\n"
                   "        return %s._termAction[ %s._termCheck[k] == sym ? k : i];\n"
                   "    }\n"
                   "    public    lookAhead( la_state : number ,  sym : number): number {\n"
                   "        let k = la_state + sym;\n"
                   "        return %s._termAction[ %s._termCheck[k] == sym ? k : la_state];\n"
                   "    }\n", option->prs_type, option->prs_type, option->prs_type, option->prs_type, option->prs_type);

    prs_buffer.Put(temp);
    return;
}


//
//
//
void TypeScriptTable::terminal_shift_default_action(void)
{
    char  temp[1024] = {};
    sprintf(temp, "    /**\n"
                   "     * assert(shift_default);\n"
                   "     */\n"
                   "    public    tAction(state : number,  sym  : number) : number{\n"
                   "        if (sym == 0)\n"
                   "            return ERROR_ACTION;\n"
                   "        let i = %s._baseAction[state],\n"
                   "            k = i + sym;\n"
                   "        if (%s._termCheck[k] == sym)\n"
                   "            return %s._termAction[k];\n"
                   "        i = %s._termAction[i];\n"
                   "        return (%s._shiftCheck[%s._shiftState[i] + sym] == sym\n"
                   "                                ? %s._defaultShift[sym]\n"
                   "                                : %s._defaultReduce[i]);\n"
                   "    }\n"
                   "    public    lookAhead( la_state : number,  sym : number) : number {\n"
                   "        let k = la_state + sym;\n"
                   "        if (%s._termCheck[k] == sym)\n"
                   "            return %s._termAction[k];\n"
                   "        let i = %s._termAction[la_state];\n"
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
void TypeScriptTable::init_file(FILE **file, const char *file_name)
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
void TypeScriptTable::init_parser_files(void)
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
void TypeScriptTable::exit_parser_files(void)
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
void TypeScriptTable::print_symbols(void)
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
        strcat(sym_line, "export namespace ");
        strcat(sym_line, option -> package);
        strcat(sym_line, "\n{\n\n");
    }
    strcat(sym_line, "export namespace  ");
    strcat(sym_line, option -> sym_type);
    strcat(sym_line, " {\n   ");

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
       
        strcpy(sym_line, "  export const ");
        strcat(sym_line, option -> prefix);
        strcat(sym_line, tok);
        strcat(sym_line, option -> suffix);
        strcat(sym_line, " :number ");
        strcat(sym_line, " = ");
        IntToString num(symbol_map[symbol]);
        strcat(sym_line, num.String());
        strcat(sym_line,  ";\n");

        symbol_name[symbol_map[symbol]] = tok;
    }

    fprintf(syssym, "%s", sym_line);

    fprintf(syssym, "\n    export const  orderedTerminalSymbols  : string[]= [\n");
    //                    "                 \"\",\n");
    for (int i = 0; i < grammar -> num_terminals; i++)
        fprintf(syssym, "                 \"%s\",\n", symbol_name[i]);
    fprintf(syssym, "                 \"%s\"\n             ];\n",
            symbol_name[grammar -> num_terminals]);
    fprintf(syssym, "\n    export const  numTokenKinds : number  = %d;", grammar->num_terminals + 1);
  
    if (strlen(option->package) > 0)
    {
        fprintf(syssym, "\n    export const  isValidForParser : boolean = true;\n}}\n");
    }
    else
    {
        fprintf(syssym, "\n    export const  isValidForParser : boolean = true;\n}\n");
    }
    return;
}


//
//
//
void TypeScriptTable::print_exports(void)
{
    Array<const char *> symbol_name(grammar -> exported_symbols.Length() + 1);
    char exp_line[Control::SYMBOL_SIZE + 64];  /* max length of a token symbol  */
                                               /* +64 for error messages lines  */
                                               /* or other fillers(blank, =,...)*/

    strcpy(exp_line, "");
    if (strlen(option->package) > 0)
    {
        strcat(exp_line, "export namespace ");
        strcat(exp_line, option->package);
        strcat(exp_line, "\n{\n\n");
    }
    strcat(exp_line, "export namespace  ");
    strcat(exp_line, option->exp_type);
    strcat(exp_line, " {\n   ");
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
        strcpy(exp_line, "  export const ");
        strcat(exp_line, option -> exp_prefix);
        strcat(exp_line, tok);
        strcat(exp_line, option -> exp_suffix);
        strcat(exp_line, " :number ");
        strcat(exp_line, " = ");
        IntToString num(i);
        strcat(exp_line, num.String());
        strcat(exp_line,  ";\n");
                          
        symbol_name[i] = tok;
    }

    fprintf(sysexp, "%s", exp_line);
    fprintf(sysexp, "\n    export const  orderedTerminalSymbols : string[] = [\n");
    //                    "                 \"\",\n");
    {
        for (int i = 0; i < grammar -> exported_symbols.Length(); i++)
        {
            fprintf(sysexp, "                 \"%s\",\n", symbol_name[i]);
            delete [] symbol_name[i];
        }
    }
    fprintf(sysexp, "                 \"%s\"\n             ];\n",
            symbol_name[grammar -> exported_symbols.Length()]);
    delete [] symbol_name[grammar -> exported_symbols.Length()];

  
    fprintf(sysexp, "\n    export const  numTokenKinds : number = %d;", grammar->num_terminals + 1);
   
    if (strlen(option->package) > 0)
    {
        fprintf(sysexp, "\n   export const   isValidForParser  : boolean = false;\n}}\n");
    }
    else
    {
        fprintf(sysexp, "\n   export const  isValidForParser  : boolean = false;\n}\n");
    }
    return;
}

//
//
//
void TypeScriptTable::print_definition(const char *variable, const char *method, int value)
{

    {
        prs_buffer.Put("    public   readonly ");
        prs_buffer.Put(variable);
        prs_buffer.Put(" : number = ");
        prs_buffer.Put(value);
        prs_buffer.Put(";\n");
        prs_buffer.Put("    public   ");
        prs_buffer.Put(method);
        prs_buffer.Put("() : number { return this.");
        prs_buffer.Put(variable);
        prs_buffer.Put("; }\n\n");
    }
}


//
//
//
void TypeScriptTable::print_definition(const char *variable, const char *method, bool value)
{

    {
        prs_buffer.Put("    public readonly  ");
        prs_buffer.Put(variable);
        prs_buffer.Put(" : boolean = ");
        prs_buffer.Put(value ? "true" : "false");
        prs_buffer.Put(";\n");
        prs_buffer.Put("    public  ");
        prs_buffer.Put(method);
        prs_buffer.Put("() :boolean { return this.");
        prs_buffer.Put(variable);
        prs_buffer.Put("; }\n\n");
    }
}


//
//
//
void TypeScriptTable::print_definitions(void)
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

    prs_buffer.Put("    public    getStartSymbol() : number { return this.lhs(0); }\n"
                   "    public   isValidForParser() : boolean { return ");
   
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
void TypeScriptTable::print_externs(void)
{
    if (option -> serialize || option -> error_maps || option -> debug)
    {
        char  temp[1024] = {};
        sprintf(temp,
       "    public   originalState(state : number): number {\n"
    	    "        return - %s._baseCheck[state];\n"
            "    }\n",option->prs_type);
        prs_buffer.Put(temp);
    }
    else
    {
        prs_buffer.Put("    public   originalState(state : number): number { return 0; }\n");
    }

    if (option -> serialize || option -> error_maps)
    {
        char  temp[1024] = {};
        sprintf(temp, "    public    asi(state : number ): number {\n"
                       "        return %s._asb[this.originalState(state)];\n"
                       "    }\n"
                       "    public   nasi(state : number ) : number {\n"
                       "        return %s._nasb[this.originalState(state)];\n"
                       "    }\n"
                       "    public   inSymbol(state : number)  : number {\n"
                       "        return %s._inSymb[this.originalState(state)];\n"
                       "    }\n", option->prs_type, option->prs_type, option->prs_type);
        prs_buffer.Put(temp);
    }
    else
    {
        prs_buffer.Put("    public  asi(state : number ): number { return 0; }\n"
                       "    public    nasi(state : number ) : number { return 0; }\n"
                       "    public    inSymbol(state : number)  : number { return 0; }\n");
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
void TypeScriptTable::print_source_tables(void)
{
    for (int i = 0; i < data.Length(); i++)
    {
        IntArrayInfo &array_info = data[i];
        Print(array_info);
        switch(array_info.name_id)
        {
        case BASE_CHECK:
        {
            prs_buffer.Put("    public static ");
            prs_buffer.Put(" _rhs : ");
            prs_buffer.Put(type_name[array_info.type_id]);
            prs_buffer.Put(" [] = ");
            prs_buffer.Put(option->prs_type);
            prs_buffer.Put("._");
            prs_buffer.Put(array_name[array_info.name_id]);
            char  temp[1024] = {};
            sprintf(temp,
            ";\n"
                "    public    rhs(index : number)  : number { return %s._rhs[index]; }\n",option->prs_type);
            prs_buffer.Put(temp);
        }

                break;
        case BASE_ACTION: 
        {
            prs_buffer.Put("    public static ");
            prs_buffer.Put("  _lhs : ");
            prs_buffer.Put(type_name[array_info.type_id]);
            prs_buffer.Put(" []  = ");
            prs_buffer.Put(option->prs_type);
            prs_buffer.Put("._");
            prs_buffer.Put(array_name[array_info.name_id]);
            char  temp[1024] = {};
            sprintf(temp, ";\n"
                "    public   lhs(index : number)  : number { return %s._lhs[index]; }\n", option->prs_type);
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
            prs_buffer.Put("    public static _scopePrefix : number[]= null;\n"
                           "    public    scopePrefix(index : number) : number { return 0;}\n\n"
                           "    public static _scopeSuffix : number[]= null;\n"
                           "    public    scopeSuffix(index : number): number { return 0;}\n\n"
                           "    public static _scopeLhs : number[]= null;\n"
                           "    public    scopeLhs(index : number): number { return 0;}\n\n"
                           "    public static _scopeLa : number[]= null;\n"
                           "    public    scopeLa(index : number): number { return 0;}\n\n"
                           "    public static  _scopeStateSet: number[] = null;\n"
                           "    public    scopeStateSet(index : number) : number{ return 0;}\n\n"
                           "    public static  _scopeRhs: number[] = null;\n"
                           "    public    scopeRhs(index : number): number { return 0;}\n\n"
                           "    public static _scopeState : number[]= null;\n"
                           "    public    scopeState(index : number): number { return 0;}\n\n"
                           "    public static _inSymb : number[]= null;\n"
                           "    public    inSymb(index : number) : number{ return 0;}\n\n");
        }

        PrintNames();
    }
    else
    {
        prs_buffer.Put("    public    asb(index : number) : number { return 0; }\n"
                       "    public    asr(index : number) : number { return 0; }\n"
                       "    public    nasb(index : number)  : number{ return 0; }\n"
                       "    public    nasr(index : number)  : number{ return 0; }\n"
                       "    public    terminalIndex(index : number) : number { return 0; }\n"
                       "    public    nonterminalIndex(index : number)  : number{ return 0; }\n"
                       "    public    scopePrefix(index : number)  : number{ return 0;}\n"
                       "    public    scopeSuffix(index : number) : number { return 0;}\n"
                       "    public    scopeLhs(index : number)  : number{ return 0;}\n"
                       "    public    scopeLa(index : number) : number { return 0;}\n"
                       "    public    scopeStateSet(index : number)  : number{ return 0;}\n"
                       "    public    scopeRhs(index : number)  : number{ return 0;}\n"
                       "    public    scopeState(index : number) : number { return 0;}\n"
                       "    public    inSymb(index : number)  : number{ return 0;}\n"
                       "    public    name(index : number)  : string{ return null; }\n");
    }

    return;
}


//
//
//
void TypeScriptTable::PrintTables(void)
{
    init_parser_files();

    print_symbols();

    if (grammar -> exported_symbols.Length() > 0)
        print_exports();
    {
        char temp[1024] = {};
        sprintf(temp, R"(import { %s } from "./%s";)", option->sym_type , option->sym_type);
    	prs_buffer.Put(temp);
        prs_buffer.Put("\n");
    }
   
    //
    // Now process the parse file
    //
    if (strlen(option -> package) > 0)
    {
        prs_buffer.Put("namespace ");
        prs_buffer.Put(option -> package);
        prs_buffer.Put("\n{\n\n");
    }
    prs_buffer.Put("export class ");
    prs_buffer.Put(option -> prs_type);
    if (option -> extends_parsetable)
    {
        prs_buffer.Put(" implements ");
        prs_buffer.Put(option -> extends_parsetable);
        prs_buffer.Put(" , ");
    }
    else
    {
        prs_buffer.Put(" implements ");
    }
  
    if (option -> parsetable_interfaces)
    {
        prs_buffer.Put(option -> parsetable_interfaces);
       
    }
   
    prs_buffer.Put(" {\n");


    print_definitions();


    print_source_tables();

    print_externs();

    if (strlen(option->package) > 0)
    {
        prs_buffer.Put("}}\n"); 
    }
    else
    {
        prs_buffer.Put("}\n");
    }

    prs_buffer.Flush();

    exit_parser_files();

    return;
}
