#include "CTC.h"
#include "NTC.h"
#include "CSharpAction.h"

#include <map>
#include <string>

#include "LCA.h"
#include "TTC.h"
#include "VisitorStaffFactory.h"


//
//
//
void CSharpAction::ExpandExportMacro(TextBuffer *buffer, SimpleMacroSymbol *simple_macro)
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
void CSharpAction::GenerateDefaultTitle(Tuple<ActionBlockElement> &notice_actions)
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
        buffer -> Put("namespace ");
        buffer -> Put(option -> package);
        buffer -> Put("\n{\n\n");
    }
    if (option -> automatic_ast &&
        strcmp(option -> package, option -> ast_package) != 0 &&
        *option -> ast_package != '\0')
    {
        buffer -> Put("using ");
        buffer -> Put(option -> ast_package);
        buffer -> Put(";\n");
    }

    return;
}


//
// First construct a file for this type. Write the title information to its header
// buffer and return the file.
//
ActionFileSymbol *CSharpAction::GenerateTitle(ActionFileLookupTable &ast_filename_table,
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
        buffer -> Put("namespace ");
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


ActionFileSymbol *CSharpAction::GenerateTitleAndGlobals(ActionFileLookupTable &ast_filename_table,
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
void CSharpAction::GenerateEnvironmentDeclaration(TextBuffer &b, const char *indentation)
{
    b.Put(indentation); b.Put("    private ");
                                 b.Put(option -> action_type);
                                 b.Put(" environment;\n");
    b.Put(indentation); b.Put("    public ");
                                 b.Put(option -> action_type);
                                 b.Put(" getEnvironment() { return environment; }\n\n");
}

 void CSharpAction::ProcessCodeActionEnd()
{
     if (*option->package != '\0')
     {
         auto  ast_filename_symbol = option->DefaultBlock()->ActionfileSymbol();
         TextBuffer& b = *(ast_filename_symbol->FinalTrailersBuffer());
         b.Put("}");
     }

}
void CSharpAction::ProcessAstActions(Tuple<ActionBlockElement>& actions,
    Tuple<ActionBlockElement>& notice_actions,
    Tuple<ActionBlockElement>& initial_actions,
    Array<const char*>& typestring,
    Tuple< Tuple<ProcessedRuleElement> >& processed_rule_map,
    SymbolLookupTable& classname_set,
    Tuple<ClassnameElement>& classname)
{
    ActionFileLookupTable ast_filename_table(4096);
  
    auto  default_file_symbol = option->DefaultBlock()->ActionfileSymbol();
    TextBuffer& b = *(default_file_symbol->BodyBuffer());
	
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
        if (option->IsNested())
        {
            GenerateAstType(default_file_symbol, "    ", option->ast_type);
            GenerateAbstractAstListType(default_file_symbol, "    ", abstract_ast_list_classname);
            GenerateAstTokenType(ntc, default_file_symbol, "    ", grammar->Get_ast_token_classname());
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
        if (option->IsNested())
            GenerateAstRootInterface(
                    default_file_symbol,
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

        if (option->IsNested())
            GenerateInterface(true /* is token */,
                              default_file_symbol,
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

            if (option->IsNested())
                GenerateInterface(ctc.IsTerminalClass(symbol),
                                  default_file_symbol,
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
    const char *indentation= (option->IsNested()
                              ? (char*)"    "
                              : (char*)"");
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
        ActionFileSymbol* top_level_file_symbol = (option->IsNested()
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
                              (option->IsNested()
	                               ? default_file_symbol
	                               : top_level_file_symbol),
                              indentation,
                              classname[i],
                              typestring);

            for (int j = 0; j < classname[i].special_arrays.Length(); j++)
            {
                //
                // Process the new special array class.
                //
                top_level_file_symbol = (option->IsNested()
                    ? NULL
                    : GenerateTitleAndGlobals(ast_filename_table, notice_actions, classname[i].special_arrays[j].name, true)); // needs_environment
                GenerateListExtensionClass(ctc,
                                           ntc,
                                           (option->IsNested()
	                                            ? default_file_symbol
	                                            : top_level_file_symbol),
                                           indentation,
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
                    if (top_level_file_symbol != NULL) // possible when option -> automatic_ast == Option::TOPLEVEL
                    {
                        for (int l = 0; l < actions.Length(); l++)
                            actions[l].buffer = top_level_file_symbol->BodyBuffer();
                    }
                    rule_allocation_map[rule_no].needs_environment = true;
                    ProcessCodeActions(actions, typestring, processed_rule_map);
                }
                if (option->IsNested())
                {
                    b.Put("    }\n\n");// Generate Class Closer
                }
                else
                {
                    TextBuffer& buffer = *top_level_file_symbol->BodyBuffer();
                    if (option->IsPackage())
                    {
                        buffer.Put(indentation); buffer.Put("}\n");// for namespace
                    }
                    buffer.Put("}\n\n");
                    top_level_file_symbol->Flush();
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
                                  (option->IsNested()
	                                   ? default_file_symbol
	                                   : top_level_file_symbol),
                                  indentation,
                                  classname[i],
                                  typestring);

                if (top_level_file_symbol != NULL) // option -> automatic_ast == Option::TOPLEVEL
                {
                    for (int j = 0; j < actions.Length(); j++)
                        actions[j].buffer = top_level_file_symbol->BodyBuffer();
                }
                ProcessCodeActions(actions, typestring, processed_rule_map);
            }
            else
            {
                assert(classname[i].specified_name != classname[i].real_name); // a classname was specified?
                if (classname[i].is_terminal_class)
                    GenerateTerminalMergedClass(ntc,
                                                (option->IsNested()
	                                                 ? default_file_symbol
	                                                 : top_level_file_symbol),
                                                indentation,
                                                classname[i],
                                                typestring);
                else GenerateMergedClass(ctc,
                                         ntc,
                                         (option->IsNested()
	                                          ? default_file_symbol
	                                          : top_level_file_symbol),
                                         indentation,
                                         classname[i],
                                         processed_rule_map,
                                         typestring);

                for (int k = 0; k < rule.Length(); k++)
                {
                    int rule_no = rule[k];
                    rule_allocation_map[rule_no].needs_environment = classname[i].needs_environment;
                    Tuple<ActionBlockElement>& actions = rule_action_map[rule_no];
                    if (top_level_file_symbol != NULL) // possible when option -> automatic_ast == Option::TOPLEVEL
                    {
                        for (int j = 0; j < actions.Length(); j++)
                            actions[j].buffer = top_level_file_symbol->BodyBuffer();
                    }
                    ProcessCodeActions(actions, typestring, processed_rule_map);
                }
            }
            if (option->IsNested())
            {
                b.Put("    }\n\n");// Generate Class Closer
            }
            else
            {
                TextBuffer& buffer = *top_level_file_symbol->BodyBuffer();
                if (option->IsPackage())
                {
                    buffer.Put(indentation); buffer.Put("}\n");// for namespace
                }
                buffer.Put("}\n\n");
                top_level_file_symbol->Flush();
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
        auto  visitor = VisitorStaffFactory(option->visitor_type);
        visitor.GenerateVisitor(this, ast_filename_table, default_file_symbol, notice_actions, type_set);
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
void CSharpAction::GenerateVisitorHeaders(TextBuffer &b, const char *indentation, const char *modifiers)
{
    if (option -> visitor != Option::NONE)
    {
        char *header = new char[strlen(indentation) + strlen(modifiers) + 9];
        strcpy(header, indentation);
        strcat(header, modifiers);


        if (option -> visitor & Option::PREORDER)
        {
            b.Put(header);
            b.Put("void accept(IAstVisitor v);\n");
        }
        if (option -> visitor & Option::DEFAULT)
        {
            b.Put(header);
            b.Put("void accept(");
            b.Put(option -> visitor_type);
            b.Put(" v);");

            b.Put("\n");

            b.Put(header);
            b.Put("void accept(Argument");
            b.Put(option -> visitor_type);
            b.Put(" v, object o);\n");

            b.Put(header);
            b.Put("object accept(Result");
            b.Put(option -> visitor_type);
            b.Put(" v);\n");

            b.Put(header);
            b.Put("object accept(ResultArgument");
            b.Put(option -> visitor_type);
            b.Put(" v, object o);");
        }
        b.Put("\n");

        delete [] header;
    }

    return;
}


//
//
//
void CSharpAction::GenerateVisitorMethods(NTC &ntc,
                                        TextBuffer &b,
                                        const char *indentation,
                                        ClassnameElement &element,
                                        BitSet &optimizable_symbol_set)
{
    if (option -> visitor & Option::DEFAULT)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("    public override void accept(");
                                     b.Put(option -> visitor_type);
                                     b.Put(" v) { v.visit(this); }\n");

        b.Put(indentation); b.Put("    public override  void accept(Argument");
                                     b.Put(option -> visitor_type);
                                     b.Put(" v, object o) { v.visit(this, o); }\n");

        b.Put(indentation); b.Put("    public override object accept(Result");
                                     b.Put(option -> visitor_type);
                                     b.Put(" v) { return v.visit(this); }\n");

        b.Put(indentation); b.Put("    public override  object accept(ResultArgument");
                                     b.Put(option -> visitor_type);
                                     b.Put(" v, object o) { return v.visit(this, o); }\n");
    }
    if (option -> visitor & Option::PREORDER)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("    public override  void accept(IAstVisitor v)\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        if (! v.preVisit(this)) return;\n");
        b.Put(indentation); b.Put("        enter((").Put(VisitorStaffFactory::preorder);;
                                     b.Put(option -> visitor_type);
                                     b.Put(") v);\n");
        b.Put(indentation); b.Put("        v.postVisit(this);\n");
        b.Put(indentation); b.Put("    }\n\n");

        b.Put(indentation); b.Put("    public   void enter(").Put(VisitorStaffFactory::preorder);;
                                     b.Put(option -> visitor_type);
                                     b.Put(" v)\n");
        b.Put(indentation); b.Put("    {\n");
        SymbolLookupTable &symbol_set = element.symbol_set;
        Tuple<int> &rhs_type_index = element.rhs_type_index;
        if (element.is_terminal_class || symbol_set.Size() == 0)
        {
            b.Put(indentation); b.Put("        v.visit(this);\n");
        }
        else
        {
            b.Put(indentation); b.Put("        bool checkChildren = v.visit(this);\n");
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
                    b.Put("if (_");
                    b.Put(symbol_set[i] -> Name());
                    b.Put(" != null) ");
                }
                b.Put("_");
                b.Put(symbol_set[i] -> Name());
                b.Put(".accept(v);\n");
            }

            if (symbol_set.Size() > 1)
            {
                b.Put(indentation); b.Put("        }\n");
            }
        }
        b.Put(indentation); b.Put("        v.endVisit(this);\n");
        b.Put(indentation); b.Put("    }\n");
    }

    return;
}


//
//
//
void CSharpAction::GenerateGetAllChildrenMethod(TextBuffer &b,
                                              const char *indentation,
                                              ClassnameElement &element)
{
    if (! element.is_terminal_class)
    {
        SymbolLookupTable &symbol_set = element.symbol_set;

        b.Put("\n");
        b.Put(indentation); b.Put("    /**\n");
        b.Put(indentation); b.Put("     * A list of all children of this node,don't including the null ones.\n");
        b.Put(indentation); b.Put("     */\n");
        b.Put(indentation); b.Put("    public override System.Collections.ArrayList getAllChildren()\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        System.Collections.ArrayList list = new System.Collections.ArrayList();\n");
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            b.Put(indentation);
        	b.Put("        if(_");b.Put(symbol_set[i]->Name());b.Put(" != null)  ");
        	           b.Put("list.Add(_");b.Put(symbol_set[i] -> Name());b.Put(");\n");
        }
        b.Put(indentation); b.Put("        return list;\n");
        b.Put(indentation); b.Put("    }\n");
    }

    return;
}


//
//
//
void CSharpAction::GenerateEqualsMethod(NTC &ntc,
                                      TextBuffer &b,
                                      const char *indentation,
                                      ClassnameElement &element,
                                      BitSet &optimizable_symbol_set)
{
    SymbolLookupTable &symbol_set = element.symbol_set;

    //
    // Note that if an AST node does not contain any field (symbol_set.Size() == 0),
    // we do not generate an "Equals" function for it.
    //
    if ((! element.is_terminal_class) && symbol_set.Size() > 0) 
    {
        Tuple<int> &rhs_type_index = element.rhs_type_index;

        b.Put("\n");
        b.Put(indentation); b.Put("    public override bool Equals(object o)\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        if (o == this) return true;\n");
        b.Put(indentation); b.Put("        if (! (o is ");
                                     b.Put(element.real_name);
                                     b.Put(")) return false;\n");
        b.Put(indentation); b.Put("        if (! base.Equals(o)) return false;\n");
        b.Put(indentation); b.Put("        ");
                                     b.Put(element.real_name);
                                     b.Put(" other = (");
                                     b.Put(element.real_name);
                                     b.Put(") o;\n");

        for (int i = 0; i < symbol_set.Size(); i++)
        {
            b.Put(indentation); b.Put("        ");
            if ((! optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
            {
                                             b.Put("if (_");
                                             b.Put(symbol_set[i] -> Name());
                                             b.Put(" == null)\n");
                b.Put(indentation); b.Put("            if (other._");
                                             b.Put(symbol_set[i] -> Name());
                                             b.Put(" != null) return false;\n");
                b.Put(indentation); b.Put("            else{}// continue\n");
                b.Put(indentation); b.Put("        else ");
            }
            b.Put("if (! _");
            b.Put(symbol_set[i] -> Name());
            b.Put(".Equals(other._");
            b.Put(symbol_set[i] -> Name());
            b.Put(")) return false;\n");
        }

        b.Put(indentation); b.Put("        return true;\n");
        b.Put(indentation); b.Put("    }\n");
    }

    return;
}


//
//
//
void CSharpAction::GenerateHashcodeMethod(NTC &ntc,
                                        TextBuffer &b,
                                        const char *indentation,
                                        ClassnameElement &element,
                                        BitSet &optimizable_symbol_set)
{
    SymbolLookupTable &symbol_set = element.symbol_set;

    //
    // Note that if an AST node does not contain any field (symbol_set.Size() == 0),
    // we do not generate an "Equals" function for it.
    //
    if ((! element.is_terminal_class) && symbol_set.Size() > 0) 
    {
        Tuple<int> &rhs_type_index = element.rhs_type_index;

        b.Put("\n");
        b.Put(indentation); b.Put("    public override int GetHashCode()\n");
        b.Put(indentation); b.Put("    {\n");

        b.Put(indentation); b.Put("        int hash = base.GetHashCode();\n");
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            b.Put(indentation); b.Put("        hash = hash * 31 + (_");
            b.Put(symbol_set[i] -> Name());
            if ((! optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
            {
                b.Put(" == null ? 0 : _");
                b.Put(symbol_set[i] -> Name());
            }
            b.Put(".GetHashCode());\n");
        }

        b.Put(indentation); b.Put("        return hash;\n");
        b.Put(indentation); b.Put("    }\n");
    }

    return;
}


//
//
//
void CSharpAction::GenerateSimpleVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                const char *indentation,
                                                const char *interface_name,
                                                SymbolLookupTable &type_set)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    b.Put(indentation); b.Put("public interface ");
                                 b.Put(interface_name);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("    void visit");
                                     b.Put("(");
                                     b.Put(symbol -> Name());
                                     b.Put(" n);\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    void visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" n);\n");

    b.Put(indentation); b.Put("}\n");
	if(option->IsTopLevel() && option->IsPackage())
    {
        b.Put(indentation); b.Put("}\n");// for namespace
	}
}

//
//
//
void CSharpAction::GenerateArgumentVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                  const char *indentation,
                                                  const char *interface_name,
                                                  SymbolLookupTable &type_set)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    b.Put(indentation); b.Put("public interface ");
                                 b.Put(interface_name);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("    void visit");
                                     b.Put("(");
                                     b.Put(symbol -> Name());
                                     b.Put(" n, object o);\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    void visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" n, object o);\n");

    b.Put(indentation); b.Put("}\n");
    if (ast_filename_symbol != option->DefaultBlock()->ActionfileSymbol())
    {
        b.Put(indentation); b.Put("}\n");// for namespace
    }
}

//
//
//
void CSharpAction::GenerateResultVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                const char *indentation,
                                                const char *interface_name,
                                                SymbolLookupTable &type_set)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    b.Put(indentation); b.Put("public interface ");
                                 b.Put(interface_name);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("    object visit");
                                     b.Put("(");
                                     b.Put(symbol -> Name());
                                     b.Put(" n);\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    object visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" n);\n");

    b.Put(indentation); b.Put("}\n");
    if (option->IsTopLevel() && option->IsPackage())
    {
        b.Put(indentation); b.Put("}\n");// for namespace
    }
}

