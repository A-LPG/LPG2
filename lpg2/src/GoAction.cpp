#include "CTC.h"
#include "NTC.h"
#include "GoAction.h"

#include <map>
#include <string>

#include "LCA.h"
#include "TTC.h"
#include "VisitorStaffFactory.h"
TextBuffer* GoAction::GetBuffer(ActionFileSymbol* ast_filename_symbol) const
{
    if (option->IsTopLevel())
    {
        return  (ast_filename_symbol->BodyBuffer());
    }
	return (ast_filename_symbol->BufferForNestAst());
    
}

GoAction::GoAction(Control* control_, Blocks* action_blocks_, Grammar* grammar_, MacroLookupTable* macro_table_): Action(control_, action_blocks_, grammar_, macro_table_)
{


}

namespace 
{
	std::string templateAnyCastToInterface(const char* interfaceName)
	{
        char temp[1024] = {};
        sprintf(temp, "func AnyCastTo%s(i interface{}) %s {\n"
            "	  if nil == i{\n"
            "		 return nil\n"
            "	  }else{\n"
            "		 return i.(%s)\n"
            "	  }\n"
            "}\n",interfaceName,interfaceName,interfaceName);
        return  temp;
	}
    std::string templateAnyCastToStruct(const char* structfaceName)
    {
        char temp[1024] = {};
        sprintf(temp, "func AnyCastTo%s(i interface{}) *%s {\n"
					         "	if nil == i{\n"
					         "		return nil\n"
					         "	}else{\n"
					         "		return i.(*%s)\n"
					         "	}\n"
            "}\n", structfaceName, structfaceName, structfaceName);
        return  temp;
    }
}
void GoAction::ProcessCodeActionEnd()
{

}

//
//
//
void GoAction::ExpandExportMacro(TextBuffer *buffer, SimpleMacroSymbol *simple_macro)
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
void GoAction::GenerateDefaultTitle(Tuple<ActionBlockElement> &notice_actions)
{
    //
    // If one or more notice blocks were specified, process and
    // print the notice at the beginning of each action file.
    //
    if (notice_actions.Length() > 0)
    {
        for (int i = 0; i < notice_actions.Length(); i++)
            ProcessActionBlock(notice_actions[i]);
        TextBuffer *buffer = notice_actions[0].buffer; // Get proper buffer from first action
        buffer -> Put("\n");
        action_blocks -> PutNotice(*buffer);
    }

    //
    // Issue the package state
    //
    TextBuffer *buffer = (option -> DefaultBlock() -> Buffer()
                              ? option -> DefaultBlock() -> Buffer()
                              : option -> DefaultBlock() -> ActionfileSymbol() -> InitialHeadersBuffer());
   
    if (*option->package != '\0')
    {
        buffer->Put("package ");
        buffer->Put(option->package);
        buffer->Put("\n\n");
    }

    return;
}


//
// First construct a file for my type. Write the title information to its header
// buffer and return the file.
//
ActionFileSymbol *GoAction::GenerateTitle(ActionFileLookupTable &ast_filename_table,
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
        // ActionBLockElement; redirect its output to my buffer
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
    if (*option->ast_package != '\0')
    {
        buffer->Put("package ");
        buffer->Put(option->ast_package);
        buffer->Put("\n\n");
    }


    delete [] filename;

    return file_symbol;
}


ActionFileSymbol *GoAction::GenerateTitleAndGlobals(ActionFileLookupTable &ast_filename_table,
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
void GoAction::GenerateEnvironmentDeclaration(TextBuffer &b, const char *, const char* def_prefix)
{

    b.Put(def_prefix); b.Put("    ");
                                
                                 b.Put(" GetEnvironment() *");
                                 b.Put(option->action_type);
                                 b.Put("{ return my.environment }\n\n");
}


void GoAction::ProcessAstActions(Tuple<ActionBlockElement>& actions,
    Tuple<ActionBlockElement>& notice_actions,
    Tuple<ActionBlockElement>& initial_actions,
    Array<const char*>& typestring,
    Tuple< Tuple<ProcessedRuleElement> >& processed_rule_map,
    SymbolLookupTable& classname_set,
    Tuple<ClassnameElement>& classname)
{
    ActionFileLookupTable ast_filename_table(4096);
  
    auto  default_file_symbol = option->DefaultBlock()->ActionfileSymbol();
    TextBuffer& b =*(default_file_symbol->BodyBuffer());
	
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
        castToAny = "CastToAnyFor";
        castToAny += option->action_type;

        astRootInterfaceName="IRootFor";
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
        // a file for my class.
        //
        ActionFileSymbol* top_level_file_symbol = (option->IsNested()
            ? NULL
            : GenerateTitleAndGlobals(ast_filename_table,
                notice_actions,
                classname[i].real_name,
                classname[i].needs_environment));
        //
        const char* indentation = (option->IsNested()? (char*)"    ": (char*)"");
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
                // Generate   info for the allocation of rules associated with my class
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
            if (option->IsNested())
            {
                GenerateSimpleVisitorInterface(default_file_symbol, "    ", visitor_type, type_set);
                GenerateArgumentVisitorInterface(default_file_symbol, "    ", argument_visitor_type, type_set);
                GenerateResultVisitorInterface(default_file_symbol, "    ", result_visitor_type, type_set);
                GenerateResultArgumentVisitorInterface(default_file_symbol, "    ", result_argument_visitor_type, type_set);

                GenerateNoResultVisitorAbstractClass(default_file_symbol, "    ", abstract_visitor_type, type_set);
                GenerateResultVisitorAbstractClass(default_file_symbol, "    ", abstract_result_visitor_type, type_set);
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
            if (option->IsNested())
            {
                GeneratePreorderVisitorInterface(default_file_symbol, "    ", visitor_type, type_set);
                GeneratePreorderVisitorAbstractClass(default_file_symbol, "    ", abstract_visitor_type, type_set);
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
                        "Since no class is associated with my production, the information in my block is unreachable");
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
void GoAction::GenerateVisitorHeaders(TextBuffer &b, const char *indentation, const char *modifiers, const char* def_prefix)
{
    if (option -> visitor != Option::NONE)
    {
        std::string header = indentation;
        header+= modifiers;
        header += " ";
        if(def_prefix)
        {
            header += def_prefix;
            header += " ";
        }
        b.Put(header.c_str());
        if (option -> visitor == Option::PREORDER)
        {
            b.Put("Accept(v IAstVisitor)");
            if (def_prefix){
                b.Put("{}");
            }
        }
        else if (option -> visitor == Option::DEFAULT)
        {
            b.Put("AcceptWithVisitor(v ");
            b.Put(option -> visitor_type);
            b.Put(")");
            if (def_prefix){
                b.Put("{}");
            }
            b.Put("\n");

            b.Put(header.c_str());
            b.Put(" AcceptWithArg(v  Argument");
            b.Put(option -> visitor_type);
            b.Put(", o interface{})");
            if (def_prefix) {
                b.Put("{}");
            }
            b.Put("\n");

            b.Put(header.c_str());
            b.Put("AcceptWithResult(v Result");
            b.Put(option -> visitor_type);
            b.Put(") interface{}");
            if (def_prefix) {
                b.Put("{return nil}");
            }
            b.Put("\n");

            b.Put(header.c_str());
            b.Put("AcceptWithResultArgument(v  ResultArgument");
            b.Put(option -> visitor_type);
            b.Put(", o interface{}) interface{}");
            if (def_prefix) {
                b.Put("{return nil}");
            }
        }
        b.Put("\n");


    }

    return;
}


//
//
//
void GoAction::GenerateVisitorMethods(NTC &,
                                        TextBuffer &b,
                                        const char *,
                                        ClassnameElement &element,
                                        BitSet &, const char* def_prefix)
{
    if (option -> visitor == Option::DEFAULT)
    {
        b.Put("\n");
        b.Put(def_prefix); b.Put("      AcceptWithVisitor(v ");
                                     b.Put(option -> visitor_type);
                                     b.Put(") { v.Visit"); b.Put(element.real_name); b.Put("(my)}\n");

        b.Put(def_prefix); b.Put("      AcceptWithArg(v Argument");
                                     b.Put(option -> visitor_type);
                                     b.Put(", o interface{}){ v.Visit");b.Put(element.real_name); b.Put("WithArg(my, o) }\n");

        b.Put(def_prefix); b.Put("      AcceptWithResult(v Result");
                                     b.Put(option -> visitor_type);
                                     b.Put(") interface{}{return v.Visit");b.Put(element.real_name); b.Put("WithResult(my) }\n");

        b.Put(def_prefix); b.Put("      AcceptWithResultArgument(v ResultArgument");
                                     b.Put(option -> visitor_type);
                                     b.Put(", o interface{}) interface{}{return v.Visit"); b.Put(element.real_name); b.Put("WithResultArgument(my, o) }\n");
    }
    else if (option -> visitor == Option::PREORDER)
    {
        b.Put("\n");
        b.Put(def_prefix); b.Put("       Accept(v IAstVisitor){\n");
       
         b.Put("        if ! v.PreVisit(my){ return }\n");
         b.Put("        var _ctor ,_ = v.(").Put(option->visitor_type).Put(")\n");
         b.Put("        my.Enter(_ctor)\n"); 
         b.Put("        v.PostVisit(my)\n");
         b.Put("}\n\n");

        b.Put(def_prefix); b.Put("       Enter(v ");
                                     b.Put(option -> visitor_type);
                                     b.Put("){\n");

        SymbolLookupTable &symbol_set = element.symbol_set;
        //Tuple<int> &rhs_type_index = element.rhs_type_index;
        if (element.is_terminal_class || symbol_set.Size() == 0)
        {
             b.Put("        v.Visit"); b.Put(element.real_name); b.Put("(my)\n");
          
        }
        else
        {
             b.Put("        var checkChildren = v.Visit");b.Put(element.real_name); b.Put("(my)\n");
             b.Put("        if checkChildren{\n");
           

            for (int i = 0; i < symbol_set.Size(); i++)
            {
                 b.Put("            ");
               
                b.Put("if nil != my._").Put(symbol_set[i] -> Name()).Put("{").Put("my._").Put(symbol_set[i] -> Name()).Put(".Accept(v)}\n");
            }

            
        	 b.Put("        }\n");
            
        }
         b.Put("        v.EndVisit"); b.Put(element.real_name); b.Put("(my)\n");
         b.Put("    }\n");
    }

    return;
}


//
//
//
void GoAction::GenerateGetAllChildrenMethod(TextBuffer &b,
                                              const char *,
                                              ClassnameElement &element, const char* def_prefix)
{
    if (! element.is_terminal_class)
    {
        SymbolLookupTable &symbol_set = element.symbol_set;

        b.Put("\n");
         b.Put("    /**\n");
         b.Put("     * A list of all children of my node, don't including the null ones.\n");
         b.Put("     */\n");
        b.Put(def_prefix); b.Put("        GetAllChildren() * ArrayList{\n");
         b.Put("        var list = NewArrayList()\n");
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            
        	b+"        if nil != my._"+symbol_set[i]->Name()+"{  list.Add(my._"+symbol_set[i] -> Name()+") }\n";

        }
         b.Put("        return list\n");
         b.Put("    }\n");
    }

    return;
}


//
//
//
void GoAction::GenerateSimpleVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                const char *,
                                                const char *interface_name,
                                                SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);

     b+"type "+ interface_name + " interface{\n";
                              
    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
    	b+"    Visit"+symbol->Name()+"(n  *"+symbol -> Name()+") \n";
    }

	b.Put("\n");
	b+"    Visit(n  IAst)\n";

	b.Put("}\n");
	b + templateAnyCastToInterface(interface_name).c_str();
}

