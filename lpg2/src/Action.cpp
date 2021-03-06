#include "NTC.h"
#include "TTC.h"
#include "CTC.h"
#include "LCA.h"
#include "symbol.h"
#include "LexStream.h"
#include "Action.h"
#include "spell.h"
#include "VisitorStaffFactory.h"
namespace BuildInMacroName
{
   
    const char* rule_number_string = "rule_number";
    const char* rule_text_string = "rule_text";
    const char* rule_size_string = "rule_size";
    const char* input_file_string = "input_file";
    const char* current_line_string = "current_line";
    const char* next_line_string = "next_line";
    const char* identifier_string = "#identifier";
    const char* symbol_declarations_string = "symbol_declarations";
    const char* num_rules_string = "num_rules";
    const char* num_terminals_string = "num_terminals";
    const char* num_nonterminals_string = "num_nonterminals";
    const char* num_non_terminals_string = "num_non_terminals";
    const char* num_symbols_string = "num_symbols";
    const char* template_string = "template";
    const char* file_prefix_string = "file_prefix";
    const char* package_string = "package";
    const char* ast_package_string = "ast_package";
    const char* ast_type_string = "ast_type";
    const char* exp_type_string = "exp_type";
    const char* prs_type_string = "prs_type";
    const char* sym_type_string = "sym_type";
    const char* action_type_string = "action_type";
    const char* visitor_type_string = "visitor_type";
    const char* prefix_string = "prefix";
    const char* suffix_string = "suffix";

    const char* entry_name_string = "entry_name";
    const char* entry_marker_string = "entry_marker";

}
//
//
//
Action::Action(Control *control_, Blocks *action_blocks_, Grammar *grammar_, MacroLookupTable *macro_table_)
       : return_code(0),
         control(control_),
         action_blocks(action_blocks_),
         grammar(grammar_),
         option(control_ -> option),
         lex_stream(control_ -> lex_stream),
         first_locally_exported_macro(0),
         locally_exported_macro_gate(0),
         macro_table(macro_table_)
{

    const char *abstract = "Abstract",
               *list = "List";
    ast_member_prefix = "lpg_";
    abstract_ast_list_classname = new char[strlen(abstract) + strlen(control_ -> option -> ast_type) + strlen(list) + 1];
    strcpy(abstract_ast_list_classname, abstract);
    strcat(abstract_ast_list_classname, control_ -> option -> ast_type);
    strcat(abstract_ast_list_classname, list);
    visitorFactory = new VisitorStaffFactory(option->visitor_type);
    const char* comment_prefix;
    if(control_->option->programming_language == Option::PYTHON2 || control_->option->programming_language == Option::PYTHON3)
    {
        comment_prefix = "#";
    }
    else
    {
        comment_prefix = "//";
       
    }
    {
        char temp_buf[128] = {};
        sprintf(temp_buf, "%s#line %ccurrent_line %cinput_file%c", comment_prefix,option->escape, option->escape, option->escape);
        current_line_input_file_info = temp_buf;
    }
    {
        char temp_buf[128] = {};
        sprintf(temp_buf, "*<li>Rule %crule_number:  %crule_text", option->escape, option->escape);
        rule_info_holder = temp_buf;

    }
    {
     
        char temp_buf[128] = {};
        sprintf(temp_buf, "    %s#line %ccurrent_line \"%cinput_file%c\"", comment_prefix, option->escape, option->escape, option->escape);
        line_header_holder = temp_buf;
    }
}


bool Action::IsNullClassname(ClassnameElement &element)
{
    return element.rule.Length() == 0 ||
           (strlen(element.real_name) == 0 &&
            (element.array_element_type_symbol == NULL ||
             element.array_element_type_symbol -> NameLength() == 0));
}

void Action::ComputeInterfaces(ClassnameElement &element, Array<const char *> &typestring, Tuple<int> &rule)
{
    SymbolLookupTable interface_set;
    for (int i = 0; i < rule.Length(); i++)
    {
        int rule_no = rule[i];
        const char *name = typestring[grammar -> rules[rule_no].lhs];
        if (interface_set.FindName(name, strlen(name)) == NULL)
        {
            interface_set.InsertName(name, strlen(name));
            element.interface_.Next() = grammar -> rules[rule_no].lhs;
        }
    }
}


//
//
//
void Action::InsertExportMacros()
{
    first_locally_exported_macro = export_macro_table.Size();

    for (int i = 0; i < grammar -> parser.exports.Length(); i++)
    {
        int export_token = grammar -> parser.exports[i];
        SimpleMacroSymbol *macro = InsertExportMacro(export_token);
        if (FindLocalMacro(macro -> Name(), macro -> NameLength()))
        {
            Tuple<const char *> msg;
            msg.Next() = "The name of the exported symbol ";
            msg.Next() = macro -> Name();
            msg.Next() = " conflicts with the predefined macro of the same name";
            option -> EmitError(export_token, msg);

            return_code = 12;
        }
    }

    locally_exported_macro_gate = export_macro_table.Size();

    return;
}


//
// Make sure that all terminals exported locally in this file were also generated locally.
//
void Action::CheckExportMacros()
{
    for (int i = first_locally_exported_macro; i < locally_exported_macro_gate; i++)
    {
        SimpleMacroSymbol *simple_macro = export_macro_table[i];
        if ( ! simple_macro -> IsUsed())
        {
            Tuple<const char *> msg;
            msg.Next() = "The exported terminal symbol ";
            msg.Next() = simple_macro -> Name() + 2;
            msg.Next() = " was not generated by any rule in this grammar";
            option -> EmitWarning(simple_macro -> Location(), msg);
        }
    }

    return;
}


void Action::InsertImportedFilterMacros()
{
    for (int i = 0; i < lex_stream -> NumImportedFilters(); i++)
    {
        int export_token = lex_stream -> ImportedFilter(i);
        SimpleMacroSymbol *macro = InsertExportMacro(export_token);
        if (FindLocalMacro(macro -> Name(), macro -> NameLength()))
        {
            Tuple<const char *> msg;
            msg.Next() = "The name of the exported symbol ";
            msg.Next() = macro -> Name();
            msg.Next() = " conflicts with the predefined macro of the same name";
            option -> EmitError(export_token, msg);

            return_code = 12;
        }
    }
}


//
// We now make sure that none of the user-defined macros appear
// in either the local macro table or the export_macro_table.
//
void Action::CheckMacrosForConsistency()
{
    for (int i = 0; i < macro_table -> Size(); i++)
    {
        MacroSymbol *macro = macro_table -> Element(i);
        if (FindLocalMacro(macro -> Name(), macro -> NameLength()))
        {
            Tuple<const char *> msg;
            msg.Next() = "The user-defined macro ";
            msg.Next() = macro -> Name();
            msg.Next() = " conflicts with the predefined macro of the same name";
            option -> EmitError(macro -> Location(), msg);

            return_code = 12;
        }
        else if (FindExportMacro(macro -> Name(), macro -> NameLength()))
        {
            Tuple<const char *> msg;
            msg.Next() = "The user-defined macro ";
            msg.Next() = macro -> Name();
            msg.Next() = " conflicts with an exported symbol of the same name";
            option -> EmitError(macro -> Location(), msg);

            return_code = 12;
        }
    }
}

void Action::SetupBuiltinMacros()
{
    using namespace BuildInMacroName;
    std::string entry_declarations_string = "$entry_declarations";
    entry_declarations_macro = FindUserDefinedMacro(entry_declarations_string.c_str(), entry_declarations_string.size());

    //
    // First, insert local macros. Then, process all actions
    //
    rule_number_macro = InsertLocalMacro(rule_number_string);
    rule_text_macro = InsertLocalMacro(rule_text_string);
    rule_size_macro = InsertLocalMacro(rule_size_string);
    input_file_macro = InsertLocalMacro(input_file_string);
    current_line_macro = InsertLocalMacro(current_line_string);
    next_line_macro = InsertLocalMacro(next_line_string);
    identifier_macro = InsertLocalMacro(identifier_string,
                                        (grammar -> parser.identifier_index == 0
                                                  ? NULL
                                                  : lex_stream -> GetVariableSymbol(grammar -> parser.identifier_index) -> Name()));
    symbol_declarations_macro = InsertLocalMacro(symbol_declarations_string, "");

    (void) InsertLocalMacro(num_rules_string, grammar -> num_rules);
    (void) InsertLocalMacro(num_terminals_string, grammar -> num_terminals);
    (void) InsertLocalMacro(num_nonterminals_string, grammar -> num_nonterminals);
    (void) InsertLocalMacro(num_non_terminals_string, grammar -> num_nonterminals); // for upward compatibility with old version
    (void) InsertLocalMacro(num_symbols_string, grammar -> num_symbols);
    (void) InsertLocalMacro(template_string, option -> template_name);
    (void) InsertLocalMacro(file_prefix_string, option -> file_prefix);
    (void) InsertLocalMacro(package_string, option -> package);
    (void) InsertLocalMacro(ast_package_string , option -> ast_package);
    (void) InsertLocalMacro(ast_type_string, option -> ast_type);
    (void) InsertLocalMacro(exp_type_string, option -> exp_type);
    (void) InsertLocalMacro(prs_type_string, option -> prs_type);
    (void) InsertLocalMacro(sym_type_string, option -> sym_type);
    (void) InsertLocalMacro(action_type_string, option -> action_type);
    (void) InsertLocalMacro(visitor_type_string, option -> visitor_type);
    (void) InsertLocalMacro(prefix_string, option -> prefix);
    (void) InsertLocalMacro(suffix_string, option -> suffix);

    entry_name_macro = InsertLocalMacro(entry_name_string); // Reserved macro. Prevent user redefinition
    entry_marker_macro = InsertLocalMacro(entry_marker_string); // Reserved macro. Prevent user redefinition

    //
    // Process filters macros 
    //
    for (int i = 0; i < lex_stream -> NumFilterMacros(); i++)
        InsertFilterMacro(lex_stream -> FilterMacro(i).macro_name, lex_stream -> FilterMacro(i).macro_value);

    return;
}