//
//
//
void CSharpAction::GenerateResultArgumentVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                        const char *indentation,
                                                        const char *interface_name,
                                                        SymbolLookupTable &type_set)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    b.Put(indentation); b.Put("public interface ");
    b.Put(interface_name);
    b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("    object visit");
                                     b.Put("(");
                                     b.Put(symbol -> Name());
                                     b.Put(" n, object o);\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    object visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" n, object o);\n");

    b.Put(indentation); b.Put("}\n");
    if (option->IsTopLevel() && option->IsPackage())
    {
        b.Put(indentation); b.Put("}\n");// for namespace
    }
}


//
//
//
void CSharpAction::GeneratePreorderVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                  const char *indentation,
                                                  const char *interface_name,
                                                  SymbolLookupTable &type_set)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    assert(option -> visitor & Option::PREORDER);
    b.Put(indentation); b.Put("public interface ");
                                 b.Put(interface_name);
                                 b.Put(" : IAstVisitor\n");
    b.Put(indentation); b.Put("{\n");

    //    b.Put(indentation); b.Put("    bool preVisit(");
    //                                 b.Put(option -> ast_type);
    //                                 b.Put(" element);\n");
    //
    //    b.Put(indentation); b.Put("    void postVisit(");
    //                                 b.Put(option -> ast_type);
    //                                 b.Put(" element);\n\n");

    b.Put(indentation); b.Put("    bool visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" n);\n");
    b.Put(indentation); b.Put("    void endVisit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" n);\n\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("    bool visit");
                                     b.Put("(");
                                     b.Put(symbol -> Name());
                                     b.Put(" n);\n");
        b.Put(indentation); b.Put("    void endVisit");
                                     b.Put("(");
                                     b.Put(symbol -> Name());
                                     b.Put(" n);\n");
        b.Put("\n");
    }

    b.Put(indentation); b.Put("}\n\n");
    if (option->IsTopLevel() && option->IsPackage())
    {
        b.Put(indentation); b.Put("}\n");// for namespace
    }
    return;
}


