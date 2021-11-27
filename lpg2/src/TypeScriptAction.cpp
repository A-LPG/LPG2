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
                                            bool)
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
void TypeScriptAction::GenerateEnvironmentDeclaration(TextBuffer &b, const char *indentation)
{
    b.Put(indentation); b.Put("    private ");
								 b.Put(" environment : ");
                                 b.Put(option -> action_type);
                                 b.Put(";\n");
    b.Put(indentation); b.Put("    public ");
                                
                                 b.Put(" getEnvironment() :");
                                 b.Put(option->action_type);
                                 b.Put("{ return this.environment; }\n\n");
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
    TextBuffer& b =*(ast_filename_symbol->BodyBuffer());
	
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
                    ProcessMacro(&b, "SplitActions", rule_no);
                    count++;
                }

                ProcessMacro(&b, "BeginAction", rule_no);

                if (rule_allocation_map[rule_no].list_kind != RuleAllocationElement::NOT_A_LIST)
                {
                    GenerateListAllocation(ctc,
                                           ntc,
                                           b,
                                           rule_no, rule_allocation_map[rule_no]);
                }
                else
                {
                    if (user_specified_null_ast[rule_no] || (grammar->RhsSize(rule_no) == 0 && rule_allocation_map[rule_no].name == NULL))
                        GenerateNullAstAllocation(b, rule_no);
                    else GenerateAstAllocation(ctc,
                                               ntc,
                                               b,
                                               rule_allocation_map[rule_no],
                                               processed_rule_map[rule_no],
                                               typestring, rule_no);
                }

                GenerateCode(&b, "\n    ", rule_no);
                ProcessMacro(&b, "EndAction", rule_no);
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

                ProcessMacro(&b, "NoAction", rule_no);
            }
        }
    }
  
    return;
}




//
//
//
void TypeScriptAction::GenerateVisitorHeaders(TextBuffer &b, const char *indentation, const char *modifiers)
{
    if (option -> visitor != Option::NONE)
    {
        char *header = new char[strlen(indentation) + strlen(modifiers) + 9];
        strcpy(header, indentation);
        strcat(header, modifiers);

        b.Put(header);
        if (option -> visitor == Option::PREORDER)
        {
            b.Put("accept(v : IAstVisitor ) : void;");
        }
        else if (option -> visitor == Option::DEFAULT)
        {
            b.Put("acceptWithVisitor(v : ");
            b.Put(option -> visitor_type);
            b.Put(") : void;");

            b.Put("\n");

            b.Put(header);
            b.Put(" acceptWithArg(v : Argument");
            b.Put(option -> visitor_type);
            b.Put(", o : any) : void;\n");

            b.Put(header);
            b.Put("acceptWithResult(v : Result");
            b.Put(option -> visitor_type);
            b.Put(") : any;\n");

            b.Put(header);
            b.Put("acceptWithResultArgument(v : ResultArgument");
            b.Put(option -> visitor_type);
            b.Put(", o : any) : any;");
        }
        b.Put("\n");

        delete [] header;
    }

    return;
}


//
//
//
void TypeScriptAction::GenerateVisitorMethods(NTC &ntc,
                                        TextBuffer &b,
                                        const char *indentation,
                                        ClassnameElement &element,
                                        BitSet &optimizable_symbol_set)
{
    if (option -> visitor == Option::DEFAULT)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("    public  acceptWithVisitor(v : ");
                                     b.Put(option -> visitor_type);
                                     b.Put(") : void{ v.visit"); b.Put(element.real_name); b.Put("(this); }\n");

        b.Put(indentation); b.Put("    public  acceptWithArg(v : Argument");
                                     b.Put(option -> visitor_type);
                                     b.Put(", o : any) : void { v.visit");b.Put(element.real_name); b.Put("(this, o); }\n");

        b.Put(indentation); b.Put("    public  acceptWithResult(v : Result");
                                     b.Put(option -> visitor_type);
                                     b.Put(") : any{ return v.visit");b.Put(element.real_name); b.Put("(this); }\n");

        b.Put(indentation); b.Put("    public   acceptWithResultArgument(v : ResultArgument");
                                     b.Put(option -> visitor_type);
                                     b.Put(", o : any) : any { return v.visit"); b.Put(element.real_name); b.Put("(this, o); }\n");
    }
    else if (option -> visitor == Option::PREORDER)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("    public   accept(v : IAstVisitor ) : void\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        if (! v.preVisit(this)) return;\n");
        b.Put(indentation); b.Put("        this.enter(<"); 
                                     b.Put(option -> visitor_type);
                                     b.Put("> v);\n");
        b.Put(indentation); b.Put("        v.postVisit(this);\n");
        b.Put(indentation); b.Put("    }\n\n");

        b.Put(indentation); b.Put("    public   enter(v : ");
                                     b.Put(option -> visitor_type);
                                     b.Put(") : void\n");
        b.Put(indentation); b.Put("    {\n");
        SymbolLookupTable &symbol_set = element.symbol_set;
        Tuple<int> &rhs_type_index = element.rhs_type_index;
        if (element.is_terminal_class || symbol_set.Size() == 0)
        {
            b.Put(indentation); b.Put("        v.visit"); b.Put(element.real_name); b.Put("(this);\n");
          
        }
        else
        {
            b.Put(indentation); b.Put("        let checkChildren = v.visit");b.Put(element.real_name); b.Put("(this);\n");
            b.Put(indentation); b.Put("        if (checkChildren)\n");
            if (symbol_set.Size() > 1)
            {
                b.Put(indentation); b.Put("        {\n");
            }

            for (int i = 0; i < symbol_set.Size(); i++)
            {
                b.Put(indentation); b.Put("            ");
                if ((! optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
                {
                    b.Put("if (this._");
                    b.Put(symbol_set[i] -> Name());
                    b.Put(") ");
                }
                b.Put("this._");
                b.Put(symbol_set[i] -> Name());
                b.Put(".accept(v);\n");
            }

            if (symbol_set.Size() > 1)
            {
                b.Put(indentation); b.Put("        }\n");
            }
        }
        b.Put(indentation); b.Put("        v.endVisit"); b.Put(element.real_name); b.Put("(this);\n");
        b.Put(indentation); b.Put("    }\n");
    }

    return;
}


//
//
//
void TypeScriptAction::GenerateGetAllChildrenMethod(TextBuffer &b,
                                              const char *indentation,
                                              ClassnameElement &element)
{
    if (! element.is_terminal_class)
    {
        SymbolLookupTable &symbol_set = element.symbol_set;

        b.Put("\n");
        b.Put(indentation); b.Put("    /**\n");
        b.Put(indentation); b.Put("     * A list of all children of this node, don't including the null ones.\n");
        b.Put(indentation); b.Put("     */\n");
        b.Put(indentation); b.Put("     public   getAllChildren() : Lpg.Util.ArrayList<IAst>\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        let list = new Lpg.Util.ArrayList<IAst>();\n");
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            b.Put(indentation);
        	b.Put("        if(this._");b.Put(symbol_set[i]->Name());
        	b.Put(")  list.add(this._");b.Put(symbol_set[i] -> Name());
        	b.Put(");\n");
        }
        b.Put(indentation); b.Put("        return list;\n");
        b.Put(indentation); b.Put("    }\n");
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
    TextBuffer& b =*GetBuffer(ast_filename_symbol);

    b.Put(indentation); b.Put("export interface ");
                                 b.Put(interface_name);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("    visit"); b.Put(symbol->Name());
                                     b.Put("(n : ");
                                     b.Put(symbol -> Name());
                                     b.Put(") : void;\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    visit");
                                 b.Put("(n : ");
                                 b.Put(option -> ast_type);
                                 b.Put(") : void;\n");

    b.Put(indentation); b.Put("}\n");


}

//
//
//
void TypeScriptAction::GenerateArgumentVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                  const char *indentation,
                                                  const char *interface_name,
                                                  SymbolLookupTable &type_set)
{
   
    TextBuffer& b =*GetBuffer(ast_filename_symbol);

    b.Put(indentation); b.Put("export interface ");
                                 b.Put(interface_name);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("    visit"); b.Put(symbol->Name());
                                     b.Put("(n : ");
                                     b.Put(symbol -> Name());
                                     b.Put(", o : any) : void;\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    visit");
                                 b.Put("(n : ");
                                 b.Put(option -> ast_type);
                                 b.Put(", o : any) : void;\n");

    b.Put(indentation); b.Put("}\n");
    
}