//
//
//
void GoAction::GenerateArgumentVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                  const char *,
                                                  const char *interface_name,
                                                  SymbolLookupTable &type_set)
{
   
    TextBuffer& b =*GetBuffer(ast_filename_symbol);

     b + "type " + interface_name + " interface{\n";

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
    	b+"    Visit"+symbol->Name()+"WithArg(n  *"+symbol -> Name()+", o interface{}) \n";
    }

	 b.Put("\n");

     b.Put("    VisitWithArg(n  IAst, o interface{}) \n");

     b.Put("}\n");
     b + templateAnyCastToInterface(interface_name).c_str();
}

//
//
//
void GoAction::GenerateResultVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                const char *,
                                                const char *interface_name,
                                                SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
     b + "type " + interface_name + " interface{\n";

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
    	b+"    Visit"+symbol->Name()+"WithResult(n  *"+symbol -> Name()+") interface{}\n";
    }

	b.Put("\n");
	b.Put("    VisitWithResult(n  IAst) interface{}\n");

	b.Put("}\n");
    b + templateAnyCastToInterface(interface_name).c_str();
}

//
//
//
void GoAction::GenerateResultArgumentVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                        const char *,
                                                        const char *interface_name,
                                                        SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
     b + "type " + interface_name + " interface{\n";

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b+"    Visit"+symbol->Name()+"WithResultArgument(n  *"+symbol -> Name()+", o interface{}) interface{}\n";
    }

	b.Put("\n");
	b.Put("    VisitWithResultArgument(n IAst, o interface{}) interface{}\n");

	b.Put("}\n");
    b + templateAnyCastToInterface(interface_name).c_str();
}


//
//
//
void GoAction::GeneratePreorderVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                  const char *,
                                                  const char *interface_name,
                                                  SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    assert(option -> visitor == Option::PREORDER);
     b + "type " + interface_name + " interface{\n";

     b.Put("  IAstVisitor\n");

     b.Put("    Visit");
                                 b.Put("(n  IAst");
                             
                                 b.Put(") bool\n");
     b.Put("    EndVisit");
                                 b.Put("(n IAst");
                             
                                 b.Put(")\n\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];

    	b+"    Visit"+symbol->Name()+"(n *"+symbol -> Name()+") bool\n";

    	b+"    EndVisit"+symbol->Name()+"(n *"+symbol -> Name()+")\n";

        b.Put("\n");
    }

     b.Put("}\n\n");
     b + templateAnyCastToInterface(interface_name).c_str();
    return;
}


//
//
//
void GoAction::GenerateNoResultVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                      const char *,
                                                      const char *classname,
                                                      SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);

    std::string plus_interface= option->visitor_type;
    plus_interface += "";
    plus_interface += "Argument";
    plus_interface += option->visitor_type;
     b + "type " + plus_interface.c_str() + " interface{\n";
     b + "   " + option->visitor_type + "\n";
     b + "   Argument" + option->visitor_type + "\n";
     b + "   }\n";

     b + "type " + classname + " struct{\n";
     b + "   dispatch " + plus_interface.c_str() + "\n";
     b + "   }\n";


     b+"func New"+classname+"(dispatch "+plus_interface.c_str()+") *"+classname+"{\n";
     b.Put("         my := new(").Put(classname).Put(")\n");
     b.Put("         if dispatch != nil{\n");
     b.Put("           my.dispatch = dispatch\n");
     b.Put("         }else{\n");
     b.Put("           my.dispatch = my\n");
     b.Put("         }\n");
     b.Put("        return my\n");
     b.Put("}\n\n");


    std::string def_prefix_holder = "func (my *";
    def_prefix_holder += classname;
    def_prefix_holder += ")";
    const auto def_prefix = def_prefix_holder.c_str();

    b.Put(def_prefix); b.Put("      UnimplementedVisitor(s  string){}\n\n");
    
    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];

        b.Put(def_prefix);
        b + "      Visit" + symbol->Name() + "(n *" + symbol->Name() + ")  { my.UnimplementedVisitor(\"Visit" + symbol->Name() + "(" + symbol->Name() + ")\") }\n";
        b.Put("\n");

        b.Put(def_prefix);
        b+"      Visit"+symbol->Name()+"WithArg(n *"+ symbol->Name()+ ", o interface{})  { my.UnimplementedVisitor(\"Visit"+ symbol->Name()+ "WithArg(" + symbol->Name() + ", interface{})\") }\n";
        b.Put("\n");
    }
    
	b.Put("\n");

   

    b.Put(def_prefix);
	b+"     Visit(n IAst){\n";
    b+"     switch n2 := n.(type) {\n";
    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
         b.Put("        case *").Put(symbol->Name()).Put(":{\n");
         b.Put("            my.dispatch.Visit").Put(symbol->Name()).Put("(n2)\n");
         b.Put("            return \n");
         b.Put("        }\n");
    }
         b.Put("        default:{}\n");
    b.Put("     }\n");
    //  b.Put("        throw new Error(\"Visit(\" + n.ToString() + \")\")\n");
	b.Put("}\n");


    b.Put(def_prefix);
    b + "     VisitWithArg(n IAst, o interface{}){\n";
    b + "     switch n2 := n.(type) {\n";
    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol* symbol = type_set[i];
        b.Put("        case *").Put(symbol->Name()).Put(":{\n");
        b.Put("            my.dispatch.Visit").Put(symbol->Name()).Put("WithArg(n2,o)\n");
        b.Put("            return \n");
        b.Put("        }\n");
    }
        b.Put("        default:{}\n");
    b.Put("     }\n");

    //  b.Put("        throw new Error(\"Visit(\" + n.ToString() + \")\")\n");
    b.Put("}\n");

    b + templateAnyCastToStruct(classname).c_str();
}