//
//
//
void CSharpAction::GenerateNoResultVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                      const char *indentation,
                                                      const char *classname,
                                                      SymbolLookupTable &type_set)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    b.Put(indentation); 
                                 b.Put("public abstract class ");
                                 b.Put(classname);
                                 b.Put(" : ");
                                 b.Put(option -> visitor_type);
                                 b.Put(", Argument");
                                 b.Put(option -> visitor_type);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("    public abstract void unimplementedVisitor(string s);\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("    public void visit");
                                         b.Put("(");
                                         b.Put(symbol -> Name());
                                         b.Put(" n) { unimplementedVisitor(\"visit(");
                                         b.Put(symbol -> Name());
                                         b.Put(")\"); }\n");
            b.Put(indentation); b.Put("    public void visit");
                                         b.Put("(");
                                         b.Put(symbol -> Name());
                                         b.Put(" n, object o) { unimplementedVisitor(\"visit(");
                                         b.Put(symbol -> Name());
                                         b.Put(", object)\"); }\n");
            b.Put("\n");
        }
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    public void visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" n)\n");
    b.Put(indentation); b.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                         b.Put(i == 0 ? "" : "else ");
                                         b.Put("if (n is ");
                                         b.Put(symbol -> Name());
                                         b.Put(") visit((");
                                         b.Put(symbol -> Name());
                                         b.Put(") n);\n");
        }
    }
    b.Put(indentation); b.Put("        else throw new System.NotSupportedException(\"visit(\" + n.GetType().ToString() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");


    b.Put(indentation); b.Put("    public void visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" n, object o)\n");
    b.Put(indentation); b.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                         b.Put(i == 0 ? "" : "else ");
                                         b.Put("if (n is ");
                                         b.Put(symbol -> Name());
                                         b.Put(") visit((");
                                         b.Put(symbol -> Name());
                                         b.Put(") n, o);\n");
        }
    }
    b.Put(indentation); b.Put("        else throw new System.NotSupportedException(\"visit(\" + n.GetType().ToString() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("}\n");
    if (option->IsTopLevel() && option->IsPackage())
    {
        b.Put(indentation); b.Put("}\n");// for namespace
    }
}

//
//
//
void CSharpAction::GenerateResultVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                    const char *indentation,
                                                    const char *classname,
                                                    SymbolLookupTable &type_set)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    b.Put(indentation); 
                                 b.Put("public abstract class ");
                                 b.Put(classname);
                                 b.Put(" : Result");
                                 b.Put(option -> visitor_type);
                                 b.Put(", ResultArgument");
                                 b.Put(option -> visitor_type);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("    public abstract object unimplementedVisitor(string s);\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("    public object visit(");
                                         b.Put(symbol -> Name());
                                         b.Put(" n) { return unimplementedVisitor(\"visit(");
                                         b.Put(symbol -> Name());
                                         b.Put(")\"); }\n");
            b.Put(indentation); b.Put("    public object visit(");
                                         b.Put(symbol -> Name());
                                         b.Put(" n, object o) { return  unimplementedVisitor(\"visit(");
                                         b.Put(symbol -> Name());
                                         b.Put(", object)\"); }\n");
            b.Put("\n");
        }
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    public object visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" n)\n");
    b.Put(indentation); b.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                         b.Put(i == 0 ? "" : "else ");
                                         b.Put("if (n is ");
                                         b.Put(symbol -> Name());
                                         b.Put(") return visit((");
                                         b.Put(symbol -> Name());
                                         b.Put(") n);\n");
        }
    }
    b.Put(indentation); b.Put("        else throw new System.NotSupportedException(\"visit(\" + n.GetType().ToString() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");


    b.Put(indentation); b.Put("    public object visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" n, object o)\n");
    b.Put(indentation); b.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                         b.Put(i == 0 ? "" : "else ");
                                         b.Put("if (n is ");
                                         b.Put(symbol -> Name());
                                         b.Put(") return visit((");
                                         b.Put(symbol -> Name());
                                         b.Put(") n, o);\n");
        }
    }
    b.Put(indentation); b.Put("        else throw new System.NotSupportedException(\"visit(\" + n.GetType().ToString() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("}\n");
    if (option->IsTopLevel() && option->IsPackage())
    {
        b.Put(indentation); b.Put("}\n");// for namespace
    }
}


//
//
//
void CSharpAction::GeneratePreorderVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                      const char *indentation,
                                                      const char *classname,
                                                      SymbolLookupTable &type_set)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    assert(option -> visitor & Option::PREORDER);
    b.Put(indentation); 
                                 b.Put("public abstract class ");
                                 b.Put(classname);
                                 b.Put(" : ").Put(VisitorStaffFactory::preorder);
                                 b.Put(option -> visitor_type);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("    public abstract void unimplementedVisitor(string s);\n\n");
    b.Put(indentation); b.Put("    public bool preVisit(IAst element) { return true; }\n\n");
    b.Put(indentation); b.Put("    public void postVisit(IAst element) {}\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("    public bool visit");
                                         b.Put("(");
                                         b.Put(symbol -> Name());
                                         b.Put(" n) { unimplementedVisitor(\"visit(");
                                         b.Put(symbol -> Name());
                                         b.Put(")\"); return true; }\n");
            b.Put(indentation); b.Put("    public void endVisit");
                                         b.Put("(");
                                         b.Put(symbol -> Name());
                                         b.Put(" n) { unimplementedVisitor(\"endVisit(");
                                         b.Put(symbol -> Name());
                                         b.Put(")\"); }\n");
            b.Put("\n");
        }
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    public bool visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" n)\n");
    b.Put(indentation); b.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                         b.Put(i == 0 ? "" : "else ");
                                         b.Put("if (n is ");
                                         b.Put(symbol -> Name());
                                         b.Put(") return visit((");
                                         b.Put(symbol -> Name());
                                         b.Put(") n);\n");
        }
    }
    b.Put(indentation); b.Put("        throw new System.NotSupportedException(\"visit(\" + n.GetType().ToString() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    public void endVisit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" n)\n");
    b.Put(indentation); b.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                         b.Put(i == 0 ? "" : "else ");
                                         b.Put("if (n is ");
                                         b.Put(symbol -> Name());
                                         b.Put(") endVisit((");
                                         b.Put(symbol -> Name());
                                         b.Put(") n);\n");
        }
    }
    b.Put(indentation); b.Put("        else throw new System.NotSupportedException(\"visit(\" + n.GetType().ToString() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("}\n");
    if (option->IsTopLevel() && option->IsPackage())
    {
        b.Put(indentation); b.Put("}\n");// for namespace
    }
    return;
}