//
//
//
void Action::ProcessAstRule(ClassnameElement &classname, int rule_no, Tuple<ProcessedRuleElement> &processed_rule_elements)
{
    SymbolLookupTable &symbol_set = classname.symbol_set;
    Tuple<int> &rhs_type_index = classname.rhs_type_index;

    assert(symbol_set.Size() == 0);
    assert(rhs_type_index.Length() == 0);

    int offset = grammar -> FirstRhsIndex(rule_no) - 1;

    for (int i = 0; i < processed_rule_elements.Length(); i++)
    {
        ProcessedRuleElement &processed_rule_element = processed_rule_elements[i];

        VariableSymbol *variable_symbol = grammar -> GetSymbol(processed_rule_element.token_index);
        int image = (variable_symbol ? variable_symbol -> SymbolIndex() : 0);
        const char *variable_name;
        int length;
        if (image != 0)
        {
            variable_name = grammar -> RetrieveString(image);
            length = strlen(variable_name);
        }
        else
        {
            variable_name = lex_stream -> NameString(processed_rule_element.token_index);
            length = lex_stream -> NameStringLength(processed_rule_element.token_index);
        }

        if (*variable_name == option -> macro_prefix)
        {
            variable_name++;
            length--;
        }
        Symbol *symbol;
        if (! symbol_set.FindName(variable_name, length))
            symbol = symbol_set.InsertName(variable_name, length);
        else
        {
            IntToString suffix(processed_rule_element.position);
            length += strlen(suffix.String());
            char *new_name = new char[length + 1];
            strcpy(new_name, variable_name);
            strcat(new_name, suffix.String());
            symbol = symbol_set.InsertName(new_name, length);

            delete [] new_name;
        }

        assert(rhs_type_index.Length() == symbol -> Index());
        processed_rule_element.symbol_index = symbol -> Index();
        processed_rule_element.type_index = grammar -> rhs_sym[offset + processed_rule_element.position];
        rhs_type_index.Next() = processed_rule_element.type_index;
    }

    return;
}


void Action::ProcessAstMergedRules(LCA &lca, ClassnameElement &element, Tuple<int> &rule, Tuple< Tuple<ProcessedRuleElement> > &processed_rule_map)
{
    SymbolLookupTable &symbol_set = element.symbol_set;
    Tuple<int> &rhs_type_index = element.rhs_type_index;
    assert(symbol_set.Size() == 0);
    assert(rhs_type_index.Length() == 0);

    Tuple<bool> is_nonterminal;
    Tuple<int> token_of_symbol;
    Tuple<int> rule_of_symbol;
    Array< Tuple<int> > rule_map(rule.Length());

    for (int k = 0; k < rule.Length(); k++)
    {
        int rule_no = rule[k],
            offset = grammar -> FirstRhsIndex(rule_no) - 1;
        Tuple<ProcessedRuleElement> &processed_rule_elements = processed_rule_map[rule_no];

        SymbolLookupTable local_set;
        for (int i = 0; i < processed_rule_elements.Length(); i++) // Insert the macros for the rules associated with this block
        {
            ProcessedRuleElement &processed_rule_element = processed_rule_elements[i];

            VariableSymbol *variable_symbol = grammar -> GetSymbol(processed_rule_element.token_index);
            int image = (variable_symbol ? variable_symbol -> SymbolIndex() : 0);
            const char *variable_name;
            int length;
            if (image != 0)
            {
                variable_name = grammar -> RetrieveString(image);
                length = strlen(variable_name);
            }
            else
            {
                variable_name = lex_stream -> NameString(processed_rule_element.token_index);
                length = lex_stream -> NameStringLength(processed_rule_element.token_index);
            }

            if (*variable_name == option -> macro_prefix)
            {
                variable_name++;
                length--;
            }

            Symbol *symbol = symbol_set.FindName(variable_name, length);
            if (! symbol)
            {
                (void) local_set.InsertName(variable_name, length);
                symbol = symbol_set.InsertName(variable_name, length);
                assert(rhs_type_index.Length() == symbol -> Index());
                rhs_type_index.Next() = grammar -> rhs_sym[offset + processed_rule_element.position];
                is_nonterminal.Next() = grammar -> IsNonTerminal(grammar -> rhs_sym[offset + processed_rule_element.position]);
                token_of_symbol.Next() = processed_rule_element.token_index - 1;
                rule_of_symbol.Next() = rule_no;
            }
            else if (local_set.FindName(variable_name, length))
            {
                IntToString suffix(processed_rule_element.position);
                length += strlen(suffix.String());
                char *new_name = new char[length + 1];
                strcpy(new_name, variable_name);
                strcat(new_name, suffix.String());
                symbol = symbol_set.FindName(new_name, length);
                if (! symbol)
                {
                    symbol = symbol_set.InsertName(new_name, length);

                    assert(rhs_type_index.Length() == symbol -> Index());
                    rhs_type_index.Next() = grammar -> rhs_sym[offset + processed_rule_element.position];
                    is_nonterminal.Next() = grammar -> IsNonTerminal(grammar -> rhs_sym[offset + processed_rule_element.position]);
                    token_of_symbol.Next() = processed_rule_element.token_index - 1;
                    rule_of_symbol.Next() = rule_no;
                }
                else
                {
                    if (grammar -> IsNonTerminal(grammar -> rhs_sym[offset + processed_rule_element.position]) &&
                        is_nonterminal[symbol -> Index()])
                        rhs_type_index[symbol -> Index()] = lca.Find(rhs_type_index[symbol -> Index()],
                                                                     grammar -> rhs_sym[offset + processed_rule_element.position]);
                }
                (void) local_set.InsertName(new_name, length);
            }
            else
            {
                (void) local_set.FindOrInsertName(variable_name, length);
                if (grammar -> IsNonTerminal(grammar -> rhs_sym[offset + processed_rule_element.position]) &&
                    is_nonterminal[symbol -> Index()])
                    rhs_type_index[symbol -> Index()] = lca.Find(rhs_type_index[symbol -> Index()],
                                                                 grammar -> rhs_sym[offset + processed_rule_element.position]);
            }

            rule_map[k].Next() = symbol -> Index();
        }
    }

    //
    //
    //
    Array<ProcessedRuleElement> symbol_map(symbol_set.Size());
    for (int i = 0; i < rule.Length(); i++)
    {
        int rule_no = rule[i];
        Tuple<ProcessedRuleElement> &processed_rule_elements = processed_rule_map[rule_no];
        assert(processed_rule_elements.Length() == rule_map[i].Length());
        symbol_map.MemReset();
        for (int j = 0; j < rule_map[i].Length(); j++)
        {
            processed_rule_elements[j].symbol_index = rule_map[i][j];
            processed_rule_elements[j].type_index = rhs_type_index[rule_map[i][j]];

            symbol_map[processed_rule_elements[j].symbol_index] = processed_rule_elements[j];
        }

        //
        // Now, we will reconstruct the processed_rule_elements for rule_no. We
        // also properly set the symbol and type indexes for null arguments.
        //
        processed_rule_elements.Reset();
        {
            for (int j = 0; j < symbol_map.Size(); j++)
            {
                ProcessedRuleElement &processed_rule_element = processed_rule_elements.Next();
                processed_rule_element = symbol_map[j];
                if (processed_rule_element.position == 0)
                {
                    processed_rule_element.symbol_index = j;
                    processed_rule_element.type_index = rhs_type_index[j];
                }
            }
        }
    }

    return;
}


