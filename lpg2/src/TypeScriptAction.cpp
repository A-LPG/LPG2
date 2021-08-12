#include "CTC.h"
#include "NTC.h"
#include "TypeScriptAction.h"

#include <map>
#include <string>

#include "LCA.h"
#include "TTC.h"

TextBuffer* TypeScriptAction::GetBuffer(ActionFileSymbol* ast_filename_symbol) const
{
    if (option->IsTopLevel())
    {
        return  (ast_filename_symbol->BodyBuffer());
    }
	return (ast_filename_symbol->BufferForTypeScriptNestAst());
    
}
void TypeScriptAction::ProcessCodeActionEnd()
{
    if (option->IsPackage())
    {
        auto  ast_filename_symbol = option->DefaultBlock()->ActionfileSymbol();
        TextBuffer& ast_buffer = *(ast_filename_symbol->FinalTrailersBuffer());
        ast_buffer.Put("}");
    }
}
//
//
//
void TypeScriptAction::ProcessRuleActionBlock(ActionBlockElement &action)
{
    
    BlockSymbol *block = lex_stream -> GetBlockSymbol(action.block_token);
    TextBuffer *buffer = action.buffer;
    int rule_number = action.rule_number;

    if (option -> automatic_ast || rule_number == 0)
    {
        ProcessActionBlock(action, /* add_location_directive = */ true);
    }
    else
    {
        int line_no = lex_stream -> Line(action.block_token),
            start = lex_stream -> StartLocation(action.block_token) + block -> BlockBeginLength(),
            end   = lex_stream -> EndLocation(action.block_token) - block -> BlockEndLength() + 1;
        const char *head = &(lex_stream -> InputBuffer(action.block_token)[start]),
                   *tail = &(lex_stream -> InputBuffer(action.block_token)[end]);
        const char escape = option->lpg_escape;
        const char beginjava[]   = { escape, 'B', 'e', 'g', 'i', 'n', 'J', 'a', 'v', 'a', '\0'},
                   endjava[]     = { escape, 'E', 'n', 'd', 'J', 'a', 'v', 'a', '\0'},
                   beginaction[] = { escape, 'B', 'e', 'g', 'i', 'n', 'A', 'c', 't', 'i', 'o', 'n', '\0'},
                   noaction[]    = { escape, 'N', 'o', 'A', 'c', 't', 'i', 'o', 'n', '\0'},
                   nullaction[]  = { escape, 'N', 'u', 'l', 'l', 'A', 'c', 't', 'i', 'o', 'n', '\0'},
                   badaction[]   = { escape, 'B', 'a', 'd', 'A', 'c', 't', 'i', 'o', 'n', '\0'};
        const char *macro_name[] = {
                                       beginjava,
                                       beginaction,
                                       noaction,
                                       nullaction,
                                       badaction,
                                       NULL // WARNING: this NULL gate must appear last in this list
                                   };
        MacroSymbol *beginjava_macro = FindUserDefinedMacro(beginjava, strlen(beginjava)),
                    *endjava_macro   = FindUserDefinedMacro(endjava, strlen(endjava));
        bool head_macro_found = false;
        for (const char *p = head; p < tail; p++)
        {
            if (*p == option -> escape)
            {
                const char *cursor = p,
                           *end_cursor; // Find end macro name.
                for (end_cursor = cursor + 1;
                     end_cursor < tail && (Code::IsAlnum(*end_cursor) && *end_cursor != option -> escape);
                     end_cursor++)
                     ;
                int k;
                for (k = 0; macro_name[k] != NULL; k++)
                {
                    if ((unsigned) (end_cursor - cursor) == strlen(macro_name[k]))
                    {
                        const char *q = cursor + 1;
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

        if (! head_macro_found)
        {
            if (beginjava_macro != NULL)
            {
                ProcessMacroBlock(action.location, beginjava_macro, buffer, rule_number, lex_stream -> FileName(action.block_token), line_no);
            }
            else if (FindUndeclaredMacro(beginjava, strlen(beginjava)) == NULL)
            {
                Tuple <const char *> msg;
                msg.Next() = "The macro \"";
                msg.Next() = beginjava;
                msg.Next() = "\" is undefined. ";

                EmitMacroWarning(lex_stream -> FileName(action.block_token), head - 1, head - 1, msg);
                InsertUndeclaredMacro(beginjava); // to avoid repeating error message about this macro
            }
        }

        ProcessActionBlock(action);

        if (! head_macro_found)
        {
            if (endjava_macro != NULL)
            {
                ProcessMacroBlock(action.location, endjava_macro, buffer, rule_number, lex_stream -> FileName(action.block_token), lex_stream -> EndLine(action.block_token));
            }
            else if (FindUndeclaredMacro(endjava, strlen(endjava)) == NULL)
            {
                Tuple <const char *> msg;
                msg.Next() = "The macro \"";
                msg.Next() = endjava;
                msg.Next() = "\" is undefined. ";

                EmitMacroWarning(lex_stream -> FileName(action.block_token), tail + 1, tail + 1, msg);
                InsertUndeclaredMacro(endjava); // to avoid repeating error message about this macro
            }
        }
    }
}


//
//
//
void TypeScriptAction::ExpandExportMacro(TextBuffer *buffer, SimpleMacroSymbol *simple_macro)
{
    buffer -> Put(option -> exp_type);
    buffer -> Put(".");
    buffer -> Put(option -> exp_prefix);
    buffer -> Put(simple_macro -> Name() + 2); // skip initial escape and '_' characters
    buffer -> Put(option -> exp_suffix);
}


//
//
//
void TypeScriptAction::GenerateDefaultTitle(Tuple<ActionBlockElement> &notice_actions)
{
    //
    // If one or more notice blocks were specified, process and
    // print the notice at the beginning of each action file.
    //
    if (notice_actions.Length() > 0)
    {
        for (int i = 0; i < notice_actions.Length(); i++)
            ProcessActionBlock(notice_actions[i]);
        TextBuffer *buffer = notice_actions[0].buffer; // get proper buffer from first action
        buffer -> Put("\n");
        action_blocks -> PutNotice(*buffer);
    }

    //
    // Issue the package state
    //
    TextBuffer *buffer = (option -> DefaultBlock() -> Buffer()
                              ? option -> DefaultBlock() -> Buffer()
                              : option -> DefaultBlock() -> ActionfileSymbol() -> InitialHeadersBuffer());
    if (*option -> package != '\0')
    {
        buffer -> Put("export namespace ");
        buffer -> Put(option -> package);
        buffer -> Put("\n{\n\n");
    }
    if (option -> automatic_ast &&
        strcmp(option -> package, option -> ast_package) != 0 &&
        *option -> ast_package != '\0')
    {
        buffer -> Put("import  * as ");
        buffer -> Put(option -> ast_package);
        buffer->Put(" from \"./");
        buffer->Put(option->ast_package);
        buffer -> Put("\";\n");
    }

    return;
}


//
// First construct a file for this type. Write the title information to its header
// buffer and return the file.
//
ActionFileSymbol *TypeScriptAction::GenerateTitle(ActionFileLookupTable &ast_filename_table,
                                            Tuple<ActionBlockElement> &notice_actions,
                                            const char *type_name,
                                            bool needs_environment)
{
    const char* filetype = option->GetFileTypeWithLanguage();
    int filename_length = strlen(option -> ast_directory_prefix) + strlen(type_name) + strlen(filetype);
    char *filename = new char[filename_length + 1];
    strcpy(filename, option -> ast_directory_prefix);
    strcat(filename, type_name);
    strcat(filename, filetype);

    ActionFileSymbol *file_symbol = ast_filename_table.FindOrInsertName(filename, filename_length);
    TextBuffer *buffer = file_symbol -> InitialHeadersBuffer();
    if (notice_actions.Length() > 0)
    {
        //
        // Copy each notice action block, in turn, into a new
        // ActionBLockElement; redirect its output to this buffer
        // and process it.
        //
        for (int i = 0; i < notice_actions.Length(); i++)
        {
            ActionBlockElement action = notice_actions[i];
            action.buffer = buffer;
            ProcessActionBlock(action);
        }
        buffer -> Put("\n");
    }
    if (*option -> ast_package != '\0')
    {
        buffer -> Put("export namespace ");
        buffer -> Put(option -> ast_package);
        buffer -> Put("\n{\n\n");
    }

    if (needs_environment &&
        strcmp(option -> ast_package, option -> package) != 0 &&
        *option -> package != '\0')
    {
        buffer -> Put("using ");
        buffer -> Put(option -> package);
        buffer -> Put(";\n");
    }

    delete [] filename;

    return file_symbol;
}


ActionFileSymbol *TypeScriptAction::GenerateTitleAndGlobals(ActionFileLookupTable &ast_filename_table,
                                                      Tuple<ActionBlockElement> &notice_actions,
                                                      const char *type_name,
                                                      bool needs_environment)
{
    ActionFileSymbol *file_symbol = GenerateTitle(ast_filename_table, notice_actions, type_name, needs_environment);
    for (int i = 0; i < grammar -> parser.global_blocks.Length(); i++)
    {
        LexStream::TokenIndex block_token = grammar -> parser.global_blocks[i];
        BlockSymbol *block = lex_stream -> GetBlockSymbol(block_token);
        if (! option -> ActionBlocks().IsIgnoredBlock(block -> BlockBegin(), block -> BlockBeginLength()))
        {
            ActionBlockElement action;
            action.rule_number = 0;
            action.location = ActionBlockElement::INITIALIZE; // does not matter - block must be default block...
            action.block_token = block_token;
            action.buffer = file_symbol -> InitialHeadersBuffer();

            ProcessActionBlock(action);
            action.buffer -> Put("\n");
        }
    }

    return file_symbol;
}


//
//
//
void TypeScriptAction::GenerateEnvironmentDeclaration(TextBuffer &ast_buffer, const char *indentation)
{
    ast_buffer.Put(indentation); ast_buffer.Put("    private ");
								 ast_buffer.Put(" environment : ");
                                 ast_buffer.Put(option -> action_type);
                                 ast_buffer.Put(";\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    public ");
                                
                                 ast_buffer.Put(" getEnvironment() :");
                                 ast_buffer.Put(option->action_type);
                                 ast_buffer.Put("{ return this.environment; }\n\n");
}


void TypeScriptAction::ProcessAstActions(Tuple<ActionBlockElement>& actions,
    Tuple<ActionBlockElement>& notice_actions,
    Tuple<ActionBlockElement>& initial_actions,
    Array<const char*>& typestring,
    Tuple< Tuple<ProcessedRuleElement> >& processed_rule_map,
    SymbolLookupTable& classname_set,
    Tuple<ClassnameElement>& classname)
{
    ActionFileLookupTable ast_filename_table(4096);
  
    auto  ast_filename_symbol = option->DefaultBlock()->ActionfileSymbol();
    TextBuffer& ast_buffer =*(ast_filename_symbol->BodyBuffer());
	
    Array<RuleAllocationElement> rule_allocation_map(grammar->num_rules + 1);

    //
    // Map each rule into the set of action blocks that is associated with it.
    //
    Array< Tuple<ActionBlockElement> > rule_action_map(grammar->num_rules + 1);
    {
        for (int i = 0; i < actions.Length(); i++)
            rule_action_map[actions[i].rule_number].Next() = actions[i];
    }

    //
    // For each nonterminal A, compute the set of rules that A produces.
    //
    BoundedArray< Tuple<int> > global_map(grammar->num_terminals + 1, grammar->num_symbols);
    {
        for (int rule_no = grammar->FirstRule(); rule_no <= grammar->LastRule(); rule_no++)
        {
            int lhs = grammar->rules[rule_no].lhs;
            global_map[lhs].Next() = rule_no;
        }
    }

    TTC ttc(global_map, processed_rule_map);

    //
    // Compute the interface depences.
    //
    assert(grammar->Get_ast_token_interface() == grammar->num_symbols + 1);
    BoundedArray< Tuple<int> > extension_of(grammar->num_terminals + 1, grammar->Get_ast_token_interface());
    BoundedArray<BitSetWithOffset> extension_set(grammar->num_terminals + 1, grammar->Get_ast_token_interface());
    for (int nt = extension_set.Lbound(); nt <= extension_set.Ubound(); nt++)
        extension_set[nt].Initialize(extension_set.Size() + 1, extension_set.Lbound() - 1);

    for (int rule_no = grammar->FirstRule(); rule_no <= grammar->LastRule(); rule_no++)
    {
        int lhs = grammar->rules[rule_no].lhs;

        if (grammar->RhsSize(rule_no) == 1)
        {
            if (lhs != grammar->accept_image)
            {
                int symbol = grammar->rhs_sym[grammar->FirstRhsIndex(rule_no)];
                if (grammar->IsNonTerminal(symbol))
                {
                    if (!extension_set[symbol][lhs])
                    {
                        int source_index = grammar->rules[rule_no].source_index,
                            array_index = grammar->parser.rules[source_index].array_element_type_index;

                        //
                        // If the left-hand side is not an array(list) then it
                        // may extend the right-hand side
                        //
                        if (array_index == 0)
                        {
                            extension_set[symbol].AddElement(lhs);
                            extension_of[symbol].Next() = lhs;
                        }
                    }
                }
                else
                {
                    if (!extension_set[lhs][grammar->Get_ast_token_interface()])
                    {
                        int source_index = grammar->rules[rule_no].source_index,
                            array_index = grammar->parser.rules[source_index].array_element_type_index;

                        //
                        // If the left-hand side is not an array(list) then it
                        // may extend the right-hand side
                        //
                        if (array_index == 0)
                        {
                            extension_set[lhs].AddElement(grammar->Get_ast_token_interface());
                            extension_of[lhs].Next() = grammar->Get_ast_token_interface();
                        }
                    }
                }
            }
        }
    }

    LCA lca(extension_of);

    CompleteClassnameInfo(lca, ttc, global_map, typestring, processed_rule_map, classname_set, classname, rule_allocation_map);

    //
    // Compute a map from each interface into the (transitive closure)
    // of the set of classes that can implement in.
    // (CTC: class transitive closure)
    //
    CTC ctc(classname, typestring, extension_of);
    BoundedArray< Tuple<int> >& interface_map = ctc.GetInterfaceMap();

    //
    // (NTC: Nonterminals that can generate null ASTs.)
    //
    // A (user-specified) NULL ast is generated for a rule that the user explicitly
    // associates with the null classname. For example:
    //
    //    A$ ::= x y z
    //
    // Note that in order to qualify the rule in question must have either a single
    // terminal on its right-hand side or contain more than one symbol on its right-hand
    // side. When the right-hand side of a rule contain no symbol then the null ast must
    // be generated for that rule. When the right-hand side of a rule consist of a single
    // nonterminal then no AST needs be generated for it. Instead, the left-hand side
    // nonterminal must inherit the ast that was produced for the right-hand side nonterminal.
    // (Otherwise, we would end up with a dangling pointer.)
    //
    Array< bool > user_specified_null_ast(grammar->num_rules + 1, false);
    {
        for (int rule_no = grammar->FirstRule(); rule_no <= grammar->LastRule(); rule_no++)
        {
            int source_index = grammar->rules[rule_no].source_index,
                classname_index = grammar->parser.rules[source_index].classname_index;
            if (lex_stream->NameStringLength(classname_index) == 1) // The classname is the null macro?
            {
                user_specified_null_ast[rule_no] = (grammar->RhsSize(rule_no) > 1 ||
                    (grammar->RhsSize(rule_no) == 1 &&
                        grammar->IsTerminal(grammar->rhs_sym[grammar->FirstRhsIndex(rule_no)])));
            }
        }
    }
    NTC ntc(global_map, user_specified_null_ast, grammar);

    //
    // First process the root class, the list class, and the Token class.
    //
    {
        if (option->automatic_ast == Option::NESTED)
        {
            GenerateAstType(ast_filename_symbol, "    ", option->ast_type);
            GenerateAbstractAstListType(ast_filename_symbol, "    ", abstract_ast_list_classname);
            GenerateAstTokenType(ntc, ast_filename_symbol, "    ", grammar->Get_ast_token_classname());
        }
        else
        {
            assert(option->automatic_ast == Option::TOPLEVEL);

            ActionFileSymbol* file_symbol = GenerateTitleAndGlobals(ast_filename_table,
                notice_actions,
                option->ast_type,
                (grammar->parser.ast_blocks.Length() > 0));
            GenerateAstType(file_symbol, "", option->ast_type);
            file_symbol->Flush();

            file_symbol = GenerateTitleAndGlobals(ast_filename_table, notice_actions, abstract_ast_list_classname, false);
            GenerateAbstractAstListType(file_symbol, "", abstract_ast_list_classname);
            file_symbol->Flush();

            file_symbol = GenerateTitleAndGlobals(ast_filename_table, notice_actions, grammar->Get_ast_token_classname(), false);
            GenerateAstTokenType(ntc, file_symbol, "", grammar->Get_ast_token_classname());
            file_symbol->Flush();
        }
    }
    //
	// Generate the token interface
	//
    {
        astRootInterfaceName.append("IRootFor");
        astRootInterfaceName += option->action_type;
        if (option->automatic_ast == Option::NESTED)
            GenerateAstRootInterface(
                ast_filename_symbol,
                (char*)"    ");
        else
        {
            ActionFileSymbol* file_symbol = GenerateTitleAndGlobals(ast_filename_table, notice_actions, 
                astRootInterfaceName.c_str(), false);
            GenerateAstRootInterface(
                file_symbol,
                (char*)"    ");
            file_symbol->Flush();
        }

        
    }
	
    //
    // Generate the token interface
    //
    {
        char* ast_token_interfacename = new char[strlen(grammar->Get_ast_token_classname()) + 2];
        strcpy(ast_token_interfacename, "I");
        strcat(ast_token_interfacename, grammar->Get_ast_token_classname());

        if (option->automatic_ast == Option::NESTED)
            GenerateInterface(true /* is token */,
                              ast_filename_symbol,
                              (char*)"    ",
                              ast_token_interfacename,
                              extension_of[grammar->Get_ast_token_interface()],
                              interface_map[grammar->Get_ast_token_interface()],
                              classname);
        else
        {
            ActionFileSymbol* file_symbol = GenerateTitleAndGlobals(ast_filename_table, notice_actions, ast_token_interfacename, false);
            GenerateInterface(true /* is token */,
                              file_symbol,
                              (char*)"",
                              ast_token_interfacename,
                              extension_of[grammar->Get_ast_token_interface()],
                              interface_map[grammar->Get_ast_token_interface()],
                              classname);
            file_symbol->Flush();
        }

        delete[] ast_token_interfacename;
    }

    //
    // Generate the nonterminal interfaces.
    //
    for (int symbol = grammar->num_terminals + 1; symbol <= grammar->num_symbols; symbol++)
    {
        if (symbol != grammar->accept_image)
        {
            char* interface_name = new char[strlen(grammar->RetrieveString(symbol)) + 2];
            strcpy(interface_name, "I");
            strcat(interface_name, grammar->RetrieveString(symbol));

            if (option->automatic_ast == Option::NESTED)
                GenerateInterface(ctc.IsTerminalClass(symbol),
                                  ast_filename_symbol,
                                  (char*)"    ",
                                  interface_name,
                                  extension_of[symbol],
                                  interface_map[symbol],
                                  classname);
            else
            {
                ActionFileSymbol* file_symbol = extension_of[symbol].Length() > 0
                    ? GenerateTitle(ast_filename_table, notice_actions, interface_name, false)
                    : GenerateTitleAndGlobals(ast_filename_table, notice_actions, interface_name, false);
                GenerateInterface(ctc.IsTerminalClass(symbol),
                                  file_symbol,
                                  (char*)"",
                                  interface_name,
                                  extension_of[symbol],
                                  interface_map[symbol],
                                  classname);
                file_symbol->Flush();
            }

            delete[] interface_name;
        }
    }

    //
    // generate the rule classes.
    //
    for (int i = 0; i < classname.Length(); i++)
    {
        //
        // No class is generated for rules that are asoociated with the null classname.
        //
        if (IsNullClassname(classname[i]))
            continue;

        //
        // Figure out whether or not, classname[i] needs the environment
        // and process it if it is a List class.
        //
        Tuple<int>& rule = classname[i].rule;
        if (classname[i].array_element_type_symbol != NULL)
        {
            for (int k = 0; k < rule.Length(); k++)
            {
                int rule_no = rule[k];
                classname[i].rhs_type_index.Reset(); // expected by ProcessAstRule
                classname[i].symbol_set.Reset();     // expected by ProcessAstRule

                ProcessAstRule(classname[i], rule_no, processed_rule_map[rule_no]);

                classname[i].needs_environment = false;
            }
        }
        else
        {
            if (rule.Length() == 1)
            {
                int rule_no = rule[0];
                Tuple<ActionBlockElement>& actions = rule_action_map[rule_no];
                classname[i].needs_environment = (actions.Length() > 0);
            }
            else
            {
                assert(classname[i].specified_name != classname[i].real_name); // a classname was specified?
                for (int k = 0; k < rule.Length(); k++)
                {
                    int rule_no = rule[k];
                    rule_allocation_map[rule_no].name = classname[i].real_name;
                    Tuple<ActionBlockElement>& actions = rule_action_map[rule_no];
                    classname[i].needs_environment = classname[i].needs_environment || (actions.Length() > 0);
                }
            }
        }

        //
        // If the classes are to be generated as top-level classes, we first obtain
        // a file for this class.
        //
        ActionFileSymbol* file_symbol = (option->automatic_ast == Option::NESTED
            ? NULL
            : GenerateTitleAndGlobals(ast_filename_table,
                notice_actions,
                classname[i].real_name,
                classname[i].needs_environment));
        //
        //
        //
        if (classname[i].array_element_type_symbol != NULL)
        {
            //
            // Generate the class
            //
            GenerateListClass(ctc,
                              ntc,
                              (option->automatic_ast == Option::NESTED
	                               ? ast_filename_symbol
	                               : file_symbol),
                              (option->automatic_ast == Option::NESTED
	                               ? (char*)"    "
	                               : (char*)""),
                              classname[i],
                              typestring);

            for (int j = 0; j < classname[i].special_arrays.Length(); j++)
            {

                //
                // Process the new special array class.
                //
                file_symbol = (option->automatic_ast == Option::NESTED
                    ? NULL
                    : GenerateTitleAndGlobals(ast_filename_table, notice_actions, classname[i].special_arrays[j].name, true)); // needs_environment
                GenerateListExtensionClass(ctc,
                                           ntc,
                                           (option->automatic_ast == Option::NESTED
	                                            ? ast_filename_symbol
	                                            : file_symbol),
                                           (option->automatic_ast == Option::NESTED
	                                            ? (char*)"    "
	                                            : (char*)""),
                                           classname[i].special_arrays[j],
                                           classname[i],
                                           typestring);

                //
                // Generate   info for the allocation of rules associated with this class
                //
                Tuple<int>& special_rule = classname[i].special_arrays[j].rules;
                for (int k = 0; k < special_rule.Length(); k++)
                {
                    int rule_no = special_rule[k];
                    Tuple<ActionBlockElement>& actions = rule_action_map[rule_no];
                    if (file_symbol != NULL) // possible when option -> automatic_ast == Option::TOPLEVEL
                    {
                        for (int l = 0; l < actions.Length(); l++)
                            actions[l].buffer = file_symbol->BodyBuffer();
                    }
                    rule_allocation_map[rule_no].needs_environment = true;
                    ProcessCodeActions(actions, typestring, processed_rule_map);
                }
            }
        }
        else
        {
            if (rule.Length() == 1)
            {
                int rule_no = rule[0];
                Tuple<ActionBlockElement>& actions = rule_action_map[rule_no];
                rule_allocation_map[rule_no].needs_environment = classname[i].needs_environment;
                GenerateRuleClass(ctc,
                                  ntc,
                                  (option->automatic_ast == Option::NESTED
	                                   ? ast_filename_symbol
	                                   : file_symbol),
                                  (option->automatic_ast == Option::NESTED
	                                   ? (char*)"    "
	                                   : (char*)""),
                                  classname[i],
                                  typestring);

                if (file_symbol != NULL) // option -> automatic_ast == Option::TOPLEVEL
                {
                    for (int j = 0; j < actions.Length(); j++)
                        actions[j].buffer = file_symbol->BodyBuffer();
                }
                ProcessCodeActions(actions, typestring, processed_rule_map);
            }
            else
            {
                assert(classname[i].specified_name != classname[i].real_name); // a classname was specified?
                if (classname[i].is_terminal_class)
                    GenerateTerminalMergedClass(ntc,
                                                (option->automatic_ast == Option::NESTED
	                                                 ? ast_filename_symbol
	                                                 : file_symbol),
                                                (option->automatic_ast == Option::NESTED
	                                                 ? (char*)"    "
	                                                 : (char*)""),
                                                classname[i],
                                                typestring);
                else GenerateMergedClass(ctc,
                                         ntc,
                                         (option->automatic_ast == Option::NESTED
	                                          ? ast_filename_symbol
	                                          : file_symbol),
                                         (option->automatic_ast == Option::NESTED
	                                          ? (char*)"    "
	                                          : (char*)""),
                                         classname[i],
                                         processed_rule_map,
                                         typestring);

                for (int k = 0; k < rule.Length(); k++)
                {
                    int rule_no = rule[k];
                    rule_allocation_map[rule_no].needs_environment = classname[i].needs_environment;
                    Tuple<ActionBlockElement>& actions = rule_action_map[rule_no];
                    if (file_symbol != NULL) // possible when option -> automatic_ast == Option::TOPLEVEL
                    {
                        for (int j = 0; j < actions.Length(); j++)
                            actions[j].buffer = file_symbol->BodyBuffer();
                    }
                    ProcessCodeActions(actions, typestring, processed_rule_map);
                }
            }
        }

    }

    //
    //
    //
    SymbolLookupTable type_set;
    type_set.FindOrInsertName(grammar->Get_ast_token_classname(), strlen(grammar->Get_ast_token_classname()));
    {
        for (int i = 0; i < classname.Length(); i++)
        {
            //
            // No class is generated for rules that are associated with the null classname.
            //
            if (!IsNullClassname(classname[i]))
            {
                //
                // Note that it is CRUCIAL that the special array names be added
                // to the type_set prior to the base array. Since they are subclasses
                // of the base array, when visiting a generic AST node, we need to check
                // first whether or not it is a special array before we check if it the base.
                //
                for (int k = 0; k < classname[i].special_arrays.Length(); k++)
                    type_set.FindOrInsertName(classname[i].special_arrays[k].name, strlen(classname[i].special_arrays[k].name));
                type_set.FindOrInsertName(classname[i].real_name, strlen(classname[i].real_name));
            }
        }
    }

    //
    // Generate the visitor interfaces and Abstract classes that implements
    // the visitors.
    //
    {
        const char* visitor_type = option->visitor_type,
            * argument = "Argument",
            * result = "Result",
            * abstract = "Abstract";
        char* argument_visitor_type = new char[strlen(argument) + strlen(visitor_type) + 1],
            * result_visitor_type = new char[strlen(result) + strlen(visitor_type) + 1],
            * result_argument_visitor_type = new char[strlen(result) + strlen(argument) + strlen(visitor_type) + 1],
            * abstract_visitor_type = new char[strlen(abstract) + strlen(visitor_type) + 1],
            * abstract_result_visitor_type = new char[strlen(abstract) + strlen(result) + strlen(visitor_type) + 1];

        strcpy(argument_visitor_type, argument);
        strcat(argument_visitor_type, visitor_type);

        strcpy(result_visitor_type, result);
        strcat(result_visitor_type, visitor_type);

        strcpy(result_argument_visitor_type, result);
        strcat(result_argument_visitor_type, argument);
        strcat(result_argument_visitor_type, visitor_type);

        strcpy(abstract_visitor_type, abstract);
        strcat(abstract_visitor_type, visitor_type);

        strcpy(abstract_result_visitor_type, abstract);
        strcat(abstract_result_visitor_type, result);
        strcat(abstract_result_visitor_type, visitor_type);

        if (option->visitor == Option::DEFAULT)
        {
            if (option->automatic_ast == Option::NESTED)
            {
                GenerateSimpleVisitorInterface(ast_filename_symbol, "    ", visitor_type, type_set);
                GenerateArgumentVisitorInterface(ast_filename_symbol, "    ", argument_visitor_type, type_set);
                GenerateResultVisitorInterface(ast_filename_symbol, "    ", result_visitor_type, type_set);
                GenerateResultArgumentVisitorInterface(ast_filename_symbol, "    ", result_argument_visitor_type, type_set);

                GenerateNoResultVisitorAbstractClass(ast_filename_symbol, "    ", abstract_visitor_type, type_set);
                GenerateResultVisitorAbstractClass(ast_filename_symbol, "    ", abstract_result_visitor_type, type_set);
            }
            else
            {
                ActionFileSymbol* file_symbol = GenerateTitle(ast_filename_table, notice_actions, visitor_type, false);
                GenerateSimpleVisitorInterface(file_symbol, "", visitor_type, type_set);
                file_symbol->Flush();

                file_symbol = GenerateTitle(ast_filename_table, notice_actions, argument_visitor_type, false);
                GenerateArgumentVisitorInterface(file_symbol, "", argument_visitor_type, type_set);
                file_symbol->Flush();

                file_symbol = GenerateTitle(ast_filename_table, notice_actions, result_visitor_type, false);
                GenerateResultVisitorInterface(file_symbol, "", result_visitor_type, type_set);
                file_symbol->Flush();

                file_symbol = GenerateTitle(ast_filename_table, notice_actions, result_argument_visitor_type, false);
                GenerateResultArgumentVisitorInterface(file_symbol, "", result_argument_visitor_type, type_set);
                file_symbol->Flush();

                file_symbol = GenerateTitle(ast_filename_table, notice_actions, abstract_visitor_type, false);
                GenerateNoResultVisitorAbstractClass(file_symbol, "", abstract_visitor_type, type_set);
                file_symbol->Flush();

                file_symbol = GenerateTitle(ast_filename_table, notice_actions, abstract_result_visitor_type, false);
                GenerateResultVisitorAbstractClass(file_symbol, "", abstract_result_visitor_type, type_set);
                file_symbol->Flush();
            }
        }
        else if (option->visitor == Option::PREORDER)
        {
            if (option->automatic_ast == Option::NESTED)
            {
                GeneratePreorderVisitorInterface(ast_filename_symbol, "    ", visitor_type, type_set);
                GeneratePreorderVisitorAbstractClass(ast_filename_symbol, "    ", abstract_visitor_type, type_set);
            }
            else
            {
                ActionFileSymbol* file_symbol = GenerateTitleAndGlobals(ast_filename_table, notice_actions, visitor_type, false);
                GeneratePreorderVisitorInterface(file_symbol, "", visitor_type, type_set);
                file_symbol->Flush();

                file_symbol = GenerateTitleAndGlobals(ast_filename_table, notice_actions, abstract_visitor_type, false);
                GeneratePreorderVisitorAbstractClass(file_symbol, "", abstract_visitor_type, type_set);
                file_symbol->Flush();
            }
        }

        delete[] argument_visitor_type;
        delete[] result_visitor_type;
        delete[] result_argument_visitor_type;
        delete[] abstract_visitor_type;
        delete[] abstract_result_visitor_type;
    }

    ProcessCodeActions(initial_actions, typestring, processed_rule_map);

    int count = 0;
    {
        //
        // Note that the start rules are skipped as no AST node is allocated for them.
        //
        for (int rule_no = grammar->GetStartSymbol().Length(); rule_no < rule_allocation_map.Size(); rule_no++)
        {
            if ((user_specified_null_ast[rule_no]) ||
                ((rule_allocation_map[rule_no].name != NULL || grammar->RhsSize(rule_no) == 0) &&
                    (rule_allocation_map[rule_no].list_kind != RuleAllocationElement::COPY_LIST || rule_allocation_map[rule_no].list_position != 1)))
            {
                count++;

                //
                // Check whether or not the rule is a single production
                // and if so, issue an error and stop.
                //
                if (grammar->IsUnitProduction(rule_no))
                {
                    int source_index = grammar->rules[rule_no].source_index;
                    option->EmitError(grammar->parser.rules[source_index].separator_index,
                        "Unable to generate Ast allocation for single production");
                    return_code = 12;
                }

                if (count % option->max_cases == 0)
                {
                    ProcessMacro(&ast_buffer, "SplitActions", rule_no);
                    count++;
                }

                ProcessMacro(&ast_buffer, "BeginAction", rule_no);

                if (rule_allocation_map[rule_no].list_kind != RuleAllocationElement::NOT_A_LIST)
                {
                    GenerateListAllocation(ctc,
                        ast_buffer,
                        rule_no,
                        rule_allocation_map[rule_no]);
                }
                else
                {
                    if (user_specified_null_ast[rule_no] || (grammar->RhsSize(rule_no) == 0 && rule_allocation_map[rule_no].name == NULL))
                        GenerateNullAstAllocation(ast_buffer, rule_no);
                    else GenerateAstAllocation(ctc,
                        ast_buffer,
                        rule_allocation_map[rule_no],
                        processed_rule_map[rule_no],
                        typestring,
                        rule_no);
                }

                GenerateCode(&ast_buffer, "\n    ", rule_no);
                ProcessMacro(&ast_buffer, "EndAction", rule_no);
            }
            else
            {
                //
                // Make sure that no action block is associated with a rule for
                // which no class is allocated when it is reduced.
                //
                for (int k = 0; k < rule_action_map[rule_no].Length(); k++)
                {
                    ActionBlockElement& action = rule_action_map[rule_no][k];
                    option->EmitError(action.block_token,
                        "Since no class is associated with this production, the information in this block is unreachable");
                    return_code = 12;
                }

                ProcessMacro(&ast_buffer, "NoAction", rule_no);
            }
        }
    }
  
    return;
}




//
//
//
void TypeScriptAction::GenerateVisitorHeaders(TextBuffer &ast_buffer, const char *indentation, const char *modifiers)
{
    if (option -> visitor != Option::NONE)
    {
        char *header = new char[strlen(indentation) + strlen(modifiers) + 9];
        strcpy(header, indentation);
        strcat(header, modifiers);

        ast_buffer.Put(header);
        if (option -> visitor == Option::PREORDER)
        {
            ast_buffer.Put("accept(v : IAstVisitor ) : void;");
        }
        else if (option -> visitor == Option::DEFAULT)
        {
            ast_buffer.Put("acceptWithVisitor(v : ");
            ast_buffer.Put(option -> visitor_type);
            ast_buffer.Put(") : void;");

            ast_buffer.Put("\n");

            ast_buffer.Put(header);
            ast_buffer.Put(" acceptWithArg(v : Argument");
            ast_buffer.Put(option -> visitor_type);
            ast_buffer.Put(", o : any) : void;\n");

            ast_buffer.Put(header);
            ast_buffer.Put("acceptWithResult(v : Result");
            ast_buffer.Put(option -> visitor_type);
            ast_buffer.Put(") : any;\n");

            ast_buffer.Put(header);
            ast_buffer.Put("acceptWthResultArgument(v : ResultArgument");
            ast_buffer.Put(option -> visitor_type);
            ast_buffer.Put(", o : any) : any;");
        }
        ast_buffer.Put("\n");

        delete [] header;
    }

    return;
}


//
//
//
void TypeScriptAction::GenerateVisitorMethods(NTC &ntc,
                                        TextBuffer &ast_buffer,
                                        const char *indentation,
                                        ClassnameElement &element,
                                        BitSet &optimizable_symbol_set)
{
    if (option -> visitor == Option::DEFAULT)
    {
        ast_buffer.Put("\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    public  acceptWithVisitor(v : ");
                                     ast_buffer.Put(option -> visitor_type);
                                     ast_buffer.Put(") : void{ v.visit"); ast_buffer.Put(element.real_name); ast_buffer.Put("(this); }\n");

        ast_buffer.Put(indentation); ast_buffer.Put("    public  acceptWithArg(v : Argument");
                                     ast_buffer.Put(option -> visitor_type);
                                     ast_buffer.Put(", o : any) : void { v.visit");ast_buffer.Put(element.real_name); ast_buffer.Put("(this, o); }\n");

        ast_buffer.Put(indentation); ast_buffer.Put("    public  acceptWithResult(v : Result");
                                     ast_buffer.Put(option -> visitor_type);
                                     ast_buffer.Put(") : any{ return v.visit");ast_buffer.Put(element.real_name); ast_buffer.Put("(this); }\n");

        ast_buffer.Put(indentation); ast_buffer.Put("    public   acceptWthResultArgument(v : ResultArgument");
                                     ast_buffer.Put(option -> visitor_type);
                                     ast_buffer.Put(", o : any) : any { return v.visit"); ast_buffer.Put(element.real_name); ast_buffer.Put("(this, o); }\n");
    }
    else if (option -> visitor == Option::PREORDER)
    {
        ast_buffer.Put("\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    public   accept(v : IAstVisitor ) : void\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        if (! v.preVisit(this)) return;\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        this.enter(<"); 
                                     ast_buffer.Put(option -> visitor_type);
                                     ast_buffer.Put("> v);\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        v.postVisit(this);\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    }\n\n");

        ast_buffer.Put(indentation); ast_buffer.Put("    public   enter(v : ");
                                     ast_buffer.Put(option -> visitor_type);
                                     ast_buffer.Put(") : void\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
        SymbolLookupTable &symbol_set = element.symbol_set;
        Tuple<int> &rhs_type_index = element.rhs_type_index;
        if (element.is_terminal_class || symbol_set.Size() == 0)
        {
            ast_buffer.Put(indentation); ast_buffer.Put("        v.visit"); ast_buffer.Put(element.real_name); ast_buffer.Put("(this);\n");
          
        }
        else
        {
            ast_buffer.Put(indentation); ast_buffer.Put("        let checkChildren = v.visit");ast_buffer.Put(element.real_name); ast_buffer.Put("(this);\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        if (checkChildren)\n");
            if (symbol_set.Size() > 1)
            {
                ast_buffer.Put(indentation); ast_buffer.Put("        {\n");
            }

            for (int i = 0; i < symbol_set.Size(); i++)
            {
                ast_buffer.Put(indentation); ast_buffer.Put("            ");
                if ((! optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
                {
                    ast_buffer.Put("if (this._");
                    ast_buffer.Put(symbol_set[i] -> Name());
                    ast_buffer.Put(" != undefined) ");
                }
                ast_buffer.Put("this._");
                ast_buffer.Put(symbol_set[i] -> Name());
                ast_buffer.Put(".accept(v);\n");
            }

            if (symbol_set.Size() > 1)
            {
                ast_buffer.Put(indentation); ast_buffer.Put("        }\n");
            }
        }
        ast_buffer.Put(indentation); ast_buffer.Put("        v.endVisit"); ast_buffer.Put(element.real_name); ast_buffer.Put("(this);\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
    }

    return;
}


//
//
//
void TypeScriptAction::GenerateGetAllChildrenMethod(TextBuffer &ast_buffer,
                                              const char *indentation,
                                              ClassnameElement &element)
{
    if (! element.is_terminal_class)
    {
        SymbolLookupTable &symbol_set = element.symbol_set;

        ast_buffer.Put("\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    /**\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     * A list of all children of this node, including the null ones.\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     */\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     public   getAllChildren() : Lpg.Util.ArrayList<IAst>\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        let list = new Lpg.Util.ArrayList<IAst>();\n");
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            ast_buffer.Put(indentation); ast_buffer.Put("        list.add(this._");
                                         ast_buffer.Put(symbol_set[i] -> Name());
                                         ast_buffer.Put(");\n");
        }
        ast_buffer.Put(indentation); ast_buffer.Put("        return list;\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
    }

    return;
}


//
//
//
void TypeScriptAction::GenerateSimpleVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                const char *indentation,
                                                const char *interface_name,
                                                SymbolLookupTable &type_set)
{
    TextBuffer& ast_buffer =*GetBuffer(ast_filename_symbol);

    ast_buffer.Put(indentation); ast_buffer.Put("export interface ");
                                 ast_buffer.Put(interface_name);
                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        ast_buffer.Put(indentation); ast_buffer.Put("    visit"); ast_buffer.Put(symbol->Name());
                                     ast_buffer.Put("(n : ");
                                     ast_buffer.Put(symbol -> Name());
                                     ast_buffer.Put(") : void;\n");
    }

                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    visit");
                                 ast_buffer.Put("(n : ");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(") : void;\n");

    ast_buffer.Put(indentation); ast_buffer.Put("}\n");

	if(option->IsTopLevel() && option->IsPackage())
	{
        ast_buffer.Put(indentation); ast_buffer.Put("}\n");// for namespace
	}
}

//
//
//
void TypeScriptAction::GenerateArgumentVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                  const char *indentation,
                                                  const char *interface_name,
                                                  SymbolLookupTable &type_set)
{
   
    TextBuffer& ast_buffer =*GetBuffer(ast_filename_symbol);

    ast_buffer.Put(indentation); ast_buffer.Put("export interface ");
                                 ast_buffer.Put(interface_name);
                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        ast_buffer.Put(indentation); ast_buffer.Put("    visit"); ast_buffer.Put(symbol->Name());
                                     ast_buffer.Put("(n : ");
                                     ast_buffer.Put(symbol -> Name());
                                     ast_buffer.Put(", o : any) : void;\n");
    }

                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    visit");
                                 ast_buffer.Put("(n : ");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(", o : any) : void;\n");

    ast_buffer.Put(indentation); ast_buffer.Put("}\n");
    if (option->IsTopLevel() && option->IsPackage())
    {
        ast_buffer.Put(indentation); ast_buffer.Put("}\n");// for namespace
    }
}

//
//
//
void TypeScriptAction::GenerateResultVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                const char *indentation,
                                                const char *interface_name,
                                                SymbolLookupTable &type_set)
{
    TextBuffer& ast_buffer =*GetBuffer(ast_filename_symbol);
    ast_buffer.Put(indentation); ast_buffer.Put("export interface ");
                                 ast_buffer.Put(interface_name);
                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        ast_buffer.Put(indentation); ast_buffer.Put("    visit"); ast_buffer.Put(symbol->Name());
                                     ast_buffer.Put("(n : ");
                                     ast_buffer.Put(symbol -> Name());
                                     ast_buffer.Put(") : any;\n");
    }

                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    visit");
                                 ast_buffer.Put("(n : ");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(") : any;\n");

    ast_buffer.Put(indentation); ast_buffer.Put("}\n");
    if (option->IsTopLevel() && option->IsPackage())
    {
        ast_buffer.Put(indentation); ast_buffer.Put("}\n");// for namespace
    }
}

//
//
//
void TypeScriptAction::GenerateResultArgumentVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                        const char *indentation,
                                                        const char *interface_name,
                                                        SymbolLookupTable &type_set)
{
    TextBuffer& ast_buffer =*GetBuffer(ast_filename_symbol);
    ast_buffer.Put(indentation); ast_buffer.Put("export interface ");
    ast_buffer.Put(interface_name);
    ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        ast_buffer.Put(indentation); ast_buffer.Put("    visit"); ast_buffer.Put(symbol->Name());
                                     ast_buffer.Put("(n : ");
                                     ast_buffer.Put(symbol -> Name());
                                     ast_buffer.Put(", o : any) : any;\n");
    }

                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    visit");
                                 ast_buffer.Put("(n : ");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(", o : any) : any;\n");

    ast_buffer.Put(indentation); ast_buffer.Put("}\n");
    if (option->IsTopLevel() && option->IsPackage())
    {
        ast_buffer.Put(indentation); ast_buffer.Put("}\n");// for namespace
    }
}