//
// Generate the the Ast root classes
//
void CSharpAction::GenerateAstType(ActionFileSymbol* ast_filename_symbol,
                                 const char *indentation,
                                 const char *classname)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    /*
     * First, generate the main root class
     */
    b.Put(indentation); 
                                 b.Put("public abstract class ");
                                 b.Put(classname);
                                 b.Put(" : IAst\n");
    b.Put(indentation); b.Put("{\n");
    if (option -> glr)
    {
        b.Put(indentation); b.Put("    private Ast nextAst = null;\n");
        b.Put(indentation); b.Put("    public IAst getNextAst() { return nextAst; }\n");
        b.Put(indentation); b.Put("    public void setNextAst(IAst n) { nextAst = n; }\n");
        b.Put(indentation); b.Put("    public void resetNextAst() { nextAst = null; }\n");
    }
    else
    {
	    b.Put(indentation); b.Put("    public IAst getNextAst() { return null; }\n");
    }

    b.Put(indentation); b.Put("    protected IToken leftIToken,\n");
    b.Put(indentation); b.Put("                     rightIToken;\n");
    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("    protected IAst parent = null;\n");
        b.Put(indentation); b.Put("    public void setParent(IAst parent) { this.parent = parent; }\n");
        b.Put(indentation); b.Put("    public IAst getParent() { return parent; }\n");\
    }
    else
    {
        b.Put(indentation); b.Put("    public IAst getParent()\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        throw new System.NotSupportedException(\"noparent-saved option in effect\");\n");
        b.Put(indentation); b.Put("    }\n");
    }
    b.Put(indentation); b.Put("    virtual public int GetRuleIndex(){ return 0; }\n");
    b.Put("\n");
    b.Put(indentation); b.Put("    public IToken getLeftIToken() { return leftIToken; }\n");
    b.Put(indentation); b.Put("    public IToken getRightIToken() { return rightIToken; }\n");
    b.Put(indentation); b.Put("    public IToken[] getPrecedingAdjuncts() { return leftIToken.getPrecedingAdjuncts(); }\n");
    b.Put(indentation); b.Put("    public IToken[] getFollowingAdjuncts() { return rightIToken.getFollowingAdjuncts(); }\n\n");

    b.Put(indentation); b.Put("    public override string ToString()\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        return leftIToken.getILexStream().ToString(leftIToken.getStartOffset(), rightIToken.getEndOffset());\n");
    b.Put(indentation); b.Put("    }\n\n");

    b.Put(indentation); b.Put("    public ");
                                 b.Put(classname);
                                 b.Put("(IToken token) { this.leftIToken = this.rightIToken = token; }\n");

    b.Put(indentation); b.Put("    public ");
                                 b.Put(classname);
                                 b.Put("(IToken leftIToken, IToken rightIToken)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        this.leftIToken = leftIToken;\n");
    b.Put(indentation); b.Put("        this.rightIToken = rightIToken;\n");
    b.Put(indentation); b.Put("    }\n\n");
    b.Put(indentation); b.Put("  public  void initialize() {}\n");
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
        b.Put(indentation); b.Put("    public System.Collections.ArrayList getChildren()\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("         ArrayListEx<object> list = new ArrayListEx<object>(getAllChildren()) ;\n");
        b.Put(indentation); b.Put("        int k = -1;\n");
        b.Put(indentation); b.Put("        for (int i = 0; i < list.Count; i++)\n");
        b.Put(indentation); b.Put("        {\n");
        b.Put(indentation); b.Put("            object element = list.get(i);\n");
        b.Put(indentation); b.Put("            if (element != null)\n");
        b.Put(indentation); b.Put("            {\n");
        b.Put(indentation); b.Put("                if (++k != i)\n");
        b.Put(indentation); b.Put("                    list.set(k, element);\n");
        b.Put(indentation); b.Put("            }\n");
        b.Put(indentation); b.Put("        }\n");
        b.Put(indentation); b.Put("        for (int i = list.Count - 1; i > k; i--) // remove extraneous elements\n");
        b.Put(indentation); b.Put("            list.remove(i);\n");
        b.Put(indentation); b.Put("        return list;\n");
        b.Put(indentation); b.Put("    }\n\n");

        b.Put(indentation); b.Put("    /**\n");
        b.Put(indentation); b.Put("     * A list of all children of this node, including the null ones.\n");
        b.Put(indentation); b.Put("     */\n");
        b.Put(indentation); b.Put("    public abstract System.Collections.ArrayList getAllChildren();\n");
    }
    else
    {
        b.Put(indentation); b.Put("    public System.Collections.ArrayList getChildren()\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        throw new System.NotSupportedException(\"noparent-saved option in effect\");\n");
        b.Put(indentation); b.Put("    }\n");
        b.Put(indentation); b.Put("    public override System.Collections.ArrayList getAllChildren() { return getChildren(); }\n");
    }

    b.Put("\n");

    b.Put(indentation); b.Put("    public override bool Equals(object o)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        if (o == this) return true;\n");
    b.Put(indentation); b.Put("        if (! (o is ");
                                 b.Put(classname);
                                 b.Put(")) return false;\n");
    b.Put(indentation); b.Put("        ");
                                 b.Put(classname);
                                 b.Put(" other = (");
                                 b.Put(classname);
                                 b.Put(") o;\n");
    b.Put(indentation); b.Put("        return getLeftIToken().getILexStream() == other.getLeftIToken().getILexStream() &&\n");
    b.Put(indentation); b.Put("               getLeftIToken().getTokenIndex() == other.getLeftIToken().getTokenIndex() &&\n");
    b.Put(indentation); b.Put("               getRightIToken().getILexStream() == other.getRightIToken().getILexStream() &&\n");
    b.Put(indentation); b.Put("               getRightIToken().getTokenIndex() == other.getRightIToken().getTokenIndex();\n");
    b.Put(indentation); b.Put("    }\n\n");

    b.Put(indentation); b.Put("    public override int GetHashCode()\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        int hash = 7;\n");
    b.Put(indentation); b.Put("        if (getLeftIToken().getILexStream() != null) hash = hash * 31 + getLeftIToken().getILexStream().GetHashCode();\n");
    b.Put(indentation); b.Put("        hash = hash * 31 + getLeftIToken().getTokenIndex();\n");
    b.Put(indentation); b.Put("        if (getRightIToken().getILexStream() != null) hash = hash * 31 + getRightIToken().getILexStream().GetHashCode();\n");
    b.Put(indentation); b.Put("        hash = hash * 31 + getRightIToken().getTokenIndex();\n");
    b.Put(indentation); b.Put("        return hash;\n");
    b.Put(indentation); b.Put("    }\n");

    GenerateVisitorHeaders(b, indentation, "    public abstract ");

    //
    // Not Preorder visitor? generate dummy accept method to satisfy IAst abstract declaration of accept(IAstVisitor);

    if (!(option -> visitor & Option::PREORDER) )
    {
        b.Put(indentation); b.Put("    public virtual void accept(IAstVisitor v) {}\n");
    }
    b.Put(indentation); b.Put("}\n\n");
    if (option->IsTopLevel() && option->IsPackage())
    {
        b.Put(indentation); b.Put("}\n");// for namespace
    }
    return;
}



typedef std::map<std::string, std::string> Substitutions;



//
// Generate the the Ast list class
//
void CSharpAction::GenerateAbstractAstListType(ActionFileSymbol* ast_filename_symbol,
                                             const char *indentation,
                                             const char *classname)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    /*
     * Generate the List root class
     */
    b.Put(indentation); 
                                 b.Put("public abstract class ");
                                 b.Put(this -> abstract_ast_list_classname);
                                 b.Put(" : ");
                                 b.Put(option -> ast_type);
                                 b.Put(" , IAbstractArrayList<");
                                 b.Put(option -> ast_type);
                                 b.Put(">\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("    private bool leftRecursive;\n");
    b.Put(indentation); b.Put("    private ArrayListEx<");  b.Put(option->ast_type);
	b.Put("> list=new ArrayListEx<"); b.Put(option->ast_type); b.Put(">();\n");

    b.Put(indentation); b.Put("    public int size() { return list.Count; }\n");
    b.Put(indentation); b.Put("    public  System.Collections.ArrayList getList() { return list; }\n");
    b.Put(indentation); b.Put("    public ");
                                 b.Put(option -> ast_type);
                                 b.Put(" getElementAt(int i) { return (");
                                 b.Put(option -> ast_type);
                                 b.Put(") list.get(leftRecursive ? i : list.Count - 1 - i); }\n");
    b.Put(indentation); b.Put("    public System.Collections.ArrayList getArrayList()\n");
   
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        if (! leftRecursive) // reverse the list \n");
    b.Put(indentation); b.Put("        {\n");
    b.Put(indentation); b.Put("            for (int i = 0, n = list.Count - 1; i < n; i++, n--)\n");
    b.Put(indentation); b.Put("            {\n");
    b.Put(indentation); b.Put("                dynamic ith = list.get(i),\n");
    b.Put(indentation); b.Put("                       nth = list.get(n);\n");
    b.Put(indentation); b.Put("                list.set(i, nth);\n");
    b.Put(indentation); b.Put("                list.set(n, ith);\n");
    b.Put(indentation); b.Put("            }\n");
    b.Put(indentation); b.Put("            leftRecursive = true;\n");
    b.Put(indentation); b.Put("        }\n");
    b.Put(indentation); b.Put("        return list;\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    /**\n");
    b.Put(indentation); b.Put("     * @deprecated replaced by {@link #addElement()}\n");
    b.Put(indentation); b.Put("     *\n");
    b.Put(indentation); b.Put("     */\n");
    b.Put(indentation); b.Put("    public bool add(");
                                 b.Put(option -> ast_type);
                                 b.Put(" element)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        addElement(element);\n");
    b.Put(indentation); b.Put("        return true;\n");
    b.Put(indentation); b.Put("    }\n\n");
    b.Put(indentation); b.Put("    public void addElement(");
                                 b.Put(option -> ast_type);
                                 b.Put(" element)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        list.Add(element);\n");
    b.Put(indentation); b.Put("        if (leftRecursive)\n");
    b.Put(indentation); b.Put("             rightIToken = element.getRightIToken();\n");
    b.Put(indentation); b.Put("        else leftIToken = element.getLeftIToken();\n");
    b.Put(indentation); b.Put("    }\n\n");

    // generate constructors for list class
    b.Put(indentation); b.Put("    public ");
                                 b.Put(classname);
                                 b.Put("(IToken leftIToken, IToken rightIToken, bool leftRecursive):base(leftIToken, rightIToken)\n");
    b.Put(indentation); b.Put("    {\n");
   
    b.Put(indentation); b.Put("        this.leftRecursive = leftRecursive;\n");
    b.Put(indentation); b.Put("       \n");
    b.Put(indentation); b.Put("    }\n\n");

    b.Put(indentation); b.Put("    public ");
                                 b.Put(classname);
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" element, bool leftRecursive)\n");
 
    b.Put(indentation); b.Put("        :this(element.getLeftIToken(), element.getRightIToken(), leftRecursive)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        list.Add(element);\n");
    b.Put(indentation); b.Put("    }\n\n");

    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("    /**\n");
        b.Put(indentation); b.Put("     * Make a copy of the list and return it. Note that we obtain the local list by\n");
        b.Put(indentation); b.Put("     * invoking getArrayList so as to make sure that the list we return is in proper order.\n");
        b.Put(indentation); b.Put("     */\n");
        b.Put(indentation); b.Put("    public override System.Collections.ArrayList getAllChildren()\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        return (System.Collections.ArrayList) getArrayList().Clone();\n");
        b.Put(indentation); b.Put("    }\n\n");
    }

    //
    // Implementation for functions in System.Collections.ArrayList
    //

    Substitutions subs;
    subs["%%AstType%%"] = option->ast_type;
    subs["%%ListClassName%%"] = classname;
    b.Put(indentation); b.Put("}\n\n");

    if (option->IsTopLevel() && option->IsPackage())
    {
        b.Put(indentation); b.Put("}\n");// for namespace
    }
  

    return;
}