void Action::ProcessCodeActions(Tuple<ActionBlockElement> &actions, Array<const char *> &typestring, Tuple< Tuple<ProcessedRuleElement> > &processed_rule_map)
{
    //
    // We now process all action blocks that are associated with the rules.
    // During this processing the output information is buffered.
    //
    for (int k = 0; k < actions.Length(); k++)
    {
        rule_macro_table.Push(); // prepare to insert macros for this rule
        local_macro_table.Push(); // prepare to insert macros for this rule

        int rule_number = actions[k].rule_number;

        //
        // Map the position of each symbol in the right-hand side of the rule to
        // the type of the symbol in question.
        //
        Array<const char *> rule_type(grammar -> RhsSize(rule_number) + 1, NULL);
        Array<bool> is_terminal(grammar -> RhsSize(rule_number) + 1, false);
        if (grammar -> parser.types.Length() > 0)
        {
            for (int i = grammar -> FirstRhsIndex(rule_number), k = 1; i < grammar -> EndRhsIndex(rule_number); i++, k++)
            {
                int image = grammar -> rhs_sym[i];
                is_terminal[k] = grammar -> IsTerminal(image);
                rule_type[k] = typestring[image];
            }
        }

        Tuple<ProcessedRuleElement> &processed_rule_elements = processed_rule_map[rule_number];
        Array<SimpleMacroSymbol *> macros(processed_rule_elements.Length());
        int length = 0;
        const char *line_header = "//#line ";
        for (int i = 0; i < processed_rule_elements.Length(); i++) // Insert the macros for the rules associated with this block
        {
            ProcessedRuleElement &processed_rule_element = processed_rule_elements[i];
            if (processed_rule_element.position == 0) // a null argument that is not associated with any symbol
                continue;

            const char *name = lex_stream -> NameString(processed_rule_element.token_index);
            int macro_name_length = strlen(name);
            if (*name != option -> macro_prefix)
            {
                if (! Code::IsValidVariableName(name))
                {
                    VariableSymbol *symbol = grammar -> GetSymbol(processed_rule_element.token_index);
                    int image = (symbol ? symbol -> SymbolIndex() : 0);
                    if (image != 0)
                    {
                        name = grammar -> RetrieveString(image);
                        macro_name_length = strlen(name);
                    }
                }
                macro_name_length++;
            }

            char *macro_name = new char[macro_name_length + 1];
            if (*name == option -> macro_prefix)
                strcpy(macro_name, name);
            else
            {
                macro_name[0] = option -> macro_prefix;
                strcpy(&(macro_name[1]), name);
            }

            //
            // This problem can occur when the same variable is used more than once on the right-hand side of a rule.
            // The user can bypass this problem by simply renaming the variable. For example:
            //
            // Rule ::= a B a$a2
            //
            // In the example above, the second instance of a is renamed a2.
            //
            if (rule_macro_table.FindName(macro_name, macro_name_length))
            {
                Tuple<const char *> msg;
                msg.Next() = "Illegal respecification of predefined macro \"";
                msg.Next() = macro_name;
                msg.Next() = "\"";
                option -> EmitError(processed_rule_element.token_index, msg);
                return_code = 12;
            }

            //
            // This check addresses the case where the right-hand side of a rule contains a variable
            // with the same name as that of a predefined local macro (e.g., package)
            //
            if (FindLocalMacro(macro_name, macro_name_length))
            {
                Tuple<const char *> msg;
                msg.Next() = "Definition of the rule macro \"";
                msg.Next() = macro_name;
                msg.Next() = "\" temporarily hides the definition of a local macro with the same name";
                option -> EmitWarning(processed_rule_element.token_index, msg);
            }

            if (! Code::IsValidVariableName(macro_name + 1))
            {
                option -> EmitError(processed_rule_element.token_index, "Invalid use of a symbol name as a macro");
                return_code = 12;
            }

            macros[i] = InsertRuleMacro(macro_name, processed_rule_element.position);

            if (grammar -> parser.types.Length() > 0 && option -> automatic_ast == Option::NONE)
            {
                length += (strlen(line_header) +
                           15 + // max integer size is 11 + one space + 2 quotes for file name + newline.
                           lex_stream -> GetFileSymbol(processed_rule_element.token_index) -> NameLength());
                if (is_terminal[processed_rule_element.position])
                     length += (2 * strlen(rule_type[processed_rule_element.position]) +
                                macro_name_length +
                                strlen("  = () getRhsIToken();\n") +
                                strlen(macros[i] -> Value())
                                + 1);
                else length += (2 * strlen(rule_type[processed_rule_element.position]) +
                                macro_name_length +
                                strlen("  = () getRhsSym();\n") +
                                strlen(macros[i] -> Value())
                                + 1);
            }
            delete [] macro_name;
        }

        SimpleMacroSymbol *save_symbol_declarations_macro = symbol_declarations_macro;
        char *str = new char[length + 1];
        if (length > 0)
        {
            *str = '\0';
            for (int i = 0; i < processed_rule_elements.Length(); i++) // Insert the macros for the rules associated with this block
            {
                ProcessedRuleElement &processed_rule_element = processed_rule_elements[i];
                SimpleMacroSymbol *macro = macros[i];
                if (macro) // A valid macro?
                {
                    int position = processed_rule_element.position;
                    char *macro_name = macro -> Name() + 1; // +1 to skip escape

                    strcat(str, line_header);
                    IntToString line(lex_stream -> Line(processed_rule_element.token_index));
                    strcat(str, line.String());
                    strcat(str, " \"");
                    strcat(str, FileWithoutPrefix(lex_stream -> GetFileSymbol(processed_rule_element.token_index) -> Name()).c_str());
                    strcat(str, "\"\n");

                    strcat(str, rule_type[position]);
                    strcat(str, " ");
                    strcat(str, macro_name);
                    strcat(str, " = (");
                    strcat(str, rule_type[position]);
                    strcat(str, ") ");
                    if (is_terminal[position])
                         strcat(str, "getRhsIToken(");
                    else strcat(str, "getRhsSym(");
                    strcat(str, macro -> Value());
                    strcat(str, ");");

                    if (i < processed_rule_elements.Length() - 1)
                        strcat(str, "\n");
                }
            }

            assert(strlen(str) <= (unsigned) length);
            symbol_declarations_macro = InsertLocalMacro("symbol_declarations", str);
        }

        ProcessRuleActionBlock(actions[k]);

        delete [] str;
        symbol_declarations_macro = save_symbol_declarations_macro;

        local_macro_table.Pop(); // remove the macros for the rules associated with this block
        rule_macro_table.Pop(); // remove the macros for the rules associated with this block
    }

    return;
}
void Action::ProcessRuleActionBlock(ActionBlockElement& action)
{


    BlockSymbol* block = lex_stream->GetBlockSymbol(action.block_token);
    TextBuffer* buffer = action.buffer;
    const int rule_number = action.rule_number;

    if (option->automatic_ast || rule_number == 0)
    {
        ProcessActionBlock(action, /* add_location_directive = */ true);
    }
    else
    {
        int line_no = lex_stream->Line(action.block_token),
            start = lex_stream->StartLocation(action.block_token) + block->BlockBeginLength(),
            end = lex_stream->EndLocation(action.block_token) - block->BlockEndLength() + 1;
        const char* head = &(lex_stream->InputBuffer(action.block_token)[start]),
            * tail = &(lex_stream->InputBuffer(action.block_token)[end]);
        const char escape = option->macro_prefix;
        const char
    	    beginjava[] = { escape, 'B', 'e', 'g', 'i', 'n', 'J', 'a', 'v', 'a', '\0' },
            endjava[] = { escape, 'E', 'n', 'd', 'J', 'a', 'v', 'a', '\0' },

            beginaction[] = { escape, 'B', 'e', 'g', 'i', 'n', 'A', 'c', 't', 'i', 'o', 'n', '\0' },
            noaction[] = { escape, 'N', 'o', 'A', 'c', 't', 'i', 'o', 'n', '\0' },
            nullaction[] = { escape, 'N', 'u', 'l', 'l', 'A', 'c', 't', 'i', 'o', 'n', '\0' },
            badaction[] = { escape, 'B', 'a', 'd', 'A', 'c', 't', 'i', 'o', 'n', '\0' };
        const char* macro_name[] = {
                                       beginjava,
                                       beginaction,
                                       noaction,
                                       nullaction,
                                       badaction,
                                       NULL // WARNING: this NULL gate must appear last in this list
        };

        bool head_macro_found = false;
        for (const char* p = head; p < tail; p++)
        {
            if (*p == option->escape)
            {
                const char* cursor = p,
                    * end_cursor; // Find end macro name.
                for (end_cursor = cursor + 1;
                    end_cursor < tail && (Code::IsAlnum(*end_cursor) && *end_cursor != option->escape);
                    end_cursor++)
                    ;
                int k;
                for (k = 0; macro_name[k] != NULL; k++)
                {
                    if ((unsigned)(end_cursor - cursor) == strlen(macro_name[k]))
                    {
                        const char* q = cursor + 1;
                        for (int i = 1; q < end_cursor; i++, q++)
                            if (tolower(*q) != tolower(macro_name[k][i]))
                                break;
                        if (q == end_cursor) // found a match
                            break;
                    }
                }
                if (macro_name[k] != NULL) // macro was found in the list... Stop searching
                {
                    head_macro_found = true;
                    break;
                }
            }
        }

       
    	
        if (!head_macro_found)
        {
            MacroSymbol* beginjava_macro = FindUserDefinedMacro(beginjava, strlen(beginjava));
            if (beginjava_macro != NULL)
            {
                ProcessMacroBlock(action.location, beginjava_macro, buffer, rule_number, lex_stream->FileName(action.block_token), line_no);
            }
            else if (FindUndeclaredMacro(beginjava, strlen(beginjava)) == NULL)
            {
                Tuple <const char*> msg;
                msg.Next() = "The macro \"";
                msg.Next() = beginjava;
                msg.Next() = "\" is undefined. ";

                EmitMacroWarning(lex_stream->FileName(action.block_token), head - 1, head - 1, msg);
                InsertUndeclaredMacro(beginjava); // to avoid repeating error message about this macro
            }
        }

        ProcessActionBlock(action);

        if (!head_macro_found)
        {
        	auto  endjava_macro = FindUserDefinedMacro(endjava, strlen(endjava));
            if (endjava_macro != NULL)
            {
                ProcessMacroBlock(action.location, endjava_macro, buffer, rule_number, lex_stream->FileName(action.block_token), lex_stream->EndLine(action.block_token));
            }
            else if (FindUndeclaredMacro(endjava, strlen(endjava)) == NULL)
            {
                Tuple <const char*> msg;
                msg.Next() = "The macro \"";
                msg.Next() = endjava;
                msg.Next() = "\" is undefined. ";

                EmitMacroWarning(lex_stream->FileName(action.block_token), tail + 1, tail + 1, msg);
                InsertUndeclaredMacro(endjava); // to avoid repeating error message about this macro
            }
        }
    }
}
void Action::CompleteClassnameInfo(LCA &lca,
                                   TTC &ttc,
                                   BoundedArray< Tuple<int> > &global_map,
                                   Array<const char *> &typestring,
                                   Tuple< Tuple<ProcessedRuleElement> > &processed_rule_map,
                                   SymbolLookupTable &classname_set,
                                   Tuple<ClassnameElement> &classname,
                                   Array<RuleAllocationElement> &rule_allocation_map)
{
    //
    // Process Array classes and Split ambiguous classnames.
    //
    for (int i = 0; i < classname.Length(); i++)
    {
        //
        // No class is generated for rules that are associated with the null classname.
        //
        if (IsNullClassname(classname[i]))
            continue;

        if (classname[i].array_element_type_symbol != NULL)
        {
            Tuple<int> &rule = classname[i].rule;

            //
            // Compute the set of nonterminals that appear on the left-hand
            // side of the set of rules associated with this list.
            //
            Tuple<int> nonterminals;
            BoundedArray< Tuple<int> > nonterminal_map(grammar -> num_terminals + 1, grammar -> num_symbols);
            {
                for (int k = 0; k < rule.Length(); k++)
                {
                    int rule_no = rule[k],
                        lhs = grammar -> rules[rule_no].lhs;
                    if (nonterminal_map[lhs].Length() == 0)
                        nonterminals.Next() = lhs;
                    nonterminal_map[lhs].Next() = rule_no;
                }
            }

            //
            // Compute the set of nonterminals that can only produce this list.
            // In other words, the set of nonterminals such that all its rules
            // are associated with this list.
            //
            BoundedArray<bool> is_list_nonterminal(grammar -> num_terminals + 1, grammar -> num_symbols);
            is_list_nonterminal.MemReset();
            for (int k = 0; k < nonterminals.Length(); k++)
            {
                int lhs = nonterminals[k];
                if (global_map[lhs].Length() == nonterminal_map[lhs].Length())
                    is_list_nonterminal[lhs] = true;
                else if (option -> warnings)
                {
                    int rule_no = global_map[lhs][0],
                        source_index = grammar -> rules[rule_no].source_index;
                    option -> EmitWarning(grammar -> parser.rules[source_index].lhs_index,
                                          "Not all rules generated by this nonterminal are associated with this list");
                }
            }

            //
            //
            //
            if (classname[i].array_element_type_symbol -> NameLength() == 0)
            {
                //
                //
                //
                int element_type_symbol_index = classname[i].array_element_type_symbol -> SymbolIndex(); //-1;
                for (int k = 0; k < rule.Length(); k++)
                {
                    int rule_no = rule[k],
                        offset = grammar -> FirstRhsIndex(rule_no) - 1;
                    rule_allocation_map[rule_no].name = classname[i].GetAllocationName(grammar -> rules[rule_no].lhs);
                    rule_allocation_map[rule_no].element_type_symbol_index = classname[i].array_element_type_symbol -> SymbolIndex();
                    Tuple<ProcessedRuleElement> &processed_rule_elements = processed_rule_map[rule_no];
                    for (int j = 0;  j < processed_rule_elements.Length(); j++)
                    {
                        ProcessedRuleElement &processed_rule_element = processed_rule_elements[j];
                        int symbol = grammar -> rhs_sym[offset + processed_rule_element.position];
                        if (grammar -> IsNonTerminal(symbol) && is_list_nonterminal[symbol])
                        {
                            if (rule_allocation_map[rule_no].list_symbol != 0)
                            {
                                int source_index = grammar -> rules[rule_no].source_index;
                                option -> EmitError(grammar -> parser.rules[source_index].classname_index,
                                                    "Multiple list specified on the right-hand side of this rule");
                                return_code = 12;
                            }
                            rule_allocation_map[rule_no].list_symbol = symbol;
                            rule_allocation_map[rule_no].list_position = processed_rule_element.position;
                        }
                        else
                        {
                            if (rule_allocation_map[rule_no].element_symbol != 0)
                            {
                                int source_index = grammar -> rules[rule_no].source_index;
                                option -> EmitError(grammar -> parser.rules[source_index].array_element_type_index,
                                                    "Multiple element symbols specified on the right-hand side of this rule");
                                return_code = 12;
                            }
                            rule_allocation_map[rule_no].element_symbol = symbol;
                            rule_allocation_map[rule_no].element_position = processed_rule_element.position;
                        }
                    }

                    //
                    // A rule of the form A ::= ... B ... where B is a list.
                    // Note that the ... does not have to be the empty string.
                    // For example, we might have a rule of the form:
                    // A ::= ( List )
                    //
                    //   or
                    //
                    // A ::= List ;
                    //
                    if (rule_allocation_map[rule_no].list_position == 0 && rule_allocation_map[rule_no].element_position == 0)
                         rule_allocation_map[rule_no].list_kind = RuleAllocationElement::EMPTY;
                    else if (rule_allocation_map[rule_no].list_symbol == 0)
                    {
                        rule_allocation_map[rule_no].list_kind = RuleAllocationElement::SINGLETON;
                        //
                        // TODO: Remove this code!
                        //
                        // if (element_type_symbol_index == -1)
                        //    element_type_symbol_index = (grammar -> IsTerminal(rule_allocation_map[rule_no].element_symbol)
                        //                                     ? grammar -> Get_ast_token_interface()
                        //                                     : rule_allocation_map[rule_no].element_symbol);
                        // else
                        if (element_type_symbol_index != rule_allocation_map[rule_no].element_symbol)
                        {
                            element_type_symbol_index =
                                     lca.Find(element_type_symbol_index,
                                              (grammar -> IsTerminal(rule_allocation_map[rule_no].element_symbol)
                                                   ? grammar -> Get_ast_token_interface()
                                                   : rule_allocation_map[rule_no].element_symbol));
                        }
                    }
                    else if (rule_allocation_map[rule_no].element_symbol == 0)
                         rule_allocation_map[rule_no].list_kind = RuleAllocationElement::COPY_LIST;
                    else rule_allocation_map[rule_no].list_kind = RuleAllocationElement::ADD_ELEMENT;
                }

                //
                // TODO: Remove this code!
                //
                // Set the array element type for the classname and each RuleAllocationElement.
                //
                // if (element_type_symbol_index == -1)
                //    element_type_symbol_index = 0; // if not initialized, initialize it to "Ast" index
                //
                CheckRecursivenessAndUpdate(nonterminals, nonterminal_map, rule_allocation_map);
                for (int j = 0; j < nonterminals.Length(); j++)
                {
                    int lhs = nonterminals[j];
                    for (int k = 0; k < nonterminal_map[lhs].Length(); k++)
                    {
                        int rule_no = nonterminal_map[lhs][k];
                        rule_allocation_map[rule_no].element_type_symbol_index = element_type_symbol_index;
                    }
                }
            }
            else
            {
                //
                //
                //
                for (int k = 0; k < rule.Length(); k++)
                {
                    int rule_no = rule[k];
                    rule_allocation_map[rule_no].name = classname[i].GetAllocationName(grammar -> rules[rule_no].lhs);
                    rule_allocation_map[rule_no].element_type_symbol_index = classname[i].array_element_type_symbol -> SymbolIndex();

                    for (int j = grammar -> FirstRhsIndex(rule_no); j < grammar -> EndRhsIndex(rule_no); j++)
                    {
                        int symbol = grammar -> rhs_sym[j];
                        if (grammar -> IsNonTerminal(symbol) && is_list_nonterminal[symbol])
                        {
                            if (rule_allocation_map[rule_no].list_symbol != 0)
                            {
                                int source_index = grammar -> rules[rule_no].source_index;
                                option -> EmitError(grammar -> parser.rules[source_index].classname_index,
                                                    "Multiple list specified on the right-hand side of this rule");
                                return_code = 12;
                            }
                            rule_allocation_map[rule_no].list_symbol = symbol;
                            rule_allocation_map[rule_no].list_position = j - grammar -> FirstRhsIndex(rule_no) + 1;
                        }
                        else if (symbol == rule_allocation_map[rule_no].element_type_symbol_index)
                        {
                            if (rule_allocation_map[rule_no].element_symbol != 0)
                            {
                                int source_index = grammar -> rules[rule_no].source_index;
                                option -> EmitError(grammar -> parser.rules[source_index].array_element_type_index,
                                                    "Multiple element symbols specified on the right-hand side of this rule");
                                return_code = 12;
                            }
                            rule_allocation_map[rule_no].element_symbol = symbol;
                            rule_allocation_map[rule_no].element_position = j - grammar -> FirstRhsIndex(rule_no) + 1;
                        }
                    }

                    //
                    // A rule of the form A ::= ... B ... where B is a list.
                    // Note that the ... does not have to be the empty string.
                    // For example, we might have a rule of the form:
                    // A ::= ( List )
                    //
                    //   or
                    //
                    // A ::= List ;
                    //
                    if (rule_allocation_map[rule_no].list_position == 0 && rule_allocation_map[rule_no].element_position == 0)
                         rule_allocation_map[rule_no].list_kind = RuleAllocationElement::EMPTY;
                    else if (rule_allocation_map[rule_no].list_symbol == 0)
                         rule_allocation_map[rule_no].list_kind = RuleAllocationElement::SINGLETON;
                    else if (rule_allocation_map[rule_no].element_symbol == 0)
                         rule_allocation_map[rule_no].list_kind = RuleAllocationElement::COPY_LIST;
                    else rule_allocation_map[rule_no].list_kind = RuleAllocationElement::ADD_ELEMENT;
                }
            }

            //
            // Add the set of left-hand side nonterminals to the list of
            // interfaces for this ArrayList class. Note that the recursive
            // nonterminal must be the first element of the list.
            //
            CheckRecursivenessAndUpdate(nonterminals, nonterminal_map, rule_allocation_map);
            for (int j = 0; j < nonterminals.Length(); j++)
            {
                int lhs = nonterminals[j];
                classname[i].interface_.Next() = lhs;
            }
            assert(classname[i].ungenerated_rule.Length() == 0);
        }
        else
        {
            Tuple<int> &rule = classname[i].rule;
            if (rule.Length() == 1)
            {
                int rule_no = rule[0];
                classname[i].interface_.Next() = grammar -> rules[rule_no].lhs;
                ProcessAstRule(classname[i], rule_no, processed_rule_map[rule_no]);
                classname[i].is_terminal_class = (grammar -> RhsSize(rule_no) == 1 &&
                                                  ttc.IsTerminalSymbol(grammar -> rhs_sym[grammar -> FirstRhsIndex(rule_no)]));
                rule_allocation_map[rule_no].name = classname[i].real_name;
                rule_allocation_map[rule_no].is_terminal_class = classname[i].is_terminal_class;
            }
            else if (classname[i].specified_name != classname[i].real_name) // a classname was specified?
            {
                ComputeInterfaces(classname[i], typestring, rule);

                ProcessAstMergedRules(lca, classname[i], rule, processed_rule_map);

                //
                // A merged class is considered a terminal class if it contains
                // only a single attribute that is a terminal attribute. This is
                // possible when each rule associated with the merged class in
                // question is a single production; all the right-hand side 
                // symbols of these single productions are mapped into the same
                // name; and each right-hand side symbol is either a terminal or
                // it's a nonterminal associated only with terminal classes.
                //
//                classname[i].is_terminal_class = (classname[i].rhs_type_index.Length() == 1);
                classname[i].is_terminal_class = true;
                for (int k = 0; classname[i].is_terminal_class && k < rule.Length(); k++)
                {
                    int rule_no = rule[k];
                    classname[i].is_terminal_class = classname[i].is_terminal_class &&
                                                     (grammar -> RhsSize(rule_no) == 1 &&
                                                      ttc.IsTerminalSymbol(grammar -> rhs_sym[grammar -> FirstRhsIndex(rule_no)]));
                }

                {
                    for (int k = 0; k < rule.Length(); k++)
                        rule_allocation_map[rule[k]].is_terminal_class = classname[i].is_terminal_class;
                }
            }
            else // generate independent classes.
            {
                if (option -> rule_classnames == Option::SEQUENTIAL)
                {
                    for (int k = 0; k < rule.Length(); k++)
                    {
                        int rule_no = rule[k];
                   
                        IntToString suffix(k); // Using suffix(k) is more invariant than suffix(rule_no);
                        int length = strlen(classname[i].real_name) + strlen(suffix.String());
                        char *new_name = new char[length + 1];
                        strcpy(new_name, classname[i].real_name);
                        strcat(new_name, suffix.String());
                   
                        ClassnameElement &additional_classname = classname.Next();
                        additional_classname.rule.Next() = rule_no;
                        additional_classname.specified_name = classname_set.FindOrInsertName(new_name, strlen(new_name)) -> Name();
                        additional_classname.real_name = additional_classname.specified_name;
                   
                        delete [] new_name;
                    }
                }
                else // assert(option -> rule_classnames == Option::STABLE)
                {
                    for (int k = 0; k < rule.Length(); k++)
                    {
                        int rule_no = rule[k];

                        Tuple<const char *> symbol_list;
                        int index = grammar -> rules[rule_no].source_index, // original rule index in source
                            length = strlen(classname[i].real_name) + 1;
                        for (int j = lex_stream -> Next(grammar -> parser.rules[index].separator_index);
                                 j < grammar -> parser.rules[index].end_rhs_index;
                                 j = lex_stream -> Next(j))
                        {
                            if (lex_stream -> Kind(j) == TK_SYMBOL)
                            {
                                VariableSymbol *variable_symbol = lex_stream -> GetVariableSymbol(j);
                                if (variable_symbol != NULL)
                                {
                                    const char *variable_name = grammar -> RetrieveString(variable_symbol -> SymbolIndex());
                                    length += (strlen(variable_name) + 1);
                                    symbol_list.Next() = variable_name;
                                }
                            }
                        }

                        char *new_name = new char[length + 1];
                            strcpy(new_name, classname[i].real_name);
                            strcat(new_name, "_");
                            for (int k = 0; k < symbol_list.Length(); k++)
                            {
                                strcat(new_name, "_");
                                strcat(new_name, symbol_list[k]);
                            }

                            ClassnameElement &additional_classname = classname.Next();
                            additional_classname.rule.Next() = rule_no;
                            additional_classname.specified_name = classname_set.FindOrInsertName(new_name, strlen(new_name)) -> Name();
                            additional_classname.real_name = additional_classname.specified_name;
                        delete [] new_name;
                    }
                }

                classname[i].rule.Reset();
            }
        }
    }

    return;
}