//
//
//
void TypeScriptAction::GenerateResultVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                const char *indentation,
                                                const char *interface_name,
                                                SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    b.Put(indentation); b.Put("export interface ");
                                 b.Put(interface_name);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("    visit"); b.Put(symbol->Name());
                                     b.Put("(n : ");
                                     b.Put(symbol -> Name());
                                     b.Put(") : any;\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    visit");
                                 b.Put("(n : ");
                                 b.Put(option -> ast_type);
                                 b.Put(") : any;\n");

    b.Put(indentation); b.Put("}\n");
    
}

//
//
//
void TypeScriptAction::GenerateResultArgumentVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                        const char *indentation,
                                                        const char *interface_name,
                                                        SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    b.Put(indentation); b.Put("export interface ");
    b.Put(interface_name);
    b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("    visit"); b.Put(symbol->Name());
                                     b.Put("(n : ");
                                     b.Put(symbol -> Name());
                                     b.Put(", o : any) : any;\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    visit");
                                 b.Put("(n : ");
                                 b.Put(option -> ast_type);
                                 b.Put(", o : any) : any;\n");

    b.Put(indentation); b.Put("}\n");
    
}


//
//
//
void TypeScriptAction::GeneratePreorderVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                  const char *indentation,
                                                  const char *interface_name,
                                                  SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    assert(option -> visitor == Option::PREORDER);
    b.Put(indentation); b.Put("export interface ");
                                 b.Put(interface_name);
                                 b.Put(" extends IAstVisitor\n");
    b.Put(indentation); b.Put("{\n");


    b.Put(indentation); b.Put("    visit");
                                 b.Put("(n : ");
                                 b.Put(option -> ast_type);
                                 b.Put(") : boolean;\n");
    b.Put(indentation); b.Put("    endVisit");
                                 b.Put("(n : ");
                                 b.Put(option -> ast_type);
                                 b.Put(") : void;\n\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("    visit"); b.Put(symbol->Name());
                                     b.Put("(n : ");
                                     b.Put(symbol -> Name());
                                     b.Put(") : boolean;\n");
        b.Put(indentation); b.Put("    endVisit"); b.Put(symbol->Name());
                                     b.Put("(n : ");
                                     b.Put(symbol -> Name());
                                     b.Put(") : void;\n");
        b.Put("\n");
    }

    b.Put(indentation); b.Put("}\n\n");
    
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
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    b.Put(indentation); 
                                 b.Put("export abstract class ");
                                 b.Put(classname);
                                 b.Put(" implements ");
                                 b.Put(option -> visitor_type);
                                 b.Put(", Argument");
                                 b.Put(option -> visitor_type);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("    public abstract  unimplementedVisitor(s : string) : void;\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];

            b.Put(indentation); b.Put("    public  visit"); b.Put(symbol->Name());
                                         b.Put("(n : ");
                                         b.Put(symbol -> Name());
                                         b.Put(", o? : any) : void { this.unimplementedVisitor(\"visit"); b.Put(symbol->Name()); b.Put("(");
                                         b.Put(symbol -> Name());
                                         b.Put(", any)\"); }\n");
            b.Put("\n");
        }
    }

                                 b.Put("\n");

   

    b.Put(indentation); b.Put("    public  visit");
                                 b.Put("(n : ");
                                 b.Put(option -> ast_type);
                                 b.Put(", o? : any) : void\n");
    b.Put(indentation); b.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                         b.Put(i == 0 ? "" : "else ");
                                         b.Put("if (n instanceof ");
                                         b.Put(symbol -> Name());
                                         b.Put(") this.visit");
                                         b.Put(symbol -> Name());
                                         b.Put("(<");
                                         b.Put(symbol->Name());
                                         b.Put("> n, o);\n");
        }
    }
    b.Put(indentation); b.Put("        else throw new Error(\"visit(\" + n.toString() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("}\n");
    
}

//
//
//
void TypeScriptAction::GenerateResultVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                    const char *indentation,
                                                    const char *classname,
                                                    SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    b.Put(indentation); 
                                 b.Put("export abstract class ");
                                 b.Put(classname);
                                 b.Put(" implements Result");
                                 b.Put(option -> visitor_type);
                                 b.Put(", ResultArgument");
                                 b.Put(option -> visitor_type);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("    public abstract  unimplementedVisitor(s : string) : any;\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
       
            b.Put(indentation); b.Put("    public visit"); b.Put(symbol->Name()); b.Put("(n : ");
                                         b.Put(symbol -> Name());
                                         b.Put(", o? : any) : any{ return  this.unimplementedVisitor(\"visit"); b.Put(symbol->Name()); b.Put("(");
                                         b.Put(symbol -> Name());
                                         b.Put(", any)\"); }\n");
            b.Put("\n");
        }
    }

                                 b.Put("\n");



    b.Put(indentation); b.Put("    public visit");
                                 b.Put("(n : ");
                                 b.Put(option -> ast_type);
                                 b.Put(", o? : any) : any\n");
    b.Put(indentation); b.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                         b.Put(i == 0 ? "" : "else ");
                                         b.Put("if (n instanceof ");
                                         b.Put(symbol -> Name());
                                         b.Put(") return this.visit");
                                         b.Put(symbol->Name());
                                         b.Put("(<");
                                         b.Put(symbol -> Name());
                                         b.Put("> n, o);\n");
        }
    }
    b.Put(indentation); b.Put("        else throw new Error(\"visit(\" + n.toString() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("}\n");
    
}


//
//
//
void TypeScriptAction::GeneratePreorderVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                      const char *indentation,
                                                      const char *classname,
                                                      SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    assert(option -> visitor == Option::PREORDER);
    b.Put(indentation); 
                                 b.Put("export abstract class ");
                                 b.Put(classname);
                                 b.Put(" implements ");
                                 b.Put(option -> visitor_type);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("    public abstract unimplementedVisitor(s : string) : void;\n\n");
    b.Put(indentation); b.Put("    public preVisit(element : IAst) : boolean{ return true; }\n\n");
    b.Put(indentation); b.Put("    public postVisit(element : IAst) : void{}\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("    public visit"); b.Put(symbol->Name());
                                         b.Put("(n : ");
                                         b.Put(symbol -> Name());
                                         b.Put(") : boolean{ this.unimplementedVisitor(\"visit(");
                                         b.Put(symbol -> Name());
                                         b.Put(")\"); return true; }\n");
            b.Put(indentation); b.Put("    public endVisit"); b.Put(symbol->Name());
                                         b.Put("(n : ");
                                         b.Put(symbol -> Name());
                                         b.Put(") : void{ this.unimplementedVisitor(\"endVisit(");
                                         b.Put(symbol -> Name());
                                         b.Put(")\"); }\n");
            b.Put("\n");
        }
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    public visit");
                                 b.Put("(n : ");
                                 b.Put(option -> ast_type);
                                 b.Put(") : boolean\n");
    b.Put(indentation); b.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                         b.Put(i == 0 ? "" : "else ");
                                         b.Put("if (n instanceof ");
                                         b.Put(symbol -> Name());
                                         b.Put(") return this.visit");
                                         b.Put(symbol->Name());
                                         b.Put("(<");
                                         b.Put(symbol -> Name());
                                         b.Put("> n);\n");
        }
    }
    b.Put(indentation); b.Put("        else throw new Error(\"visit(\" + n.toString() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    public endVisit");
                                 b.Put("(n : ");
                                 b.Put(option -> ast_type);
                                 b.Put(") : void\n");
    b.Put(indentation); b.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                         b.Put(i == 0 ? "" : "else ");
                                         b.Put("if (n instanceof ");
                                         b.Put(symbol -> Name());
                                         b.Put(") this.endVisit");
                                         b.Put(symbol -> Name());
                                         b.Put("(<");
                                         b.Put(symbol->Name());
                                         b.Put("> n);\n");
        }
    }
    b.Put(indentation); b.Put("        else throw new Error(\"visit(\" + n.toString() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("}\n");
    
    return;
}