//
//
//
void TypeScriptAction::GeneratePreorderVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                  const char *indentation,
                                                  const char *interface_name,
                                                  SymbolLookupTable &type_set)
{
    TextBuffer& ast_buffer =*GetBuffer(ast_filename_symbol);
    assert(option -> visitor == Option::PREORDER);
    ast_buffer.Put(indentation); ast_buffer.Put("export interface ");
                                 ast_buffer.Put(interface_name);
                                 ast_buffer.Put(" implements IAstVisitor\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");


    ast_buffer.Put(indentation); ast_buffer.Put("    visit");
                                 ast_buffer.Put("(n : ");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(") : boolean;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    endVisit");
                                 ast_buffer.Put("(n : ");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(") : void;\n\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        ast_buffer.Put(indentation); ast_buffer.Put("    visit"); ast_buffer.Put(symbol->Name());
                                     ast_buffer.Put("(n : ");
                                     ast_buffer.Put(symbol -> Name());
                                     ast_buffer.Put(") : boolean;\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    endVisit"); ast_buffer.Put(symbol->Name());
                                     ast_buffer.Put("(n : ");
                                     ast_buffer.Put(symbol -> Name());
                                     ast_buffer.Put(") : void;\n");
        ast_buffer.Put("\n");
    }

    ast_buffer.Put(indentation); ast_buffer.Put("}\n\n");
    if (option->IsTopLevel() && option->IsPackage())
    {
        ast_buffer.Put(indentation); ast_buffer.Put("}\n");// for namespace
    }
    return;
}