void Action::ProcessAstActions(Tuple<ActionBlockElement> &,
                               Tuple<ActionBlockElement> &,
                               Tuple<ActionBlockElement> &,
                               Array<const char *> &,
                               Tuple< Tuple<ProcessedRuleElement> > &,
                               SymbolLookupTable &,
                               Tuple<ClassnameElement> &)
{
	
}


void Action::CheckRecursivenessAndUpdate(Tuple<int> &nonterminal_list,
                                         BoundedArray< Tuple<int> > &nonterminal_map,
                                         Array<RuleAllocationElement> &rule_allocation_map)
{
    enum { NONE, LEFT_RECURSIVE, RIGHT_RECURSIVE, AMBIGUOUS };
    for (int k = 0; k < nonterminal_list.Length(); k++)
    {
        int lhs = nonterminal_list[k];
        Tuple<int> &rule = nonterminal_map[lhs];
        int recursive = NONE;

        for (int i = 0; i < rule.Length(); i++)
        {
            int rule_no = rule[i];
            if (rule_allocation_map[rule_no].list_kind == RuleAllocationElement::ADD_ELEMENT)
            {
                if (rule_allocation_map[rule_no].list_position < rule_allocation_map[rule_no].element_position)
                {
                    if (recursive == NONE)
                         recursive = LEFT_RECURSIVE;
                    else if (recursive != LEFT_RECURSIVE)
                         recursive = AMBIGUOUS;
                }
                else
                {
                    if (recursive == NONE)
                         recursive = RIGHT_RECURSIVE;
                    else if (recursive != RIGHT_RECURSIVE)
                         recursive = AMBIGUOUS;
                }
            }
        }
    
        if (recursive == AMBIGUOUS)
        {
            int source_index = grammar -> rules[rule[0]].source_index;
            option -> EmitError(grammar -> parser.rules[source_index].lhs_index,
                                "This nonterminal is associated with both left-recursive and right-recursive rule(s)");
            return_code = 12;
        }
        else if (recursive != NONE)
        {
            for (int i = 0; i < rule.Length(); i++)
            {
                int rule_no = rule[i];
                if (rule_allocation_map[rule_no].list_kind == RuleAllocationElement::EMPTY)
                    rule_allocation_map[rule_no].list_kind = (recursive == LEFT_RECURSIVE
                                                                  ? RuleAllocationElement::LEFT_RECURSIVE_EMPTY
                                                                  : RuleAllocationElement::RIGHT_RECURSIVE_EMPTY);
                else if (rule_allocation_map[rule_no].list_kind == RuleAllocationElement::SINGLETON)
                    rule_allocation_map[rule_no].list_kind = (recursive == LEFT_RECURSIVE
                                                                  ? RuleAllocationElement::LEFT_RECURSIVE_SINGLETON
                                                                  : RuleAllocationElement::RIGHT_RECURSIVE_SINGLETON);
            }
        }
        else
        {
            assert(recursive == NONE);

            //
            // Nonrecursive rules are tagged as left-recursive.
            //
            for (int i = 0; i < rule.Length(); i++)
            {
                int rule_no = rule[i];
                if (rule_allocation_map[rule_no].list_kind == RuleAllocationElement::EMPTY)
                     rule_allocation_map[rule_no].list_kind = RuleAllocationElement::LEFT_RECURSIVE_EMPTY;
                else if (rule_allocation_map[rule_no].list_kind == RuleAllocationElement::SINGLETON)
                     rule_allocation_map[rule_no].list_kind = RuleAllocationElement::LEFT_RECURSIVE_SINGLETON;
            }
        }
    }

    return;
}