//
// Generate the the Ast root classes
//
void TypeScriptAction::GenerateAstType(ActionFileSymbol* ast_filename_symbol,
                                 const char *indentation,
                                 const char *classname)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    /*
     * First, generate the main root class
     */
    b.Put(indentation); 
                                 b.Put("export abstract class ");
                                 b.Put(classname);
                                 b.Put(" implements IAst\n");
    b.Put(indentation); b.Put("{\n");
    if (option -> glr)
    {
        b.Put(indentation); b.Put("    private nextAst : Ast | null = null;\n");
        b.Put(indentation); b.Put("    public getNextAst() : IAst  | null{ return this.nextAst; }\n");
        b.Put(indentation); b.Put("    public setNextAst(n : IAst) : void{ this.nextAst = n; }\n");
        b.Put(indentation); b.Put("    public resetNextAst() : void { this.nextAst = null; }\n");
    }
    else
    {
	    b.Put(indentation); b.Put("    public getNextAst() : IAst | null { return null; }\n");
    }

    b.Put(indentation); b.Put("    protected leftIToken : IToken ;\n");
    b.Put(indentation); b.Put("    protected rightIToken: IToken ;\n");
    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("    protected parent : IAst | null = null;\n");
        b.Put(indentation); b.Put("    public  setParent(parent : IAst ) : void { this.parent = parent; }\n");
        b.Put(indentation); b.Put("    public  getParent() : IAst | null{ return this.parent; }\n");\
    }
    else
    {
        b.Put(indentation); b.Put("    public getParent(): IAst | null \n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        throw new Error(\"noparent-saved option in effect\");\n");
        b.Put(indentation); b.Put("    }\n");
    }

    b.Put("\n");
    b.Put(indentation); b.Put("    public getLeftIToken() : IToken { return this.leftIToken; }\n");
    b.Put(indentation); b.Put("    public getRightIToken() : IToken { return this.rightIToken; }\n");
    b.Put(indentation); b.Put("    public getPrecedingAdjuncts() : IToken[] { return this.leftIToken.getPrecedingAdjuncts(); }\n");
    b.Put(indentation); b.Put("    public getFollowingAdjuncts() : IToken[] { return this.rightIToken.getFollowingAdjuncts(); }\n\n");

    b.Put(indentation); b.Put("    public  toString() : string \n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        let str = this.leftIToken.getILexStream()?.toString(this.leftIToken.getStartOffset(), this.rightIToken.getEndOffset());\n");
    b.Put(indentation); b.Put("        return str? str : \"\";\n");
    b.Put(indentation); b.Put("    }\n\n");


    b.Put(indentation);b.Put("constructor(leftIToken : IToken , rightIToken? : IToken )\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        this.leftIToken = leftIToken;\n");
    b.Put(indentation); b.Put("        if(rightIToken) this.rightIToken = rightIToken;\n");
    b.Put(indentation); b.Put("        else            this.rightIToken = leftIToken;\n");
    b.Put(indentation); b.Put("    }\n\n");
    b.Put(indentation); b.Put("  public   initialize() : void {}\n");
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
            action.buffer = &b;
            ProcessActionBlock(action);
        }
    }

    b.Put("\n");
    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("    /**\n");
        b.Put(indentation); b.Put("     * A list of all children of this node, excluding the null ones.\n");
        b.Put(indentation); b.Put("     */\n");
        b.Put(indentation); b.Put("    public  getChildren() : Lpg.Util.ArrayList<IAst>\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("         let list = this.getAllChildren() ;\n");
        b.Put(indentation); b.Put("        let k = -1;\n");
        b.Put(indentation); b.Put("        for (let i = 0; i < list.size(); i++)\n");
        b.Put(indentation); b.Put("        {\n");
        b.Put(indentation); b.Put("            let element = list.get(i);\n");
        b.Put(indentation); b.Put("            if (element)\n");
        b.Put(indentation); b.Put("            {\n");
        b.Put(indentation); b.Put("                if (++k != i)\n");
        b.Put(indentation); b.Put("                    list.set(k, element);\n");
        b.Put(indentation); b.Put("            }\n");
        b.Put(indentation); b.Put("        }\n");
        b.Put(indentation); b.Put("        for (let i = list.size() - 1; i > k; i--) // remove extraneous elements\n");
        b.Put(indentation); b.Put("            list.remove(i);\n");
        b.Put(indentation); b.Put("        return list;\n");
        b.Put(indentation); b.Put("    }\n\n");

        b.Put(indentation); b.Put("    /**\n");
        b.Put(indentation); b.Put("     * A list of all children of this node, don't including the null ones.\n");
        b.Put(indentation); b.Put("     */\n");
        b.Put(indentation); b.Put("    public abstract  getAllChildren() : Lpg.Util.ArrayList<IAst>;\n");
    }
    else
    {
        b.Put(indentation); b.Put("    public  getChildren() : Lpg.Util.ArrayList<IAst>\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        throw new Error(\"noparent-saved option in effect\");\n");
        b.Put(indentation); b.Put("    }\n");
        b.Put(indentation); b.Put("     public   getAllChildren() : Lpg.Util.ArrayList<IAst> { return this.getChildren(); }\n");
    }

    b.Put("\n");

    GenerateVisitorHeaders(b, indentation, "    public abstract ");

    //
    // Not Preorder visitor? generate dummy accept method to satisfy IAst abstract declaration of accept(IAstVisitor);
    // TODO: Should IAstVisitor be used for default visitors also? If (when) yes then we should remove it from the test below
    //
    if (option -> visitor == Option::NONE || option -> visitor == Option::DEFAULT) // ??? Don't need this for DEFAULT case after upgrade
    {
        b.Put(indentation); b.Put("    public  accept(v : IAstVisitor ) : void {}\n");
    }
    b.Put(indentation); b.Put("}\n\n");
    
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
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    /*
     * Generate the List root class
     */
    b.Put(indentation); 
                                 b.Put("export abstract class ");
                                 b.Put(this -> abstract_ast_list_classname);
                                 b.Put(" extends ");
                                 b.Put(option -> ast_type);
                                 b.Put(" implements IAbstractArrayList<");
                                 b.Put(option -> ast_type);
                                 b.Put(">\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("    private leftRecursive : boolean ;\n");
    b.Put(indentation); b.Put("    public "); 
	b.Put(" list  = new Lpg.Util.ArrayList<"); b.Put(option->ast_type); b.Put(">();\n");

    b.Put(indentation); b.Put("    public  size() : number { return this.list.size(); }\n");
    b.Put(indentation); b.Put("    public   getList() : Lpg.Util.ArrayList<"); b.Put(option->ast_type); b.Put(indentation); b.Put("> { return this.list; }\n");
    b.Put(indentation); b.Put("    public ");
                              
                                 b.Put(" getElementAt(i : number ) : ");
                                 b.Put(option->ast_type);
                                 b.Put(" { return <");
                                 b.Put(option -> ast_type);
                                 b.Put("> this.list.get(this.leftRecursive ? i : this.list.size() - 1 - i); }\n");
    b.Put(indentation); b.Put("    public  getArrayList() : Lpg.Util.ArrayList<"); b.Put(option->ast_type); b.Put(">\n");
   
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        if (! this.leftRecursive) // reverse the list \n");
    b.Put(indentation); b.Put("        {\n");
    b.Put(indentation); b.Put("            for (let i = 0, n = this.list.size() - 1; i < n; i++, n--)\n");
    b.Put(indentation); b.Put("            {\n");
    b.Put(indentation); b.Put("                let ith = this.list.get(i),\n");
    b.Put(indentation); b.Put("                       nth = this.list.get(n);\n");
    b.Put(indentation); b.Put("                this.list.set(i, nth);\n");
    b.Put(indentation); b.Put("                this.list.set(n, ith);\n");
    b.Put(indentation); b.Put("            }\n");
    b.Put(indentation); b.Put("            this.leftRecursive = true;\n");
    b.Put(indentation); b.Put("        }\n");
    b.Put(indentation); b.Put("        return this.list;\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    /**\n");
    b.Put(indentation); b.Put("     * @deprecated replaced by {@link #addElement()}\n");
    b.Put(indentation); b.Put("     *\n");
    b.Put(indentation); b.Put("     */\n");
    b.Put(indentation); b.Put("    public  add(element : ");
                                 b.Put(option -> ast_type);
                                 b.Put(") : boolean\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        this.addElement(element);\n");
    b.Put(indentation); b.Put("        return true;\n");
    b.Put(indentation); b.Put("    }\n\n");

    b.Put(indentation); b.Put("    public  addElement(element : ");
                                 b.Put(option -> ast_type);
                                 b.Put(") : void\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        this.list.add(element);\n");
    b.Put(indentation); b.Put("        if (this.leftRecursive)\n");
    b.Put(indentation); b.Put("             this.rightIToken = element.getRightIToken();\n");
    b.Put(indentation); b.Put("        else this.leftIToken = element.getLeftIToken();\n");
    b.Put(indentation); b.Put("    }\n\n");





    // generate constructors for list class
    b.Put(indentation); b.Put("    constructor(leftToken : IToken, rightToken : IToken , leftRecursive : boolean )");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("          super(leftToken, rightToken);\n");
    b.Put(indentation); b.Put("          this.leftRecursive = leftRecursive;\n");
    b.Put(indentation); b.Put("    }\n\n");

  
    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("    /**\n");
        b.Put(indentation); b.Put("     * Make a copy of the list and return it. Note that we obtain the local list by\n");
        b.Put(indentation); b.Put("     * invoking getArrayList so as to make sure that the list we return is in proper order.\n");
        b.Put(indentation); b.Put("     */\n");
        b.Put(indentation); b.Put("     public   getAllChildren() : Lpg.Util.ArrayList<IAst>\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        return this.getArrayList().clone();\n");
        b.Put(indentation); b.Put("    }\n\n");
    }

    //
    // Implementation for functions in Lpg.Util.ArrayList<IAst>
    //

    Substitutions subs;
    subs["%%AstType%%"] = option->ast_type;
    subs["%%ListClassName%%"] = classname;
    b.Put(indentation); b.Put("}\n\n");

    
  

    return;
}