//
//
//
void GoAction::GenerateResultVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                    const char *,
                                                    const char *classname,
                                                    SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    std::string plus_interface = "Result";
	plus_interface += option->visitor_type;

    plus_interface += "";
    plus_interface += "ResultArgument";
    plus_interface += option->visitor_type;
     b + "type " + plus_interface.c_str() + " interface{\n";
     b + "   Result" + option->visitor_type + "\n";
     b + "   ResultArgument" + option->visitor_type + "\n";
     b + "   }\n";

     b + "type " + classname + " struct{\n";
     b + "   dispatch " + plus_interface.c_str() + "\n";
     b + "   }\n";


     b + "func New" + classname + "(dispatch " + plus_interface.c_str() + ") *" + classname + "{\n";
     b.Put("         my := new(").Put(classname).Put(")\n");
     b.Put("         if dispatch != nil{\n");
     b.Put("           my.dispatch = dispatch\n");
     b.Put("         }else{\n");
     b.Put("           my.dispatch = my\n");
     b.Put("         }\n");
     b.Put("        return my\n");
     b.Put("}\n\n");


    std::string def_prefix_holder = "func (my *";
    def_prefix_holder += classname;
    def_prefix_holder += ")";
    const auto def_prefix = def_prefix_holder.c_str();


    b.Put(def_prefix); b.Put("       UnimplementedVisitor(s  string) interface{}{ return nil }\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
       
            b.Put(def_prefix);
        	b+"     Visit"+symbol->Name()+"WithResult(n *"+ symbol->Name()+") interface{}{ return  my.UnimplementedVisitor(\"Visit" +
                symbol->Name()+ "WithResult(*" + symbol -> Name() + ")\")}\n\n";

            b.Put(def_prefix);
            b + "     Visit" + symbol->Name() + "WithResultArgument(n *" + symbol->Name()
        	+ ", o interface{}) interface{}{ return  my.UnimplementedVisitor(\"Visit" +
                symbol->Name() + "WithResultArgument(*" + symbol->Name() + ", interface{})\")}\n\n";

        }
    }

	b.Put("\n");

    b.Put(def_prefix) + "     VisitWithResult(n IAst) interface{}{\n";

    b + "     switch n2 := n.(type) {\n";
    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol* symbol = type_set[i];
        b.Put("        case *").Put(symbol->Name()).Put(":{\n");
        b.Put("            return my.dispatch.Visit").Put(symbol->Name()).Put("WithResult(n2)\n");
        b.Put("        }\n");
    }
    b.Put("        default:{ return nil}\n");
    b.Put("     }\n}\n");



	b.Put(def_prefix)+"     VisitWithResultArgument(n IAst, o interface{}) interface{}{\n";
    b + "     switch n2 := n.(type) {\n";
    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol* symbol = type_set[i];
        b.Put("        case *").Put(symbol->Name()).Put(":{\n");
        b.Put("            return my.dispatch.Visit").Put(symbol->Name()).Put("WithResultArgument(n2,o)\n");
        b.Put("        }\n");
    }
    b.Put("        default:{ return nil}\n");
    b.Put("     }\n}\n");

    b + templateAnyCastToStruct(classname).c_str();
}


//
//
//
void GoAction::GeneratePreorderVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                      const char */*indentation*/,
                                                      const char *classname,
                                                      SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    assert(option -> visitor == Option::PREORDER);

    std::string plus_interface = option->visitor_type;

	  b + "type " + classname + " struct{\n";
	  b + "   dispatch " + plus_interface.c_str() + "\n";
	  b + "   }\n";

      b + "func New" + classname + "(dispatch " + plus_interface.c_str() + ") *" + classname + "{\n";
      b.Put("         my := new(").Put(classname).Put(")\n");
      b.Put("         if dispatch != nil{\n");
      b.Put("           my.dispatch = dispatch\n");
      b.Put("         }else{\n");
      b.Put("           my.dispatch = my\n");
      b.Put("         }\n");
      b.Put("        return my\n");
      b.Put("}\n\n");

     std::string def_prefix_holder = "func (my *";
     def_prefix_holder += classname;
     def_prefix_holder += ")";
     const auto def_prefix = def_prefix_holder.c_str();
    b.Put(def_prefix); b.Put("     UnimplementedVisitor(s  string)bool { return true }\n\n");
    b.Put(def_prefix); b.Put("     PreVisit(element IAst) bool{ return true }\n\n");
    b.Put(def_prefix); b.Put("     PostVisit(element  IAst) {}\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(def_prefix);
        	b+"     Visit"+symbol->Name()+
                "(n  *"+symbol -> Name()+") bool{ return my.UnimplementedVisitor(\"Visit(*"+symbol -> Name()+")\") }\n";
            
            b.Put(def_prefix);
        	b+"     EndVisit"+symbol->Name()+
                "(n  *"+symbol -> Name()+") { my.UnimplementedVisitor(\"EndVisit(*"+symbol -> Name()+")\") }\n";
            b.Put("\n");
        }
    }

	b.Put("\n");
    b.Put(def_prefix);
	b.Put("     Visit(n IAst) bool{\n");
	b + "     switch n2 := n.(type) {\n";
	for (int i = 0; i < type_set.Size(); i++)
	{
	 Symbol* symbol = type_set[i];
	 b.Put("        case *").Put(symbol->Name()).Put(":{\n");
	 b.Put("            return my.dispatch.Visit").Put(symbol->Name()).Put("(n2)\n");
	 b.Put("        }\n");
	}
	b.Put("        default:{ return false}\n");
	b.Put("     }\n}\n");


    b.Put(def_prefix);
	b+"     EndVisit(n  IAst){\n";
    b+ "     switch n2 := n.(type) {\n";
    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol* symbol = type_set[i];
        b.Put("        case *").Put(symbol->Name()).Put(":{\n");
        b.Put("            my.dispatch.EndVisit").Put(symbol->Name()).Put("(n2)\n");
        b.Put("        }\n");
    }
    b.Put("        default:{ }\n");
    b.Put("     }\n}\n");
    // b.Put("        throw new Error(\"Visit(\" + n.ToString() + \")\")\n");

     b + templateAnyCastToStruct(classname).c_str();
    return;
}


//
// Generate the the Ast root classes
//
void GoAction::GenerateAstType(ActionFileSymbol* ast_filename_symbol,
                                 const char *,
                                 const char *classname)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    /*
     * First, generate the main root class
     */

	b+"type " + classname + " struct{\n";
     b.Put("     leftIToken  IToken \n");
     b.Put("     rightIToken IToken \n");
    if (option->glr)
    {
	      b.Put("    nextAst Ast \n");
    }
    if (option->parent_saved)
    {
	       b.Put("     parent IAst\n");
    }
    b.Put("}\n");

     b.Put("func New").Put(classname).Put("2(leftIToken  IToken , rightIToken  IToken ) *").Put(classname).Put("{\n");
     b.Put("        my := new(").Put(classname).Put(")\n");
     b.Put("        my.leftIToken = leftIToken\n");
     b.Put("        my.rightIToken = rightIToken\n");
     b.Put("        return my\n");
     b.Put("}\n\n");

     b.Put("func New").Put(classname).Put("(token  IToken) *").Put(classname).Put("{\n");
     b.Put("        my := new(").Put(classname).Put(")\n");
     b.Put("        my.leftIToken = token\n");
     b.Put("        my.rightIToken = token\n");
     b.Put("        return my\n");
     b.Put("}\n\n");


    std::string def_prefix_holder = "func (my *";
    def_prefix_holder += classname;
    def_prefix_holder += ")";
    const auto def_prefix = def_prefix_holder.c_str();

    if (option -> glr)
    {
         b.Put(def_prefix); b.Put("    GetNextAst() IAst  { return my.nextAst }\n");
         b.Put(def_prefix); b.Put("    SetNextAst(n  IAst) { my.nextAst = n }\n");
         b.Put(def_prefix); b.Put("    ResetNextAst()  { my.nextAst = nil }\n");
    }
    else
    {
	     b.Put(def_prefix); b.Put("     GetNextAst() IAst  { return nil }\n");
    }


    if (option -> parent_saved)
    {
      
         b.Put(def_prefix); b.Put("      SetParent(parent IAst )  { my.parent = parent }\n");
         b.Put(def_prefix); b.Put("      GetParent() IAst { return my.parent }\n");\
    }
    else
    {
         b.Put(def_prefix); b.Put("     GetParent()IAst{\n");
         b.Put("        return nil\n");
          b.Put("    }\n");
    }

    b.Put("\n");
     b.Put(def_prefix); b.Put("     GetLeftIToken()  IToken { return my.leftIToken }\n");
     b.Put(def_prefix); b.Put("     GetRightIToken()  IToken { return my.rightIToken }\n");
     b.Put(def_prefix); b.Put("     GetPrecedingAdjuncts()  []IToken { return my.leftIToken.GetPrecedingAdjuncts() }\n");
     b.Put(def_prefix); b.Put("     GetFollowingAdjuncts()  []IToken { return my.rightIToken.GetFollowingAdjuncts() }\n\n");

     b.Put(def_prefix); b.Put("      ToString()string{\n");
     b.Put("        return my.leftIToken.GetILexStream().ToString(my.leftIToken.GetStartOffset(), my.rightIToken.GetEndOffset())\n");
     b.Put("}\n\n");


     b.Put(def_prefix); b.Put("     Initialize()  {}\n");
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
         b.Put("    /**\n");
         b.Put("     * A list of all children of my node, excluding the null ones.\n");
         b.Put("     */\n");
         b.Put(def_prefix);  b.Put("      GetChildren() *ArrayList{\n");
         b.Put("        var list = my.GetAllChildren() \n");
         b.Put("        var k = -1\n");
         b.Put("        var i = 0\n");
         b.Put("        for ; i < list.Size(); i++{\n");
         b.Put("            var element = list.Get(i)\n");
         b.Put("            if element != nil{\n");
         b.Put("                k += i\n");
         b.Put("                if k != i{\n");
         b.Put("                    list.Set(k, element)\n");
         b.Put("                }\n");
         b.Put("            }\n");
         b.Put("        }\n");
         b.Put("        i = list.Size() - 1\n");
         b.Put("        for ; i > k; i--{ // remove extraneous elements\n");
         b.Put("            list.RemoveAt(i)\n");
         b.Put("        }\n");
         b.Put("        return list\n");
         b.Put("}\n\n");

         b.Put("    /**\n");
         b.Put("     * A list of all children of my node, don't including the null ones.\n");
         b.Put("     */\n");
         b.Put(def_prefix); b.Put("    GetAllChildren() *ArrayList{return nil}\n");
    }
    else
    {
         b.Put(def_prefix); b.Put("      GetChildren() *ArrayList{\n");
         b.Put("        panic(\"noparent-saved option in effect\")\n");
         b.Put("    }\n");
         b.Put(def_prefix); b.Put("        GetAllChildren() *ArrayList { return my.GetChildren() }\n");
    }

    b.Put("\n");

    GenerateVisitorHeaders(b, "", "", def_prefix);

    //
    // Not Preorder visitor? generate dummy accept method to satisfy IAst abstract declaration of Accept(IAstVisitor);
    // TODO: Should IAstVisitor be used for default visitors also? If (when) yes then we should remove it from the test below
    //
    if (option -> visitor == Option::NONE || option -> visitor == Option::DEFAULT) // ??? Don't need my for DEFAULT case after upgrade
    {
         b.Put(def_prefix); b.Put("      Accept(v IAstVisitor) {}\n");
    }
     b.Put("\n\n");
     b + templateAnyCastToStruct(classname).c_str();
    return;
}