//
//
//
void TypeScriptAction::GenerateNoResultVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                      const char *indentation,
                                                      const char *classname,
                                                      SymbolLookupTable &type_set)
{
    TextBuffer& ast_buffer =*GetBuffer(ast_filename_symbol);
    ast_buffer.Put(indentation); 
                                 ast_buffer.Put("export abstract class ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put(" implements ");
                                 ast_buffer.Put(option -> visitor_type);
                                 ast_buffer.Put(", Argument");
                                 ast_buffer.Put(option -> visitor_type);
                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    public abstract  unimplementedVisitor(s : string) : void;\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];

            ast_buffer.Put(indentation); ast_buffer.Put("    public  visit"); ast_buffer.Put(symbol->Name());
                                         ast_buffer.Put("(n : ");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(", o? : any) : void { this.unimplementedVisitor(\"visit"); ast_buffer.Put(symbol->Name()); ast_buffer.Put("(");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(", any)\"); }\n");
            ast_buffer.Put("\n");
        }
    }

                                 ast_buffer.Put("\n");

   

    ast_buffer.Put(indentation); ast_buffer.Put("    public  visit");
                                 ast_buffer.Put("(n : ");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(", o? : any) : void\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            ast_buffer.Put(indentation); ast_buffer.Put("        ");
                                         ast_buffer.Put(i == 0 ? "" : "else ");
                                         ast_buffer.Put("if (n instanceof ");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(") this.visit");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put("(<");
                                         ast_buffer.Put(symbol->Name());
                                         ast_buffer.Put("> n, o);\n");
        }
    }
    ast_buffer.Put(indentation); ast_buffer.Put("        throw new Error(\"visit(\" + n.toString() + \")\");\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n");

    ast_buffer.Put(indentation); ast_buffer.Put("}\n");
    if (option->IsTopLevel() && option->IsPackage())
    {
        ast_buffer.Put(indentation); ast_buffer.Put("}\n");// for namespace
    }
}