//
//
//
const char *Action::SkipMargin(TextBuffer *buffer, const char *cursor, const char *tail)
{
    int length = option -> margin;

    for (int i = 0; length > 0 && cursor < tail; i++, cursor++)
    {
        if (*cursor == Code::HORIZONTAL_TAB)
             length -= (Tab::TabSize() - (i % Tab::TabSize()));
        else if (Code::IsSpaceButNotNewline(*cursor))
             length--;
        else break;
    }

    //
    // If length has a negative value, the absolute value of length
    // indicates that the number of blank characters that should be
    // written to the output file. Note that this situation occurs
    // iff a tab character is used as the last character in a margin
    // space and, when fully expanded, it overlaps the data space.
    //
    if (length < 0)
    {
        for (int j = length; j > 0; j++)
             buffer -> Put(' ');
    }

    return cursor;
}


//
//
//
void Action::ProcessActionBlock(ActionBlockElement &action, bool add_location_directive)
{
    BlockSymbol *block = lex_stream -> GetBlockSymbol(action.block_token);
    TextBuffer *buffer = action.buffer;
    int rule_number = action.rule_number,
        line_no = lex_stream -> Line(action.block_token),
        start = lex_stream -> StartLocation(action.block_token) + block -> BlockBeginLength(),
        end   = lex_stream -> EndLocation(action.block_token) - block -> BlockEndLength() + 1;
    const char *head = &(lex_stream -> InputBuffer(action.block_token)[start]),
               *tail = &(lex_stream -> InputBuffer(action.block_token)[end]);

    //
    // If the block opener marker is immediately followed by a newline
    // character, skip the new line.
    // When we encounter the sequence \r\n we only consider \n as end-of-line.
    //
    // if (Code::IsNewline(*head))
    // {
    //     head++;
    //     line_no++;
    // }
    //
    //
    // If the block closer marker is immediately preceded by a newline
    // character, skip the new line.
    //
    // if (Code::IsNewline(*(tail - 1)))
    //     tail--;

    //
    // If a location directive should be emitted, do so here
    //
    if (add_location_directive)
    {
        const char *line_header = line_header_holder.c_str();
        buffer -> PutChar('\n');
        ProcessActionLine(block,action.location,
                          buffer,
                          lex_stream -> FileName(action.block_token),
                          &(line_header[0]),
                          &(line_header[strlen(line_header) - 1]),
                          rule_number,
                          lex_stream -> FileName(action.block_token),
                          line_no);
        buffer -> PutChar('\n');
    }

    //
    //
    //
    while (head < tail)
    {
        const char *cursor;

        //
        // When we encounter the sequence \r\n we only consider \n as end-of-line.
        //
        // for (cursor = head; Code::IsNewline(*cursor); cursor++)
        //
        for (cursor = head; *cursor == '\n'; cursor++)
        {
            buffer -> PutChar(*cursor);
            line_no++;
        }

        if (cursor > head) // any '\n' processed?
            cursor = SkipMargin(buffer, cursor, tail);

        //
        // When we encounter the sequence \r\n we only consider \n as end-of-line.
        //
        // for (head = cursor; cursor < tail && (! Code::IsNewline(*cursor)); cursor++)
        //
        for (head = cursor; (cursor < tail) && (*cursor != '\n'); cursor++)
            ;
        if (cursor > head)
            ProcessActionLine(block, action.location,
                              buffer,
                              lex_stream -> FileName(action.block_token),
                              head,
                              cursor,
                              rule_number,
                              lex_stream -> FileName(action.block_token),
                              line_no);
        head = cursor;
    }

    //
    // If the action block spanned only a single line, add a
    // line break to the output.
    //
    if (lex_stream -> Line(action.block_token) == lex_stream -> EndLine(action.block_token))
        buffer -> Put("\n");

    return;
}