//
// Generate the the Ast token class
//
void TypeScriptAction::GenerateAstTokenType(NTC &ntc, ActionFileSymbol* ast_filename_symbol,
                                      const char *indentation,
                                      const char *classname)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    /*
     * Generate the Token root class
     */
    b.Put(indentation); 
                                 b.Put("export class ");
                                 b.Put(classname);
                                 b.Put(" extends ");
                                 b.Put(option -> ast_type);
                                 b.Put(" implements I");
                                 b.Put(classname);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    b.Put(indentation); b.Put("    "); b.Put("constructor(token : IToken ) { super(token); }\n");
    b.Put(indentation); b.Put("    public  getIToken() : IToken{ return this.leftIToken; }\n");
    b.Put(indentation); b.Put("    public  toString() : string  { return this.leftIToken.toString(); }\n\n");

    ClassnameElement element; // generate a temporary element with no symbols in its symbol set.
    element.real_name = (char *) classname;
    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);

    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("    /**\n");
        b.Put(indentation); b.Put("     * A token class has no children. So, we return the empty list.\n");
        b.Put(indentation); b.Put("     */\n");
        b.Put(indentation); b.Put("     public   getAllChildren() : Lpg.Util.ArrayList<IAst> { return new Lpg.Util.ArrayList<IAst>(); }\n\n");
    }

    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set);

    b.Put(indentation); b.Put("}\n\n");
    
    return;
}


//
//
//
void TypeScriptAction::GenerateCommentHeader(TextBuffer &b,
                                       const char *indentation,
                                       Tuple<int> &ungenerated_rule,
                                       Tuple<int> &generated_rule)
{
    BlockSymbol* scope_block = nullptr;
    const char* rule_info = rule_info_holder.c_str();

    b.Put(indentation); b.Put("/**");
    if (ungenerated_rule.Length() > 0)
    {
        b.Put("\n");
        b.Put(indentation);
        b.Put(" *<em>");
        for (int i = 0; i < ungenerated_rule.Length(); i++)
        {
            int rule_no = ungenerated_rule[i];

            LexStream::TokenIndex separator_token = grammar -> parser.rules[grammar -> rules[rule_no].source_index].separator_index;
            int line_no = lex_stream -> Line(separator_token),
                start = lex_stream -> StartLocation(separator_token),
                end   = lex_stream -> EndLocation(separator_token) + 1;
            const char *start_cursor_location = &(lex_stream -> InputBuffer(separator_token)[start]),
                       *end_cursor_location = &(lex_stream -> InputBuffer(separator_token)[end]);

            b.Put("\n");
            b.Put(indentation);
            ProcessActionLine(scope_block, ActionBlockElement::BODY,
                              &b,
                              lex_stream -> FileName(separator_token),
                              rule_info,
                              &rule_info[strlen(rule_info)],
                              rule_no,
                              lex_stream -> FileName(separator_token),
                              line_no,
                              start_cursor_location,
                              end_cursor_location);
        }
        b.Put("\n");
        b.Put(indentation);
        b.Put(" *</em>\n");
        b.Put(indentation);
        b.Put(" *<p>");
    }
    b.Put("\n");
    b.Put(indentation);
    b.Put(" *<b>");
    for (int i = 0; i < generated_rule.Length(); i++)
    {
        int rule_no = generated_rule[i];

            LexStream::TokenIndex separator_token = grammar -> parser.rules[grammar -> rules[rule_no].source_index].separator_index;
            int line_no = lex_stream -> Line(separator_token),
                start = lex_stream -> StartLocation(separator_token),
                end   = lex_stream -> EndLocation(separator_token) + 1;
            const char *start_cursor_location = &(lex_stream -> InputBuffer(separator_token)[start]),
                       *end_cursor_location = &(lex_stream -> InputBuffer(separator_token)[end]);

        b.Put("\n");
        b.Put(indentation);
        ProcessActionLine(scope_block, ActionBlockElement::BODY,
                          &b,
                          lex_stream -> FileName(separator_token), // option -> DefaultBlock() -> ActionfileSymbol() -> Name(),
                          rule_info,
                          &rule_info[strlen(rule_info)],
                          rule_no,
                          lex_stream -> FileName(separator_token),
                          line_no,
                          start_cursor_location,
                          end_cursor_location);
    }

    b.Put("\n");
    b.Put(indentation);
    b.Put(" *</b>\n");
    b.Put(indentation);
    b.Put(" */\n");
}