typedef std::map<std::string, std::string> Substitutions;



//
// Generate the the Ast list class
//
void GoAction::GenerateAbstractAstListType(ActionFileSymbol* ast_filename_symbol,
                                             const char *,
                                             const char *classname)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    /*
     * Generate the List root class
     *///IAbstractArrayList
     
	b+"type "+ abstract_ast_list_classname + " struct{\n";
    b + "     *" + option->ast_type + "\n";
    b + "     leftRecursive bool \n";
    b + "     list *ArrayList \n}\n";

    // generate constructors for list class
     b + "func New" + abstract_ast_list_classname + 
        "(leftToken  IToken, rightToken  IToken, leftRecursive bool)*" + abstract_ast_list_classname +"{\n";
     b + "      my := new(" + abstract_ast_list_classname + ")\n";
     b + "      my." + option->ast_type +" = New"+ option->ast_type   + "2(leftToken, rightToken)\n";
     b + "      my.list = NewArrayList()\n";
     b + "      my.leftRecursive = leftRecursive\n";
     b + "      return my\n";
     b + "}\n\n";



    std::string def_prefix_holder = "func (my *";
    def_prefix_holder += abstract_ast_list_classname;
    def_prefix_holder += ")";
    const auto def_prefix = def_prefix_holder.c_str();

     b.Put(def_prefix); b.Put("      Size() int { return my.list.Size(); }\n");
     b.Put(def_prefix); b.Put("      GetList() *ArrayList{ return my.list }\n");

     b.Put(def_prefix); b.Put("      GetElementAt(i int ) IAst{\n");
     b + "    var k int\n";
     b + "    if my.leftRecursive {\n";
     b + "       k =i\n";
     b + "    }else{\n";
     b + "       k =my.list.Size() - 1 - i\n";
     b + "    }\n";
     b.Put("    return my.list.Get(k).(IAst) \n");
     b + "    }\n";

     b.Put(def_prefix); b.Put("      GetArrayList() *ArrayList{\n"); 
     b.Put("        if ! my.leftRecursive{ // reverse the list \n");
     b.Put("           var i = 0\n");
     b.Put("           var n = my.list.Size() - 1\n");
     b.Put("           for ; i < n;  n--{\n");
     b.Put("                var ith = my.list.Get(i)\n");
     b.Put("                var nth = my.list.Get(n)\n");
     b.Put("                my.list.Set(i, nth)\n");
     b.Put("                my.list.Set(n, ith)\n");
     b.Put("               i++\n");

     b.Put("           }\n");
     b.Put("           my.leftRecursive = true\n");
     b.Put("        }\n");
     b.Put("        return my.list\n");
     b.Put("    }\n");

     b.Put("    /**\n");
     b.Put("     * @deprecated replaced by {@link #AddElement()}\n");
     b.Put("     *\n");
     b.Put("     */\n");

     b.Put(def_prefix); b.Put("      Add(element IAst) bool {\n");
     b.Put("        my.AddElement(element)\n");
     b.Put("        return true\n");
     b.Put("}\n\n");

     b.Put(def_prefix);
						b.Put("      AddElement(element IAst){\n");

     b.Put("        my.list.Add(element)\n");
     b.Put("        if my.leftRecursive{\n");
     b.Put("             my.rightIToken = element.GetRightIToken()\n");
     b.Put("        }else{\n");
     b.Put("          my.leftIToken = element.GetLeftIToken()\n");
     b.Put("        }\n");
     b.Put("}\n\n");


    if (option -> parent_saved)
    {
         b.Put("    /**\n");
         b.Put("     * Make a copy of the list and return it. Note that we obtain the local list by\n");
         b.Put("     * invoking GetArrayList so as to make sure that the list we return is in proper order.\n");
         b.Put("     */\n");
         b.Put(def_prefix); b.Put("      GetAllChildren() *ArrayList{\n");
         b.Put("        return my.GetArrayList().Clone()\n");
         b.Put("}\n\n");
    }

    //
    // Implementation for functions in ArrayList
    //

    Substitutions subs;
    subs["%%AstType%%"] = option->ast_type;
    subs["%%ListClassName%%"] = classname;
     b.Put("\n\n");

    
     b + templateAnyCastToStruct(classname).c_str();

    return;
}


//
// Generate the the Ast token class
//
void GoAction::GenerateAstTokenType(NTC &ntc, ActionFileSymbol* ast_filename_symbol,
                                      const char *indentation,
                                      const char *classname)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);

    std::string def_prefix_holder = "func (my *";
    def_prefix_holder += classname;
    def_prefix_holder += ")";

    const auto def_prefix = def_prefix_holder.c_str();
    /*
     * Generate the Token root class
     */
     b + "type " + classname + " struct{\n";
     b + "    *" + option->ast_type + "\n";
     b + " }\n";

     b + "func New" + classname + "(token  IToken)*" + classname+"{\n";
     b + "      my := new(" + classname + ")\n";
     b + "      my." + option->ast_type + " = New" + option->ast_type + "(token)\n";
     b + "      return my\n";
     b + "}\n\n";


    b.Put(def_prefix); b.Put("      GetIToken()  IToken{ return my.leftIToken }\n");
    b.Put(def_prefix); b.Put("      ToString()  string  { return my.leftIToken.ToString() }\n\n");

    ClassnameElement element; // generate a temporary element with no symbols in its symbol set.
    element.real_name = (char *) classname;
    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);

    if (option -> parent_saved)
    {
         b.Put("    /**\n");
         b.Put("     * A token class has no children. So, we return the empty list.\n");
         b.Put("     */\n");
        b.Put(def_prefix); b.Put("        GetAllChildren()  *ArrayList { return nil }\n\n");
    }

    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set, def_prefix);

     b.Put("\n\n");
     b + templateAnyCastToStruct(classname).c_str();
    return;
}


//
//
//
void GoAction::GenerateCommentHeader(TextBuffer &b,
                                       const char *,
                                       Tuple<int> &ungenerated_rule,
                                       Tuple<int> &generated_rule)
{
    BlockSymbol* scope_block = nullptr;
    const char* rule_info = rule_info_holder.c_str();

     b.Put("/**");
    if (ungenerated_rule.Length() > 0)
    {
        b.Put("\n");
        
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
        
        b.Put(" *</em>\n");
        
        b.Put(" *<p>");
    }
    b.Put("\n");
    
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
    
    b.Put(" *</b>\n");
    
    b.Put(" */\n");
}