//
//
//
void Action::ProcessMacroBlock(int location, MacroSymbol *macro, TextBuffer *default_buffer, int rule_number, const char *source_filename, int source_line_no)
{
    BlockSymbol* scope_block = nullptr;
    int block_token = macro -> Block();
    BlockSymbol *block = lex_stream -> GetBlockSymbol(block_token);
    assert(block);

    TextBuffer *buffer = (block == option -> DefaultBlock()
                                 ? default_buffer
                                 : block -> Buffer()
                                          ? block -> Buffer()
                                          : location == ActionBlockElement::INITIALIZE
                                                      ? block -> ActionfileSymbol() -> InitialHeadersBuffer()
                                                      : location == ActionBlockElement::BODY
                                                                  ? block -> ActionfileSymbol() -> BodyBuffer()
                                                                  : block -> ActionfileSymbol() -> FinalTrailersBuffer());

    int line_no = lex_stream -> Line(block_token),
        start = lex_stream -> StartLocation(block_token) + block -> BlockBeginLength(),
        end   = lex_stream -> EndLocation(block_token) - block -> BlockEndLength() + 1;
    const char *head = &(lex_stream -> InputBuffer(block_token)[start]),
               *tail = &(lex_stream -> InputBuffer(block_token)[end]);

    //
    //
    //
    while (head < tail)
    {
        const char *cursor;

        //
        // When we encounter the sequence \r\n we only consider \n as end-of-line.
        //
        // for (cursor = head; Code::IsNewline(*cursor); cursor++)
        //
        for (cursor = head; *cursor == '\n'; cursor++)
        {
            buffer -> PutChar(*cursor);
            line_no++;
        }

        if (cursor > head) // any '\n' processed?
            cursor = SkipMargin(buffer, cursor, tail);

        //
        // When we encounter the sequence \r\n we only consider \n as end-of-line.
        //
        // for (head = cursor; cursor < tail && (! Code::IsNewline(*cursor)); cursor++)
        //
        for (head = cursor; (cursor < tail) && (*cursor != '\n'); cursor++)
            ;
        if (cursor > head)
        {
            //
            // Note that when processing the last line in a macro block, if it contains a reference to one
            // of the macros $next_line or $input_file, then the macro should be expanded as if they were
            // specified in the containing Action block.
            //
            ProcessActionLine( scope_block, location,
                              buffer,
                              lex_stream -> FileName(block_token),
                              head,
                              cursor,
                              rule_number,
                              source_filename,
                              source_line_no);
        }
        head = cursor;
    }

    return;
}