void TypeScriptAction::GenerateListMethods(CTC &ctc,
                                     NTC &ntc,
                                     TextBuffer &b,
                                     const char *indentation,
                                     const char *classname,
                                     ClassnameElement &element,
                                     Array<const char *> &)
{
    const char *element_name = element.array_element_type_symbol -> Name(),
               *element_type = ctc.FindBestTypeFor(element.array_element_type_symbol -> SymbolIndex());

    //
    // Generate ADD method
    //
    b.Put(indentation); b.Put("    public ");
                                 b.Put(" addElement(");
                                 b.Put(" _");
                                 b.Put(element_name);
                                 b.Put(" : ");
                                 b.Put(element_type);
                                 b.Put(") : void\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        super.addElement(<");
                                 b.Put(option -> ast_type);
                                 b.Put("> _");
                                 b.Put(element_name);
                                 b.Put(");\n");
    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("        ");
        if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
        {
            b.Put("if (this._");
            b.Put(element_name);
            b.Put(") ");
        }
        b.Put("(<");
        b.Put(option -> ast_type);
        b.Put("> _");
        b.Put(element_name);
        b.Put(").setParent(this);\n");
    }
    b.Put(indentation); b.Put("    }\n");

    b.Put("\n");
   
    //
    // Generate visitor methods.
    //
    if (option -> visitor == Option::DEFAULT)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("    public  acceptWithVisitor(v : ");
                                     b.Put(option -> visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
            b.Put(") :void { for (let i = 0; i < this.size(); i++) v.visit"
                           "("
                           "this.get");
            b.Put(element_name);
            b.Put("At(i)"
                           "); }\n");
        }
        else
        {
            b.Put(") : void{ for (let i = 0; i < this.size(); i++) this.get");
            b.Put(element_name);
            b.Put("At(i).acceptWithVisitor(v); }\n");
        }

        b.Put(indentation); b.Put("    public  acceptWithArg(v : Argument");
                                     b.Put(option -> visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
            b.Put(", o : any)  : void{ for (let i = 0; i < this.size(); i++) v.visit"
                           "("
                           "this.get");
            b.Put(element_name);
            b.Put("At(i), o");
            b.Put("); }\n");
        }
        else
        {
            b.Put(", o : any) : void { for (let i = 0; i < this.size(); i++) this.get");
            b.Put(element_name);
            b.Put("At(i).acceptWithArg(v, o); }\n");
        }

        //
        // Code cannot be generated to automatically visit a node that
        // can return a value. These cases are left up to the user.
        //
        b.Put(indentation); b.Put("    public  acceptWithResult(v : Result");
                                     b.Put(option -> visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
                                         b.Put(") : any\n");
            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        let result = new Lpg.Util.ArrayList<IAst>();\n");
            b.Put(indentation); b.Put("        for (let i = 0; i < this.size(); i++)\n");
            b.Put(indentation); b.Put("            result.add(v.visit(this.get");
                                         b.Put(element_name);
                                         b.Put("At(i)));\n");
            b.Put(indentation); b.Put("        return result;\n");
            b.Put(indentation); b.Put("    }\n");
        }
        else
        {
                                         b.Put(") : any\n");
            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        let result = new Lpg.Util.ArrayList<IAst>();\n");
            b.Put(indentation); b.Put("        for (let i = 0; i < this.size(); i++)\n");
            b.Put(indentation); b.Put("            result.add(this.get");
                                         b.Put(element_name);
                                         b.Put("At(i).acceptWithResult(v));\n");
            b.Put(indentation); b.Put("        return result;\n");
            b.Put(indentation); b.Put("    }\n");
        }

        b.Put(indentation); b.Put("    public  acceptWithResultArgument(v: ResultArgument");
                                     b.Put(option -> visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
                                         b.Put(", o : any) : any\n");
            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        let result = new Lpg.Util.ArrayList<IAst>();\n");
            b.Put(indentation); b.Put("        for (let i = 0; i < this.size(); i++)\n");
            b.Put(indentation); b.Put("            result.add(v.visit(this.get");
                                         b.Put(element_name);
                                         b.Put("At(i), o));\n");
            b.Put(indentation); b.Put("        return result;\n");
            b.Put(indentation); b.Put("    }\n");
        }
        else
        {
                                         b.Put(", o : any) : any \n");
            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        let result = new Lpg.Util.ArrayList<IAst>();\n");
            b.Put(indentation); b.Put("        for (let i = 0; i < this.size(); i++)\n");
            b.Put(indentation); b.Put("            result.add(this.get");
                                         b.Put(element_name);
                                         b.Put("At(i).acceptWithResultArgument(v, o));\n");
            b.Put(indentation); b.Put("        return result;\n");
            b.Put(indentation); b.Put("    }\n");
        }
    }
    else if (option -> visitor == Option::PREORDER)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("    public  accept(v : IAstVisitor ) : void\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        if (! v.preVisit(this)) return;\n");
        b.Put(indentation); b.Put("        this.enter(<");
                                     b.Put(option -> visitor_type);
                                     b.Put("> v);\n");
        b.Put(indentation); b.Put("        v.postVisit(this);\n");
        b.Put(indentation); b.Put("    }\n");
        b.Put(indentation); b.Put("    public enter(v : ");
                                     b.Put(option -> visitor_type);
                                     b.Put(") : void\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        let checkChildren = v.visit").Put(classname).Put("(this);\n");
        b.Put(indentation); b.Put("        if (checkChildren)\n");
        b.Put(indentation); b.Put("        {\n");
        b.Put(indentation); b.Put("            for (let i = 0; i < this.size(); i++)\n");
        b.Put(indentation); b.Put("            {\n");
        b.Put(indentation); b.Put("                ");

        const char *element_typename = ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex());
        if (element_typename != NULL)
        {
            //b.Put(element_typename);
            b.Put(" let element = this.get");
            b.Put(element_name);
            b.Put("At(i);\n");
            if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
            {
                b.Put(indentation); b.Put("                if (element)");
                b.Put(indentation); b.Put("                {\n");
                b.Put(indentation); b.Put("                    if (! v.preVisit(element)) continue;\n");
                b.Put(indentation); b.Put("                    element.enter(v);\n");
                b.Put(indentation); b.Put("                    v.postVisit(element);\n");
                b.Put(indentation); b.Put("                }\n");
            }
            else
            {
                b.Put(indentation); b.Put("                if (! v.preVisit(element)) continue;\n");
                b.Put(indentation); b.Put("                element.enter(v);\n");
                b.Put(indentation); b.Put("                v.postVisit(element);\n");
            }
        }
        else
        {
            //b.Put(typestring[element.array_element_type_symbol -> SymbolIndex()]);
            b.Put("let element = this.get");
            b.Put(element_name);
            b.Put("At(i);\n");
            b.Put(indentation); b.Put("                ");
            if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
                b.Put("if (element) ");
            b.Put("element.accept(v);\n");
        }
        b.Put(indentation); b.Put("            }\n");
        b.Put(indentation); b.Put("        }\n");
        b.Put(indentation); b.Put("        v.endVisit").Put(classname).Put("(this);\n");
        b.Put(indentation); b.Put("    }\n");
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
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    Tuple<int> &interface = element.interface_;
    assert(element.array_element_type_symbol != NULL);
    const char *classname = element.real_name,
               *element_name = element.array_element_type_symbol -> Name(),
               *element_type = ctc.FindBestTypeFor(element.array_element_type_symbol -> SymbolIndex());

    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);

    b.Put(indentation); 
                                 b.Put("export class ");
                                 b.Put(classname);
                                 b.Put(" extends ");
                                 b.Put(this -> abstract_ast_list_classname);
                                 b.Put(" implements ");
    for (int i = 0; i < interface.Length() - 1; i++)
    {
        b.Put(typestring[element.interface_[i]]);
        b.Put(", ");
    }
    b.Put(typestring[element.interface_[interface.Length() - 1]]);
    b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
    {
        b.Put(indentation); b.Put("    /**\n");
        b.Put(indentation); b.Put("     * The value returned by <b>get");
                                     b.Put(element_name);
                                     b.Put("At</b> may be <b>null</b>\n");
        b.Put(indentation); b.Put("     */\n");
    }
    b.Put(indentation); b.Put("    public ");
                                 b.Put(" get");
                                 b.Put(element_name);
                                 b.Put("At(i : number) : ");
                                 b.Put(element_type);
                                 b.Put("{ return <");
                                 b.Put(element_type);
                                 b.Put("> this.getElementAt(i); }\n\n");

    //
    // generate constructors
    //
	b.Put(indentation); b.Put("    constructor(leftToken : IToken, rightToken : IToken , leftRecursive : boolean )\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        super(leftToken, rightToken, leftRecursive);\n");
    b.Put(indentation); b.Put("    }\n\n");

    b.Put(indentation); b.Put("   public static ");b.Put(classname); b.Put("fromElement(");
    b.Put("element");
    b.Put(" : ");
    b.Put(element_type);
    b.Put(",leftRecursive : boolean) : ");  b.Put(classname); b.Put("\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        let obj = new ");b.Put(classname);
	b.Put("(element.getLeftIToken(),element.getRightIToken(), leftRecursive);\n");
    b.Put(indentation); b.Put("        obj.list.add(element);\n");
    if (option->parent_saved)
    {
        b.Put(indentation); b.Put("        ");
        if (ntc.CanProduceNullAst(element.array_element_type_symbol->SymbolIndex()))
        {
            b.Put("if (element)");
        }
        b.Put("(<");
        b.Put(option->ast_type);
        b.Put("> element");
        b.Put(").setParent(obj);\n");
    }
    b.Put(indentation); b.Put("        return obj;\n");
    b.Put(indentation); b.Put("    }\n");
    b.Put("\n");



    GenerateListMethods(ctc, ntc, b, indentation, classname, element, typestring);

   b.Put("    }\n\n");// Generate Class Closer
    

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
    Array<const char*>&)