void GoAction::GenerateListMethods(CTC &ctc,
                                     NTC &,
                                     TextBuffer &b,
                                     const char *,
                                     const char *classname,
                                     ClassnameElement &element,
    const char* super_prefix, const char* def_prefix)
{
    const char *element_name = element.array_element_type_symbol -> Name(),
               *element_type = ctc.FindBestTypeFor(element.array_element_type_symbol -> SymbolIndex());

    //
    // Generate ADD method
    //
    b.Put(def_prefix);
	b + "     " + " AddElement(_" + element_name + " IAst){ \n";
    b + "      " + super_prefix+ "AddElement(_" + element_name + ")\n";

    if (option -> parent_saved)
    {
         b.Put("        ");
       
         b+ "_" + element_name + ".SetParent(my)\n";
    }
    b.Put("    }\n");

    b.Put("\n");
   
    //
    // Generate visitor methods.
    //
    if (option -> visitor == Option::DEFAULT)
    {
        b.Put("\n");
        
    	b.Put(def_prefix);
    	b+"      AcceptWithVisitor(v "+option -> visitor_type+"){\n";

        
        b + "      " + "var i=0\n";
        
        b + "      " + "for ; i < my.Size(); i++{\n";
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol->SymbolIndex()) != NULL)
        {
            
            b + "      " + "  v.Visit(my.Get" + element_name + "At(i))\n";
        }
        else {
            
            b + "      " + "  my.Get" + element_name + "At(i).AcceptWithVisitor(v)\n";
        }
        b + "      " + "}\n";

        b.Put("}\n");

        b.Put(def_prefix);
    	b+"      AcceptWithArg(v  Argument"+option -> visitor_type + ", o interface{}){\n";

        b + "      " + "var i=0\n";
        b + "      " + "for i=0; i < my.Size(); i++{\n";
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol->SymbolIndex()) != NULL)
        {
            
            b + "      " + "  v.VisitWithArg(my.Get" + element_name + "At(i),o)\n";
        }
        else {
            
            b + "      " + "  my.Get" + element_name + "At(i).AcceptWithArg(v,o)\n";
        }
        b + "      " + "}\n";
        b.Put("}\n");

        //
        // Code cannot be generated to automatically visit a node that
        // can return a value. These cases are left up to the user.
        //
        b.Put(def_prefix);
    	b+"      AcceptWithResult(v Result"+option -> visitor_type + ") interface{}{\n";
        b + "      " + "var i=0\n";
        b + "       var result = NewArrayList()\n";
        b + "      " + "for i=0; i < my.Size(); i++{\n";
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol->SymbolIndex()) != NULL)
        {
            
            b + "      " + "  result.Add(v.VisitWithResult(my.Get" + element_name + "At(i)))\n";
        }
        else {
            
            b + "      " + "  result.Add(my.Get" + element_name + "At(i).AcceptWithResult(v))\n";
        }
        b + "      " + "}\n";
    	b.Put("        return result\n");
        



         b.Put("}\n");


        b.Put(def_prefix);
    	b+"      AcceptWithResultArgument(v ResultArgument"+option -> visitor_type + ", o interface{}) interface{}{\n";
        b + "      " + "var i=0\n";
        b + "       var result = NewArrayList()\n";
        b + "      " + "for i=0; i < my.Size(); i++{\n";
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol->SymbolIndex()) != NULL)
        {
            
            b + "      " + "  result.Add(v.VisitWithResultArgument(my.Get" + element_name + "At(i),o))\n";
        }
        else {
            
            b + "      " + "  result.Add(my.Get" + element_name + "At(i).AcceptWithResultArgument(v,o))\n";
        }
        b + "      " + "}\n";
    	b.Put("        return result\n");
        



         b.Put("}\n");

    }
    else if (option -> visitor == Option::PREORDER)
    {
        b.Put("\n");
        b.Put(def_prefix); b.Put("      Accept(v IAstVisitor ){\n");
         b.Put("        if ! v.PreVisit(my){ return }\n");
         b.Put("        var _ctor ,_ = v.(").Put(option->visitor_type).Put(")\n");
         b.Put("        my.Enter(_ctor)\n");
         b.Put("        v.PostVisit(my)\n");
         b.Put("    }\n");

        b.Put(def_prefix); b.Put("     Enter(v  ");
                                     b.Put(option -> visitor_type);
                                     b.Put("){\n");

        
    	b.Put("        var checkChildren = v.Visit").Put(classname).Put("(my)\n");
        
    	b.Put("        if checkChildren{\n");
        
    	b.Put("           var i = 0\n");
        
    	b.Put("           for ; i < my.Size(); i++{\n");

	         b.Put("                ");
	        b + " var element = my.Get" + element_name + "At(i)\n";
	        
    		b + "                if nil !=element{";
	        const char *element_typename = ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex());
	        if (element_typename != NULL)
	        {
	          
	             b.Put("                    if ! v.PreVisit(element){ continue}\n");
	             b.Put("                    element.Enter(v)\n");
	             b.Put("                    v.PostVisit(element)\n");
	           

	        }
	        else
	        {

	             b.Put("                    element.Accept(v)\n");
	        }
	        
    		b+"                }\n";


        
    	b.Put("            }\n");
        
    	b.Put("        }\n");
        
    	b.Put("        v.EndVisit").Put(classname).Put("(my)\n");
        
    	b.Put("    }\n");
    }

    return;
}


//
//
//
void GoAction::GenerateListClass(CTC &ctc,
                                   NTC &ntc,
                                   ActionFileSymbol* ast_filename_symbol,
                                   const char *indentation,
                                   ClassnameElement &element,
                                   Array<const char *> &)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    //Tuple<int> &interface = element.interface_;
    assert(element.array_element_type_symbol != NULL);
    const char* classname = element.real_name,
        * element_name = element.array_element_type_symbol->Name();
               
    bool isInterface = ctc.IsInterface(element.array_element_type_symbol->SymbolIndex());
    std::string type_name;
    
    if(!isInterface){
        type_name = "*";
    }
    type_name  += ctc.FindBestTypeFor(element.array_element_type_symbol->SymbolIndex());
    const auto element_type = type_name.c_str();

    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);

    std::string def_prefix_holder = "func (my *";
    def_prefix_holder += classname;
    def_prefix_holder += ")";
    const auto def_prefix = def_prefix_holder.c_str();
     b + "type " + classname + " struct{\n";
     b + "    *" + abstract_ast_list_classname + "\n}\n";

    if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
    {
         b.Put("    /**\n");
         b.Put("     * The value returned by <b>Get");
                                     b.Put(element_name);
                                     b.Put("At</b> may be <b>null</b>\n");
         b.Put("     */\n");
    }
    b.Put(def_prefix);
	b+ "     " +" Get"+element_name+"At(i int) "+ element_type+"{\n";
    
    b + "     var r,_=my.GetElementAt(i).(" + element_type + ")\n";
     b + "     return r\n";
     b + "     }\n\n";


    //
    // generate constructors
    //
	
	b + "func New" + classname + "(leftToken  IToken, rightToken  IToken , leftRecursive bool)*" + classname + "{\n";
	 b + "      my := new(" + classname + ")\n";
	 b + "      my." + abstract_ast_list_classname + " = New" + abstract_ast_list_classname + "(leftToken, rightToken, leftRecursive)\n";
	 b + "      return my\n";
	 b + "}\n\n";


     b+"    func  New"+classname+"FromElement(element " + element_type+",leftRecursive bool)*"+ classname+"{\n";

    
    b + "        var obj = New" + classname + "(element.GetLeftIToken(),element.GetRightIToken(), leftRecursive)\n";

     b.Put("        obj.list.Add(element)\n");

    if (option->parent_saved)
    {
         b.Put("        ");
         b+castToAny.c_str()+"(element).(IAst).SetParent(obj)\n";
    }
     b.Put("        return obj\n");
     b.Put("    }\n");
    b.Put("\n");

    std::string super_prefix = "my.";
    super_prefix += abstract_ast_list_classname;
    super_prefix += ".";
    GenerateListMethods(ctc, ntc, b, indentation, classname, element, super_prefix.c_str(),def_prefix);

    b.Put("\n\n");// Generate Class Closer
    b + templateAnyCastToStruct(classname).c_str();

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
void GoAction::GenerateListExtensionClass(CTC& ctc,
    NTC&,
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

    std::string def_prefix_holder = "func (my *";
    def_prefix_holder += special_array.name;
    def_prefix_holder += ")";
    const auto def_prefix = def_prefix_holder.c_str();


     b + "type " + special_array.name + " struct{\n";
     b + "    *" + classname + "\n";
     b + "     environment *" + option->action_type + "\n}\n";


    GenerateEnvironmentDeclaration(b, indentation, def_prefix);

      
	b +  "func New" + special_array.name + 
        "(environment *" +option->action_type + ", leftIToken  IToken,  rightIToken  IToken, leftRecursive bool)*"+ special_array.name+"{\n";

     b + "      my := new(" + special_array.name + ")\n";
     b + "      my." + classname + " = New" + classname + "(leftIToken, rightIToken, leftRecursive)\n";
     b + "      my.environment = environment;\n";
     b + "      my.Initialize()\n";
     b + "      return my\n";
     b + "}\n\n";

    b + "    func  New" + special_array.name + "FromElement(environment *" + option->action_type +
        ", element " + element_type + ",leftRecursive bool) *" + special_array.name + "{\n";

    
	b+"        var obj = New"+special_array.name + "(environment,element.GetLeftIToken(),element.GetRightIToken(), leftRecursive)\n";
   
     b.Put("        obj.list.Add(element)\n");

    if (option->parent_saved)
    {
         b.Put("        ");
        b.Put("element.SetParent(obj)\n");
    }
     b.Put("        return obj\n");
     b.Put("    }\n");
    b.Put("\n");


    b.Put("\n\n");// Generate Class Closer
    
    b + templateAnyCastToStruct(classname).c_str();
    if (option->IsTopLevel())
    {
        ast_filename_symbol->Flush();
    }

}