//
// Process an action line consisting only of the macro whose name is
// specified by the parameter name. Note that it is assumed that:
//
//  1. The name does not contain the escape prefix which is added here.
//  2. The location of the output is ActionBlockElement::BODY
//  3. The file name associated with this output is the filename of the
//     default block.
//  4. The line number associated with this output is 0.
//
void Action::ProcessMacro(TextBuffer *buffer, const char *name, int rule_no)
{
    int length = strlen(name) + 1;
    char *macroname = new char[length + 1];
    macroname[0] = option -> macro_prefix;
    strcpy(&macroname[1], name);
    BlockSymbol* scope_block = nullptr;
    if (FindUserDefinedMacro(macroname, length))
    {
        const char *filename = lex_stream -> FileName(grammar -> rules[rule_no].first_token_index);
        int line_offset = lex_stream -> Line(grammar -> rules[rule_no].first_token_index) - 1;
        macroname[0] = option->escape;
        ProcessActionLine(scope_block,ActionBlockElement::BODY,
                          buffer,
                          filename,
                          macroname,
                          &macroname[length],
                          rule_no,
                          filename,
                          line_offset);
    }
    else if (! FindUndeclaredMacro(macroname, length))
    {
        Tuple <const char *> msg;
        msg.Next() = "The macro \"";
        msg.Next() = macroname;
        msg.Next() = "\" must be defined when the \"automatic_ast\" option is in effect";

        option -> EmitWarning(0, msg);
        InsertUndeclaredMacro(macroname); // to avoid repeating error message about this macro
    }

    delete [] macroname;

    return;
}


//
// When an error is detected during the expansion of a macro,
// if that that macro was invoked while expanding other macros
// we add the location info from all the calling macros.
//
void Action::GetCallingMacroLocations(Tuple<Token *> &locations)
{
    if (! file_location_stack.IsEmpty())
    {
        assert (file_location_stack.Length() == cursor_location_stack.Length() &&
                file_location_stack.Length() == end_cursor_location_stack.Length());

        Array<const char *> filename_copy(file_location_stack.Length()),
                            cursor_copy(cursor_location_stack.Length()),
                            end_cursor_copy(end_cursor_location_stack.Length());
        do
        {
            const char *filename = file_location_stack.Pop(),
                       *cursor = cursor_location_stack.Pop(),
                       *end_cursor = end_cursor_location_stack.Pop();
            filename_copy[file_location_stack.Length()] = filename;
            cursor_copy[cursor_location_stack.Length()] = cursor;
            end_cursor_copy[end_cursor_location_stack.Length()] = end_cursor;
        	
            InputFileSymbol *file_symbol = lex_stream -> FindOrInsertFile(filename);
            int error_location = cursor - file_symbol -> Buffer();
            Token *error_token = lex_stream -> GetErrorToken(file_symbol, error_location);
            int end_loc = error_location + end_cursor - cursor - 1;

            error_token -> SetEndLocation(end_loc);
            error_token -> SetKind(0);

            locations.Next() = error_token;
        } while (! file_location_stack.IsEmpty());

        //
        // restore the stack
        //
        for (int i = 0; i < filename_copy.Size(); i++)
        {
            file_location_stack.Push(filename_copy[i]);
            cursor_location_stack.Push(cursor_copy[i]);
            end_cursor_location_stack.Push(end_cursor_copy[i]);
        }
    }

    return;
}



//
//
//
Token *Action::GetMacroErrorToken(const char *filename, const char *start_cursor_location, const char *end_cursor_location)
{
    InputFileSymbol *file_symbol = lex_stream -> FindOrInsertFile(filename);
    int error_location = start_cursor_location - file_symbol -> Buffer();
    Token *error_token = lex_stream -> GetErrorToken(file_symbol, error_location);
    error_token -> SetEndLocation(error_location + end_cursor_location - start_cursor_location - 1);
    error_token -> SetKind(0);

    return error_token;
}

//
//
//
void Action::EmitMacroError(const char *filename, const char *start_cursor_location, const char *end_cursor_location, Tuple<const char *> &msg)
{
    Tuple<Token *> macro_token;
    GetCallingMacroLocations(macro_token);

    option -> EmitWarning((macro_token.Length() == 0
                                                 ? GetMacroErrorToken(filename, start_cursor_location, end_cursor_location)
                                                 : macro_token[0]),
                          msg);
    control -> Exit(12);
}


//
//
//
void Action::EmitMacroWarning(const char *filename, const char *start_cursor_location, const char *end_cursor_location, Tuple<const char *> &msg)
{
    Tuple<Token *> macro_token;
    GetCallingMacroLocations(macro_token);

    option -> EmitWarning((macro_token.Length() == 0
                                                 ? GetMacroErrorToken(filename, start_cursor_location, end_cursor_location)
                                                 : macro_token[0]),
                          msg);
}


//
//
//
Symbol *Action::FindClosestMatchForMacro(const char *filename, const char *cursor, const char *end_cursor, const char *start_cursor_location, const char *end_cursor_location)
{
    int length = end_cursor - cursor;
    char *macro_name = new char[length + 1];
    strncpy(macro_name, cursor, length);
    macro_name[length] = '\0';

    Symbol *symbol = NULL;
    int index = 0;

    //
    // First look for a close match in the user-defined macros (give them precedence in case of a tie).
    //
    for (int i = 0; i < macro_table -> Size(); i++)
    {
        MacroSymbol *macro = (*macro_table)[i];
        int new_index = Spell::Index(macro -> Name(), macro_name);
        if (macro -> Block() != 0 && new_index > index)
        {
            index = new_index;
            symbol = macro;
        }
    }

    //
    // Next, look for a close match in the rule macros, export macros, filter macros or local macros.
    //
    SimpleMacroLookupTable *table[] = { &rule_macro_table, &export_macro_table, &filter_macro_table, &local_macro_table };
    for (int k = 0; k < 4; k++)
    {
        for (int i = 0; i < table[k] -> Size(); i++)
        {
            SimpleMacroSymbol *macro = (*table[k])[i];
            int new_index = Spell::Index(macro -> Name(), macro_name);
            if (new_index > index)
            {
                index = new_index;
                symbol = macro;
            }
        }
    }


    //
    // Issue the message
    //
    Tuple <const char *> msg;
    msg.Next() = "The macro \"";
    msg.Next() = macro_name;
    msg.Next() = "\" is undefined. ";
    if (index < 4)
        msg.Next() = "No substitution made";
    else if (index < 10)
    {
        msg.Next() = "Do you mean \"";
        msg.Next() = symbol -> Name();
        msg.Next() = "\"";
    }
    else // same name except for case
    {
        msg.Next() = "LPG will assume you meant \"";
        msg.Next() = symbol -> Name();
        msg.Next() = "\"";
    }
    std::string temp222 = macro_name;

    EmitMacroWarning(filename, start_cursor_location, end_cursor_location, msg);
    
    InsertUndeclaredMacro(macro_name); // to avoid repeating error message about this macro

    delete [] macro_name;

    return (index == 10 ? symbol : NULL); // an index 0f 10 indicates a perfect match except for case differences.
}