{
    TextBuffer& b = *GetBuffer(ast_filename_symbol);
    const char* classname = element.real_name,
        //* element_name = element.array_element_type_symbol->Name(),
        * element_type = ctc.FindBestTypeFor(element.array_element_type_symbol->SymbolIndex());

    GenerateCommentHeader(b, indentation, element.ungenerated_rule, special_array.rules);

    b.Put(indentation); 
    b.Put("export class ");
    b.Put(special_array.name);
    b.Put(" extends ");
    b.Put(classname);
    b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    GenerateEnvironmentDeclaration(b, indentation);


    b.Put(indentation); b.Put("    constructor(");
    b.Put("environment : "); b.Put(option->action_type);
    b.Put(", leftIToken : IToken,  rightIToken : IToken, leftRecursive : boolean)\n");

    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        super(leftIToken, rightIToken, leftRecursive);\n");
    b.Put(indentation); b.Put("        this.environment = environment;\n");
    b.Put(indentation); b.Put("        this.initialize();\n");
    b.Put(indentation); b.Put("    }\n\n");


    b.Put(indentation); b.Put("   public static  "); b.Put(special_array.name); b.Put("fromElement(environment : ");
    b.Put(option->action_type);
    b.Put(",element");
    b.Put(" : ");
    b.Put(element_type);
    b.Put(",leftRecursive : boolean) : ");  b.Put(special_array.name); b.Put("\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        let obj = new "); b.Put(special_array.name);
    b.Put("(environment,element.getLeftIToken(),element.getRightIToken(), leftRecursive);\n");
    b.Put(indentation); b.Put("        obj.list.add(element);\n");
    if (option->parent_saved)
    {
        b.Put(indentation); b.Put("        ");
        if (ntc.CanProduceNullAst(element.array_element_type_symbol->SymbolIndex()))
        {
            b.Put("if (element)");
        }
        b.Put("(<");
        b.Put(option->ast_type);
        b.Put("> element");
        b.Put(").setParent(obj);\n");
    }
    b.Put(indentation); b.Put("        return obj;\n");
    b.Put(indentation); b.Put("    }\n");
    b.Put("\n");



    //GenerateListMethods(ctc, ntc, b, indentation, classname, element, typestring);

    b.Put("    }\n\n");// Generate Class Closer
    

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
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    char *classname = element.real_name;
    SymbolLookupTable &symbol_set = element.symbol_set;
    Tuple<int> &rhs_type_index = element.rhs_type_index;

    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);

    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);

    assert(element.rule.Length() == 1);
    int rule_no = element.rule[0];

    b.Put(indentation); 
                                 b.Put("export class ");
                                 b.Put(classname);
                                 b.Put(" extends ");
    if (element.is_terminal_class)
    {
        b.Put(grammar -> Get_ast_token_classname());
        b.Put(" implements ");
        b.Put(typestring[grammar -> rules[rule_no].lhs]);
        b.Put("\n");
        b.Put(indentation); b.Put("{\n");
        if (element.needs_environment)
            GenerateEnvironmentDeclaration(b, indentation);
        if (symbol_set.Size() == 1) // if the right-hand side contains a symbol ...
        {
            b.Put(indentation); b.Put("    public  get");
                                         b.Put(symbol_set[0] -> Name());
                                         b.Put("() : IToken { return this.leftIToken; }\n\n");
        }
        b.Put(indentation);  b.Put("constructor(");
        if (element.needs_environment)
        {
            b.Put("environment : ");
            b.Put(option -> action_type);
            b.Put(", token : IToken )");

            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        super(token)\n");
            b.Put(indentation); b.Put("        this.environment = environment;\n");
            b.Put(indentation); b.Put("        this.initialize();\n");
            b.Put(indentation); b.Put("    }\n");
        }
        else b.Put("token : IToken ) { super(token); this.initialize(); }\n");
    }
    else 
    {
        b.Put(option -> ast_type);
        b.Put(" implements ");
        b.Put(typestring[grammar -> rules[rule_no].lhs]);
        b.Put("\n");
        b.Put(indentation); b.Put("{\n");
        if (element.needs_environment)
            GenerateEnvironmentDeclaration(b, indentation);

        if (symbol_set.Size() > 0)
        {
            {
                for (int i = 0; i < symbol_set.Size(); i++)
                {
                    b.Put(indentation); b.Put("    private ");
                                                 b.Put(" _");
                                                 b.Put(symbol_set[i] -> Name());
                                                 b.Put(" : ");
                                                 b.Put(ctc.FindBestTypeFor(rhs_type_index[i]));
                                                 if (ntc.CanProduceNullAst(rhs_type_index[i]))
                                                     b.Put("| null");
                                                 b.Put(";\n");
                }
            }
            b.Put("\n");

            {
                for (int i = 0; i < symbol_set.Size(); i++)
                {
                    const char *symbolName = symbol_set[i] -> Name();
                    const char *bestType = ctc.FindBestTypeFor(rhs_type_index[i]);
                    bool nullAst = false;
                    if (ntc.CanProduceNullAst(rhs_type_index[i]))
                    {
                        b.Put(indentation); b.Put("    /**\n");
                        b.Put(indentation); b.Put("     * The value returned by <b>get");
                                                     b.Put(symbolName);
                                                     b.Put("</b> may be <b>null</b>\n");
                        b.Put(indentation); b.Put("     */\n");
                        nullAst = true;
                    }

                    // Generate getter method
                    b.Put(indentation); b.Put("    public ");
                                                 b.Put(" get");
                                                 b.Put(symbolName);
                                                 b.Put("()");
                                                 b.Put(" : ");
                                                 b.Put(bestType);
                                                 if(nullAst)  b.Put(" | null ");
                                                 b.Put("{ return this._");
                                                 b.Put(symbolName);
                                                 b.Put("; }\n");

                    // Generate setter method
                    b.Put(indentation); b.Put("    public  set");
                    b.Put(symbolName);
                    b.Put("(");
                    b.Put(" _"); // add "_" prefix to arg name in case symbol happens to be a Java keyword
                    b.Put(symbolName);
                    b.Put(" : ");
                    b.Put(bestType);
                    b.Put(") : void");
                    b.Put(" { this._");
                    b.Put(symbolName);
                    b.Put(" = _");
                    b.Put(symbolName);
                    b.Put("; }\n");
                }
            }
            b.Put("\n");
        }

        //
        // generate constructor
        //
        const char *header = "    constructor";
        b.Put(indentation);
        b.Put(header);
       
        int length = strlen(indentation) + strlen(header);

        b.Put("(");
        if (element.needs_environment)
        {
            b.Put("environment : ");
            b.Put(option -> action_type);
            b.Put(",");
        }
        b.Put("leftIToken : IToken , rightIToken : IToken ");
        b.Put(symbol_set.Size() == 0 ? ")\n" : ",\n");
        {
            for (int i = 0; i < symbol_set.Size(); i++)
            {
                for (int k = 0; k <= length; k++)
                    b.PutChar(' ');
             
                b.Put(" _");
                b.Put(symbol_set[i] -> Name());
                b.Put(" : ");
                b.Put(ctc.FindBestTypeFor(rhs_type_index[i]));
                if (ntc.CanProduceNullAst(rhs_type_index[i]))
                {
                    b.Put("| null");
                }
                b.Put(i == symbol_set.Size() - 1 ? ")\n" : ",\n");
            }
        }

        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        super(leftIToken, rightIToken)\n\n");
        if (element.needs_environment)
        {
            b.Put(indentation);
            b.Put("        this.environment = environment;\n");
        }

        {
            for (int i = 0; i < symbol_set.Size(); i++)
            {
                b.Put(indentation); b.Put("        this._");
                                             b.Put(symbol_set[i] -> Name());
                                             b.Put(" = _");
                                             b.Put(symbol_set[i] -> Name());
                                             b.Put(";\n");

                if (option -> parent_saved)
                {
                    b.Put(indentation); b.Put("        ");
                    if ((! optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
                    {
                        b.Put("if (_");
                        b.Put(symbol_set[i] -> Name());
                        b.Put(") ");
                    }
    
                    b.Put("(<");
                    b.Put(option -> ast_type);
                    b.Put("> _");
                    b.Put(symbol_set[i] -> Name());
                    b.Put(").setParent(this);\n");
                }
            }
        }

        b.Put(indentation); b.Put("        this.initialize();\n");
        b.Put(indentation); b.Put("    }\n");
    }

    if (option -> parent_saved)
        GenerateGetAllChildrenMethod(b, indentation, element);
   
    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set);
   b.Put("    }\n\n");// Generate Class Closer
    

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
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    char *classname = element.real_name;
    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);

    b.Put(indentation); 
                                 b.Put("export class ");
                                 b.Put(classname);
                                 b.Put(" extends ");
                                 b.Put(grammar -> Get_ast_token_classname());
                                 b.Put(" implements ");
    for (int i = 0; i < element.interface_.Length() - 1; i++)
    {
        b.Put(typestring[element.interface_[i]]);
        b.Put(", ");
    }
    b.Put(typestring[element.interface_[element.interface_.Length() - 1]]);
    b.Put("\n");
    b.Put(indentation); b.Put("{\n");
    if (element.needs_environment)
        GenerateEnvironmentDeclaration(b, indentation);
    SymbolLookupTable &symbol_set = element.symbol_set;
    if (symbol_set.Size() == 1) // if the right-hand side contains a symbol ...
    {
        b.Put(indentation); b.Put("    public  get");
                                     b.Put(symbol_set[0] -> Name());
                                     b.Put("() : IToken{ return this.leftIToken; }\n\n");
    }
    b.Put(indentation); b.Put("    constructor(");
                                 if (element.needs_environment)
                                 {
                                     b.Put("environment : ");
                                     b.Put(option -> action_type);
                                     b.Put(", token : IToken )");

                                     b.Put(indentation); b.Put("    {\n");
                                     b.Put(indentation); b.Put("        super(token);\n");
                                     b.Put(indentation); b.Put("        this.environment = environment;\n");
                                     b.Put(indentation); b.Put("        this.initialize();\n");
                                     b.Put(indentation); b.Put("    }\n");
                                 }
                                 else b.Put("token : IToken ){ super(token); initialize(); }\n");

    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);
  
    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set);

   b.Put("    }\n\n");// Generate Class Closer
    

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
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    char *classname = element.real_name;
    SymbolLookupTable &symbol_set = element.symbol_set;
    Tuple<int> &rhs_type_index = element.rhs_type_index;

    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);

    b.Put(indentation); 
                                 b.Put("export class ");
                                 b.Put(classname);
                                 b.Put(" extends ");
                                 b.Put(option -> ast_type);
                                 b.Put(" implements ");
    {
        for (int i = 0; i < element.interface_.Length() - 1; i++)
        {
            b.Put(typestring[element.interface_[i]]);
            b.Put(", ");
        }
    }
    b.Put(typestring[element.interface_[element.interface_.Length() - 1]]);
    b.Put("\n");
    b.Put(indentation); b.Put("{\n");
    if (element.needs_environment)
        GenerateEnvironmentDeclaration(b, indentation);


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
            b.Put(indentation); b.Put("    private ");
                                         b.Put(" _");
                                         b.Put(symbol_set[i] -> Name());

                                         b.Put(" : ");
                                         b.Put(ctc.FindBestTypeFor(rhs_type_index[i]));
                                         if ((!optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
                                             b.Put(" | null");
                                         b.Put(";\n");
        }
    }
    b.Put("\n");



    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            bool nullAst = false;
            if ((! optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
            {
                b.Put(indentation); b.Put("    /**\n");
                b.Put(indentation); b.Put("     * The value returned by <b>get");
                                             b.Put(symbol_set[i] -> Name());
                                             b.Put("</b> may be <b>null</b>\n");
                b.Put(indentation); b.Put("     */\n");
                nullAst = true;
            }

            b.Put(indentation); b.Put("    public ");
                                         b.Put(" get");
                                         b.Put(symbol_set[i] -> Name());
                                         b.Put("() : ");
                                         b.Put(ctc.FindBestTypeFor(rhs_type_index[i]));
							  if(nullAst)b.Put(" | null ");
                                         b.Put("{ return this._");
                                         b.Put(symbol_set[i] -> Name());
                                         b.Put("; }\n");
        }
    }
    b.Put("\n");


    //
    // generate merged constructor
    //
    const char *header = "    constructor";
    b.Put(indentation);
    b.Put(header);
    int length = strlen(indentation) + strlen(header);

    b.Put("(");
    if (element.needs_environment)
    {
        b.Put("environment : ");
        b.Put(option -> action_type);
        b.Put(", ");
    }
    b.Put("leftIToken : IToken , rightIToken : IToken ");
    b.Put(symbol_set.Size() == 0 ? ")\n" : ",\n");
    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            for (int k = 0; k <= length; k++)
                b.PutChar(' ');
            b.Put(" _");
            b.Put(symbol_set[i] -> Name());
            b.Put(" : ");
            b.Put(ctc.FindBestTypeFor(rhs_type_index[i]));
            if ((!optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
            {
                b.Put(" | null");
            }
         
            b.Put(i == symbol_set.Size() - 1 ? ")\n" : ",\n");
        }
    }

    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        super(leftIToken, rightIToken);\n\n");
    if (element.needs_environment)
    {
        b.Put(indentation);
        b.Put("        this.environment = environment;\n");
    }

    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            b.Put(indentation); b.Put("        this._");
                                         b.Put(symbol_set[i] -> Name());
                                         b.Put(" = _");
                                         b.Put(symbol_set[i] -> Name());
                                         b.Put(";\n");
    
            if (option -> parent_saved)
            {
                b.Put(indentation); b.Put("        ");
                if ((! optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
                {
                    b.Put("if (_");
                    b.Put(symbol_set[i] -> Name());
                    b.Put(") ");
                }
    
                b.Put("(<");
                b.Put(option -> ast_type);
                b.Put("> _");
                b.Put(symbol_set[i] -> Name());
                b.Put(").setParent(this);\n");
            }
        }
    }

    b.Put(indentation); b.Put("        this.initialize();\n");
    b.Put(indentation); b.Put("    }\n");

    if (option -> parent_saved)
        GenerateGetAllChildrenMethod(b, indentation, element);
  
    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set);

   b.Put("    }\n\n");// Generate Class Closer
    

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
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    b.Put(indentation); b.Put("export interface ");
    b.Put(astRootInterfaceName.c_str());
    
    b.Put("\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("     getLeftIToken() : IToken;\n");
    b.Put(indentation); b.Put("     getRightIToken() : IToken;\n");
    b.Put("\n");
    GenerateVisitorHeaders(b, indentation, "    ");
    b.Put(indentation); b.Put("}\n\n");
    
    
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
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    b.Put(indentation); b.Put("/**");
    if (is_terminal)
    {
        b.Put("\n");
        b.Put(indentation);  b.Put(" * is always implemented by <b>");
                                      b.Put(grammar -> Get_ast_token_classname());
                                      b.Put("</b>. It is also implemented by");
    }
    else 
    {
        b.Put("\n");
        b.Put(indentation);
        b.Put(" * is implemented by");
    }

    if (classes.Length() == 1)
    {
        b.Put(" <b>");
        b.Put(classname[classes[0]].real_name);
        b.Put("</b>");
    }
    else
    {
        b.Put(":\n");
        b.Put(indentation);
        b.Put(" *<b>\n");
        b.Put(indentation); b.Put(" *<ul>");
        for (int i = 0; i < classes.Length(); i++)
        {
            b.Put("\n");
            b.Put(indentation);
            b.Put(" *<li>");
            b.Put(classname[classes[i]].real_name);
        }
        b.Put("\n");
        b.Put(indentation);
        b.Put(" *</ul>\n");
        b.Put(indentation);
        b.Put(" *</b>");
    }

    b.Put("\n");
    b.Put(indentation);
    b.Put(" */\n");

    b.Put(indentation); b.Put("export interface ");
                                 b.Put(interface_name);
    if (extension.Length() > 0)
    {
        b.Put(" extends ");
        for (int k = 0; k < extension.Length() - 1; k++)
        {
            b.PutChar('I');
            b.Put(extension[k] == grammar -> Get_ast_token_interface()
                               ? grammar -> Get_ast_token_classname()
                               : grammar -> RetrieveString(extension[k]));
            b.Put(", ");
        }
        b.PutChar('I');
        b.Put(extension[extension.Length() - 1] == grammar -> Get_ast_token_interface()
                               ? grammar -> Get_ast_token_classname()
                               : grammar -> RetrieveString(extension[extension.Length() - 1]));
        b.Put(" {}\n\n");
    }
    else
    {
        b.Put(" extends ");
        b.Put(astRootInterfaceName.c_str());
        b.Put(indentation); b.Put("{\n");
      
        b.Put(indentation); b.Put("}\n\n");
    }
	
    
    return;
}