//
//
//
void TypeScriptAction::GenerateResultVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                    const char *indentation,
                                                    const char *classname,
                                                    SymbolLookupTable &type_set)
{
    TextBuffer& ast_buffer =*GetBuffer(ast_filename_symbol);
    ast_buffer.Put(indentation); 
                                 ast_buffer.Put("export abstract class ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put(" implements Result");
                                 ast_buffer.Put(option -> visitor_type);
                                 ast_buffer.Put(", ResultArgument");
                                 ast_buffer.Put(option -> visitor_type);
                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    public abstract  unimplementedVisitor(s : string) : any;\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
       
            ast_buffer.Put(indentation); ast_buffer.Put("    public visit"); ast_buffer.Put(symbol->Name()); ast_buffer.Put("(n : ");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(", o? : any) : any{ return  this.unimplementedVisitor(\"visit"); ast_buffer.Put(symbol->Name()); ast_buffer.Put("(");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(", any)\"); }\n");
            ast_buffer.Put("\n");
        }
    }

                                 ast_buffer.Put("\n");



    ast_buffer.Put(indentation); ast_buffer.Put("    public visit");
                                 ast_buffer.Put("(n : ");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(", o? : any) : any\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            ast_buffer.Put(indentation); ast_buffer.Put("        ");
                                         ast_buffer.Put(i == 0 ? "" : "else ");
                                         ast_buffer.Put("if (n instanceof ");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(") return this.visit");
                                         ast_buffer.Put(symbol->Name());
                                         ast_buffer.Put("(<");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put("> n, o);\n");
        }
    }
    ast_buffer.Put(indentation); ast_buffer.Put("        throw new Error(\"visit(\" + n.toString() + \")\");\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n");

    ast_buffer.Put(indentation); ast_buffer.Put("}\n");
    if (option->IsTopLevel() && option->IsPackage())
    {
        ast_buffer.Put(indentation); ast_buffer.Put("}\n");// for namespace
    }
}


//
//
//
void TypeScriptAction::GeneratePreorderVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                      const char *indentation,
                                                      const char *classname,
                                                      SymbolLookupTable &type_set)
{
    TextBuffer& ast_buffer =*GetBuffer(ast_filename_symbol);
    assert(option -> visitor == Option::PREORDER);
    ast_buffer.Put(indentation); 
                                 ast_buffer.Put("export abstract class ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put(" implements ");
                                 ast_buffer.Put(option -> visitor_type);
                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    public abstract unimplementedVisitor(s : string) : void;\n\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    public preVisit(element : IAst) : boolean{ return true; }\n\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    public postVisit(element : IAst) : void{}\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            ast_buffer.Put(indentation); ast_buffer.Put("    public visit"); ast_buffer.Put(symbol->Name());
                                         ast_buffer.Put("(n : ");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(") : boolean{ this.unimplementedVisitor(\"visit(");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(")\"); return true; }\n");
            ast_buffer.Put(indentation); ast_buffer.Put("    public endVisit"); ast_buffer.Put(symbol->Name());
                                         ast_buffer.Put("(n : ");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(") : void{ this.unimplementedVisitor(\"endVisit(");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(")\"); }\n");
            ast_buffer.Put("\n");
        }
    }

                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    public visit");
                                 ast_buffer.Put("(n : ");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(") : boolean\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            ast_buffer.Put(indentation); ast_buffer.Put("        ");
                                         ast_buffer.Put(i == 0 ? "" : "else ");
                                         ast_buffer.Put("if (n instanceof ");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(") return this.visit");
                                         ast_buffer.Put(symbol->Name());
                                         ast_buffer.Put("(<");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put("> n);\n");
        }
    }
    ast_buffer.Put(indentation); ast_buffer.Put("        throw new Error(\"visit(\" + n.toString() + \")\");\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n");

    ast_buffer.Put(indentation); ast_buffer.Put("    public endVisit");
                                 ast_buffer.Put("(n : ");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(") : void\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            ast_buffer.Put(indentation); ast_buffer.Put("        ");
                                         ast_buffer.Put(i == 0 ? "" : "else ");
                                         ast_buffer.Put("if (n instanceof ");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(") this.endVisit");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put("(<");
                                         ast_buffer.Put(symbol->Name());
                                         ast_buffer.Put("> n);\n");
        }
    }
    ast_buffer.Put(indentation); ast_buffer.Put("        throw new Error(\"visit(\" + n.toString() + \")\");\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n");

    ast_buffer.Put(indentation); ast_buffer.Put("}\n");
    if (option->IsTopLevel() && option->IsPackage())
    {
        ast_buffer.Put(indentation); ast_buffer.Put("}\n");// for namespace
    }
    return;
}