//
// PROCESS_ACTION_LINE takes as arguments a line of text from an action
// block and the rule number with which the block is associated.
// It first scans the text for local macro names and then for
// user defined macro names. If one is found, the macro definition is sub-
// stituted for the name. The modified action text is then printed out in
// the action file.
//
void Action::ProcessActionLine(BlockSymbol* scope_block,
							   int location,
                               TextBuffer *buffer,
                               const char *filename,
                               const char *cursor,
                               const char *tail,
                               int rule_no,
                               const char *source_filename,
                               int source_line_no,
                               const char *start_cursor_location,
                               const char *end_cursor_location)
{
    assert(buffer);

    const char *start = cursor;
    while(cursor < tail)
    {
        //
        // If not the escape character, just print it !!!
        //
        if (*cursor != option -> escape)
        {
            buffer -> PutChar(*cursor);

            //
            // If the character just output is a new line marker then
            // skip the margin if one is specified.
            //
            if (Code::IsNewline(*cursor++))
            {
                cursor = SkipMargin(buffer, cursor, tail);
                start = cursor;
            }
        }
        else // all macro names begin with ESCAPE
        {
            //
            // Find macro name.
            //
            const char *end_cursor;
            for (end_cursor = cursor + 1;
                 end_cursor < tail && (Code::IsAlnum(*end_cursor) && *end_cursor != option -> escape);
                 end_cursor++)
                ;

            //
            // First, see if the macro is a Rule macro.
            // Next, check to see if it is a Local macro, a filter macro, an export_symbol macro
            // Finally, check to see if it is a user-defined macro
            //
            std::string macro_name(cursor, end_cursor);
            macro_name[0] = option->macro_prefix;

            SimpleMacroSymbol *simple_macro = nullptr;
            MacroSymbol *macro = nullptr;
            if ((simple_macro = FindRuleMacro(macro_name.c_str(), macro_name.size())) != NULL)
            {
                char *value = simple_macro -> Value();
                assert(value);
                buffer -> Put(value);

                cursor = end_cursor + (*end_cursor == option -> escape ? 1 : 0);
            }
            else if ((simple_macro = FindLocalMacro(macro_name.c_str(), macro_name.size())) != NULL)
            {
            	
                char *value = simple_macro -> Value();
                if (value)
                {
                    if (simple_macro == symbol_declarations_macro)
                    {
                        char *p = value;
                        while(*p)
                        {
                            char *q;
                            for (q = p + 1; *q && *q != '\n'; q++)
                                ;

                            ProcessActionLine(scope_block,location,
                                              buffer,
                                              filename,
                                              p,
                                              q,
                                              rule_no,
                                              source_filename,
                                              source_line_no);

                            p = q;
                            if (*p != '\0') // if more info to process, add offset
                            {
                                assert(*p == '\n');
                                buffer -> PutChar(*p++);
                                for (int i = 0; i < cursor - start; i++)
                                    buffer -> PutChar(' ');
                            }
                        }
                    }
                    else buffer -> Put(value);
                }
                else if (simple_macro == rule_number_macro)
                    buffer -> Put(rule_no);
                else if (simple_macro == rule_text_macro)
                {
                    if (rule_no == 0)
                        buffer -> Put("*** No Rule ***");
                    else
                    {
                        int index = grammar -> rules[rule_no].source_index; // original rule index in source
                        buffer -> Put(lex_stream -> NameString(grammar -> parser.rules[index].lhs_index));
                        buffer -> PutChar(' ');
                        buffer -> Put(grammar -> IsUnitProduction(rule_no) ? "->" : "::=");

                        for (int j = lex_stream -> Next(grammar -> parser.rules[index].separator_index);
                             j < grammar -> parser.rules[index].end_rhs_index;
                             j = lex_stream -> Next(j))
                        {
                            if (lex_stream -> Kind(j) == TK_SYMBOL)
                            {
                                buffer -> PutChar(' ');
                                buffer -> Put(lex_stream -> NameString(j));
                            }
                            else if (lex_stream -> Kind(j) == TK_MACRO_NAME)
                                buffer -> Put(lex_stream -> NameString(j));
                            else if (lex_stream -> Kind(j) == TK_EMPTY_KEY)
                            {
                                buffer -> PutChar(' ');
                                buffer -> PutChar(option -> escape);
                                buffer -> Put("Empty");
                            }
                        }
                    }
                }
                else if (simple_macro == rule_size_macro)
                     buffer -> Put(grammar -> RhsSize(rule_no));
                else if (simple_macro == input_file_macro)
                     buffer -> Put(FileWithoutPrefix(source_filename).c_str());
                else if (simple_macro == current_line_macro)
                     buffer -> Put(source_line_no);
                else if (simple_macro == next_line_macro)
                     buffer -> Put(source_line_no + 1);
                else if (simple_macro == identifier_macro)
                {
                    buffer -> Put("IDENTIFIER");

                    Tuple <const char *> msg;
                    msg.Next() = "No explicit user definition for $identfier - IDENTIFIER is assumed";
                    EmitMacroWarning(filename, cursor, end_cursor, msg);
                }
                else assert(false);

                cursor = end_cursor + (*end_cursor == option -> escape ? 1 : 0);
            }
            else if ((simple_macro = FindFilterMacro(macro_name.c_str(), macro_name.size())) != NULL)
            {
                char *value = simple_macro -> Value();
                assert(value);
                buffer -> Put(value);

                cursor = end_cursor + (*end_cursor == option -> escape ? 1 : 0);
            }
            else if ((simple_macro = FindExportMacro(macro_name.c_str(), macro_name.size())) != NULL)
            {
                simple_macro -> MarkUsed();

                ExpandExportMacro(buffer, simple_macro);

                cursor = end_cursor + (*end_cursor == option -> escape ? 1 : 0);
            }
            else if ((simple_macro = FindUndeclaredMacro(macro_name.c_str(), macro_name.size())) != NULL) // just skip undeclared macro
            {
            	
                  cursor = end_cursor + (*end_cursor == option -> escape ? 1 : 0);
            }

            else if ((macro = FindUserDefinedMacro(macro_name.c_str(), macro_name.size())) != NULL)
            {
                int block_token = macro -> Block();

                if (macro -> IsInUse())
                {
                    Tuple <const char *> msg;
                    msg.Next() = "Loop detected during the expansion of the macro \"";
                    msg.Next() = macro -> Name();
                    msg.Next() = "\"";
                    EmitMacroError(filename, cursor, end_cursor, msg);
                }

                BlockSymbol *block = lex_stream -> GetBlockSymbol(block_token);
                if (block == NULL) // if the macro was not found, see if there is a close match.
                {
                    Symbol *symbol = FindClosestMatchForMacro(filename,
                        macro_name.c_str(),
                        macro_name.c_str()+ macro_name.size(),
                                                              start_cursor_location == NULL ? cursor : start_cursor_location,
                                                              end_cursor_location == NULL ? end_cursor : end_cursor_location);
                    if (symbol == NULL)
                    {
                        buffer -> PutChar(*cursor);
                        cursor++;
                    }
                    else // A perfect substitution (except for case difference) was found. Use it.
                    {
                        ProcessActionLine(scope_block,location,
                                          buffer,
                                          filename,
                                          symbol -> Name(),
                                          &(symbol -> Name()[symbol -> NameLength()]),
                                          rule_no,
                                          source_filename,
                                          source_line_no);
                        cursor = end_cursor + (*end_cursor == option -> escape ? 1 : 0);
                    }
                }
                else
                {
		  //                    int start = lex_stream -> StartLocation(block_token) + block -> BlockBeginLength(),
		  //                        end   = lex_stream -> EndLocation(block_token) - block -> BlockEndLength() + 1;

                    file_location_stack.Push(filename);
                    cursor_location_stack.Push(start_cursor_location == NULL ? cursor : start_cursor_location);
                    end_cursor_location_stack.Push(end_cursor_location == NULL ? end_cursor : end_cursor_location);

                    macro -> MarkInUse();

                    //
                    // If the block containing the macro is a "default" block, then its body should
                    // be output in the current file. Otherwise, output it in the file with which
                    // it is associated.
                    //
                    if (macro == entry_declarations_macro)
                    {
                        //
                        // Process this macro for each starting symbol, in turn. At each
                        // iteration, declare the relevant entry_name and entry_marker macro.
                        //
                        for (int i = 1; i < grammar -> start_symbol.Length(); i++)
                        {
                            local_macro_table.Push(); // prepare to insert special macros for this environment

                            assert(InsertLocalMacro(BuildInMacroName::entry_name_string, grammar -> RetrieveString(grammar -> start_symbol[i] -> SymbolIndex())));
                            const char *marker = grammar -> RetrieveString(grammar -> declared_terminals_in_g[i]);
                            char *str = new char[strlen(option -> prefix) + strlen(marker) + strlen(option -> suffix) + 1];
                            strcpy(str, option -> prefix);
                            strcat(str, marker);
                            strcat(str, option -> suffix);
                            assert(InsertLocalMacro(BuildInMacroName::entry_marker_string, str));
                            delete [] str;

                            ProcessMacroBlock(location, macro, buffer, rule_no, source_filename, source_line_no);

                            local_macro_table.Pop(); // Leaving this environment
                        }
                    }
                    else
                    {
                        ProcessMacroBlock(location, macro, buffer, rule_no, source_filename, source_line_no);
                    }

                    macro -> MarkNotInUse();

                    end_cursor_location_stack.Pop();
                    cursor_location_stack.Pop();
                    file_location_stack.Pop();

                    cursor = end_cursor + (*end_cursor == option -> escape ? 1 : 0);
                }
            }
            else // undefined macro
            {
                Symbol *symbol = FindClosestMatchForMacro(filename,
                    macro_name.c_str(),
                    macro_name.c_str() + macro_name.size(),
                                                          start_cursor_location == NULL ? cursor : start_cursor_location,
                                                          end_cursor_location == NULL ? end_cursor : end_cursor_location);
                if (symbol == NULL)
                {
                    buffer -> PutChar(*cursor); // process the escape symbol
                    cursor++;
                }
                else // A perfect substitution (except for case difference) was found. Use it.
                {
                   
                    ProcessActionLine(scope_block, location,
                                      buffer,
                                      filename,
                                      symbol -> Name(),
                                      &(symbol -> Name()[symbol -> NameLength()]),
                                      rule_no,
                                      source_filename,
                                      source_line_no);
                    cursor = end_cursor + (*end_cursor == option -> escape ? 1 : 0);
                }
            }

        }
    }

    return;
}


void Action::GenerateCode(TextBuffer *b, const char *code, int rule_no)
{
    LexStream::TokenIndex separator_token = grammar -> parser.rules[grammar -> rules[rule_no].source_index].separator_index;
    int line_no = lex_stream -> Line(separator_token),
        start = lex_stream -> StartLocation(separator_token),
        end   = lex_stream -> EndLocation(separator_token) + 1;
    const char *start_cursor_location = &(lex_stream -> InputBuffer(separator_token)[start]),
               *end_cursor_location = &(lex_stream -> InputBuffer(separator_token)[end]);

    ProcessActionLine(nullptr, ActionBlockElement::BODY,
                      b,
                      lex_stream -> FileName(separator_token),
                      code,
                      &code[strlen(code)],
                      rule_no,
                      lex_stream -> FileName(separator_token),
                      line_no,
                      start_cursor_location,
                      end_cursor_location);
}

Action::~Action() {
    delete [] abstract_ast_list_classname;
    delete visitorFactory;
}