//
//
//
void TypeScriptAction::GenerateNullAstAllocation(TextBuffer &b, int rule_no)
{
    const char *code = "\n                    this.setResult(null);";
    GenerateCode(&b, code, rule_no);

    return;
}


//
//
//
void TypeScriptAction::GenerateAstAllocation(CTC &ctc,
                                             NTC&,
                                             TextBuffer &b,
                                             RuleAllocationElement &allocation_element,
                                             Tuple<ProcessedRuleElement> &processed_rule_elements,
                                             Array<const char *> &, int rule_no)
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
    //        GenerateCode(&b, space, rule_no);
    //        GenerateCode(&b, "// When garbage collection is not available, delete ", rule_no);
    //        GenerateCode(&b, "getRhsSym(", rule_no);
    //        IntToString index(position[0]);
    //        GenerateCode(&b, index.string(), rule_no);
    //        GenerateCode(&b, rparen, rule_no);
    //    }
    //
    if (allocation_element.is_terminal_class && (grammar -> RhsSize(rule_no) == 1 && grammar -> IsNonTerminal(grammar -> rhs_sym[grammar -> FirstRhsIndex(rule_no)])))
    {
        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, "//", rule_no);
        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, "// When garbage collection is not available, delete ", rule_no);
        GenerateCode(&b, "this.getRhsSym(1)", rule_no);
        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, "//", rule_no);
    }
    GenerateCode(&b, space, rule_no);
    GenerateCode(&b, "this.setResult(", rule_no);
    GenerateCode(&b, space, rule_no);
    GenerateCode(&b, space4, rule_no);
    GenerateCode(&b, current_line_input_file_info.c_str(), rule_no);
    GenerateCode(&b, space, rule_no);
    GenerateCode(&b, space4, rule_no);

    GenerateCode(&b, newkey, rule_no);
    GenerateCode(&b, classname, rule_no);
    GenerateCode(&b, lparen, rule_no);
    if (allocation_element.needs_environment)
    {
        GenerateCode(&b, "this, ", rule_no);
    }
    if (allocation_element.is_terminal_class)
    {
        GenerateCode(&b, "this.getRhsIToken(1)", rule_no);
        //
        // TODO: Old bad idea. Remove at some point...
        //
        //
        //        assert(position.Length() <= 1);
        //
        //        GenerateCode(&b, "getRhsIToken(", rule_no);
        //        IntToString index(position.Length() == 0 ? 1 : position[0]);
        //        GenerateCode(&b, index.string(), rule_no);
        //        GenerateCode(&b, rparen, rule_no);
        //
    }
    else
    {
        GenerateCode(&b, "this.getLeftIToken()", rule_no);
        GenerateCode(&b, ", ", rule_no);
        GenerateCode(&b, "this.getRightIToken()", rule_no);
        if (position.Length() > 0)
        {
            GenerateCode(&b, comma, rule_no);
            GenerateCode(&b, extra_space, rule_no);
            GenerateCode(&b, current_line_input_file_info.c_str(), rule_no);
            GenerateCode(&b, extra_space, rule_no);

            int offset = grammar -> FirstRhsIndex(rule_no) - 1;
            for (int i = 0; i < position.Length(); i++)
            {
                if (position[i] == 0)
                {
             /*       GenerateCode(&b, lparen, rule_no);
                    GenerateCode(&b, ctc.FindBestTypeFor(type_index[i]), rule_no);
                    GenerateCode(&b, rparen, rule_no);*/
                    GenerateCode(&b, "null", rule_no);
                }
                else
                {
                    int symbol = grammar -> rhs_sym[offset + position[i]];
                    if (grammar -> IsTerminal(symbol))
                    {
                        const char *actual_type = ctc.FindBestTypeFor(type_index[i]);

                        if (strcmp(actual_type, grammar -> Get_ast_token_classname()) != 0)
                        {
                            GenerateCode(&b, "<", rule_no);
                            GenerateCode(&b, actual_type, rule_no);
                            GenerateCode(&b, ">", rule_no);
                        }

                        GenerateCode(&b, newkey, rule_no);
                        GenerateCode(&b, grammar -> Get_ast_token_classname(), rule_no);
                        GenerateCode(&b, lparen, rule_no);
                        GenerateCode(&b, "this.getRhsIToken(", rule_no);
                        IntToString index(position[i]);
                        GenerateCode(&b, index.String(), rule_no);
                        GenerateCode(&b, rparen, rule_no);
                    }
                    else
                    {
                        GenerateCode(&b, "<", rule_no);
                        GenerateCode(&b, ctc.FindBestTypeFor(type_index[i]), rule_no);
                        GenerateCode(&b, ">", rule_no);
                        GenerateCode(&b, "this.getRhsSym(", rule_no);
                        IntToString index(position[i]);
                        GenerateCode(&b, index.String(), rule_no);
                    }
    
                    GenerateCode(&b, rparen, rule_no);
                }
        
                if (i != position.Length() - 1)
                {
                    GenerateCode(&b, comma, rule_no);
                    GenerateCode(&b, extra_space, rule_no);
                    GenerateCode(&b, current_line_input_file_info.c_str(), rule_no);
                    GenerateCode(&b, extra_space, rule_no);
                }
            }
        }
    }

    GenerateCode(&b, rparen, rule_no);
    GenerateCode(&b, space, rule_no);
    GenerateCode(&b, current_line_input_file_info.c_str(), rule_no);
    GenerateCode(&b, space, rule_no);
    GenerateCode(&b, trailer, rule_no);

    delete [] extra_space;

    return;
}