//
// Generate the the Ast token class
//
void CSharpAction::GenerateAstTokenType(NTC &ntc, ActionFileSymbol* ast_filename_symbol,
                                      const char *indentation,
                                      const char *classname)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    /*
     * Generate the Token root class
     */
    b.Put(indentation); b.Put(option -> automatic_ast == Option::NESTED ? " " : "");
                                 b.Put("public   class ");
                                 b.Put(classname);
                                 b.Put(" : ");
                                 b.Put(option -> ast_type);
                                 b.Put(" , I");
                                 b.Put(classname);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    b.Put(indentation); b.Put("    public ");
                                 b.Put(classname);
                                 b.Put("(IToken token) :base(token){  }\n");
    b.Put(indentation); b.Put("    public IToken getIToken() { return leftIToken; }\n");
    b.Put(indentation); b.Put("    public override string ToString() { return leftIToken.ToString(); }\n\n");

    ClassnameElement element; // generate a temporary element with no symbols in its symbol set.
    element.real_name = (char *) classname;
    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);

    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("    /**\n");
        b.Put(indentation); b.Put("     * A token class has no children. So, we return the empty list.\n");
        b.Put(indentation); b.Put("     */\n");
        b.Put(indentation); b.Put("    public override System.Collections.ArrayList getAllChildren() { return new System.Collections.ArrayList(); }\n\n");
    }

    b.Put(indentation); b.Put("    public override bool Equals(object o)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        if (o == this) return true;\n");
    b.Put(indentation); b.Put("        if (! (o is ");
                                 b.Put(classname);
                                 b.Put(")) return false;\n");
    b.Put(indentation); b.Put("        ");
                                 b.Put(classname);
                                 b.Put(" other = (");
                                 b.Put(classname);
                                 b.Put(") o;\n");
    b.Put(indentation); b.Put("        return getIToken().getILexStream() == other.getIToken().getILexStream() &&\n");
    b.Put(indentation); b.Put("               getIToken().getTokenIndex() == other.getIToken().getTokenIndex();\n");
    b.Put(indentation); b.Put("    }\n\n");

    b.Put(indentation); b.Put("    public override int GetHashCode()\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        int hash = 7;\n");
    b.Put(indentation); b.Put("        if (getIToken().getILexStream() != null) hash = hash * 31 + getIToken().getILexStream().GetHashCode();\n");
    b.Put(indentation); b.Put("        hash = hash * 31 + getIToken().getTokenIndex();\n");
    b.Put(indentation); b.Put("        return hash;\n");
    b.Put(indentation); b.Put("    }\n");

    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set);

    b.Put(indentation); b.Put("}\n\n");
    if (option->IsTopLevel() && option->IsPackage())
    {
        b.Put(indentation); b.Put("}\n");// for namespace
    }
    return;
}


//
//
//
void CSharpAction::GenerateCommentHeader(TextBuffer &b,
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


void CSharpAction::GenerateListMethods(CTC &ctc,
                                     NTC &ntc,
                                     TextBuffer &b,
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
    b.Put(indentation); b.Put("    public ");
                                 b.Put("void addElement(");
                                 b.Put(element_type);
                                 b.Put(" _");
                                 b.Put(element_name);
                                 b.Put(")\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        base.addElement((");
                                 b.Put(option -> ast_type);
                                 b.Put(") _");
                                 b.Put(element_name);
                                 b.Put(");\n");
    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("        ");
        if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
        {
            b.Put("if (_");
            b.Put(element_name);
            b.Put(" != null) ");
        }
        b.Put("((");
        b.Put(option -> ast_type);
        b.Put(") _");
        b.Put(element_name);
        b.Put(").setParent(this);\n");
    }
    b.Put(indentation); b.Put("    }\n");

    //
    // Generate the "Equals" method for this list
    //
    b.Put("\n");
    b.Put(indentation); b.Put("    public override bool Equals(object o)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        if (o == this) return true;\n");
    b.Put(indentation); b.Put("        if (! (o is ");
                                 b.Put(classname);
                                 b.Put(")) return false;\n");
    b.Put(indentation); b.Put("        if (! base.Equals(o)) return false;\n");
    b.Put(indentation); b.Put("        ");
                                 b.Put(classname);
                                 b.Put(" other = (");
                                 b.Put(classname);
    b.Put(indentation); b.Put(") o;\n");
    b.Put(indentation); b.Put("        if (size() != other.size()) return false;\n");
    b.Put(indentation); b.Put("        for (int i = 0; i < size(); i++)\n");
    b.Put(indentation); b.Put("        {\n");
    b.Put(indentation); b.Put("            ");
    const char *element_typename = ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex());
    if (element_typename != NULL)
         b.Put(element_typename);
    else b.Put(typestring[element.array_element_type_symbol -> SymbolIndex()]);
    b.Put(" element = get");
    b.Put(element_name);
    b.Put("At(i);\n");
    b.Put(indentation); b.Put("            ");
    if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
    {
        b.Put("if (element == null && other.get");
                                     b.Put(element_name);
                                     b.Put("At(i) != null) return false;\n");
        b.Put(indentation); b.Put("            else ");
    }
    b.Put(indentation); b.Put("if (! element.Equals(other.get");
                                 b.Put(element_name);
                                 b.Put("At(i))) return false;\n");
    b.Put(indentation); b.Put("        }\n");
    b.Put(indentation); b.Put("        return true;\n");
    b.Put(indentation); b.Put("    }\n");

    //
    // Generate the "hashCode" method for a list node
    //
    b.Put("\n");
    b.Put(indentation); b.Put("    public override int GetHashCode()\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        int hash = base.GetHashCode();\n");
    b.Put(indentation); b.Put("        for (int i = 0; i < size(); i++)\n");
    b.Put(indentation); b.Put("            hash = hash * 31 + (get");
                                 b.Put(element_name);
    if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
    {
        b.Put("At(i) == null ? 0 : get");
        b.Put(element_name);
    }
    b.Put("At(i).GetHashCode());\n");
    b.Put(indentation); b.Put("        return hash;\n");
    b.Put(indentation); b.Put("    }\n");

    //
    // Generate visitor methods.
    //
    if (option -> visitor & Option::DEFAULT)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("    public override void accept(");
                                     b.Put(option -> visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
            b.Put(" v) { for (int i = 0; i < size(); i++) v.visit"
                           "("
                           "get");
            b.Put(element_name);
            b.Put("At(i)"
                           "); }\n");
        }
        else
        {
            b.Put(" v) { for (int i = 0; i < size(); i++) get");
            b.Put(element_name);
            b.Put("At(i).accept(v); }\n");
        }

        b.Put(indentation); b.Put("    public override void accept(Argument");
                                     b.Put(option -> visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
            b.Put(" v, object o) { for (int i = 0; i < size(); i++) v.visit"
                           "("
                           "get");
            b.Put(element_name);
            b.Put("At(i), o");
            b.Put("); }\n");
        }
        else
        {
            b.Put(" v, object o) { for (int i = 0; i < size(); i++) get");
            b.Put(element_name);
            b.Put("At(i).accept(v, o); }\n");
        }

        //
        // Code cannot be generated to automatically visit a node that
        // can return a value. These cases are left up to the user.
        //
        b.Put(indentation); b.Put("    public override object accept(Result");
                                     b.Put(option -> visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
                                         b.Put(" v)\n");
            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        System.Collections.ArrayList result = new System.Collections.ArrayList();\n");
            b.Put(indentation); b.Put("        for (int i = 0; i < size(); i++)\n");
            b.Put(indentation); b.Put("            result.Add(v.visit(get");
                                         b.Put(element_name);
                                         b.Put("At(i)));\n");
            b.Put(indentation); b.Put("        return result;\n");
            b.Put(indentation); b.Put("    }\n");
        }
        else
        {
                                         b.Put(" v)\n");
            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        System.Collections.ArrayList result = new System.Collections.ArrayList();\n");
            b.Put(indentation); b.Put("        for (int i = 0; i < size(); i++)\n");
            b.Put(indentation); b.Put("            result.Add(get");
                                         b.Put(element_name);
                                         b.Put("At(i).accept(v));\n");
            b.Put(indentation); b.Put("        return result;\n");
            b.Put(indentation); b.Put("    }\n");
        }

        b.Put(indentation); b.Put("    public override object accept(ResultArgument");
                                     b.Put(option -> visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
                                         b.Put(" v, object o)\n");
            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        System.Collections.ArrayList result = new System.Collections.ArrayList();\n");
            b.Put(indentation); b.Put("        for (int i = 0; i < size(); i++)\n");
            b.Put(indentation); b.Put("            result.Add(v.visit(get");
                                         b.Put(element_name);
                                         b.Put("At(i), o));\n");
            b.Put(indentation); b.Put("        return result;\n");
            b.Put(indentation); b.Put("    }\n");
        }
        else
        {
                                         b.Put(" v, object o)\n");
            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        System.Collections.ArrayList result = new System.Collections.ArrayList();\n");
            b.Put(indentation); b.Put("        for (int i = 0; i < size(); i++)\n");
            b.Put(indentation); b.Put("            result.Add(get");
                                         b.Put(element_name);
                                         b.Put("At(i).accept(v, o));\n");
            b.Put(indentation); b.Put("        return result;\n");
            b.Put(indentation); b.Put("    }\n");
        }
    }
    if (option -> visitor & Option::PREORDER)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("    public override void accept(IAstVisitor v)\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        if (! v.preVisit(this)) return;\n");
        b.Put(indentation); b.Put("        enter((").Put(VisitorStaffFactory::preorder);
                                     b.Put(option -> visitor_type);
                                     b.Put(") v);\n");
        b.Put(indentation); b.Put("        v.postVisit(this);\n");
        b.Put(indentation); b.Put("    }\n");
        b.Put(indentation); b.Put("    public void enter(").Put(VisitorStaffFactory::preorder);;
                                     b.Put(option -> visitor_type);
                                     b.Put(" v)\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        bool checkChildren = v.visit(this);\n");
        b.Put(indentation); b.Put("        if (checkChildren)\n");
        b.Put(indentation); b.Put("        {\n");
        b.Put(indentation); b.Put("            for (int i = 0; i < size(); i++)\n");
        b.Put(indentation); b.Put("            {\n");
        b.Put(indentation); b.Put("                ");

        const char *element_typename = ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex());
        if (element_typename != NULL)
        {
            b.Put(element_typename);
            b.Put(" element = get");
            b.Put(element_name);
            b.Put("At(i);\n");
            if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
            {
                b.Put(indentation); b.Put("                if (element != null)");
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
            b.Put(typestring[element.array_element_type_symbol -> SymbolIndex()]);
            b.Put(" element = get");
            b.Put(element_name);
            b.Put("At(i);\n");
            b.Put(indentation); b.Put("                ");
            if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
                b.Put("if (element != null) ");
            b.Put("element.accept(v);\n");
        }
        b.Put(indentation); b.Put("            }\n");
        b.Put(indentation); b.Put("        }\n");
        b.Put(indentation); b.Put("        v.endVisit(this);\n");
        b.Put(indentation); b.Put("    }\n");
    }

    return;
}