//
// Generate a generic rule class
//
void GoAction::GenerateRuleClass(CTC &ctc,
                                   NTC &ntc,
                                   ActionFileSymbol* ast_filename_symbol,
                                   const char *indentation,
                                   ClassnameElement &element,
                                   Array<const char *> &)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    char *classname = element.real_name;
    SymbolLookupTable &symbol_set = element.symbol_set;
    Tuple<int> &rhs_type_index = element.rhs_type_index;

    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);

    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);

    assert(element.rule.Length() == 1);
    //int rule_no = element.rule[0];

    std::string def_prefix_holder = "func (my *";
    def_prefix_holder += classname;
    def_prefix_holder += ")";
    const auto def_prefix = def_prefix_holder.c_str();

	 b + "type " + classname + " struct{\n";
    if (element.is_terminal_class)
    {

         b + "    *" + grammar->Get_ast_token_classname() + "\n";
      
        
        if (element.needs_environment){
            b + "     environment *" + option->action_type + "\n";
        }
        b.Put("}\n");


        if (element.needs_environment) {
            GenerateEnvironmentDeclaration(b, indentation, def_prefix);
        }
        if (symbol_set.Size() == 1) // if the right-hand side contains a symbol ...
        {
            b.Put(def_prefix); b.Put("      Get");
                                         b.Put(symbol_set[0] -> Name());
                                         b.Put("()IToken{ return my.leftIToken; }\n\n");
        }

         b + "func New" + classname + "(";
        if (element.needs_environment)
        {
            b+"environment *"+ option->action_type + ",token IToken )*"+ classname+"{\n";
             b + "      my := new(" + classname + ")\n";
             b + "      my.environment = environment;\n";
        }
        else
        {
            b + "token IToken )*" + classname + "{\n";
             b + "      my := new(" + classname + ")\n";
        }

         b + "      my." + grammar->Get_ast_token_classname() + " = New" + grammar->Get_ast_token_classname() + "(token)\n";
         b + "      my.Initialize()\n";
         b + "      return my\n";
         b + "    }\n";

    }
    else 
    {

         b + "    *" + option->ast_type + "\n";

        
        if (element.needs_environment) {
            b + "     environment *" + option->action_type + "\n";
        }



        for (int i = 0; i < symbol_set.Size(); i++)
        {
             b+"      _"+symbol_set[i] -> Name()+" ";
             if(!ctc.IsInterface(rhs_type_index[i]))
             {
                 b + "*";
             }
        	 b+ctc.FindBestTypeFor(rhs_type_index[i])+"\n";

        }
    
        b.Put("}\n");

        for (int i = 0; i < symbol_set.Size(); i++)
        {
            const char* symbolName = symbol_set[i]->Name();
            const char* bestType = ctc.FindBestTypeFor(rhs_type_index[i]);
            auto is_interface = ctc.IsInterface(rhs_type_index[i]);
            if (ntc.CanProduceNullAst(rhs_type_index[i]))
            {
            	b.Put("    /**\n");
            	b.Put("     * The value returned by <b>Get");
                b.Put(symbolName);
                b.Put("</b> may be <b>null</b>\n");
            	b.Put("     */\n");
              
            }

            // Generate getter method
            b.Put(def_prefix); b.Put("     ");
            b.Put(" Get");
            b.Put(symbolName);
            b.Put("()");
            b.Put(" ");
            if(!is_interface){
                b.Put("*");
            }
            b.Put(bestType);
           
            b.Put("{ return my._");
            b.Put(symbolName);
            b.Put("}\n");

            // Generate setter method
            b.Put(def_prefix); b.Put("      Set");
            b.Put(symbolName);
            b.Put("(");
            b.Put(" _"); // add "_" prefix to arg name in case symbol happens to be a Java keyword
            b.Put(symbolName);
            b.Put(" ");
            if (!is_interface) {
                b.Put("*");
            }
            b.Put(bestType);
            b.Put(") ");
            b.Put(" { my._");
            b.Put(symbolName);
            b.Put(" = _");
            b.Put(symbolName);
            b.Put(" }\n");
        }
        b.Put("\n");
        

        if (element.needs_environment)
        {
            GenerateEnvironmentDeclaration(b, indentation, def_prefix);
        }
        //
        // generate constructor
        //
        const char *header = "func New";
         b + header + classname + "(";

        int length = strlen(indentation) + strlen(header);

 
        if (element.needs_environment)
        {
            b.Put("environment *");
            b.Put(option -> action_type);
            b.Put(",");
        }
        b.Put("leftIToken IToken, rightIToken IToken ");
        b.Put(symbol_set.Size() == 0 ? "" : ",\n");
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            for (int k = 0; k <= length; k++)
                b.PutChar(' ');
         
            b.Put(" _");
            b.Put(symbol_set[i] -> Name());
            b.Put(" ");
            if(!ctc.IsInterface(rhs_type_index[i])){
                b.Put("*");
            }
            b.Put(ctc.FindBestTypeFor(rhs_type_index[i]));

            b.Put(i == symbol_set.Size() - 1 ? ")" : ",\n");
        }
        b.Put("*").Put(classname).Put("{\n");

         b + "      my := new(" + classname + ")\n";
         b + "      my." + option->ast_type + " = New" + option->ast_type + "2(leftIToken, rightIToken)\n";
        if (element.needs_environment)
        {
            b.Put("        my.environment = environment\n");
        }

        for (int i = 0; i < symbol_set.Size(); i++)
        {
             b.Put("        my._");
                                         b.Put(symbol_set[i] -> Name());
                                         b.Put(" = _");
                                         b.Put(symbol_set[i] -> Name());
                                         b.Put(";\n");

            if (option -> parent_saved)
            {
                 b.Put("        ");
               
                b+"if nil != _" + symbol_set[i]->Name()+"{\n";
                b.Put("        var trait_ interface{} = _").Put(symbol_set[i]->Name()).Put("\n");
            	b.Put("        ");
                b + " trait_.(IAst).SetParent(my)\n";

            	b.Put("}\n");
            }
        }
         b.Put("        my.Initialize()\n");
         b.Put("        return my\n");
         b.Put("    }\n");
    }

    if (option -> parent_saved)
    {
	    GenerateGetAllChildrenMethod(b, indentation, element,def_prefix);
    }

	GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set,def_prefix);


    b.Put("\n\n");
    b + templateAnyCastToStruct(classname).c_str();
    if (option->IsTopLevel())
    {
        ast_filename_symbol->Flush();
    }
    return;
}


//
// Generate Ast class
//
void GoAction::GenerateTerminalMergedClass(NTC &ntc,
                                             ActionFileSymbol* ast_filename_symbol,
                                             const char *indentation,
                                             ClassnameElement &element,
                                             Array<const char *> &)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    char *classname = element.real_name;
    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);

    std::string def_prefix_holder = "func (my *";
    def_prefix_holder += classname;
    def_prefix_holder += ")";
    const auto def_prefix = def_prefix_holder.c_str();

    b + "type " + classname + " struct{\n";
	b + "    *" + grammar->Get_ast_token_classname() + "\n";

    
    if (element.needs_environment) {
        b + "     environment *" + option->action_type + "\n";
    }
    b.Put("}\n");


    if (element.needs_environment){
	    GenerateEnvironmentDeclaration(b, indentation, def_prefix);
    }

    SymbolLookupTable &symbol_set = element.symbol_set;
    if (symbol_set.Size() == 1) // if the right-hand side contains a symbol ...
    {
        b.Put(def_prefix); b.Put("      Get");
                                     b.Put(symbol_set[0] -> Name());
                                     b.Put("() IToken{ return my.leftIToken; }\n\n");
    }

      b + "func New" + classname + "(";
     if (element.needs_environment)
     {
         b + "environment *" + option->action_type + ",token IToken )*"+ classname+"{\n";
          b + "      my := new(" + classname + ")\n";
          b + "      my.environment = environment;\n";

     }
     else
     {
         b + "token IToken )*" + classname + "{\n";
          b + "      my := new(" + classname + ")\n";
     }
      b + "      my." + grammar->Get_ast_token_classname() + " = New" + grammar->Get_ast_token_classname() + "(token)\n";
      b + "      my.Initialize()\n";
      b + "      return my\n";
      b + "    }\n";

    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);
  
    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set, def_prefix);

    b.Put("\n\n");// Generate Class Closer
    
    b + templateAnyCastToStruct(classname).c_str();
    if (option->IsTopLevel()){
        ast_filename_symbol->Flush();
    }

}