//
//
//
void TypeScriptAction::GenerateListAllocation(CTC &ctc,
                                              NTC&,
                                              TextBuffer &b,
                                              int rule_no, RuleAllocationElement &allocation_element)
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

        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, "this.setResult(", rule_no);
        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, space4, rule_no);
        GenerateCode(&b, current_line_input_file_info.c_str(), rule_no);
        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, space4, rule_no);


        if (allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_EMPTY ||
            allocation_element.list_kind == RuleAllocationElement::RIGHT_RECURSIVE_EMPTY)
        {

 

            GenerateCode(&b, newkey, rule_no);
            GenerateCode(&b, allocation_element.name, rule_no);
            GenerateCode(&b, lparen, rule_no);
            if (allocation_element.needs_environment)
            {
                GenerateCode(&b, "this, ", rule_no);
            }

            GenerateCode(&b, "this.getLeftIToken()", rule_no);
            GenerateCode(&b, ", ", rule_no);
            GenerateCode(&b, "this.getRightIToken()", rule_no);
            GenerateCode(&b, comma, rule_no);
            if (allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_EMPTY)
                 GenerateCode(&b, " true /* left recursive */", rule_no);
            else GenerateCode(&b, " false /* not left recursive */", rule_no);
        }
        else
        {
         
            GenerateCode(&b, allocation_element.name, rule_no);
            GenerateCode(&b, ".", rule_no);
            GenerateCode(&b, allocation_element.name, rule_no);
            GenerateCode(&b, "fromElement", rule_no);
            GenerateCode(&b, lparen, rule_no);
            if (allocation_element.needs_environment)
            {
                GenerateCode(&b, "this, ", rule_no);
            }
            assert(allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_SINGLETON ||
                   allocation_element.list_kind == RuleAllocationElement::RIGHT_RECURSIVE_SINGLETON);

            if (grammar -> IsTerminal(allocation_element.element_symbol))
            {
                GenerateCode(&b, newkey, rule_no);
                GenerateCode(&b, grammar -> Get_ast_token_classname(), rule_no);
                GenerateCode(&b, lparen, rule_no);
                GenerateCode(&b, "this.getRhsIToken(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&b, index.String(), rule_no);
                GenerateCode(&b, rparen, rule_no);
            }
            else
            {
                GenerateCode(&b, "<", rule_no);
                GenerateCode(&b, ctc.FindBestTypeFor(allocation_element.element_type_symbol_index), rule_no);
                GenerateCode(&b, ">", rule_no);
                GenerateCode(&b, "this.getRhsSym(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&b, index.String(), rule_no);
            }
    
            GenerateCode(&b, rparen, rule_no);
            GenerateCode(&b, comma, rule_no);
            if (allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_SINGLETON)
                 GenerateCode(&b, " true /* left recursive */", rule_no);
            else GenerateCode(&b, " false /* not left recursive */", rule_no);
        }

        GenerateCode(&b, rparen, rule_no);
        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, current_line_input_file_info.c_str(), rule_no);
        GenerateCode(&b, space, rule_no);
    }
    else
    {
        //
        // Add new element to list
        //
        if (allocation_element.list_kind == RuleAllocationElement::ADD_ELEMENT)
        {
            GenerateCode(&b, space, rule_no);
            GenerateCode(&b, lparen, rule_no);
            GenerateCode(&b, "<", rule_no);
            GenerateCode(&b, allocation_element.name, rule_no);
            GenerateCode(&b, ">", rule_no);
            GenerateCode(&b, "this.getRhsSym(", rule_no);
            IntToString index(allocation_element.list_position);
            GenerateCode(&b, index.String(), rule_no);
            GenerateCode(&b, ")).addElement(", rule_no);
            if (grammar -> IsTerminal(allocation_element.element_symbol))
            {
                GenerateCode(&b, newkey, rule_no);
                GenerateCode(&b, grammar -> Get_ast_token_classname(), rule_no);
                GenerateCode(&b, lparen, rule_no);
                GenerateCode(&b, "this.getRhsIToken(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&b, index.String(), rule_no);
                GenerateCode(&b, rparen, rule_no);
            }
            else
            {
                GenerateCode(&b, "<", rule_no);
                GenerateCode(&b, ctc.FindBestTypeFor(allocation_element.element_type_symbol_index), rule_no);
                GenerateCode(&b, ">", rule_no);
                GenerateCode(&b, "this.getRhsSym(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&b, index.String(), rule_no);
            }

            if (allocation_element.list_position != 1) // a right-recursive rule? set the list as result
            {
                GenerateCode(&b, rparen, rule_no);
                GenerateCode(&b, trailer, rule_no);

                GenerateCode(&b, space, rule_no);
                GenerateCode(&b, "this.setResult(", rule_no);
                GenerateCode(&b, "this.getRhsSym(", rule_no);
                IntToString index(allocation_element.list_position);
                GenerateCode(&b, index.String(), rule_no);
            }
        }

        //
        // Copy a list that is not the first element on the right-hand side of the rule
        //
        else
        {
            assert(allocation_element.list_kind == RuleAllocationElement::COPY_LIST);

            GenerateCode(&b, space, rule_no);
            GenerateCode(&b, "this.setResult(", rule_no);
            GenerateCode(&b, "<", rule_no);
            GenerateCode(&b, allocation_element.name, rule_no);
            GenerateCode(&b, ">", rule_no);
            GenerateCode(&b, "this.getRhsSym(", rule_no);
            IntToString index(allocation_element.list_position);
            GenerateCode(&b, index.String(), rule_no);
        }

        GenerateCode(&b, rparen, rule_no);
    }

    GenerateCode(&b, trailer, rule_no);
 
    return;
}