//
// Generate the the Ast root classes
//
void TypeScriptAction::GenerateAstType(ActionFileSymbol* ast_filename_symbol,
                                 const char *indentation,
                                 const char *classname)
{
    TextBuffer& ast_buffer =*GetBuffer(ast_filename_symbol);
    /*
     * First, generate the main root class
     */
    ast_buffer.Put(indentation); 
                                 ast_buffer.Put("export abstract class ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put(" implements IAst\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");
    if (option -> glr)
    {
        ast_buffer.Put(indentation); ast_buffer.Put("    private nextAst? : Ast ;\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    public getNextAst() : IAst  | undefined{ return this.nextAst; }\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    public setNextAst(n : IAst) : void{ this.nextAst = n; }\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    public resetNextAst() : void { this.nextAst = undefined; }\n");
    }
    else ast_buffer.Put(indentation); ast_buffer.Put("    public getNextAst() : IAst | undefined { return undefined; }\n");

    ast_buffer.Put(indentation); ast_buffer.Put("    protected leftIToken : IToken ;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    protected rightIToken: IToken ;\n");
    if (option -> parent_saved)
    {
        ast_buffer.Put(indentation); ast_buffer.Put("    protected parent? : IAst ;\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    public  setParent(parent : IAst ) : void { this.parent = parent; }\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    public  getParent() : IAst | undefined{ return this.parent; }\n");\
    }
    else
    {
        ast_buffer.Put(indentation); ast_buffer.Put("    public getParent(): IAst | undefined \n");
        ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        throw new Error(\"noparent-saved option in effect\");\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
    }

    ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    public getLeftIToken() : IToken { return this.leftIToken; }\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    public getRightIToken() : IToken { return this.rightIToken; }\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    public getPrecedingAdjuncts() : IToken[] { return this.leftIToken.getPrecedingAdjuncts(); }\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    public getFollowingAdjuncts() : IToken[] { return this.rightIToken.getFollowingAdjuncts(); }\n\n");

    ast_buffer.Put(indentation); ast_buffer.Put("    public  toString() : string \n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        let str = this.leftIToken.getILexStream()?.toString(this.leftIToken.getStartOffset(), this.rightIToken.getEndOffset());\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        return str? str : \"\";\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n\n");


    ast_buffer.Put(indentation);ast_buffer.Put("constructor(leftIToken : IToken , rightIToken? : IToken )\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        this.leftIToken = leftIToken;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        if(rightIToken) this.rightIToken = rightIToken;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        else            this.rightIToken = leftIToken;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n\n");
    ast_buffer.Put(indentation); ast_buffer.Put("  public   initialize() : void {}\n");
    for (int i = 0; i < grammar -> parser.ast_blocks.Length(); i++)
    {
        LexStream::TokenIndex block_token = grammar -> parser.ast_blocks[i];
        BlockSymbol *block = lex_stream -> GetBlockSymbol(block_token);
        if (! option -> ActionBlocks().IsIgnoredBlock(block -> BlockBegin(), block -> BlockBeginLength()))
        {
            ActionBlockElement action;
            action.rule_number = 0;
            action.location = ActionBlockElement::INITIALIZE; // does not matter - block must be default block...
            action.block_token = block_token;
            action.buffer = &ast_buffer;
            ProcessActionBlock(action);
        }
    }

    ast_buffer.Put("\n");
    if (option -> parent_saved)
    {
        ast_buffer.Put(indentation); ast_buffer.Put("    /**\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     * A list of all children of this node, excluding the undefined ones.\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     */\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    public  getChildren() : Lpg.Util.ArrayList<IAst>\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("         let list = this.getAllChildren() ;\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        let k = -1;\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        for (let i = 0; i < list.size(); i++)\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("            let element = list.get(i);\n");
        ast_buffer.Put(indentation); ast_buffer.Put("            if (element != undefined)\n");
        ast_buffer.Put(indentation); ast_buffer.Put("            {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("                if (++k != i)\n");
        ast_buffer.Put(indentation); ast_buffer.Put("                    list.set(k, element);\n");
        ast_buffer.Put(indentation); ast_buffer.Put("            }\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        }\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        for (let i = list.size() - 1; i > k; i--) // remove extraneous elements\n");
        ast_buffer.Put(indentation); ast_buffer.Put("            list.remove(i);\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        return list;\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    }\n\n");

        ast_buffer.Put(indentation); ast_buffer.Put("    /**\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     * A list of all children of this node, including the undefined ones.\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     */\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    public abstract  getAllChildren() : Lpg.Util.ArrayList<IAst>;\n");
    }
    else
    {
        ast_buffer.Put(indentation); ast_buffer.Put("    public  getChildren() : Lpg.Util.ArrayList<IAst>\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        throw new Error(\"noparent-saved option in effect\");\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     public   getAllChildren() : Lpg.Util.ArrayList<IAst> { return this.getChildren(); }\n");
    }

    ast_buffer.Put("\n");

    GenerateVisitorHeaders(ast_buffer, indentation, "    public abstract ");

    //
    // Not Preorder visitor? generate dummy accept method to satisfy IAst abstract declaration of accept(IAstVisitor);
    // TODO: Should IAstVisitor be used for default visitors also? If (when) yes then we should remove it from the test below
    //
    if (option -> visitor == Option::NONE || option -> visitor == Option::DEFAULT) // ??? Don't need this for DEFAULT case after upgrade
    {
        ast_buffer.Put(indentation); ast_buffer.Put("    public  accept(v : IAstVisitor ) : void {}\n");
    }
    ast_buffer.Put(indentation); ast_buffer.Put("}\n\n");
    if (option->IsTopLevel() && option->IsPackage())
    {
        ast_buffer.Put(indentation); ast_buffer.Put("}\n");// for namespace
    }
    return;
}



typedef std::map<std::string, std::string> Substitutions;



//
// Generate the the Ast list class
//
void TypeScriptAction::GenerateAbstractAstListType(ActionFileSymbol* ast_filename_symbol,
                                             const char *indentation,
                                             const char *classname)
{
    TextBuffer& ast_buffer =*GetBuffer(ast_filename_symbol);
    /*
     * Generate the List root class
     */
    ast_buffer.Put(indentation); 
                                 ast_buffer.Put("export abstract class ");
                                 ast_buffer.Put(this -> abstract_ast_list_classname);
                                 ast_buffer.Put(" extends ");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(" implements IAbstractArrayList<");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(">\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    private leftRecursive : boolean ;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    public "); 
	ast_buffer.Put(" list  = new Lpg.Util.ArrayList<"); ast_buffer.Put(option->ast_type); ast_buffer.Put(">();\n");

    ast_buffer.Put(indentation); ast_buffer.Put("    public  size() : number { return this.list.size(); }\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    public   getList() : Lpg.Util.ArrayList<"); ast_buffer.Put(option->ast_type); ast_buffer.Put(indentation); ast_buffer.Put("> { return this.list; }\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    public ");
                              
                                 ast_buffer.Put(" getElementAt(i : number ) : ");
                                 ast_buffer.Put(option->ast_type);
                                 ast_buffer.Put(" { return <");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put("> this.list.get(this.leftRecursive ? i : this.list.size() - 1 - i); }\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    public  getArrayList() : Lpg.Util.ArrayList<"); ast_buffer.Put(option->ast_type); ast_buffer.Put(">\n");
   
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        if (! this.leftRecursive) // reverse the list \n");
    ast_buffer.Put(indentation); ast_buffer.Put("        {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("            for (let i = 0, n = this.list.size() - 1; i < n; i++, n--)\n");
    ast_buffer.Put(indentation); ast_buffer.Put("            {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("                let ith = this.list.get(i),\n");
    ast_buffer.Put(indentation); ast_buffer.Put("                       nth = this.list.get(n);\n");
    ast_buffer.Put(indentation); ast_buffer.Put("                this.list.set(i, nth);\n");
    ast_buffer.Put(indentation); ast_buffer.Put("                this.list.set(n, ith);\n");
    ast_buffer.Put(indentation); ast_buffer.Put("            }\n");
    ast_buffer.Put(indentation); ast_buffer.Put("            this.leftRecursive = true;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        }\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        return this.list;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n");

    ast_buffer.Put(indentation); ast_buffer.Put("    /**\n");
    ast_buffer.Put(indentation); ast_buffer.Put("     * @deprecated replaced by {@link #addElement()}\n");
    ast_buffer.Put(indentation); ast_buffer.Put("     *\n");
    ast_buffer.Put(indentation); ast_buffer.Put("     */\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    public  add(element : ");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(") : boolean\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        this.addElement(element);\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        return true;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n\n");

    ast_buffer.Put(indentation); ast_buffer.Put("    public  addElement(element : ");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(") : void\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        this.list.add(element);\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        if (this.leftRecursive)\n");
    ast_buffer.Put(indentation); ast_buffer.Put("             this.rightIToken = element.getRightIToken();\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        else this.leftIToken = element.getLeftIToken();\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n\n");





    // generate constructors for list class
    ast_buffer.Put(indentation); ast_buffer.Put("    constructor(leftToken : IToken, rightToken : IToken , leftRecursive : boolean )");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("          super(leftToken, rightToken);\n");
    ast_buffer.Put(indentation); ast_buffer.Put("          this.leftRecursive = leftRecursive;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n\n");

  
    if (option -> parent_saved)
    {
        ast_buffer.Put(indentation); ast_buffer.Put("    /**\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     * Make a copy of the list and return it. Note that we obtain the local list by\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     * invoking getArrayList so as to make sure that the list we return is in proper order.\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     */\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     public   getAllChildren() : Lpg.Util.ArrayList<IAst>\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        return this.getArrayList().clone();\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    }\n\n");
    }

    //
    // Implementation for functions in Lpg.Util.ArrayList<IAst>
    //

    Substitutions subs;
    subs["%%AstType%%"] = option->ast_type;
    subs["%%ListClassName%%"] = classname;
    ast_buffer.Put(indentation); ast_buffer.Put("}\n\n");

    if (option->IsTopLevel() && option->IsPackage())
    {
        ast_buffer.Put(indentation); ast_buffer.Put("}\n");// for namespace
    }
  

    return;
}


//
// Generate the the Ast token class
//
void TypeScriptAction::GenerateAstTokenType(NTC &ntc, ActionFileSymbol* ast_filename_symbol,
                                      const char *indentation,
                                      const char *classname)
{
    TextBuffer& ast_buffer =*GetBuffer(ast_filename_symbol);
    /*
     * Generate the Token root class
     */
    ast_buffer.Put(indentation); 
                                 ast_buffer.Put("export class ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put(" extends ");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(" implements I");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");

    ast_buffer.Put(indentation); ast_buffer.Put("    "); ast_buffer.Put("constructor(token : IToken ) { super(token); }\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    public  getIToken() : IToken{ return this.leftIToken; }\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    public  toString() : string  { return this.leftIToken.toString(); }\n\n");

    ClassnameElement element; // generate a temporary element with no symbols in its symbol set.
    element.real_name = (char *) classname;
    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);

    if (option -> parent_saved)
    {
        ast_buffer.Put(indentation); ast_buffer.Put("    /**\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     * A token class has no children. So, we return the empty list.\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     */\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     public   getAllChildren() : Lpg.Util.ArrayList<IAst> { return new Lpg.Util.ArrayList<IAst>(); }\n\n");
    }

    GenerateVisitorMethods(ntc, ast_buffer, indentation, element, optimizable_symbol_set);

    ast_buffer.Put(indentation); ast_buffer.Put("}\n\n");
    if (option->IsTopLevel() && option->IsPackage())
    {
        ast_buffer.Put(indentation); ast_buffer.Put("}\n");// for namespace
    }
    return;
}


//
//
//
void TypeScriptAction::GenerateCommentHeader(TextBuffer &ast_buffer,
                                       const char *indentation,
                                       Tuple<int> &ungenerated_rule,
                                       Tuple<int> &generated_rule)
{
    BlockSymbol* scope_block = nullptr;
    const char* rule_info = rule_info_holder.c_str();

    ast_buffer.Put(indentation); ast_buffer.Put("/**");
    if (ungenerated_rule.Length() > 0)
    {
        ast_buffer.Put("\n");
        ast_buffer.Put(indentation);
        ast_buffer.Put(" *<em>");
        for (int i = 0; i < ungenerated_rule.Length(); i++)
        {
            int rule_no = ungenerated_rule[i];

            LexStream::TokenIndex separator_token = grammar -> parser.rules[grammar -> rules[rule_no].source_index].separator_index;
            int line_no = lex_stream -> Line(separator_token),
                start = lex_stream -> StartLocation(separator_token),
                end   = lex_stream -> EndLocation(separator_token) + 1;
            const char *start_cursor_location = &(lex_stream -> InputBuffer(separator_token)[start]),
                       *end_cursor_location = &(lex_stream -> InputBuffer(separator_token)[end]);

            ast_buffer.Put("\n");
            ast_buffer.Put(indentation);
            ProcessActionLine(scope_block, ActionBlockElement::BODY,
                              &ast_buffer,
                              lex_stream -> FileName(separator_token),
                              rule_info,
                              &rule_info[strlen(rule_info)],
                              rule_no,
                              lex_stream -> FileName(separator_token),
                              line_no,
                              start_cursor_location,
                              end_cursor_location);
        }
        ast_buffer.Put("\n");
        ast_buffer.Put(indentation);
        ast_buffer.Put(" *</em>\n");
        ast_buffer.Put(indentation);
        ast_buffer.Put(" *<p>");
    }
    ast_buffer.Put("\n");
    ast_buffer.Put(indentation);
    ast_buffer.Put(" *<b>");
    for (int i = 0; i < generated_rule.Length(); i++)
    {
        int rule_no = generated_rule[i];

            LexStream::TokenIndex separator_token = grammar -> parser.rules[grammar -> rules[rule_no].source_index].separator_index;
            int line_no = lex_stream -> Line(separator_token),
                start = lex_stream -> StartLocation(separator_token),
                end   = lex_stream -> EndLocation(separator_token) + 1;
            const char *start_cursor_location = &(lex_stream -> InputBuffer(separator_token)[start]),
                       *end_cursor_location = &(lex_stream -> InputBuffer(separator_token)[end]);

        ast_buffer.Put("\n");
        ast_buffer.Put(indentation);
        ProcessActionLine(scope_block, ActionBlockElement::BODY,
                          &ast_buffer,
                          lex_stream -> FileName(separator_token), // option -> DefaultBlock() -> ActionfileSymbol() -> Name(),
                          rule_info,
                          &rule_info[strlen(rule_info)],
                          rule_no,
                          lex_stream -> FileName(separator_token),
                          line_no,
                          start_cursor_location,
                          end_cursor_location);
    }

    ast_buffer.Put("\n");
    ast_buffer.Put(indentation);
    ast_buffer.Put(" *</b>\n");
    ast_buffer.Put(indentation);
    ast_buffer.Put(" */\n");
}


void TypeScriptAction::GenerateListMethods(CTC &ctc,
                                     NTC &ntc,
                                     TextBuffer &ast_buffer,
                                     const char *indentation,
                                     const char *classname,
                                     ClassnameElement &element,
                                     Array<const char *> &typestring)
{
    const char *element_name = element.array_element_type_symbol -> Name(),
               *element_type = ctc.FindBestTypeFor(element.array_element_type_symbol -> SymbolIndex());

    //
    // Generate ADD method
    //
    ast_buffer.Put(indentation); ast_buffer.Put("    public ");
                                 ast_buffer.Put(" addElement(");
                                 ast_buffer.Put(" _");
                                 ast_buffer.Put(element_name);
                                 ast_buffer.Put(" : ");
                                 ast_buffer.Put(element_type);
                                 ast_buffer.Put(") : void\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        super.addElement(<");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put("> _");
                                 ast_buffer.Put(element_name);
                                 ast_buffer.Put(");\n");
    if (option -> parent_saved)
    {
        ast_buffer.Put(indentation); ast_buffer.Put("        ");
        if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
        {
            ast_buffer.Put("if (this._");
            ast_buffer.Put(element_name);
            ast_buffer.Put(" != undefined) ");
        }
        ast_buffer.Put("(<");
        ast_buffer.Put(option -> ast_type);
        ast_buffer.Put("> _");
        ast_buffer.Put(element_name);
        ast_buffer.Put(").setParent(this);\n");
    }
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n");

    ast_buffer.Put("\n");
   
    //
    // Generate visitor methods.
    //
    if (option -> visitor == Option::DEFAULT)
    {
        ast_buffer.Put("\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    public  acceptWithVisitor(v : ");
                                     ast_buffer.Put(option -> visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
            ast_buffer.Put(") :void { for (let i = 0; i < this.size(); i++) v.visit"
                           "("
                           "this.get");
            ast_buffer.Put(element_name);
            ast_buffer.Put("At(i)"
                           "); }\n");
        }
        else
        {
            ast_buffer.Put(") : void{ for (let i = 0; i < this.size(); i++) this.get");
            ast_buffer.Put(element_name);
            ast_buffer.Put("At(i).acceptWithVisitor(v); }\n");
        }

        ast_buffer.Put(indentation); ast_buffer.Put("    public  acceptWithArg(v : Argument");
                                     ast_buffer.Put(option -> visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
            ast_buffer.Put(", o : any)  : void{ for (let i = 0; i < this.size(); i++) v.visit"
                           "("
                           "this.get");
            ast_buffer.Put(element_name);
            ast_buffer.Put("At(i), o");
            ast_buffer.Put("); }\n");
        }
        else
        {
            ast_buffer.Put(", o : any) : void { for (let i = 0; i < this.size(); i++) this.get");
            ast_buffer.Put(element_name);
            ast_buffer.Put("At(i).acceptWithArg(v, o); }\n");
        }

        //
        // Code cannot be generated to automatically visit a node that
        // can return a value. These cases are left up to the user.
        //
        ast_buffer.Put(indentation); ast_buffer.Put("    public  acceptWithResult(v : Result");
                                     ast_buffer.Put(option -> visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
                                         ast_buffer.Put(") : any\n");
            ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        let result = new Lpg.Util.ArrayList<IAst>();\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        for (let i = 0; i < this.size(); i++)\n");
            ast_buffer.Put(indentation); ast_buffer.Put("            result.add(v.visit(this.get");
                                         ast_buffer.Put(element_name);
                                         ast_buffer.Put("At(i)));\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        return result;\n");
            ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
        }
        else
        {
                                         ast_buffer.Put(") : any\n");
            ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        let result = new Lpg.Util.ArrayList<IAst>();\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        for (let i = 0; i < this.size(); i++)\n");
            ast_buffer.Put(indentation); ast_buffer.Put("            result.add(this.get");
                                         ast_buffer.Put(element_name);
                                         ast_buffer.Put("At(i).acceptWithResult(v));\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        return result;\n");
            ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
        }

        ast_buffer.Put(indentation); ast_buffer.Put("    public  acceptWthResultArgument(v: ResultArgument");
                                     ast_buffer.Put(option -> visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
                                         ast_buffer.Put(", o : any) : any\n");
            ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        let result = new Lpg.Util.ArrayList<IAst>();\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        for (let i = 0; i < this.size(); i++)\n");
            ast_buffer.Put(indentation); ast_buffer.Put("            result.add(v.visit(this.get");
                                         ast_buffer.Put(element_name);
                                         ast_buffer.Put("At(i), o));\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        return result;\n");
            ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
        }
        else
        {
                                         ast_buffer.Put(", o : any) : any \n");
            ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        let result = new Lpg.Util.ArrayList<IAst>();\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        for (let i = 0; i < this.size(); i++)\n");
            ast_buffer.Put(indentation); ast_buffer.Put("            result.add(this.get");
                                         ast_buffer.Put(element_name);
                                         ast_buffer.Put("At(i).acceptWthResultArgument(v, o));\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        return result;\n");
            ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
        }
    }
    else if (option -> visitor == Option::PREORDER)
    {
        ast_buffer.Put("\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    public  accept(v : IAstVisitor ) : void\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        if (! v.preVisit(this)) return;\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        this.enter(<");
                                     ast_buffer.Put(option -> visitor_type);
                                     ast_buffer.Put("> v);\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        v.postVisit(this);\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    public enter(v : ");
                                     ast_buffer.Put(option -> visitor_type);
                                     ast_buffer.Put(") : void\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        let checkChildren = v.visit(this);\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        if (checkChildren)\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("            for (let i = 0; i < this.size(); i++)\n");
        ast_buffer.Put(indentation); ast_buffer.Put("            {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("                ");

        const char *element_typename = ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex());
        if (element_typename != NULL)
        {
            //ast_buffer.Put(element_typename);
            ast_buffer.Put(" let element = this.get");
            ast_buffer.Put(element_name);
            ast_buffer.Put("At(i);\n");
            if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
            {
                ast_buffer.Put(indentation); ast_buffer.Put("                if (element != undefined)");
                ast_buffer.Put(indentation); ast_buffer.Put("                {\n");
                ast_buffer.Put(indentation); ast_buffer.Put("                    if (! v.preVisit(element)) continue;\n");
                ast_buffer.Put(indentation); ast_buffer.Put("                    element.enter(v);\n");
                ast_buffer.Put(indentation); ast_buffer.Put("                    v.postVisit(element);\n");
                ast_buffer.Put(indentation); ast_buffer.Put("                }\n");
            }
            else
            {
                ast_buffer.Put(indentation); ast_buffer.Put("                if (! v.preVisit(element)) continue;\n");
                ast_buffer.Put(indentation); ast_buffer.Put("                element.enter(v);\n");
                ast_buffer.Put(indentation); ast_buffer.Put("                v.postVisit(element);\n");
            }
        }
        else
        {
            //ast_buffer.Put(typestring[element.array_element_type_symbol -> SymbolIndex()]);
            ast_buffer.Put("let element = this.get");
            ast_buffer.Put(element_name);
            ast_buffer.Put("At(i);\n");
            ast_buffer.Put(indentation); ast_buffer.Put("                ");
            if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
                ast_buffer.Put("if (element != undefined) ");
            ast_buffer.Put("element.accept(v);\n");
        }
        ast_buffer.Put(indentation); ast_buffer.Put("            }\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        }\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        v.endVisit(this);\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
    }

    return;
}


//
//
//
void TypeScriptAction::GenerateListClass(CTC &ctc,
                                   NTC &ntc,
                                   ActionFileSymbol* ast_filename_symbol,
                                   const char *indentation,
                                   ClassnameElement &element,
                                   Array<const char *> &typestring)
{
    TextBuffer& ast_buffer =*GetBuffer(ast_filename_symbol);
    Tuple<int> &interface = element.interface_;
    assert(element.array_element_type_symbol != NULL);
    const char *classname = element.real_name,
               *element_name = element.array_element_type_symbol -> Name(),
               *element_type = ctc.FindBestTypeFor(element.array_element_type_symbol -> SymbolIndex());

    GenerateCommentHeader(ast_buffer, indentation, element.ungenerated_rule, element.rule);

    ast_buffer.Put(indentation); 
                                 ast_buffer.Put("export class ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put(" extends ");
                                 ast_buffer.Put(this -> abstract_ast_list_classname);
                                 ast_buffer.Put(" implements ");
    for (int i = 0; i < interface.Length() - 1; i++)
    {
        ast_buffer.Put(typestring[element.interface_[i]]);
        ast_buffer.Put(", ");
    }
    ast_buffer.Put(typestring[element.interface_[interface.Length() - 1]]);
    ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");

    if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
    {
        ast_buffer.Put(indentation); ast_buffer.Put("    /**\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     * The value returned by <b>get");
                                     ast_buffer.Put(element_name);
                                     ast_buffer.Put("At</b> may be <b>undefined</b>\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     */\n");
    }
    ast_buffer.Put(indentation); ast_buffer.Put("    public ");
                                 ast_buffer.Put(" get");
                                 ast_buffer.Put(element_name);
                                 ast_buffer.Put("At(i : number) : ");
                                 ast_buffer.Put(element_type);
                                 ast_buffer.Put("{ return <");
                                 ast_buffer.Put(element_type);
                                 ast_buffer.Put("> this.getElementAt(i); }\n\n");

    //
    // generate constructors
    //
	ast_buffer.Put(indentation); ast_buffer.Put("    constructor(leftToken : IToken, rightToken : IToken , leftRecursive : boolean )\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        super(leftToken, rightToken, leftRecursive);\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n\n");

    ast_buffer.Put(indentation); ast_buffer.Put("   public static ");ast_buffer.Put(classname); ast_buffer.Put("fromElement(");
    ast_buffer.Put("element");
    ast_buffer.Put(" : ");
    ast_buffer.Put(element_type);
    ast_buffer.Put(",leftRecursive : boolean) : ");  ast_buffer.Put(classname); ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        let obj = new ");ast_buffer.Put(classname);
	ast_buffer.Put("(element.getLeftIToken(),element.getRightIToken(), leftRecursive);\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        obj.list.add(element);\n");
    if (option->parent_saved)
    {
        ast_buffer.Put(indentation); ast_buffer.Put("        ");
        if (ntc.CanProduceNullAst(element.array_element_type_symbol->SymbolIndex()))
        {
            ast_buffer.Put("if (element)");
        }
        ast_buffer.Put("(<");
        ast_buffer.Put(option->ast_type);
        ast_buffer.Put("> element");
        ast_buffer.Put(").setParent(obj);\n");
    }
    ast_buffer.Put(indentation); ast_buffer.Put("        return obj;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
    ast_buffer.Put("\n");



    GenerateListMethods(ctc, ntc, ast_buffer, indentation, classname, element, typestring);

   ast_buffer.Put("    }\n\n");// Generate Class Closer
    if (option->IsTopLevel() && option->IsPackage())
    {
        ast_buffer.Put(indentation); ast_buffer.Put("}\n");// for namespace
    }

    if (option->IsTopLevel())
    {
        ast_filename_symbol->Flush();
    }

}


//
// Generate a class that extends a basic list class. This is necessary when the user
// specifies action blocks to be associated with a generic list class - in which case,
// we have to generate a (new) unique class (that extends the generic class) to hold the content
// of the action blocks.
//
void TypeScriptAction::GenerateListExtensionClass(CTC& ctc,
    NTC& ntc,
    ActionFileSymbol* ast_filename_symbol,
    const char* indentation,
    SpecialArrayElement& special_array,
    ClassnameElement& element,
    Array<const char*>& typestring)

{
    TextBuffer& ast_buffer = *GetBuffer(ast_filename_symbol);
    const char* classname = element.real_name,
        * element_name = element.array_element_type_symbol->Name(),
        * element_type = ctc.FindBestTypeFor(element.array_element_type_symbol->SymbolIndex());

    GenerateCommentHeader(ast_buffer, indentation, element.ungenerated_rule, special_array.rules);

    ast_buffer.Put(indentation); 
    ast_buffer.Put("export class ");
    ast_buffer.Put(special_array.name);
    ast_buffer.Put(" extends ");
    ast_buffer.Put(classname);
    ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");

    GenerateEnvironmentDeclaration(ast_buffer, indentation);


    ast_buffer.Put(indentation); ast_buffer.Put("    constructor(");
    ast_buffer.Put("environment : "); ast_buffer.Put(option->action_type);
    ast_buffer.Put(", leftIToken : IToken,  rightIToken : IToken, leftRecursive : boolean)\n");

    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        super(leftIToken, rightIToken, leftRecursive);\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        this.environment = environment;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        this.initialize();\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n\n");


    ast_buffer.Put(indentation); ast_buffer.Put("   public static  "); ast_buffer.Put(special_array.name); ast_buffer.Put("fromElement(environment : ");
    ast_buffer.Put(option->action_type);
    ast_buffer.Put(",element");
    ast_buffer.Put(" : ");
    ast_buffer.Put(element_type);
    ast_buffer.Put(",leftRecursive : boolean) : ");  ast_buffer.Put(special_array.name); ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        let obj = new "); ast_buffer.Put(special_array.name);
    ast_buffer.Put("(environment,element.getLeftIToken(),element.getRightIToken(), leftRecursive);\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        obj.list.add(element);\n");
    if (option->parent_saved)
    {
        ast_buffer.Put(indentation); ast_buffer.Put("        ");
        if (ntc.CanProduceNullAst(element.array_element_type_symbol->SymbolIndex()))
        {
            ast_buffer.Put("if (element)");
        }
        ast_buffer.Put("(<");
        ast_buffer.Put(option->ast_type);
        ast_buffer.Put("> element");
        ast_buffer.Put(").setParent(obj);\n");
    }
    ast_buffer.Put(indentation); ast_buffer.Put("        return obj;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
    ast_buffer.Put("\n");



    //GenerateListMethods(ctc, ntc, ast_buffer, indentation, classname, element, typestring);

    ast_buffer.Put("    }\n\n");// Generate Class Closer
    if (option->IsTopLevel() && option->IsPackage())
    {
        ast_buffer.Put(indentation); ast_buffer.Put("}\n");// for namespace
    }

    if (option->IsTopLevel())
    {
        ast_filename_symbol->Flush();
    }

}


//
// Generate a generic rule class
//
void TypeScriptAction::GenerateRuleClass(CTC &ctc,
                                   NTC &ntc,
                                   ActionFileSymbol* ast_filename_symbol,
                                   const char *indentation,
                                   ClassnameElement &element,
                                   Array<const char *> &typestring)
{
    TextBuffer& ast_buffer =*GetBuffer(ast_filename_symbol);
    char *classname = element.real_name;
    SymbolLookupTable &symbol_set = element.symbol_set;
    Tuple<int> &rhs_type_index = element.rhs_type_index;

    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);

    GenerateCommentHeader(ast_buffer, indentation, element.ungenerated_rule, element.rule);

    assert(element.rule.Length() == 1);
    int rule_no = element.rule[0];

    ast_buffer.Put(indentation); 
                                 ast_buffer.Put("export class ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put(" extends ");
    if (element.is_terminal_class)
    {
        ast_buffer.Put(grammar -> Get_ast_token_classname());
        ast_buffer.Put(" implements ");
        ast_buffer.Put(typestring[grammar -> rules[rule_no].lhs]);
        ast_buffer.Put("\n");
        ast_buffer.Put(indentation); ast_buffer.Put("{\n");
        if (element.needs_environment)
            GenerateEnvironmentDeclaration(ast_buffer, indentation);
        if (symbol_set.Size() == 1) // if the right-hand side contains a symbol ...
        {
            ast_buffer.Put(indentation); ast_buffer.Put("    public  get");
                                         ast_buffer.Put(symbol_set[0] -> Name());
                                         ast_buffer.Put("() : IToken { return this.leftIToken; }\n\n");
        }
        ast_buffer.Put(indentation);  ast_buffer.Put("constructor(");
        if (element.needs_environment)
        {
            ast_buffer.Put("environment : ");
            ast_buffer.Put(option -> action_type);
            ast_buffer.Put(", token : IToken )");

            ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        super(token)\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        this.environment = environment;\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        this.initialize();\n");
            ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
        }
        else ast_buffer.Put("token : IToken ) { super(token); this.initialize(); }\n");
    }
    else 
    {
        ast_buffer.Put(option -> ast_type);
        ast_buffer.Put(" implements ");
        ast_buffer.Put(typestring[grammar -> rules[rule_no].lhs]);
        ast_buffer.Put("\n");
        ast_buffer.Put(indentation); ast_buffer.Put("{\n");
        if (element.needs_environment)
            GenerateEnvironmentDeclaration(ast_buffer, indentation);

        if (symbol_set.Size() > 0)
        {
            {
                for (int i = 0; i < symbol_set.Size(); i++)
                {
                    ast_buffer.Put(indentation); ast_buffer.Put("    private ");
                                                 ast_buffer.Put(" _");
                                                 ast_buffer.Put(symbol_set[i] -> Name());
                                                 if (ntc.CanProduceNullAst(rhs_type_index[i]))
                                                     ast_buffer.Put("?");
                                                 ast_buffer.Put(" : ");
                                                 ast_buffer.Put(ctc.FindBestTypeFor(rhs_type_index[i]));
                                                 ast_buffer.Put(";\n");
                }
            }
            ast_buffer.Put("\n");

            {
                for (int i = 0; i < symbol_set.Size(); i++)
                {
                    const char *symbolName = symbol_set[i] -> Name();
                    const char *bestType = ctc.FindBestTypeFor(rhs_type_index[i]);
                    bool nullAst = false;
                    if (ntc.CanProduceNullAst(rhs_type_index[i]))
                    {
                        ast_buffer.Put(indentation); ast_buffer.Put("    /**\n");
                        ast_buffer.Put(indentation); ast_buffer.Put("     * The value returned by <b>get");
                                                     ast_buffer.Put(symbolName);
                                                     ast_buffer.Put("</b> may be <b>undefined</b>\n");
                        ast_buffer.Put(indentation); ast_buffer.Put("     */\n");
                        nullAst = true;
                    }

                    // Generate getter method
                    ast_buffer.Put(indentation); ast_buffer.Put("    public ");
                                                 ast_buffer.Put(" get");
                                                 ast_buffer.Put(symbolName);
                                                 ast_buffer.Put("()");
                                                 ast_buffer.Put(" : ");
                                                 ast_buffer.Put(bestType);
                                                 if(nullAst)  ast_buffer.Put(" | undefined ");
                                                 ast_buffer.Put("{ return this._");
                                                 ast_buffer.Put(symbolName);
                                                 ast_buffer.Put("; }\n");

                    // Generate setter method
                    ast_buffer.Put(indentation); ast_buffer.Put("    public  set");
                    ast_buffer.Put(symbolName);
                    ast_buffer.Put("(");
                    ast_buffer.Put(" _"); // add "_" prefix to arg name in case symbol happens to be a Java keyword
                    ast_buffer.Put(symbolName);
                    ast_buffer.Put(" : ");
                    ast_buffer.Put(bestType);
                    ast_buffer.Put(") : void");
                    ast_buffer.Put(" { this._");
                    ast_buffer.Put(symbolName);
                    ast_buffer.Put(" = _");
                    ast_buffer.Put(symbolName);
                    ast_buffer.Put("; }\n");
                }
            }
            ast_buffer.Put("\n");
        }

        //
        // generate constructor
        //
        const char *header = "    constructor";
        ast_buffer.Put(indentation);
        ast_buffer.Put(header);
       
        int length = strlen(indentation) + strlen(header);

        ast_buffer.Put("(");
        if (element.needs_environment)
        {
            ast_buffer.Put("environment : ");
            ast_buffer.Put(option -> action_type);
            ast_buffer.Put(",");
        }
        ast_buffer.Put("leftIToken : IToken , rightIToken : IToken ");
        ast_buffer.Put(symbol_set.Size() == 0 ? ")\n" : ",\n");
        {
            for (int i = 0; i < symbol_set.Size(); i++)
            {
                for (int k = 0; k <= length; k++)
                    ast_buffer.PutChar(' ');
             
                ast_buffer.Put(" _");
                ast_buffer.Put(symbol_set[i] -> Name());
                ast_buffer.Put(" : ");
                ast_buffer.Put(ctc.FindBestTypeFor(rhs_type_index[i]));
                if (ntc.CanProduceNullAst(rhs_type_index[i]))
                {
                    ast_buffer.Put("| undefined");
                }
                ast_buffer.Put(i == symbol_set.Size() - 1 ? ")\n" : ",\n");
            }
        }

        ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        super(leftIToken, rightIToken)\n\n");
        if (element.needs_environment)
        {
            ast_buffer.Put(indentation);
            ast_buffer.Put("        this.environment = environment;\n");
        }

        {
            for (int i = 0; i < symbol_set.Size(); i++)
            {
                ast_buffer.Put(indentation); ast_buffer.Put("        this._");
                                             ast_buffer.Put(symbol_set[i] -> Name());
                                             ast_buffer.Put(" = _");
                                             ast_buffer.Put(symbol_set[i] -> Name());
                                             ast_buffer.Put(";\n");

                if (option -> parent_saved)
                {
                    ast_buffer.Put(indentation); ast_buffer.Put("        ");
                    if ((! optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
                    {
                        ast_buffer.Put("if (_");
                        ast_buffer.Put(symbol_set[i] -> Name());
                        ast_buffer.Put(") ");
                    }
    
                    ast_buffer.Put("(<");
                    ast_buffer.Put(option -> ast_type);
                    ast_buffer.Put("> _");
                    ast_buffer.Put(symbol_set[i] -> Name());
                    ast_buffer.Put(").setParent(this);\n");
                }
            }
        }

        ast_buffer.Put(indentation); ast_buffer.Put("        this.initialize();\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
    }

    if (option -> parent_saved)
        GenerateGetAllChildrenMethod(ast_buffer, indentation, element);
   
    GenerateVisitorMethods(ntc, ast_buffer, indentation, element, optimizable_symbol_set);
   ast_buffer.Put("    }\n\n");// Generate Class Closer
    if (option->IsTopLevel() && option->IsPackage())
    {
        ast_buffer.Put(indentation); ast_buffer.Put("}\n");// for namespace
    }

    if (option->IsTopLevel())
    {
        ast_filename_symbol->Flush();
    }
    return;
}


//
// Generate Ast class
//
void TypeScriptAction::GenerateTerminalMergedClass(NTC &ntc,
                                             ActionFileSymbol* ast_filename_symbol,
                                             const char *indentation,
                                             ClassnameElement &element,
                                             Array<const char *> &typestring)
{
    TextBuffer& ast_buffer =*GetBuffer(ast_filename_symbol);
    char *classname = element.real_name;
    GenerateCommentHeader(ast_buffer, indentation, element.ungenerated_rule, element.rule);

    ast_buffer.Put(indentation); 
                                 ast_buffer.Put("export class ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put(" extends ");
                                 ast_buffer.Put(grammar -> Get_ast_token_classname());
                                 ast_buffer.Put(" implements ");
    for (int i = 0; i < element.interface_.Length() - 1; i++)
    {
        ast_buffer.Put(typestring[element.interface_[i]]);
        ast_buffer.Put(", ");
    }
    ast_buffer.Put(typestring[element.interface_[element.interface_.Length() - 1]]);
    ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");
    if (element.needs_environment)
        GenerateEnvironmentDeclaration(ast_buffer, indentation);
    SymbolLookupTable &symbol_set = element.symbol_set;
    if (symbol_set.Size() == 1) // if the right-hand side contains a symbol ...
    {
        ast_buffer.Put(indentation); ast_buffer.Put("    public  get");
                                     ast_buffer.Put(symbol_set[0] -> Name());
                                     ast_buffer.Put("() : IToken{ return this.leftIToken; }\n\n");
    }
    ast_buffer.Put(indentation); ast_buffer.Put("    constructor(");
                                 if (element.needs_environment)
                                 {
                                     ast_buffer.Put("environment : ");
                                     ast_buffer.Put(option -> action_type);
                                     ast_buffer.Put(", token : IToken )");

                                     ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
                                     ast_buffer.Put(indentation); ast_buffer.Put("        super(token);\n");
                                     ast_buffer.Put(indentation); ast_buffer.Put("        this.environment = environment;\n");
                                     ast_buffer.Put(indentation); ast_buffer.Put("        this.initialize();\n");
                                     ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
                                 }
                                 else ast_buffer.Put("token : IToken ){ super(token); initialize(); }\n");

    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);
  
    GenerateVisitorMethods(ntc, ast_buffer, indentation, element, optimizable_symbol_set);

   ast_buffer.Put("    }\n\n");// Generate Class Closer
    if (option->IsTopLevel() && option->IsPackage())
    {
        ast_buffer.Put(indentation); ast_buffer.Put("}\n");// for namespace
    }

    if (option->IsTopLevel())
    {
        ast_filename_symbol->Flush();
    }
    return;
}


//
// Generate Ast class
//
void TypeScriptAction::GenerateMergedClass(CTC &ctc,
                                     NTC &ntc,
                                     ActionFileSymbol* ast_filename_symbol,
                                     const char *indentation,
                                     ClassnameElement &element,
                                     Tuple< Tuple<ProcessedRuleElement> > &processed_rule_map,
                                     Array<const char *> &typestring)
{
    TextBuffer& ast_buffer =*GetBuffer(ast_filename_symbol);
    char *classname = element.real_name;
    SymbolLookupTable &symbol_set = element.symbol_set;
    Tuple<int> &rhs_type_index = element.rhs_type_index;

    GenerateCommentHeader(ast_buffer, indentation, element.ungenerated_rule, element.rule);

    ast_buffer.Put(indentation); 
                                 ast_buffer.Put("export class ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put(" extends ");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(" implements ");
    {
        for (int i = 0; i < element.interface_.Length() - 1; i++)
        {
            ast_buffer.Put(typestring[element.interface_[i]]);
            ast_buffer.Put(", ");
        }
    }
    ast_buffer.Put(typestring[element.interface_[element.interface_.Length() - 1]]);
    ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");
    if (element.needs_environment)
        GenerateEnvironmentDeclaration(ast_buffer, indentation);


    //
	// Compute the set of symbols that always appear in an instance creation
	// of this merged class for which a null instance allocation will never occur.
	//
    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);
    Tuple<int>& rule = element.rule;
    {
        for (int i = 0; i < rule.Length(); i++)
        {
            int rule_no = rule[i];
            Tuple<ProcessedRuleElement>& processed_rule_elements = processed_rule_map[rule_no];
            for (int k = 0; k < processed_rule_elements.Length(); k++)
            {
                if (processed_rule_elements[k].position == 0 ||
                    ntc.CanProduceNullAst(grammar->rhs_sym[processed_rule_elements[k].position]))
                    optimizable_symbol_set.RemoveElement(k);
            }
        }
    }
    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            ast_buffer.Put(indentation); ast_buffer.Put("    private ");
                                         ast_buffer.Put(" _");
                                         ast_buffer.Put(symbol_set[i] -> Name());

                                         if ((!optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
                                             ast_buffer.Put("?");

                                         ast_buffer.Put(" : ");
                                         ast_buffer.Put(ctc.FindBestTypeFor(rhs_type_index[i]));
                                         ast_buffer.Put(";\n");
        }
    }
    ast_buffer.Put("\n");



    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            bool nullAst = false;
            if ((! optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
            {
                ast_buffer.Put(indentation); ast_buffer.Put("    /**\n");
                ast_buffer.Put(indentation); ast_buffer.Put("     * The value returned by <b>get");
                                             ast_buffer.Put(symbol_set[i] -> Name());
                                             ast_buffer.Put("</b> may be <b>undefined</b>\n");
                ast_buffer.Put(indentation); ast_buffer.Put("     */\n");
                nullAst = true;
            }

            ast_buffer.Put(indentation); ast_buffer.Put("    public ");
                                         ast_buffer.Put(" get");
                                         ast_buffer.Put(symbol_set[i] -> Name());
                                         ast_buffer.Put("() : ");
                                         ast_buffer.Put(ctc.FindBestTypeFor(rhs_type_index[i]));
							  if(nullAst)ast_buffer.Put(" | undefined ");
                                         ast_buffer.Put("{ return this._");
                                         ast_buffer.Put(symbol_set[i] -> Name());
                                         ast_buffer.Put("; }\n");
        }
    }
    ast_buffer.Put("\n");


    //
    // generate merged constructor
    //
    const char *header = "    constructor";
    ast_buffer.Put(indentation);
    ast_buffer.Put(header);
    int length = strlen(indentation) + strlen(header);

    ast_buffer.Put("(");
    if (element.needs_environment)
    {
        ast_buffer.Put("environment : ");
        ast_buffer.Put(option -> action_type);
        ast_buffer.Put(", ");
    }
    ast_buffer.Put("leftIToken : IToken , rightIToken : IToken ");
    ast_buffer.Put(symbol_set.Size() == 0 ? ")\n" : ",\n");
    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            for (int k = 0; k <= length; k++)
                ast_buffer.PutChar(' ');
            ast_buffer.Put(" _");
            ast_buffer.Put(symbol_set[i] -> Name());
            ast_buffer.Put(" : ");
            ast_buffer.Put(ctc.FindBestTypeFor(rhs_type_index[i]));
            if ((!optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
            {
                ast_buffer.Put(" | undefined");
            }
         
            ast_buffer.Put(i == symbol_set.Size() - 1 ? ")\n" : ",\n");
        }
    }

    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        super(leftIToken, rightIToken);\n\n");
    if (element.needs_environment)
    {
        ast_buffer.Put(indentation);
        ast_buffer.Put("        this.environment = environment;\n");
    }

    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            ast_buffer.Put(indentation); ast_buffer.Put("        this._");
                                         ast_buffer.Put(symbol_set[i] -> Name());
                                         ast_buffer.Put(" = _");
                                         ast_buffer.Put(symbol_set[i] -> Name());
                                         ast_buffer.Put(";\n");
    
            if (option -> parent_saved)
            {
                ast_buffer.Put(indentation); ast_buffer.Put("        ");
                if ((! optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
                {
                    ast_buffer.Put("if (_");
                    ast_buffer.Put(symbol_set[i] -> Name());
                    ast_buffer.Put(") ");
                }
    
                ast_buffer.Put("(<");
                ast_buffer.Put(option -> ast_type);
                ast_buffer.Put("> _");
                ast_buffer.Put(symbol_set[i] -> Name());
                ast_buffer.Put(").setParent(this);\n");
            }
        }
    }

    ast_buffer.Put(indentation); ast_buffer.Put("        this.initialize();\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n");

    if (option -> parent_saved)
        GenerateGetAllChildrenMethod(ast_buffer, indentation, element);
  
    GenerateVisitorMethods(ntc, ast_buffer, indentation, element, optimizable_symbol_set);

   ast_buffer.Put("    }\n\n");// Generate Class Closer
    if (option->IsTopLevel() && option->IsPackage())
    {
        ast_buffer.Put(indentation); ast_buffer.Put("}\n");// for namespace
    }

    if (option->IsTopLevel())
    {
        ast_filename_symbol->Flush();
    }
    return;
}

void TypeScriptAction::GenerateAstRootInterface(
    ActionFileSymbol* ast_filename_symbol,
    const char* indentation)
{
    TextBuffer& ast_buffer =*GetBuffer(ast_filename_symbol);
    ast_buffer.Put(indentation); ast_buffer.Put("export interface ");
    ast_buffer.Put(astRootInterfaceName.c_str());
    
    ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");
    ast_buffer.Put(indentation); ast_buffer.Put("     getLeftIToken() : IToken;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("     getRightIToken() : IToken;\n");
    ast_buffer.Put("\n");
    GenerateVisitorHeaders(ast_buffer, indentation, "    ");
    ast_buffer.Put(indentation); ast_buffer.Put("}\n\n");
    
    if (option->IsTopLevel() && option->IsPackage())
    {
        ast_buffer.Put(indentation); ast_buffer.Put("}\n");// for namespace
    }
    return;
}
void TypeScriptAction::GenerateInterface(bool is_terminal,
                                   ActionFileSymbol* ast_filename_symbol,
                                   const char *indentation,
                                   const char *interface_name,
                                   Tuple<int> &extension,
                                   Tuple<int> &classes,
                                   Tuple<ClassnameElement> &classname)
{
    TextBuffer& ast_buffer =*GetBuffer(ast_filename_symbol);
    ast_buffer.Put(indentation); ast_buffer.Put("/**");
    if (is_terminal)
    {
        ast_buffer.Put("\n");
        ast_buffer.Put(indentation);  ast_buffer.Put(" * is always implemented by <b>");
                                      ast_buffer.Put(grammar -> Get_ast_token_classname());
                                      ast_buffer.Put("</b>. It is also implemented by");
    }
    else 
    {
        ast_buffer.Put("\n");
        ast_buffer.Put(indentation);
        ast_buffer.Put(" * is implemented by");
    }

    if (classes.Length() == 1)
    {
        ast_buffer.Put(" <b>");
        ast_buffer.Put(classname[classes[0]].real_name);
        ast_buffer.Put("</b>");
    }
    else
    {
        ast_buffer.Put(":\n");
        ast_buffer.Put(indentation);
        ast_buffer.Put(" *<b>\n");
        ast_buffer.Put(indentation); ast_buffer.Put(" *<ul>");
        for (int i = 0; i < classes.Length(); i++)
        {
            ast_buffer.Put("\n");
            ast_buffer.Put(indentation);
            ast_buffer.Put(" *<li>");
            ast_buffer.Put(classname[classes[i]].real_name);
        }
        ast_buffer.Put("\n");
        ast_buffer.Put(indentation);
        ast_buffer.Put(" *</ul>\n");
        ast_buffer.Put(indentation);
        ast_buffer.Put(" *</b>");
    }

    ast_buffer.Put("\n");
    ast_buffer.Put(indentation);
    ast_buffer.Put(" */\n");

    ast_buffer.Put(indentation); ast_buffer.Put("export interface ");
                                 ast_buffer.Put(interface_name);
    if (extension.Length() > 0)
    {
        ast_buffer.Put(" extends ");
        for (int k = 0; k < extension.Length() - 1; k++)
        {
            ast_buffer.PutChar('I');
            ast_buffer.Put(extension[k] == grammar -> Get_ast_token_interface()
                               ? grammar -> Get_ast_token_classname()
                               : grammar -> RetrieveString(extension[k]));
            ast_buffer.Put(", ");
        }
        ast_buffer.PutChar('I');
        ast_buffer.Put(extension[extension.Length() - 1] == grammar -> Get_ast_token_interface()
                               ? grammar -> Get_ast_token_classname()
                               : grammar -> RetrieveString(extension[extension.Length() - 1]));
        ast_buffer.Put(" {}\n\n");
    }
    else
    {
        ast_buffer.Put(" extends ");
        ast_buffer.Put(astRootInterfaceName.c_str());
        ast_buffer.Put(indentation); ast_buffer.Put("{\n");
      
        ast_buffer.Put(indentation); ast_buffer.Put("}\n\n");
    }
	
    if (option->IsTopLevel() && option->IsPackage())
    {
        ast_buffer.Put(indentation); ast_buffer.Put("}\n");// for namespace
    }
    return;
}


//
//
//
void TypeScriptAction::GenerateNullAstAllocation(TextBuffer &ast_buffer, int rule_no)
{
    const char *code = "\n                    this.setResult(undefined);";
    GenerateCode(&ast_buffer, code, rule_no);

    return;
}


//
//
//
void TypeScriptAction::GenerateAstAllocation(CTC &ctc,
                                       TextBuffer &ast_buffer,
                                       RuleAllocationElement &allocation_element,
                                       Tuple<ProcessedRuleElement> &processed_rule_elements,
                                       Array<const char *> &typestring,
                                       int rule_no)
{
    const char *classname = allocation_element.name;

    // 
    // Copy these two arrays into a local tuple for CONVENIENCE.
    // 
    Tuple<int> position,
               type_index;
    for (int i = 0; i < processed_rule_elements.Length(); i++)
    {
        position.Next() = processed_rule_elements[i].position;
        type_index.Next() = processed_rule_elements[i].type_index;
    }

    // 
    // Convenient constant declarations.
    // 
    const char *space = "\n                    ",
               *space4 = "    ",
               *newkey = option -> factory,
               *lparen = "(",
               *comma = ",",
               *rparen = ")",
               *trailer = ");";
    int extra_space_length = strlen(space) + strlen(space4) + strlen(newkey) + strlen(classname) + 1;
    char *extra_space = new char[extra_space_length + 1];
    extra_space[0] = '\n';
    {
        for (int i = 1; i < extra_space_length; i++)
            extra_space[i] = ' ';
    }
    extra_space[extra_space_length] = '\0';

    //
    // TODO: We simply generate a comment as a reminder that the previous nonterminal
    // allocated for this token should be deleted when using a language such as C/C++
    // that does not have a garbage collector.
    //
    //    if (allocation_element.is_terminal_class && type_index.Length() == 1 && IsNonTerminal(type_index[0]))
    //    {
    //        GenerateCode(&ast_buffer, space, rule_no);
    //        GenerateCode(&ast_buffer, "// When garbage collection is not available, delete ", rule_no);
    //        GenerateCode(&ast_buffer, "getRhsSym(", rule_no);
    //        IntToString index(position[0]);
    //        GenerateCode(&ast_buffer, index.string(), rule_no);
    //        GenerateCode(&ast_buffer, rparen, rule_no);
    //    }
    //
    if (allocation_element.is_terminal_class && (grammar -> RhsSize(rule_no) == 1 && grammar -> IsNonTerminal(grammar -> rhs_sym[grammar -> FirstRhsIndex(rule_no)])))
    {
        GenerateCode(&ast_buffer, space, rule_no);
        GenerateCode(&ast_buffer, "//", rule_no);
        GenerateCode(&ast_buffer, space, rule_no);
        GenerateCode(&ast_buffer, "// When garbage collection is not available, delete ", rule_no);
        GenerateCode(&ast_buffer, "this.getRhsSym(1)", rule_no);
        GenerateCode(&ast_buffer, space, rule_no);
        GenerateCode(&ast_buffer, "//", rule_no);
    }
    GenerateCode(&ast_buffer, space, rule_no);
    GenerateCode(&ast_buffer, "this.setResult(", rule_no);
    GenerateCode(&ast_buffer, space, rule_no);
    GenerateCode(&ast_buffer, space4, rule_no);
    GenerateCode(&ast_buffer, current_line_input_file_info.c_str(), rule_no);
    GenerateCode(&ast_buffer, space, rule_no);
    GenerateCode(&ast_buffer, space4, rule_no);

    GenerateCode(&ast_buffer, newkey, rule_no);
    GenerateCode(&ast_buffer, classname, rule_no);
    GenerateCode(&ast_buffer, lparen, rule_no);
    if (allocation_element.needs_environment)
    {
        GenerateCode(&ast_buffer, "this, ", rule_no);
    }
    if (allocation_element.is_terminal_class)
    {
        GenerateCode(&ast_buffer, "this.getRhsIToken(1)", rule_no);
        //
        // TODO: Old bad idea. Remove at some point...
        //
        //
        //        assert(position.Length() <= 1);
        //
        //        GenerateCode(&ast_buffer, "getRhsIToken(", rule_no);
        //        IntToString index(position.Length() == 0 ? 1 : position[0]);
        //        GenerateCode(&ast_buffer, index.string(), rule_no);
        //        GenerateCode(&ast_buffer, rparen, rule_no);
        //
    }
    else
    {
        GenerateCode(&ast_buffer, "this.getLeftIToken()", rule_no);
        GenerateCode(&ast_buffer, ", ", rule_no);
        GenerateCode(&ast_buffer, "this.getRightIToken()", rule_no);
        if (position.Length() > 0)
        {
            GenerateCode(&ast_buffer, comma, rule_no);
            GenerateCode(&ast_buffer, extra_space, rule_no);
            GenerateCode(&ast_buffer, current_line_input_file_info.c_str(), rule_no);
            GenerateCode(&ast_buffer, extra_space, rule_no);

            int offset = grammar -> FirstRhsIndex(rule_no) - 1;
            for (int i = 0; i < position.Length(); i++)
            {
                if (position[i] == 0)
                {
             /*       GenerateCode(&ast_buffer, lparen, rule_no);
                    GenerateCode(&ast_buffer, ctc.FindBestTypeFor(type_index[i]), rule_no);
                    GenerateCode(&ast_buffer, rparen, rule_no);*/
                    GenerateCode(&ast_buffer, "undefined", rule_no);
                }
                else
                {
                    int symbol = grammar -> rhs_sym[offset + position[i]];
                    if (grammar -> IsTerminal(symbol))
                    {
                        const char *actual_type = ctc.FindBestTypeFor(type_index[i]);

                        if (strcmp(actual_type, grammar -> Get_ast_token_classname()) != 0)
                        {
                            GenerateCode(&ast_buffer, "<", rule_no);
                            GenerateCode(&ast_buffer, actual_type, rule_no);
                            GenerateCode(&ast_buffer, ">", rule_no);
                        }

                        GenerateCode(&ast_buffer, newkey, rule_no);
                        GenerateCode(&ast_buffer, grammar -> Get_ast_token_classname(), rule_no);
                        GenerateCode(&ast_buffer, lparen, rule_no);
                        GenerateCode(&ast_buffer, "this.getRhsIToken(", rule_no);
                        IntToString index(position[i]);
                        GenerateCode(&ast_buffer, index.String(), rule_no);
                        GenerateCode(&ast_buffer, rparen, rule_no);
                    }
                    else
                    {
                        GenerateCode(&ast_buffer, "<", rule_no);
                        GenerateCode(&ast_buffer, ctc.FindBestTypeFor(type_index[i]), rule_no);
                        GenerateCode(&ast_buffer, ">", rule_no);
                        GenerateCode(&ast_buffer, "this.getRhsSym(", rule_no);
                        IntToString index(position[i]);
                        GenerateCode(&ast_buffer, index.String(), rule_no);
                    }
    
                    GenerateCode(&ast_buffer, rparen, rule_no);
                }
        
                if (i != position.Length() - 1)
                {
                    GenerateCode(&ast_buffer, comma, rule_no);
                    GenerateCode(&ast_buffer, extra_space, rule_no);
                    GenerateCode(&ast_buffer, current_line_input_file_info.c_str(), rule_no);
                    GenerateCode(&ast_buffer, extra_space, rule_no);
                }
            }
        }
    }

    GenerateCode(&ast_buffer, rparen, rule_no);
    GenerateCode(&ast_buffer, space, rule_no);
    GenerateCode(&ast_buffer, current_line_input_file_info.c_str(), rule_no);
    GenerateCode(&ast_buffer, space, rule_no);
    GenerateCode(&ast_buffer, trailer, rule_no);

    delete [] extra_space;

    return;
}

//
//
//
void TypeScriptAction::GenerateListAllocation(CTC &ctc,
                                        TextBuffer &ast_buffer,
                                        int rule_no,
                                        RuleAllocationElement &allocation_element)
{
    const char *space = "\n                    ",
               *space4 = "    ",
               *newkey = option -> factory,
               *lparen = "(",
               *comma = ",",
               *rparen = ")",
               *trailer = ");";

    if (allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_EMPTY ||
        allocation_element.list_kind == RuleAllocationElement::RIGHT_RECURSIVE_EMPTY ||
        allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_SINGLETON ||
        allocation_element.list_kind == RuleAllocationElement::RIGHT_RECURSIVE_SINGLETON)
    {

        GenerateCode(&ast_buffer, space, rule_no);
        GenerateCode(&ast_buffer, "this.setResult(", rule_no);
        GenerateCode(&ast_buffer, space, rule_no);
        GenerateCode(&ast_buffer, space4, rule_no);
        GenerateCode(&ast_buffer, current_line_input_file_info.c_str(), rule_no);
        GenerateCode(&ast_buffer, space, rule_no);
        GenerateCode(&ast_buffer, space4, rule_no);


        if (allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_EMPTY ||
            allocation_element.list_kind == RuleAllocationElement::RIGHT_RECURSIVE_EMPTY)
        {

 

            GenerateCode(&ast_buffer, newkey, rule_no);
            GenerateCode(&ast_buffer, allocation_element.name, rule_no);
            GenerateCode(&ast_buffer, lparen, rule_no);
            if (allocation_element.needs_environment)
            {
                GenerateCode(&ast_buffer, "this, ", rule_no);
            }

            GenerateCode(&ast_buffer, "this.getLeftIToken()", rule_no);
            GenerateCode(&ast_buffer, ", ", rule_no);
            GenerateCode(&ast_buffer, "this.getRightIToken()", rule_no);
            GenerateCode(&ast_buffer, comma, rule_no);
            if (allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_EMPTY)
                 GenerateCode(&ast_buffer, " true /* left recursive */", rule_no);
            else GenerateCode(&ast_buffer, " false /* not left recursive */", rule_no);
        }
        else
        {
         
            GenerateCode(&ast_buffer, allocation_element.name, rule_no);
            GenerateCode(&ast_buffer, ".", rule_no);
            GenerateCode(&ast_buffer, allocation_element.name, rule_no);
            GenerateCode(&ast_buffer, "fromElement", rule_no);
            GenerateCode(&ast_buffer, lparen, rule_no);
            if (allocation_element.needs_environment)
            {
                GenerateCode(&ast_buffer, "this, ", rule_no);
            }
            assert(allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_SINGLETON ||
                   allocation_element.list_kind == RuleAllocationElement::RIGHT_RECURSIVE_SINGLETON);

            if (grammar -> IsTerminal(allocation_element.element_symbol))
            {
                GenerateCode(&ast_buffer, newkey, rule_no);
                GenerateCode(&ast_buffer, grammar -> Get_ast_token_classname(), rule_no);
                GenerateCode(&ast_buffer, lparen, rule_no);
                GenerateCode(&ast_buffer, "this.getRhsIToken(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&ast_buffer, index.String(), rule_no);
                GenerateCode(&ast_buffer, rparen, rule_no);
            }
            else
            {
                GenerateCode(&ast_buffer, "<", rule_no);
                GenerateCode(&ast_buffer, ctc.FindBestTypeFor(allocation_element.element_type_symbol_index), rule_no);
                GenerateCode(&ast_buffer, ">", rule_no);
                GenerateCode(&ast_buffer, "this.getRhsSym(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&ast_buffer, index.String(), rule_no);
            }
    
            GenerateCode(&ast_buffer, rparen, rule_no);
            GenerateCode(&ast_buffer, comma, rule_no);
            if (allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_SINGLETON)
                 GenerateCode(&ast_buffer, " true /* left recursive */", rule_no);
            else GenerateCode(&ast_buffer, " false /* not left recursive */", rule_no);
        }

        GenerateCode(&ast_buffer, rparen, rule_no);
        GenerateCode(&ast_buffer, space, rule_no);
        GenerateCode(&ast_buffer, current_line_input_file_info.c_str(), rule_no);
        GenerateCode(&ast_buffer, space, rule_no);
    }
    else
    {
        //
        // Add new element to list
        //
        if (allocation_element.list_kind == RuleAllocationElement::ADD_ELEMENT)
        {
            GenerateCode(&ast_buffer, space, rule_no);
            GenerateCode(&ast_buffer, lparen, rule_no);
            GenerateCode(&ast_buffer, "<", rule_no);
            GenerateCode(&ast_buffer, allocation_element.name, rule_no);
            GenerateCode(&ast_buffer, ">", rule_no);
            GenerateCode(&ast_buffer, "this.getRhsSym(", rule_no);
            IntToString index(allocation_element.list_position);
            GenerateCode(&ast_buffer, index.String(), rule_no);
            GenerateCode(&ast_buffer, ")).addElement(", rule_no);
            if (grammar -> IsTerminal(allocation_element.element_symbol))
            {
                GenerateCode(&ast_buffer, newkey, rule_no);
                GenerateCode(&ast_buffer, grammar -> Get_ast_token_classname(), rule_no);
                GenerateCode(&ast_buffer, lparen, rule_no);
                GenerateCode(&ast_buffer, "this.getRhsIToken(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&ast_buffer, index.String(), rule_no);
                GenerateCode(&ast_buffer, rparen, rule_no);
            }
            else
            {
                GenerateCode(&ast_buffer, "<", rule_no);
                GenerateCode(&ast_buffer, ctc.FindBestTypeFor(allocation_element.element_type_symbol_index), rule_no);
                GenerateCode(&ast_buffer, ">", rule_no);
                GenerateCode(&ast_buffer, "this.getRhsSym(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&ast_buffer, index.String(), rule_no);
            }

            if (allocation_element.list_position != 1) // a right-recursive rule? set the list as result
            {
                GenerateCode(&ast_buffer, rparen, rule_no);
                GenerateCode(&ast_buffer, trailer, rule_no);

                GenerateCode(&ast_buffer, space, rule_no);
                GenerateCode(&ast_buffer, "this.setResult(", rule_no);
                GenerateCode(&ast_buffer, "this.getRhsSym(", rule_no);
                IntToString index(allocation_element.list_position);
                GenerateCode(&ast_buffer, index.String(), rule_no);
            }
        }

        //
        // Copy a list that is not the first element on the right-hand side of the rule
        //
        else
        {
            assert(allocation_element.list_kind == RuleAllocationElement::COPY_LIST);

            GenerateCode(&ast_buffer, space, rule_no);
            GenerateCode(&ast_buffer, "this.setResult(", rule_no);
            GenerateCode(&ast_buffer, "<", rule_no);
            GenerateCode(&ast_buffer, allocation_element.name, rule_no);
            GenerateCode(&ast_buffer, ">", rule_no);
            GenerateCode(&ast_buffer, "this.getRhsSym(", rule_no);
            IntToString index(allocation_element.list_position);
            GenerateCode(&ast_buffer, index.String(), rule_no);
        }

        GenerateCode(&ast_buffer, rparen, rule_no);
    }

    GenerateCode(&ast_buffer, trailer, rule_no);
 
    return;
}