//
//
//
void CSharpAction::GenerateListClass(CTC &ctc,
                                   NTC &ntc,
                                   ActionFileSymbol* ast_filename_symbol,
                                   const char *indentation,
                                   ClassnameElement &element,
                                   Array<const char *> &typestring)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    Tuple<int> &interface = element.interface_;
    assert(element.array_element_type_symbol != NULL);
    const char *classname = element.real_name,
               *element_name = element.array_element_type_symbol -> Name(),
               *element_type = ctc.FindBestTypeFor(element.array_element_type_symbol -> SymbolIndex());

    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);

    b.Put(indentation); 
                                 b.Put("public class ");
                                 b.Put(classname);
                                 b.Put(" : ");
                                 b.Put(this -> abstract_ast_list_classname);
                                 b.Put(" , ");
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
                                 b.Put(element_type);
                                 b.Put(" get");
                                 b.Put(element_name);
                                 b.Put("At(int i) { return (");
                                 b.Put(element_type);
                                 b.Put(") getElementAt(i); }\n\n");

    //
    // generate constructors
    //
    b.Put(indentation); b.Put("    public ");
                                 b.Put(classname);
                                 b.Put("(");
                                 b.Put("IToken leftIToken, IToken rightIToken, bool leftRecursive):base(leftIToken, rightIToken, leftRecursive)\n");
    b.Put(indentation); b.Put("    {\n");
  
    b.Put(indentation); b.Put("    }\n\n");

    b.Put(indentation); b.Put("    public ");
                                 b.Put(classname);
                                 b.Put("(");
                                 b.Put(element_type);
                                 b.Put(" _");
                                 b.Put(element_name);
                                 b.Put(", bool leftRecursive)\n");
  
    b.Put(indentation); b.Put("        :base((");
                                 b.Put(option -> ast_type);
                                 b.Put(") _");
                                 b.Put(element_name);
                                 b.Put(", leftRecursive)\n");
    b.Put(indentation); b.Put("    {\n");
    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("        ");
        if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
        {
            b.Put("if (_");
            b.Put(element_name);
            b.Put(" != null) ");
        }
        b.Put("((");
        b.Put(option -> ast_type);
        b.Put(") _");
        b.Put(element_name);
        b.Put(").setParent(this);\n");
    }
    b.Put(indentation); b.Put("    }\n");
    b.Put("\n");

    GenerateListMethods(ctc, ntc, b, indentation, classname, element, typestring);

    b.Put(indentation);
    IntToString num(element.GetRuleNo());
    b+ "    public override int GetRuleIndex() { return " + num.String() + " ;}\n";

    if (option->IsNested())
    {
        b.Put("    }\n\n"); // Generate Class Closer
    }
    else
    {
        if (option->IsPackage())
        {
            b.Put(indentation); b.Put("}\n");// for namespace
        }
        b.Put("}\n\n");
        ast_filename_symbol->Flush();
    }
    return;
}


//
// Generate a class that : a basic list class. This is necessary when the user
// specifies action blocks to be associated with a generic list class - in which case,
// we have to generate a (new) unique class (that : the generic class) to hold the content
// of the action blocks.
//
void CSharpAction::GenerateListExtensionClass(CTC &ctc,
                                            NTC &ntc,
                                            ActionFileSymbol* ast_filename_symbol,
                                            const char *indentation,
                                            SpecialArrayElement &special_array,
                                            ClassnameElement &element,
                                            Array<const char *> &typestring)

{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    const char *classname = element.real_name,
               *element_name = element.array_element_type_symbol -> Name(),
               *element_type = ctc.FindBestTypeFor(element.array_element_type_symbol -> SymbolIndex());

    GenerateCommentHeader(b, indentation, element.ungenerated_rule, special_array.rules);

    b.Put(indentation); 
                                 b.Put("public class ");
                                 b.Put(special_array.name);
                                 b.Put(" : ");
                                 b.Put(classname);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    GenerateEnvironmentDeclaration(b, indentation);

    b.Put(indentation); b.Put("    public ");
                                 b.Put(special_array.name);
                                 b.Put("(");
                                 b.Put(option -> action_type);
                                 b.Put(" environment, ");
                                 b.Put("IToken leftIToken, IToken rightIToken, bool leftRecursive)\n");
 
    b.Put(indentation); b.Put("        :base(leftIToken, rightIToken, leftRecursive)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        this.environment = environment;\n");
    b.Put(indentation); b.Put("        initialize();\n");
    b.Put(indentation); b.Put("    }\n\n");

    b.Put(indentation); b.Put("    public ");
                                 b.Put(special_array.name);
                                 b.Put("(");
                                 b.Put(option -> action_type);
                                 b.Put(" environment, ");
                                 b.Put(element_type);
                                 b.Put(" _");
                                 b.Put(element_name);
                                 b.Put(", bool leftRecursive)\n");
 
    b.Put(indentation); b.Put("        :base(_");
                                 b.Put(element_name);
                                 b.Put(", leftRecursive)\n");
                                 b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        this.environment = environment;\n");
    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("        ");
        if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
        {
            b.Put("if (_");
            b.Put(element_name);
            b.Put(" != null) ");
        }
        b.Put("((");
        b.Put(option -> ast_type);
        b.Put(") _");
        b.Put(element_name);
        b.Put(").setParent(this);\n");
    }
    b.Put(indentation); b.Put("        initialize();\n");
    b.Put(indentation); b.Put("    }\n\n");

    GenerateListMethods(ctc, ntc, b, indentation, special_array.name, element, typestring);
    b.Put(indentation);
    IntToString num(element.GetRuleNo());
    b+ "    public override int GetRuleIndex() { return " + num.String() + " ;}\n";
    if (option->IsTopLevel() && option->IsPackage())
    {
        b.Put(indentation); b.Put("}\n");// for namespace
    }
    return;
}