//
// Generate Ast class
//
void GoAction::GenerateMergedClass(CTC &ctc,
                                     NTC &ntc,
                                     ActionFileSymbol* ast_filename_symbol,
                                     const char *indentation,
                                     ClassnameElement &element,
                                     Tuple< Tuple<ProcessedRuleElement> > &processed_rule_map,
                                     Array<const char *> &)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    char *classname = element.real_name;
    SymbolLookupTable &symbol_set = element.symbol_set;
    Tuple<int> &rhs_type_index = element.rhs_type_index;

    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);
    std::string def_prefix_holder = "func (my *";
    def_prefix_holder += classname;
    def_prefix_holder += ")";
    const auto def_prefix = def_prefix_holder.c_str();
     b + "type " + classname + " struct{\n";
     b + "    *" + option->ast_type + "\n";
    
    if (element.needs_environment) {
        b + "     environment *" + option->action_type + "\n";
    }
    //
	// Compute the set of symbols that always appear in an instance creation
	// of my merged class for which a null instance allocation will never occur.
	//
    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);
    Tuple<int>& rule = element.rule;
    
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
    
    
    for (int i = 0; i < symbol_set.Size(); i++)
    {
         b + "      _" + symbol_set[i]->Name() + " ";
         if (!ctc.IsInterface(rhs_type_index[i]))
         {
             b + "*";
         }
         b + ctc.FindBestTypeFor(rhs_type_index[i]) + "\n";
    }

    b.Put("}\n");

    for (int i = 0; i < symbol_set.Size(); i++)
    {
        const char* symbolName = symbol_set[i]->Name();
        const char* bestType = ctc.FindBestTypeFor(rhs_type_index[i]);
        auto is_interface = ctc.IsInterface(rhs_type_index[i]);
        if ((! optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
        {
             b.Put("    /**\n");
             b.Put("     * The value returned by <b>Get");
             b.Put(symbolName);
             b.Put("</b> may be <b>null</b>\n");
             b.Put("     */\n");
        }

        // Generate getter method
        b.Put(def_prefix); b.Put("     ");
        b.Put(" Get");
        b.Put(symbolName);
        b.Put("()");
        b.Put(" ");
        if (!is_interface) {
            b.Put("*");
        }
        b.Put(bestType);

        b.Put("{ return my._");
        b.Put(symbolName);
        b.Put("}\n");
    }

    b.Put("\n");
    if (element.needs_environment) {
        GenerateEnvironmentDeclaration(b, indentation, def_prefix);
    }

    //
    // generate merged constructor
    //
    const char* header = "func New";
     b + header + classname + "(";

    int length = strlen(indentation) + strlen(header);


    if (element.needs_environment)
    {
        b.Put("environment *");
        b.Put(option->action_type);
        b.Put(",");
    }
    b.Put("leftIToken IToken, rightIToken IToken ");

    b.Put(symbol_set.Size() == 0 ? "" : ",\n");
    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            for (int k = 0; k <= length; k++)
                b.PutChar(' ');
            b.Put(" _");
            b.Put(symbol_set[i] -> Name());
            b.Put(" ");
            if (!ctc.IsInterface(rhs_type_index[i])) {
                b.Put("*");
            }
            b.Put(ctc.FindBestTypeFor(rhs_type_index[i]));
            if ((!optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
            {
                b.Put(" ");
            }
         
            b.Put(i == symbol_set.Size() - 1 ? ")" : ",\n");
        }
    }
    b.Put("*").Put(classname).Put("{\n");

     b + "      my := new(" + classname + ")\n";
     b + "      my." + option->ast_type + " = New" + option->ast_type + "2(leftIToken, rightIToken)\n";
    if (element.needs_environment)
    {
        b.Put("        my.environment = environment\n");
    }

    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
             b.Put("        my._");
                                         b.Put(symbol_set[i] -> Name());
                                         b.Put(" = _");
                                         b.Put(symbol_set[i] -> Name());
                                         b.Put("\n");

            if (option->parent_saved){
            	b.Put("        ");
                b + "if nil != _" + symbol_set[i]->Name()+
                    "{" + castToAny.c_str() + "(_" + symbol_set[i]->Name() + ").(IAst).SetParent(my) }\n"; 
            }
        }
    }

     b.Put("        my.Initialize()\n");
     b.Put("        return my\n");
     b.Put("      }\n");

    if (option -> parent_saved)
        GenerateGetAllChildrenMethod(b, indentation, element,def_prefix);
  
    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set,def_prefix);

   b.Put("\n\n");// Generate Class Closer

   b + templateAnyCastToStruct(classname).c_str();

    if (option->IsTopLevel())
    {
        ast_filename_symbol->Flush();
    }
    return;
}

void GoAction::GenerateAstRootInterface(
    ActionFileSymbol* ast_filename_symbol,
    const char*)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    
	b+"type "+ astRootInterfaceName.c_str() + " interface{\n";
     b.Put("     GetLeftIToken()  IToken\n");
     b.Put("     GetRightIToken()  IToken\n");

    GenerateVisitorHeaders(b, "", "", nullptr);
	b.Put("}\n\n");

    b + "func " + castToAny.c_str() + "(i interface{}) interface{}{return i}";
    b.Put("\n\n");
    //castToAny
    b + templateAnyCastToInterface(astRootInterfaceName.c_str()).c_str();
    return;
}
void GoAction::GenerateInterface(bool is_terminal,
                                   ActionFileSymbol* ast_filename_symbol,
                                   const char *,
                                   const char *interface_name,
                                   Tuple<int> &extension,
                                   Tuple<int> &classes,
                                   Tuple<ClassnameElement> &classname)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
     b.Put("/**");
    if (is_terminal)
    {
        b.Put("\n");
          b.Put(" * is always implemented by <b>");
                                      b.Put(grammar -> Get_ast_token_classname());
                                      b.Put("</b>. It is also implemented by");
    }
    else 
    {
        b.Put("\n");
        
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
        
        b.Put(" *<b>\n");
         b.Put(" *<ul>");
        for (int i = 0; i < classes.Length(); i++)
        {
            b.Put("\n");
            
            b.Put(" *<li>");
            b.Put(classname[classes[i]].real_name);
        }
        b.Put("\n");
        
        b.Put(" *</ul>\n");
        
        b.Put(" *</b>");
    }

    b.Put("\n");
    
    b.Put(" */\n");
    b + "type " + interface_name + " interface{\n";
    if (extension.Length() > 0)
    {
        for (int k = 0; k < extension.Length() - 1; k++)
        {
            b.PutChar('I');
            b.Put(extension[k] == grammar->Get_ast_token_interface()
                ? grammar->Get_ast_token_classname()
                : grammar->RetrieveString(extension[k]));
            b.Put("\n");
        }
        b.PutChar('I');
        b.Put(extension[extension.Length() - 1] == grammar->Get_ast_token_interface()
            ? grammar->Get_ast_token_classname()
            : grammar->RetrieveString(extension[extension.Length() - 1]));
        b.Put("\n");
    }
    else
    {
      

        b.Put(astRootInterfaceName.c_str()).Put("\n");
       
    }
    b.Put("}\n\n");
    b + templateAnyCastToInterface(interface_name).c_str();
}


//
//
//
void GoAction::GenerateNullAstAllocation(TextBuffer &b, int rule_no)
{
    const char *code = "\n                    my.SetResult(nil);";
    GenerateCode(&b, code, rule_no);

    return;
}