//
// Generate a generic rule class
//
void CSharpAction::GenerateRuleClass(CTC &ctc,
                                   NTC &ntc,
                                   ActionFileSymbol* ast_filename_symbol,
                                   const char *indentation,
                                   ClassnameElement &element,
                                   Array<const char *> &typestring)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    char *classname = element.real_name;
    SymbolLookupTable &symbol_set = element.symbol_set;
    Tuple<int> &rhs_type_index = element.rhs_type_index;

    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);

    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);

    assert(element.rule.Length() == 1);
    int rule_no = element.rule[0];

    b.Put(indentation); 
                                 b.Put("public class ");
                                 b.Put(classname);
                                 b.Put(" : ");
    if (element.is_terminal_class)
    {
        b.Put(grammar -> Get_ast_token_classname());
        b.Put(" , ");
        b.Put(typestring[grammar -> rules[rule_no].lhs]);
        b.Put("\n");
        b.Put(indentation); b.Put("{\n");
        if (element.needs_environment)
            GenerateEnvironmentDeclaration(b, indentation);
        if (symbol_set.Size() == 1) // if the right-hand side contains a symbol ...
        {
            b.Put(indentation); b.Put("    public IToken get");
                                         b.Put(symbol_set[0] -> Name());
                                         b.Put("() { return leftIToken; }\n\n");
        }
        b.Put(indentation); b.Put("    public ");
                                     b.Put(classname);
                                     b.Put("(");
        if (element.needs_environment)
        {
            b.Put(option -> action_type);
            b.Put(" environment, IToken token)");
          
            b.Put(indentation); b.Put("        :base(token)\n");
            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        this.environment = environment;\n");
            b.Put(indentation); b.Put("        initialize();\n");
            b.Put(indentation); b.Put("    }\n");
        }
        else b.Put("IToken token):base(token) {  initialize(); }\n");
    }
    else 
    {
        b.Put(option -> ast_type);
        b.Put(" , ");
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
                                                 b.Put(ctc.FindBestTypeFor(rhs_type_index[i]));
                                                 b.Put(" _");
                                                 b.Put(symbol_set[i] -> Name());
                                                 b.Put(";\n");
                }
            }
            b.Put("\n");

            {
                for (int i = 0; i < symbol_set.Size(); i++)
                {
                    const char *symbolName = symbol_set[i] -> Name();
                    const char *bestType = ctc.FindBestTypeFor(rhs_type_index[i]);

                    if (ntc.CanProduceNullAst(rhs_type_index[i]))
                    {
                        b.Put(indentation); b.Put("    /**\n");
                        b.Put(indentation); b.Put("     * The value returned by <b>get");
                                                     b.Put(symbolName);
                                                     b.Put("</b> may be <b>null</b>\n");
                        b.Put(indentation); b.Put("     */\n");
                    }

                    // Generate getter method
                    b.Put(indentation); b.Put("    public ");
                                                 b.Put(bestType);
                                                 b.Put(" get");
                                                 b.Put(symbolName);
                                                 b.Put("() { return _");
                                                 b.Put(symbolName);
                                                 b.Put("; }\n");

                    // Generate setter method
                    b.Put(indentation); b.Put("    public void set");
                    b.Put(symbolName);
                    b.Put("(");
                    b.Put(bestType);
                    b.Put(" _"); // add "_" prefix to arg name in case symbol happens to be a Java keyword
                    b.Put(symbolName);
                    b.Put(")");
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
        const char *header = "    public ";
        b.Put(indentation);
        b.Put(header);
        b.Put(classname);
        int length = strlen(indentation) + strlen(header) + strlen(classname);

        b.Put("(");
        if (element.needs_environment)
        {
            b.Put(option -> action_type);
            b.Put(" environment, ");
        }
        b.Put("IToken leftIToken, IToken rightIToken");
        b.Put(symbol_set.Size() == 0 ? ")\n" : ",\n");
        {
            for (int i = 0; i < symbol_set.Size(); i++)
            {
                for (int k = 0; k <= length; k++)
                    b.PutChar(' ');
                b.Put(ctc.FindBestTypeFor(rhs_type_index[i]));
                b.Put(" _");
                b.Put(symbol_set[i] -> Name());
                b.Put(i == symbol_set.Size() - 1 ? ")\n" : ",\n");
            }
        }
     
        b.Put(indentation); b.Put("        :base(leftIToken, rightIToken)\n\n");
        b.Put(indentation); b.Put("    {\n");
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
                        b.Put(" != null) ");
                    }
    
                    b.Put("((");
                    b.Put(option -> ast_type);
                    b.Put(") _");
                    b.Put(symbol_set[i] -> Name());
                    b.Put(").setParent(this);\n");
                }
            }
        }

        b.Put(indentation); b.Put("        initialize();\n");
        b.Put(indentation); b.Put("    }\n");
    }

    if (option -> parent_saved)
        GenerateGetAllChildrenMethod(b, indentation, element);
    GenerateEqualsMethod(ntc, b, indentation, element, optimizable_symbol_set);
    GenerateHashcodeMethod(ntc, b, indentation, element, optimizable_symbol_set);
    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set);

    b.Put(indentation);
    IntToString num(rule_no);
    b+ "    public override int GetRuleIndex() { return " + num.String() + " ;}\n";

    return;
}


//
// Generate Ast class
//
void CSharpAction::GenerateTerminalMergedClass(NTC &ntc,
                                             ActionFileSymbol* ast_filename_symbol,
                                             const char *indentation,
                                             ClassnameElement &element,
                                             Array<const char *> &typestring)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    char *classname = element.real_name;
    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);

    b.Put(indentation); 
                                 b.Put("public class ");
                                 b.Put(classname);
                                 b.Put(" : ");
                                 b.Put(grammar -> Get_ast_token_classname());
                                 b.Put(" , ");
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
        b.Put(indentation); b.Put("    public IToken get");
                                     b.Put(symbol_set[0] -> Name());
                                     b.Put("() { return leftIToken; }\n\n");
    }
    b.Put(indentation); b.Put("    public ");
                                 b.Put(classname);
                                 b.Put("(");
                                 if (element.needs_environment)
                                 {
                                     b.Put(option -> action_type);
                                     b.Put(" environment, IToken token)");
                                  
                                     b.Put(indentation); b.Put("        :base(token)\n");
                                     b.Put(indentation); b.Put("    {\n");
                                     b.Put(indentation); b.Put("        this.environment = environment;\n");
                                     b.Put(indentation); b.Put("        initialize();\n");
                                     b.Put(indentation); b.Put("    }\n");
                                 }
                                 else b.Put("IToken token): base(token){  initialize(); }\n");

    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);
    GenerateHashcodeMethod(ntc, b, indentation, element, optimizable_symbol_set);
    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set);

    b.Put(indentation);
    IntToString num(element.GetRuleNo());
    b+ "    public override int GetRuleIndex() { return " + num.String() + " ;}\n";

    return;
}


//
// Generate Ast class
//
void CSharpAction::GenerateMergedClass(CTC &ctc,
                                     NTC &ntc,
                                     ActionFileSymbol* ast_filename_symbol,
                                     const char *indentation,
                                     ClassnameElement &element,
                                     Tuple< Tuple<ProcessedRuleElement> > &processed_rule_map,
                                     Array<const char *> &typestring)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    char *classname = element.real_name;
    SymbolLookupTable &symbol_set = element.symbol_set;
    Tuple<int> &rhs_type_index = element.rhs_type_index;

    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);

    b.Put(indentation);
                                 b.Put("public class ");
                                 b.Put(classname);
                                 b.Put(" : ");
                                 b.Put(option -> ast_type);
                                 b.Put(" , ");
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
    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            b.Put(indentation); b.Put("    private ");
                                         b.Put(ctc.FindBestTypeFor(rhs_type_index[i]));
                                         b.Put(" _");
                                         b.Put(symbol_set[i] -> Name());
                                         b.Put(";\n");
        }
    }
    b.Put("\n");

    //
    // Compute the set of symbols that always appear in an instance creation
    // of this merged class for which a null instance allocation will never occur.
    //
    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);
    Tuple<int> &rule = element.rule;
    {
        for (int i = 0; i < rule.Length(); i++)
        {
            int rule_no = rule[i];
            Tuple<ProcessedRuleElement> &processed_rule_elements = processed_rule_map[rule_no];
            for (int k = 0; k < processed_rule_elements.Length(); k++)
            {
                if (processed_rule_elements[k].position == 0 ||
                    ntc.CanProduceNullAst(grammar -> rhs_sym[processed_rule_elements[k].position]))
                     optimizable_symbol_set.RemoveElement(k);
            }
        }
    }

    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            if ((! optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
            {
                b.Put(indentation); b.Put("    /**\n");
                b.Put(indentation); b.Put("     * The value returned by <b>get");
                                             b.Put(symbol_set[i] -> Name());
                                             b.Put("</b> may be <b>null</b>\n");
                b.Put(indentation); b.Put("     */\n");
            }

            b.Put(indentation); b.Put("    public ");
                                         b.Put(ctc.FindBestTypeFor(rhs_type_index[i]));
                                         b.Put(" get");
                                         b.Put(symbol_set[i] -> Name());
                                         b.Put("() { return _");
                                         b.Put(symbol_set[i] -> Name());
                                         b.Put("; }\n");
        }
    }
    b.Put("\n");


    //
    // generate merged constructor
    //
    const char *header = "    public ";
    b.Put(indentation);
    b.Put(header);
    b.Put(classname);
    int length = strlen(indentation) + strlen(header) + strlen(classname);

    b.Put("(");
    if (element.needs_environment)
    {
        b.Put(option -> action_type);
        b.Put(" environment, ");
    }
    b.Put("IToken leftIToken, IToken rightIToken");
    b.Put(symbol_set.Size() == 0 ? ")\n" : ",\n");
    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            for (int k = 0; k <= length; k++)
                b.PutChar(' ');
            b.Put(ctc.FindBestTypeFor(rhs_type_index[i]));
            b.Put(" _");
            b.Put(symbol_set[i] -> Name());
            b.Put(i == symbol_set.Size() - 1 ? ")\n" : ",\n");
        }
    }
 
    b.Put(indentation); b.Put("        :base(leftIToken, rightIToken)\n\n");
    b.Put(indentation); b.Put("    {\n");
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
                    b.Put(" != null) ");
                }
    
                b.Put("((");
                b.Put(option -> ast_type);
                b.Put(") _");
                b.Put(symbol_set[i] -> Name());
                b.Put(").setParent(this);\n");
            }
        }
    }

    b.Put(indentation); b.Put("        initialize();\n");
    b.Put(indentation); b.Put("    }\n");

    if (option -> parent_saved)
        GenerateGetAllChildrenMethod(b, indentation, element);
    GenerateEqualsMethod(ntc, b, indentation, element, optimizable_symbol_set);
    GenerateHashcodeMethod(ntc, b, indentation, element, optimizable_symbol_set);
    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set);

    b.Put(indentation);
    IntToString num(element.GetRuleNo());
    b+ "    public override int GetRuleIndex() { return " + num.String() + " ;}\n";

    return;
}