//
//
//
void GoAction::GenerateAstAllocation(CTC& ctc,
    NTC& ntc,
    TextBuffer& b,
    RuleAllocationElement& allocation_element,
    Tuple<ProcessedRuleElement>& processed_rule_elements,
    Array<const char*>&, int rule_no)
{
    const char* classname = allocation_element.name;

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
    const char* space = "\n                    ",
        * space4 = "    ",
        * newkey = option->factory,
        * lparen = "(",
        * comma = ",",
        * rparen = ")",
        * trailer = ")";
    int extra_space_length = strlen(space) + strlen(space4) + strlen(newkey) + strlen(classname) + 1;
    char* extra_space = new char[extra_space_length + 1];
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
    //        GenerateCode(&b, "GetRhsSym(", rule_no);
    //        IntToString index(position[0]);
    //        GenerateCode(&b, index.string(), rule_no);
    //        GenerateCode(&b, rparen, rule_no);
    //    }
    //
    if (allocation_element.is_terminal_class && (grammar->RhsSize(rule_no) == 1 && grammar->IsNonTerminal(grammar->rhs_sym[grammar->FirstRhsIndex(rule_no)])))
    {
        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, "//", rule_no);
        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, "// When garbage collection is not available, delete ", rule_no);
        GenerateCode(&b, "GetRhsSym(1)", rule_no);
        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, "//", rule_no);
    }
    GenerateCode(&b, space, rule_no);
    GenerateCode(&b, "my.SetResult(", rule_no);
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
        GenerateCode(&b, "my, ", rule_no);
    }
    if (allocation_element.is_terminal_class)
    {
        GenerateCode(&b, "my.GetRhsIToken(1)", rule_no);
        //
        // TODO: Old bad idea. Remove at some point...
        //
        //
        //        assert(position.Length() <= 1);
        //
        //        GenerateCode(&b, "GetRhsIToken(", rule_no);
        //        IntToString index(position.Length() == 0 ? 1 : position[0]);
        //        GenerateCode(&b, index.string(), rule_no);
        //        GenerateCode(&b, rparen, rule_no);
        //
    }
    else
    {
        GenerateCode(&b, "my.GetLeftIToken()", rule_no);
        GenerateCode(&b, ", ", rule_no);
        GenerateCode(&b, "my.GetRightIToken()", rule_no);
        if (position.Length() > 0)
        {
            GenerateCode(&b, comma, rule_no);
            GenerateCode(&b, extra_space, rule_no);
            GenerateCode(&b, current_line_input_file_info.c_str(), rule_no);
            GenerateCode(&b, extra_space, rule_no);

            int offset = grammar->FirstRhsIndex(rule_no) - 1;
            for (int i = 0; i < position.Length(); i++)
            {
                if (position[i] == 0)
                {
                    /*       GenerateCode(&b, lparen, rule_no);
                           GenerateCode(&b, ctc.FindBestTypeFor(type_index[i]), rule_no);
                           GenerateCode(&b, rparen, rule_no);*/
                    GenerateCode(&b, "nil", rule_no);
                }
                else
                {
                    int symbol = grammar->rhs_sym[offset + position[i]];
                    bool manybeNull = false;
                    if (ntc.CanProduceNullAst(type_index[i]))
                    {
                        manybeNull = true;
                    }
                    const char* actual_type = ctc.FindBestTypeFor(type_index[i]);

                    if (grammar->IsTerminal(symbol))
                    {
                       

                        GenerateCode(&b, newkey, rule_no);
                        GenerateCode(&b, grammar->Get_ast_token_classname(), rule_no);
                        GenerateCode(&b, lparen, rule_no);
                        if(!manybeNull)
                        {
                            GenerateCode(&b, "my.GetRhsIToken(", rule_no);
                            IntToString index(position[i]);
                            GenerateCode(&b, index.String(), rule_no);
                            GenerateCode(&b, rparen, rule_no);

                            if (strcmp(actual_type, grammar->Get_ast_token_classname()) != 0)
                            {
                                GenerateCode(&b, ".(* ", rule_no);
                                GenerateCode(&b, actual_type, rule_no);
                                GenerateCode(&b, ")", rule_no);
                            }
                        }else
                        {
  
                            if (strcmp(actual_type, grammar->Get_ast_token_classname()) != 0)
                            {
                                std::string anyCastToHelper = "AnyCastTo";
                                anyCastToHelper += actual_type;
                                anyCastToHelper += "(";
                                GenerateCode(&b, anyCastToHelper.c_str(), rule_no);

                                GenerateCode(&b, "my.GetRhsIToken(", rule_no);
                                IntToString index(position[i]);
                                GenerateCode(&b, index.String(), rule_no);
                                GenerateCode(&b, rparen, rule_no);


                                GenerateCode(&b, ")", rule_no);
                            }else
                            {
                                GenerateCode(&b, "my.GetRhsIToken(", rule_no);
                                IntToString index(position[i]);
                                GenerateCode(&b, index.String(), rule_no);
                                GenerateCode(&b, rparen, rule_no);
                            }
                        }
                        GenerateCode(&b, rparen, rule_no);

                    }
                    else
                    {
                        if (!manybeNull)
                        {
                            GenerateCode(&b, "my.GetRhsSym(", rule_no);
                            IntToString index(position[i]);
                            GenerateCode(&b, index.String(), rule_no);
                            GenerateCode(&b, rparen, rule_no);


                            if (!ctc.IsInterface(type_index[i])) {
                                GenerateCode(&b, ".(*", rule_no);
                            }
                            else
                            {
                                GenerateCode(&b, ".(", rule_no);
                            }
                            GenerateCode(&b, ctc.FindBestTypeFor(type_index[i]), rule_no);
                            GenerateCode(&b, ")", rule_no);
                        }
                    	else
                        {
                            std::string anyCastToHelper = "AnyCastTo";
                            anyCastToHelper += actual_type;
                            anyCastToHelper += "(";
                            GenerateCode(&b, anyCastToHelper.c_str(), rule_no);

                            GenerateCode(&b, "my.GetRhsSym(", rule_no);
                            IntToString index(position[i]);
                            GenerateCode(&b, index.String(), rule_no);
                            GenerateCode(&b, rparen, rule_no);

                            GenerateCode(&b, ")", rule_no);
                        }

                    }


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
    GenerateCode(&b, ",", rule_no);
    GenerateCode(&b, space, rule_no);
    GenerateCode(&b, current_line_input_file_info.c_str(), rule_no);
    GenerateCode(&b, space, rule_no);
    GenerateCode(&b, trailer, rule_no);

    delete[] extra_space;

    return;
}

//
//
//
void GoAction::GenerateListAllocation(CTC& ctc,
    NTC&,
    TextBuffer& b,
    int rule_no, RuleAllocationElement& allocation_element)
{
    const char* space = "\n                    ",
        * space4 = "    ",
        * newkey = option->factory,
        * lparen = "(",
        * comma = ",",
        * rparen = ")",
        * trailer = ")";

    if (allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_EMPTY ||
        allocation_element.list_kind == RuleAllocationElement::RIGHT_RECURSIVE_EMPTY ||
        allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_SINGLETON ||
        allocation_element.list_kind == RuleAllocationElement::RIGHT_RECURSIVE_SINGLETON)
    {

        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, "my.SetResult(", rule_no);
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
                GenerateCode(&b, "my, ", rule_no);
            }

            GenerateCode(&b, "my.GetLeftIToken()", rule_no);
            GenerateCode(&b, ", ", rule_no);
            GenerateCode(&b, "my.GetRightIToken()", rule_no);
            GenerateCode(&b, comma, rule_no);
            if (allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_EMPTY)
                GenerateCode(&b, " true /* left recursive */", rule_no);
            else GenerateCode(&b, " false /* not left recursive */", rule_no);
        }
        else
        {

           
            GenerateCode(&b, "New", rule_no);
            GenerateCode(&b, allocation_element.name, rule_no);
            GenerateCode(&b, "FromElement", rule_no);
            GenerateCode(&b, lparen, rule_no);
            if (allocation_element.needs_environment)
            {
                GenerateCode(&b, "my, ", rule_no);
            }
            assert(allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_SINGLETON ||
                allocation_element.list_kind == RuleAllocationElement::RIGHT_RECURSIVE_SINGLETON);

            if (grammar->IsTerminal(allocation_element.element_symbol))
            {
                GenerateCode(&b, newkey, rule_no);
                GenerateCode(&b, grammar->Get_ast_token_classname(), rule_no);
                GenerateCode(&b, lparen, rule_no);
                GenerateCode(&b, "my.GetRhsIToken(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&b, index.String(), rule_no);
                GenerateCode(&b, rparen, rule_no);
                GenerateCode(&b, rparen, rule_no);
            }
            else
            {
                GenerateCode(&b, "my.GetRhsSym(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&b, index.String(), rule_no);
                GenerateCode(&b, rparen, rule_no);
               
                if(!ctc.IsInterface(allocation_element.element_type_symbol_index)){
                    GenerateCode(&b, ".(*", rule_no);
                }else
                {
                    GenerateCode(&b, ".(", rule_no);
                }
                GenerateCode(&b, ctc.FindBestTypeFor(allocation_element.element_type_symbol_index), rule_no);
                GenerateCode(&b, ")", rule_no);

            }


            GenerateCode(&b, comma, rule_no);
            if (allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_SINGLETON)
                GenerateCode(&b, " true /* left recursive */", rule_no);
            else GenerateCode(&b, " false /* not left recursive */", rule_no);
        }

        GenerateCode(&b, rparen, rule_no);
        GenerateCode(&b, ",", rule_no);
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

	            GenerateCode(&b, "my.GetRhsSym(", rule_no);
	            IntToString index(allocation_element.list_position);
	            GenerateCode(&b, index.String(), rule_no);
	            GenerateCode(&b, ")", rule_no);

	            GenerateCode(&b, ".(*", rule_no);
	            GenerateCode(&b, allocation_element.name, rule_no);
	            GenerateCode(&b, ")", rule_no);

            GenerateCode(&b, ")", rule_no);

            GenerateCode(&b, ".AddElement(", rule_no);
            if (grammar->IsTerminal(allocation_element.element_symbol))
            {
                GenerateCode(&b, newkey, rule_no);
                GenerateCode(&b, grammar->Get_ast_token_classname(), rule_no);
                GenerateCode(&b, lparen, rule_no);
                GenerateCode(&b, "my.GetRhsIToken(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&b, index.String(), rule_no);
                GenerateCode(&b, rparen, rule_no);
                if (allocation_element.list_position != 1) // a right-recursive rule? set the list as result
                {
                    GenerateCode(&b, rparen, rule_no);
                    GenerateCode(&b, trailer, rule_no);

                    GenerateCode(&b, space, rule_no);
                    GenerateCode(&b, "my.SetResult(", rule_no);
                    GenerateCode(&b, "my.GetRhsSym(", rule_no);
                    IntToString index(allocation_element.list_position);
                    GenerateCode(&b, index.String(), rule_no);
                }
                GenerateCode(&b, rparen, rule_no);
               
            }
            else
            {
                GenerateCode(&b, "my.GetRhsSym(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&b, index.String(), rule_no);


                if (allocation_element.list_position != 1) // a right-recursive rule? set the list as result
                {
                    GenerateCode(&b, rparen, rule_no);
                    GenerateCode(&b, ".(IAst)", rule_no);
                    GenerateCode(&b, trailer, rule_no);

                    GenerateCode(&b, space, rule_no);
                    GenerateCode(&b, "my.SetResult(", rule_no);
                    GenerateCode(&b, "my.GetRhsSym(", rule_no);
                    IntToString index(allocation_element.list_position);
                    GenerateCode(&b, index.String(), rule_no);
                    GenerateCode(&b, rparen, rule_no);
            
                }
                else
                {
                    GenerateCode(&b, rparen, rule_no);
                    GenerateCode(&b, ".(IAst)", rule_no);
                }
            }

        }

        //
        // Copy a list that is not the first element on the right-hand side of the rule
        //
        else
        {
            assert(allocation_element.list_kind == RuleAllocationElement::COPY_LIST);

            GenerateCode(&b, space, rule_no);
            GenerateCode(&b, "my.SetResult(", rule_no);
            GenerateCode(&b, "my.GetRhsSym(", rule_no);
            IntToString index(allocation_element.list_position);
            GenerateCode(&b, index.String(), rule_no);
            GenerateCode(&b, rparen, rule_no);
            GenerateCode(&b, ".(*", rule_no);
            GenerateCode(&b, allocation_element.name, rule_no);
            GenerateCode(&b, ")", rule_no);
        }


    }

    GenerateCode(&b, trailer, rule_no);

    return;
}