void CSharpAction::GenerateAstRootInterface(
    ActionFileSymbol* ast_filename_symbol,
    const char* indentation)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    b.Put(indentation); b.Put("public interface ");
    b.Put(astRootInterfaceName.c_str());
    
    b.Put("\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("    public IToken getLeftIToken();\n");
    b.Put(indentation); b.Put("    public IToken getRightIToken();\n");
    b.Put("\n");
    GenerateVisitorHeaders(b, indentation, "    ");
    b.Put(indentation); b.Put("}\n\n");
    
    if (option->IsTopLevel() && option->IsPackage())
    {
        b.Put(indentation); b.Put("}\n");// for namespace
    }
    return;
}
void CSharpAction::GenerateInterface(bool is_terminal,
                                   ActionFileSymbol* ast_filename_symbol,
                                   const char *indentation,
                                   const char *interface_name,
                                   Tuple<int> &extension,
                                   Tuple<int> &classes,
                                   Tuple<ClassnameElement> &classname)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
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

    b.Put(indentation); b.Put("public interface ");
                                 b.Put(interface_name);
    if (extension.Length() > 0)
    {
        b.Put(" : ");
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
        b.Put(":");
        b.Put(astRootInterfaceName.c_str());
        b.Put(indentation); b.Put("{\n");
      
        b.Put(indentation); b.Put("}\n\n");
    }
	
    if (option->IsTopLevel() && option->IsPackage())
    {
        b.Put(indentation); b.Put("}\n");// for namespace
    }
    return;
}


//
//
//
void CSharpAction::GenerateNullAstAllocation(TextBuffer &b, int rule_no)
{
    const char *code = "\n                    setResult(null);";
    GenerateCode(&b, code, rule_no);

    return;
}


//
//
//
void CSharpAction::GenerateAstAllocation(CTC &ctc,
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
        GenerateCode(&b, "getRhsSym(1)", rule_no);
        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, "//", rule_no);
    }
    GenerateCode(&b, space, rule_no);
    GenerateCode(&b, "setResult(", rule_no);
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
        GenerateCode(&b, "getRhsIToken(1)", rule_no);
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
        GenerateCode(&b, "getLeftIToken()", rule_no);
        GenerateCode(&b, ", ", rule_no);
        GenerateCode(&b, "getRightIToken()", rule_no);
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
                    GenerateCode(&b, lparen, rule_no);
                    GenerateCode(&b, ctc.FindBestTypeFor(type_index[i]), rule_no);
                    GenerateCode(&b, rparen, rule_no);
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
                            GenerateCode(&b, lparen, rule_no);
                            GenerateCode(&b, actual_type, rule_no);
                            GenerateCode(&b, rparen, rule_no);
                        }

                        GenerateCode(&b, newkey, rule_no);
                        GenerateCode(&b, grammar -> Get_ast_token_classname(), rule_no);
                        GenerateCode(&b, lparen, rule_no);
                        GenerateCode(&b, "getRhsIToken(", rule_no);
                        IntToString index(position[i]);
                        GenerateCode(&b, index.String(), rule_no);
                        GenerateCode(&b, rparen, rule_no);
                    }
                    else
                    {
                        GenerateCode(&b, lparen, rule_no);
                        GenerateCode(&b, ctc.FindBestTypeFor(type_index[i]), rule_no);
                        GenerateCode(&b, rparen, rule_no);
                        GenerateCode(&b, "getRhsSym(", rule_no);
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
void CSharpAction::GenerateListAllocation(CTC &ctc,
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
        GenerateCode(&b, "setResult(", rule_no);
        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, space4, rule_no);
        GenerateCode(&b, current_line_input_file_info.c_str(), rule_no);
        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, space4, rule_no);

        GenerateCode(&b, newkey, rule_no);
        GenerateCode(&b, allocation_element.name, rule_no);
        GenerateCode(&b, lparen, rule_no);
        if (allocation_element.needs_environment)
        {
            GenerateCode(&b, "this, ", rule_no);
        }
        if (allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_EMPTY ||
            allocation_element.list_kind == RuleAllocationElement::RIGHT_RECURSIVE_EMPTY)
        {
            GenerateCode(&b, "getLeftIToken()", rule_no);
            GenerateCode(&b, ", ", rule_no);
            GenerateCode(&b, "getRightIToken()", rule_no);
            GenerateCode(&b, comma, rule_no);
            if (allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_EMPTY)
                 GenerateCode(&b, " true /* left recursive */", rule_no);
            else GenerateCode(&b, " false /* not left recursive */", rule_no);
        }
        else
        {
            assert(allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_SINGLETON ||
                   allocation_element.list_kind == RuleAllocationElement::RIGHT_RECURSIVE_SINGLETON);

            if (grammar -> IsTerminal(allocation_element.element_symbol))
            {
                GenerateCode(&b, newkey, rule_no);
                GenerateCode(&b, grammar -> Get_ast_token_classname(), rule_no);
                GenerateCode(&b, lparen, rule_no);
                GenerateCode(&b, "getRhsIToken(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&b, index.String(), rule_no);
                GenerateCode(&b, rparen, rule_no);
            }
            else
            {
                GenerateCode(&b, lparen, rule_no);
                GenerateCode(&b, ctc.FindBestTypeFor(allocation_element.element_type_symbol_index), rule_no);
                GenerateCode(&b, rparen, rule_no);
                GenerateCode(&b, "getRhsSym(", rule_no);
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
            GenerateCode(&b, lparen, rule_no);
            GenerateCode(&b, allocation_element.name, rule_no);
            GenerateCode(&b, rparen, rule_no);
            GenerateCode(&b, "getRhsSym(", rule_no);
            IntToString index(allocation_element.list_position);
            GenerateCode(&b, index.String(), rule_no);
            GenerateCode(&b, ")).addElement(", rule_no);
            if (grammar -> IsTerminal(allocation_element.element_symbol))
            {
                GenerateCode(&b, newkey, rule_no);
                GenerateCode(&b, grammar -> Get_ast_token_classname(), rule_no);
                GenerateCode(&b, lparen, rule_no);
                GenerateCode(&b, "getRhsIToken(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&b, index.String(), rule_no);
                GenerateCode(&b, rparen, rule_no);
            }
            else
            {
                GenerateCode(&b, lparen, rule_no);
                GenerateCode(&b, ctc.FindBestTypeFor(allocation_element.element_type_symbol_index), rule_no);
                GenerateCode(&b, rparen, rule_no);
                GenerateCode(&b, "getRhsSym(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&b, index.String(), rule_no);
            }

            if (allocation_element.list_position != 1) // a right-recursive rule? set the list as result
            {
                GenerateCode(&b, rparen, rule_no);
                GenerateCode(&b, trailer, rule_no);

                GenerateCode(&b, space, rule_no);
                GenerateCode(&b, "setResult(", rule_no);
                GenerateCode(&b, "getRhsSym(", rule_no);
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
            GenerateCode(&b, "setResult(", rule_no);
            GenerateCode(&b, lparen, rule_no);
            GenerateCode(&b, allocation_element.name, rule_no);
            GenerateCode(&b, rparen, rule_no);
            GenerateCode(&b, "getRhsSym(", rule_no);
            IntToString index(allocation_element.list_position);
            GenerateCode(&b, index.String(), rule_no);
        }

        GenerateCode(&b, rparen, rule_no);
    }

    GenerateCode(&b, trailer, rule_no);
 
    return;
}
