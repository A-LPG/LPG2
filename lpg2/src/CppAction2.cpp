#include "CTC.h"
#include "NTC.h"
#include "CppAction2.h"

#include <map>
#include <string>

#include "LCA.h"
#include "TTC.h"


//
//
//
void CppAction2::ExpandExportMacro(TextBuffer *buffer, SimpleMacroSymbol *simple_macro)
{
    buffer -> Put(option -> exp_type);
    buffer -> Put("::");
    buffer -> Put(option -> exp_prefix);
    buffer -> Put(simple_macro -> Name() + 2); // skip initial escape and '_' characters
    buffer -> Put(option -> exp_suffix);
}


//
//
//
void CppAction2::GenerateDefaultTitle(Tuple<ActionBlockElement> &notice_actions)
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


    return;
}

namespace
{
    void add_forward_class_def(ActionFileSymbol* ast_filename_symbol, const char* classname)
    {
        TextBuffer& header_buffer = *(ast_filename_symbol->InitialHeadersBuffer());
        header_buffer.Put("  struct ");
        header_buffer.Put(classname);
        header_buffer.Put(";\n");
    }
}
//
// First construct a file for this type. Write the title information to its header
// buffer and return the file.
//
ActionFileSymbol *CppAction2::GenerateTitle(ActionFileLookupTable &ast_filename_table,
                                            Tuple<ActionBlockElement> &notice_actions,
                                            const char *type_name,
                                            bool needs_environment)
{
    const char* filetype = option-> GetFileTypeWithLanguage();
   
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


ActionFileSymbol *CppAction2::GenerateTitleAndGlobals(ActionFileLookupTable &ast_filename_table,
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


void CppAction2::ProcessAstActions(Tuple<ActionBlockElement>& actions,
    Tuple<ActionBlockElement>& notice_actions,
    Tuple<ActionBlockElement>& initial_actions,
    Array<const char*>& typestring,
    Tuple< Tuple<ProcessedRuleElement> >& processed_rule_map,
    SymbolLookupTable& classname_set,
    Tuple<ClassnameElement>& classname)
{
    ActionFileLookupTable ast_filename_table(4096);
	
    auto  ast_filename_symbol = option->DefaultBlock()->ActionfileSymbol();
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
	
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
    ActionFileSymbol* top_level_ast_file_symbol = nullptr;
    //
    // First process the root class, the list class, and the Token class.
    //
    if (option->automatic_ast == Option::TOPLEVEL)
    {
	    {
            top_level_ast_file_symbol = GenerateTitleAndGlobals(ast_filename_table, notice_actions, option->top_level_ast_file_prefix, false);
            TextBuffer& header_buffer = *(top_level_ast_file_symbol->InitialHeadersBuffer());
            header_buffer.Put("#pragma once\n");
            header_buffer.Put(R"(#include "IAbstractArrayList.h")");
            header_buffer.Put("\n");
	    	if(option->visitor  == Option::DEFAULT)
	    	{
                header_buffer.Put(R"(#include "Any.h")");
                header_buffer.Put("\n");
	    	}
            header_buffer.Put(R"(#include "IAst.h")");
            header_buffer.Put("\n");
            header_buffer.Put("namespace ");
            header_buffer.Put(option->top_level_ast_file_prefix);
            header_buffer.Put("{\n");
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
            * result_argument_visitor_type = new char[strlen(result) + strlen(argument) + strlen(visitor_type) + 1];
       
        strcpy(argument_visitor_type, argument);
        strcat(argument_visitor_type, visitor_type);

        strcpy(result_visitor_type, result);
        strcat(result_visitor_type, visitor_type);

        strcpy(result_argument_visitor_type, result);
        strcat(result_argument_visitor_type, argument);
        strcat(result_argument_visitor_type, visitor_type);

    
        if (option->visitor == Option::DEFAULT)
        {
            if (option->IsNested())
            {
                GenerateSimpleVisitorInterface(ast_filename_symbol, "    ", visitor_type, type_set);
                GenerateArgumentVisitorInterface(ast_filename_symbol, "    ", argument_visitor_type, type_set);
                GenerateResultVisitorInterface(ast_filename_symbol, "    ", result_visitor_type, type_set);
                GenerateResultArgumentVisitorInterface(ast_filename_symbol, "    ", result_argument_visitor_type, type_set);

            }
            else
            {
              
                GenerateSimpleVisitorInterface(top_level_ast_file_symbol, "", visitor_type, type_set);



                GenerateArgumentVisitorInterface(top_level_ast_file_symbol, "", argument_visitor_type, type_set);



                GenerateResultVisitorInterface(top_level_ast_file_symbol, "", result_visitor_type, type_set);



                GenerateResultArgumentVisitorInterface(top_level_ast_file_symbol, "", result_argument_visitor_type, type_set);



            }
        }
        else if (option->visitor == Option::PREORDER)
        {
            if (option->IsNested())
            {
                GeneratePreorderVisitorInterface(ast_filename_symbol, "    ", visitor_type, type_set);

            }
            else
            {
               
                GeneratePreorderVisitorInterface(top_level_ast_file_symbol, "", visitor_type, type_set);

            }
        }

        delete[] argument_visitor_type;
        delete[] result_visitor_type;
        delete[] result_argument_visitor_type;

    }
	
    {
        if (option->IsNested())
        {
            GenerateAstType(ast_filename_symbol, "    ", option->ast_type);
            GenerateAbstractAstListType(ast_filename_symbol, "    ", abstract_ast_list_classname);
        }
        else
        {
            assert(option->automatic_ast == Option::TOPLEVEL);

  
            GenerateAstType(top_level_ast_file_symbol, "", option->ast_type);
           

        
            GenerateAbstractAstListType(top_level_ast_file_symbol, "", abstract_ast_list_classname);
           

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
        {
	        GenerateInterface(true /* is token */,
						ast_filename_symbol,
						(char*)"    ",
						ast_token_interfacename,
						extension_of[grammar->Get_ast_token_interface()],
						interface_map[grammar->Get_ast_token_interface()],
						classname);
            GenerateAstTokenType(ntc, ast_filename_symbol, "    ", grammar->Get_ast_token_classname());
        }
        else
        {
          
            GenerateInterface(true /* is token */,
                              top_level_ast_file_symbol,
                              (char*)"",
                              ast_token_interfacename,
                              extension_of[grammar->Get_ast_token_interface()],
                              interface_map[grammar->Get_ast_token_interface()],
                              classname);
           

           
            GenerateAstTokenType(ntc, top_level_ast_file_symbol, "", grammar->Get_ast_token_classname());
       
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
                                  ast_filename_symbol,
                                  (char*)"    ",
                                  interface_name,
                                  extension_of[symbol],
                                  interface_map[symbol],
                                  classname);
            else
            {
              
                GenerateInterface(ctc.IsTerminalClass(symbol),
                                  top_level_ast_file_symbol,
                                  (char*)"",
                                  interface_name,
                                  extension_of[symbol],
                                  interface_map[symbol],
                                  classname);
               
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
	                               ? ast_filename_symbol
	                               : top_level_ast_file_symbol),
                              (option->IsNested()
	                               ? (char*)"    "
	                               : (char*)""),
                              classname[i],
                              typestring);

            for (int j = 0; j < classname[i].special_arrays.Length(); j++)
            {
                //
                // Finish up the previous class we were procesing
                //
                if (option->IsNested()) // Generate Class Closer
                    b.Put("    };\n\n");
                else
                {
                    top_level_ast_file_symbol->BodyBuffer()->Put("};\n\n");
                    top_level_ast_file_symbol->Flush();
                }

                //
                // Process the new special array class.
                //

                GenerateListExtensionClass(ctc,
                                           ntc,
                                           (option->IsNested()
	                                            ? ast_filename_symbol
	                                            : top_level_ast_file_symbol),
                                           (option->IsNested()
	                                            ? (char*)"    "
	                                            : (char*)""),
                                           classname[i].special_arrays[j],
                                           classname[i],
                                           typestring);

                //
                // Generate final info for the allocation of rules associated with this class
                //
                Tuple<int>& special_rule = classname[i].special_arrays[j].rules;
                for (int k = 0; k < special_rule.Length(); k++)
                {
                    int rule_no = special_rule[k];
                    Tuple<ActionBlockElement>& actions = rule_action_map[rule_no];
                    if (top_level_ast_file_symbol != NULL) // possible when option -> automatic_ast == Option::TOPLEVEL
                    {
                        for (int l = 0; l < actions.Length(); l++)
                            actions[l].buffer = top_level_ast_file_symbol->BodyBuffer();
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
	                                   ? ast_filename_symbol
	                                   : top_level_ast_file_symbol),
                                  (option->IsNested()
	                                   ? (char*)"    "
	                                   : (char*)""),
                                  classname[i],
                                  typestring);

                if (top_level_ast_file_symbol != NULL) // option -> automatic_ast == Option::TOPLEVEL
                {
                    for (int j = 0; j < actions.Length(); j++)
                        actions[j].buffer = top_level_ast_file_symbol->BodyBuffer();
                }
                ProcessCodeActions(actions, typestring, processed_rule_map);
            }
            else
            {
                assert(classname[i].specified_name != classname[i].real_name); // a classname was specified?
                if (classname[i].is_terminal_class)
                    GenerateTerminalMergedClass(ntc,
                                                (option->IsNested()
	                                                 ? ast_filename_symbol
	                                                 : top_level_ast_file_symbol),
                                                (option->IsNested()
	                                                 ? (char*)"    "
	                                                 : (char*)""),
                                                classname[i],
                                                typestring);
                else GenerateMergedClass(ctc,
                                         ntc,
                                         (option->IsNested()
	                                          ? ast_filename_symbol
	                                          : top_level_ast_file_symbol),
                                         (option->IsNested()
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
                    if (top_level_ast_file_symbol != NULL) // possible when option -> automatic_ast == Option::TOPLEVEL
                    {
                        for (int j = 0; j < actions.Length(); j++)
                            actions[j].buffer = top_level_ast_file_symbol->BodyBuffer();
                    }
                    ProcessCodeActions(actions, typestring, processed_rule_map);
                }
            }
        }

        if (option->IsNested()) // Generate Class Closer
            b.Put("    };\n\n");
        else
        {
            top_level_ast_file_symbol->BodyBuffer()->Put("};\n\n");
         
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
        char 
         
          
            * abstract_visitor_type = new char[strlen(abstract) + strlen(visitor_type) + 1],
            * abstract_result_visitor_type = new char[strlen(abstract) + strlen(result) + strlen(visitor_type) + 1];

      

        strcpy(abstract_visitor_type, abstract);
        strcat(abstract_visitor_type, visitor_type);

        strcpy(abstract_result_visitor_type, abstract);
        strcat(abstract_result_visitor_type, result);
        strcat(abstract_result_visitor_type, visitor_type);

        if (option->visitor == Option::DEFAULT)
        {
            if (option->IsNested())
            {
             
                GenerateNoResultVisitorAbstractClass(ast_filename_symbol, "    ", abstract_visitor_type, type_set);
                GenerateResultVisitorAbstractClass(ast_filename_symbol, "    ", abstract_result_visitor_type, type_set);
            }
            else
            {
           
                GenerateNoResultVisitorAbstractClass(top_level_ast_file_symbol, "", abstract_visitor_type, type_set);
              

  
                GenerateResultVisitorAbstractClass(top_level_ast_file_symbol, "", abstract_result_visitor_type, type_set);
              
            }
        }
        else if (option->visitor == Option::PREORDER)
        {
            if (option->IsNested())
            {
              
                GeneratePreorderVisitorAbstractClass(ast_filename_symbol, "    ", abstract_visitor_type, type_set);
            }
            else
            {
             
                GeneratePreorderVisitorAbstractClass(top_level_ast_file_symbol, "", abstract_visitor_type, type_set);
               
            }
        }
        delete[] abstract_visitor_type;
        delete[] abstract_result_visitor_type;
    }
	

    ProcessCodeActions(initial_actions, typestring, processed_rule_map);

    auto  ast_Allocation_symbol = option->AstBlock()->ActionfileSymbol();
    TextBuffer& ast_ast_Allocation_symbol_buffer = *(ast_Allocation_symbol->BodyBuffer());

    if (option->automatic_ast == Option::TOPLEVEL)
    {
        ast_ast_Allocation_symbol_buffer.Put("using namespace ");
        ast_ast_Allocation_symbol_buffer.Put(option->top_level_ast_file_prefix);
        ast_ast_Allocation_symbol_buffer.Put(";\n");

        TextBuffer& _buffer = *(top_level_ast_file_symbol->BodyBuffer());
        _buffer.Put("\n};");
        top_level_ast_file_symbol->Flush();
    }

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
                    ProcessMacro(&ast_ast_Allocation_symbol_buffer, "SplitActions", rule_no);
                    count++;
                }

                ProcessMacro(&ast_ast_Allocation_symbol_buffer, "BeginAction", rule_no);

                if (rule_allocation_map[rule_no].list_kind != RuleAllocationElement::NOT_A_LIST)
                {
                    GenerateListAllocation(ctc,
                                           ntc,
                                           ast_ast_Allocation_symbol_buffer,
                                           rule_no, rule_allocation_map[rule_no]);
                }
                else
                {
                    if (user_specified_null_ast[rule_no] || (grammar->RhsSize(rule_no) == 0 && rule_allocation_map[rule_no].name == NULL))
                        GenerateNullAstAllocation(ast_ast_Allocation_symbol_buffer, rule_no);
                    else GenerateAstAllocation(ctc,
                                               ntc,
                                               ast_ast_Allocation_symbol_buffer,
                                               rule_allocation_map[rule_no],
                                               processed_rule_map[rule_no],
                                               typestring, rule_no);
                }

                GenerateCode(&ast_ast_Allocation_symbol_buffer, "\n    ", rule_no);
                ProcessMacro(&ast_ast_Allocation_symbol_buffer, "EndAction", rule_no);
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

                ProcessMacro(&ast_ast_Allocation_symbol_buffer, "NoAction", rule_no);
            }
        }
    }
          
      
    return;
}



//
//
//
void CppAction2::GenerateEnvironmentDeclaration(TextBuffer &b, const char *indentation)
{
    b.Put(indentation); b.Put("    ");
                                 b.Put(option -> action_type);
                                 b.Put("* environment;\n");
    b.Put(indentation); b.Put("    ");
                                 b.Put(option -> action_type);
                                 b.Put("* getEnvironment() { return environment; }\n\n");
}

//
//
//
void CppAction2::GenerateVisitorHeaders(TextBuffer &b, const char *indentation, const char *modifiers)
{
    if (option -> visitor != Option::NONE)
    {
        char *header = new char[strlen(indentation) + strlen(modifiers) + 9];
        strcpy(header, indentation);
        strcat(header, modifiers);

        b.Put(header);
        if (option -> visitor == Option::PREORDER)
        {
            b.Put("virtual void accept(IAstVisitor* v)=0;");
        }
        else if (option -> visitor == Option::DEFAULT)
        {
            b.Put("virtual void accept(");
            b.Put(option -> visitor_type);
            b.Put("* v)=0;");

            b.Put("\n");

            b.Put(header);
            b.Put("virtual void accept(Argument");
            b.Put(option -> visitor_type);
            b.Put(" *v, Object* o)=0;\n");

            b.Put(header);
            b.Put("virtual lpg::Any accept(Result");
            b.Put(option -> visitor_type);
            b.Put(" *v)=0;\n");

            b.Put(header);
            b.Put("virtual lpg::Any accept(ResultArgument");
            b.Put(option -> visitor_type);
            b.Put(" *v, Object* o)=0;");
        }
        b.Put("\n");

        delete [] header;
    }

    return;
}


//
//
//
void CppAction2::GenerateVisitorMethods(NTC &ntc,
                                        TextBuffer &b,
                                        const char *indentation,
                                        ClassnameElement &element,
                                        BitSet &optimizable_symbol_set)
{
    if (option -> visitor == Option::DEFAULT)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("    void accept(");
                                     b.Put(option -> visitor_type);
                                     b.Put(" *v) { v->visit(this); }\n");

        b.Put(indentation); b.Put("    void accept(Argument");
                                     b.Put(option -> visitor_type);
                                     b.Put(" *v, Object* o) { v->visit(this, o); }\n");

        b.Put(indentation); b.Put("    lpg::Any accept(Result");
                                     b.Put(option -> visitor_type);
                                     b.Put(" *v) { return v->visit(this); }\n");

        b.Put(indentation); b.Put("    lpg::Any accept(ResultArgument");
                                     b.Put(option -> visitor_type);
                                     b.Put(" *v, Object* o) { return v->visit(this, o); }\n");
    }
    else if (option -> visitor == Option::PREORDER)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("    void accept(IAstVisitor* v)\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        if (! v->preVisit(this)) return;\n");
        b.Put(indentation); b.Put("        enter(("); 
                                     b.Put(option -> visitor_type);
                                     b.Put("*) v);\n");
        b.Put(indentation); b.Put("        v->postVisit(this);\n");
        b.Put(indentation); b.Put("    }\n\n");

        b.Put(indentation); b.Put("    void enter(");
                                     b.Put(option -> visitor_type);
                                     b.Put(" *v)\n");
        b.Put(indentation); b.Put("    {\n");
        SymbolLookupTable &symbol_set = element.symbol_set;
        Tuple<int> &rhs_type_index = element.rhs_type_index;
        if (element.is_terminal_class || symbol_set.Size() == 0)
        {
            b.Put(indentation); b.Put("        v->visit(this);\n");
        }
        else
        {
            b.Put(indentation); b.Put("        bool checkChildren = v->visit(this);\n");
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
                    b.Put("if (");
                    b.Put(ast_member_prefix.c_str());
                    b.Put(symbol_set[i] -> Name());
                    b.Put(" != nullptr) ");
                }
                b.Put("((IAst*)");
                b.Put(ast_member_prefix.c_str());
                b.Put(symbol_set[i] -> Name());
                b.Put(")->accept(v);\n");
            }

            if (symbol_set.Size() > 1)
            {
                b.Put(indentation); b.Put("        }\n");
            }
        }
        b.Put(indentation); b.Put("        v->endVisit(this);\n");
        b.Put(indentation); b.Put("    }\n");
    }

    return;
}


//
//
//
void CppAction2::GenerateGetAllChildrenMethod(TextBuffer &b,
                                              const char *indentation,
                                              ClassnameElement &element)
{
    if (! element.is_terminal_class)
    {
        SymbolLookupTable &symbol_set = element.symbol_set;

        b.Put("\n");
        b.Put(indentation); b.Put("    /**\n");
        b.Put(indentation); b.Put("     * A list of all children of this node,don't including the nullptr ones.\n");
        b.Put(indentation); b.Put("     */\n");
        b.Put(indentation); b.Put("    std::vector<IAst*> getAllChildren()\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        std::vector<IAst*> list;\n");
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            b.Put(indentation);
            b.Put("        if(");
        	b.Put(ast_member_prefix.c_str());b.Put(symbol_set[i]->Name());
        	b.Put(")  list.push_back((IAst*)");
        	b.Put(ast_member_prefix.c_str());b.Put(symbol_set[i] -> Name());
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
void CppAction2::GenerateSimpleVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                const char *indentation,
                                                const char *interface_name,
                                                SymbolLookupTable &type_set)
{
    add_forward_class_def(ast_filename_symbol, interface_name);
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    b.Put(indentation); b.Put("struct ");
                                 b.Put(interface_name);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("    virtual void visit");
                                     b.Put("(");
                                     b.Put(symbol -> Name());
                                     b.Put(" *n)=0;\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    virtual void visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" *n)=0;\n");

    b.Put(indentation); b.Put("};\n");
}

//
//
//
void CppAction2::GenerateArgumentVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                  const char *indentation,
                                                  const char *interface_name,
                                                  SymbolLookupTable &type_set)
{
    add_forward_class_def(ast_filename_symbol, interface_name);
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    b.Put(indentation); b.Put("struct ");
                                 b.Put(interface_name);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("    virtual void visit");
                                     b.Put("(");
                                     b.Put(symbol -> Name());
                                     b.Put(" *n, Object* o)=0;\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("  virtual  void visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" *n, Object* o)=0;\n");

    b.Put(indentation); b.Put("};\n");
}

//
//
//
void CppAction2::GenerateResultVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                const char *indentation,
                                                const char *interface_name,
                                                SymbolLookupTable &type_set)
{
    add_forward_class_def(ast_filename_symbol, interface_name);
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    b.Put(indentation); b.Put("struct ");
                                 b.Put(interface_name);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("  virtual  lpg::Any visit");
                                     b.Put("(");
                                     b.Put(symbol -> Name());
                                     b.Put(" *n)=0;\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("  virtual  lpg::Any visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" *n)=0;\n");

    b.Put(indentation); b.Put("};\n");
}

//
//
//
void CppAction2::GenerateResultArgumentVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                        const char *indentation,
                                                        const char *interface_name,
                                                        SymbolLookupTable &type_set)
{
    add_forward_class_def(ast_filename_symbol, interface_name);
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    b.Put(indentation); b.Put("struct ");
    b.Put(interface_name);
    b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("  virtual  lpg::Any visit");
                                     b.Put("(");
                                     b.Put(symbol -> Name());
                                     b.Put(" *n, Object* o)=0;\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("  virtual  lpg::Any visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" *n, Object* o)=0;\n");

    b.Put(indentation); b.Put("};\n");
}


//
//
//
void CppAction2::GeneratePreorderVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                  const char *indentation,
                                                  const char *interface_name,
                                                  SymbolLookupTable &type_set)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());

    add_forward_class_def(ast_filename_symbol, interface_name);
    assert(option -> visitor == Option::PREORDER);
    b.Put(indentation); b.Put("struct ");
                                 b.Put(interface_name);
                                 b.Put(" :public IAstVisitor\n");
    b.Put(indentation); b.Put("{\n");

    //    b.Put(indentation); b.Put("    bool preVisit(");
    //                                 b.Put(option -> ast_type);
    //                                 b.Put(" element);\n");
    //
    //    b.Put(indentation); b.Put("    void postVisit(");
    //                                 b.Put(option -> ast_type);
    //                                 b.Put(" element);\n\n");

    b.Put(indentation); b.Put("  virtual  bool visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" *n)=0;\n");
    b.Put(indentation); b.Put(" virtual   void endVisit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" *n)=0;\n\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put(" virtual   bool visit");
                                     b.Put("(");
                                     b.Put(symbol -> Name());
                                     b.Put(" *n)=0;\n");
        b.Put(indentation); b.Put("  virtual  void endVisit");
                                     b.Put("(");
                                     b.Put(symbol -> Name());
                                     b.Put(" *n)=0;\n");
        b.Put("\n");
    }

    b.Put(indentation); b.Put("};\n\n");

    return;
}


//
//
//
void CppAction2::GenerateNoResultVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                      const char *indentation,
                                                      const char *classname,
                                                      SymbolLookupTable &type_set)
{
    add_forward_class_def(ast_filename_symbol, classname);
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    b.Put(indentation); 
                                 b.Put("struct ");
                                 b.Put(classname);
                                 b.Put(" :public ");
                                 b.Put(option -> visitor_type);
                                 b.Put(", public Argument");
                                 b.Put(option -> visitor_type);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("    virtual void unimplementedVisitor(const std::string &s)=0;\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put(" virtual   void visit");
                                         b.Put("(");
                                         b.Put(symbol -> Name());
                                         b.Put(" *n) { unimplementedVisitor(\"visit(");
                                         b.Put(symbol -> Name());
                                         b.Put(")\"); }\n");
            b.Put(indentation); b.Put("  virtual  void visit");
                                         b.Put("(");
                                         b.Put(symbol -> Name());
                                         b.Put(" *n, Object* o) { unimplementedVisitor(\"visit(");
                                         b.Put(symbol -> Name());
                                         b.Put(", Object)\"); }\n");
            b.Put("\n");
        }
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("  virtual  void visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" *n)\n");
    b.Put(indentation); b.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                        // b.Put(i == 0 ? "" : "else ");
                                         b.Put("if (dynamic_cast< ");                        
                                         b.Put(symbol -> Name());
                                         b.Put("*>(n)");
                                         b.Put("){ visit((");
                                         b.Put(symbol -> Name());
                                         b.Put("*) n);return;}\n");
        }
    }
    b.Put(indentation); b.Put("        throw UnsupportedOperationException(\"visit(\" + n->to_utf8_string() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");


    b.Put(indentation); b.Put("    void visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" *n, Object* o)\n");
    b.Put(indentation); b.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                        // b.Put(i == 0 ? "" : "else ");
                                         b.Put("if (dynamic_cast<");
                                         b.Put(symbol -> Name());
                                         b.Put("*>(n)");
                                         b.Put(") {visit((");
                                         b.Put(symbol -> Name());
                                         b.Put("*) n, o);return;}\n");
        }
    }
    b.Put(indentation); b.Put("        throw UnsupportedOperationException(\"visit(\" + n->to_utf8_string() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("};\n");
}

//
//
//
void CppAction2::GenerateResultVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                    const char *indentation,
                                                    const char *classname,
                                                    SymbolLookupTable &type_set)
{
    add_forward_class_def(ast_filename_symbol, classname);
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    b.Put(indentation); 
                                 b.Put("struct ");
                                 b.Put(classname);
                                 b.Put(" :public Result");
                                 b.Put(option -> visitor_type);
                                 b.Put(",public ResultArgument");
                                 b.Put(option -> visitor_type);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("    virtual lpg::Any unimplementedVisitor(const std::string& s);\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("    lpg::Any visit(");
                                         b.Put(symbol -> Name());
                                         b.Put(" *n) { return unimplementedVisitor(\"visit(");
                                         b.Put(symbol -> Name());
                                         b.Put(")\"); }\n");
            b.Put(indentation); b.Put("    lpg::Any visit(");
                                         b.Put(symbol -> Name());
                                         b.Put(" *n, Object* o) { return  unimplementedVisitor(\"visit(");
                                         b.Put(symbol -> Name());
                                         b.Put(", Object)\"); }\n");
            b.Put("\n");
        }
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    lpg::Any visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" *n)\n");
    b.Put(indentation); b.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                         //b.Put(i == 0 ? "" : "else ");
                                         b.Put("if (dynamic_cast<");
                                         b.Put(symbol -> Name());
                                         b.Put("*>(n) ");                                    
                                         b.Put(") return visit((");
                                         b.Put(symbol -> Name());
                                         b.Put("*) n);\n");
        }
    }
    b.Put(indentation); b.Put("        throw UnsupportedOperationException(\"visit(\" + n->to_utf8_string() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");


    b.Put(indentation); b.Put("    lpg::Any visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" *n, Object *o)\n");
    b.Put(indentation); b.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                        // b.Put(i == 0 ? "" : "else ");
                                         b.Put("if (dynamic_cast<");
                                         b.Put(symbol->Name());
                                         b.Put("*>(n) ");
                                         b.Put(") return visit((");
                                         b.Put(symbol -> Name());
                                         b.Put("*) n, o);\n");
        }
    }
    b.Put(indentation); b.Put("        throw UnsupportedOperationException(\"visit(\" + n->to_utf8_string() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("};\n");
}


//
//
//
void CppAction2::GeneratePreorderVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                      const char *indentation,
                                                      const char *classname,
                                                      SymbolLookupTable &type_set)
{
    add_forward_class_def(ast_filename_symbol, classname);
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    assert(option -> visitor == Option::PREORDER);
    b.Put(indentation);
                                 b.Put("struct ");
                                 b.Put(classname);
                                 b.Put(" :public ");
                                 b.Put(option -> visitor_type);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("    virtual void unimplementedVisitor(const std::string& s)=0;\n\n");
    b.Put(indentation); b.Put("   virtual bool preVisit(IAst* element) { return true; }\n\n");
    b.Put(indentation); b.Put("   virtual void postVisit(IAst* element) {}\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put(" virtual   bool visit");
                                         b.Put("(");
                                         b.Put(symbol -> Name());
                                         b.Put(" *n) { unimplementedVisitor(\"visit(");
                                         b.Put(symbol -> Name());
                                         b.Put(")\"); return true; }\n");
            b.Put(indentation); b.Put("  virtual  void endVisit");
                                         b.Put("(");
                                         b.Put(symbol -> Name());
                                         b.Put(" *n) { unimplementedVisitor(\"endVisit(");
                                         b.Put(symbol -> Name());
                                         b.Put(")\"); }\n");
            b.Put("\n");
        }
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put(" virtual   bool visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" *n)\n");
    b.Put(indentation); b.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                        // b.Put(i == 0 ? "" : "else ");
                                         b.Put("if (dynamic_cast<");
                                         b.Put(symbol->Name());
                                         b.Put("*>(n) ");
                                         b.Put(") return visit((");
                                         b.Put(symbol -> Name());
                                         b.Put("*) n);\n");
        }
    }
    b.Put(indentation); b.Put("        throw UnsupportedOperationException(\"visit(\" + n->to_utf8_string() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("  virtual  void endVisit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" *n)\n");
    b.Put(indentation); b.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                        // b.Put(i == 0 ? "" : "else ");
                                         b.Put("if (dynamic_cast<");
                                         b.Put(symbol->Name());
                                         b.Put("*>(n) ");
                                         b.Put("){ endVisit((");
                                         b.Put(symbol -> Name());
                                         b.Put("*) n);return;}\n");
        }
    }
    b.Put(indentation); b.Put("        throw UnsupportedOperationException(\"visit(\" + n->to_utf8_string() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("};\n");

    return;
}


//
// Generate the the Ast root classes
//
void CppAction2::GenerateAstType(ActionFileSymbol* ast_filename_symbol,
                                 const char *indentation,
                                 const char *classname)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    /*
     * First, generate the main root class
     */
    add_forward_class_def(ast_filename_symbol, classname);
	
    b.Put(indentation);
                                 b.Put("struct ");
                                 b.Put(classname);
                                 b.Put(" :public IAst\n");
    b.Put(indentation); b.Put("{\n");
    if (option -> glr)
    {
        b.Put(indentation); b.Put("    Ast* nextAst = nullptr;\n");
        b.Put(indentation); b.Put("    IAst* getNextAst() { return nextAst; }\n");
        b.Put(indentation); b.Put("    void setNextAst(IAst* n) { nextAst = n; }\n");
        b.Put(indentation); b.Put("    void resetNextAst() { nextAst = nullptr; }\n");
    }
    else
    {
	    b.Put(indentation); b.Put("    IAst* getNextAst() { return nullptr; }\n");
    }

    b.Put(indentation); b.Put("     IToken* leftIToken=nullptr;\n");
    b.Put(indentation); b.Put("        IToken*    rightIToken=nullptr;\n");
    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("     IAst* parent = nullptr;\n");
        b.Put(indentation); b.Put("     void setParent(IAst* parent) { this->parent = parent; }\n");
        b.Put(indentation); b.Put("    IAst* getParent() { return parent; }\n");\
    }
    else
    {
        b.Put(indentation); b.Put("    IAst* getParent()\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        throw UnsupportedOperationException(\"noparent-saved option in effect\");\n");
        b.Put(indentation); b.Put("    }\n");
    }

    b.Put("\n");
    b.Put(indentation); b.Put("    IToken* getLeftIToken() { return leftIToken; }\n");
    b.Put(indentation); b.Put("    IToken* getRightIToken() { return rightIToken; }\n");
    b.Put(indentation); b.Put("    std::vector<IToken*> getPrecedingAdjuncts() { return leftIToken->getPrecedingAdjuncts(); }\n");
    b.Put(indentation); b.Put("    std::vector<IToken*> getFollowingAdjuncts() { return rightIToken->getFollowingAdjuncts(); }\n\n");

    b.Put(indentation); b.Put("    std::wstring toString()\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        return leftIToken->getILexStream()->toString(leftIToken->getStartOffset(), rightIToken->getEndOffset());\n");
    b.Put(indentation); b.Put("    }\n\n");

    b.Put(indentation); b.Put("    ");
                                 b.Put(classname);
                                 b.Put("(IToken* token) { this->leftIToken = this->rightIToken = token; }\n");

    b.Put(indentation); b.Put("    ");
                                 b.Put(classname);
                                 b.Put("(IToken* leftIToken, IToken* rightIToken)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        this->leftIToken = leftIToken;\n");
    b.Put(indentation); b.Put("        this->rightIToken = rightIToken;\n");
    b.Put(indentation); b.Put("    }\n\n");
    b.Put(indentation); b.Put("    void initialize() {}\n");
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
        b.Put(indentation); b.Put("     * A list of all children of this node, excluding the nullptr ones.\n");
        b.Put(indentation); b.Put("     */\n");
        b.Put(indentation); b.Put("    std::vector<IAst*> getChildren()\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        std::vector<IAst*> list = getAllChildren();\n");
        b.Put(indentation); b.Put("        int k = -1;\n");
        b.Put(indentation); b.Put("        for (int i = 0; i < list.size(); i++)\n");
        b.Put(indentation); b.Put("        {\n");
        b.Put(indentation); b.Put("            IAst* element = list[i];\n");
        b.Put(indentation); b.Put("            if (element != nullptr)\n");
        b.Put(indentation); b.Put("            {\n");
        b.Put(indentation); b.Put("                if (++k != i)\n");
        b.Put(indentation); b.Put("                    list[k]=(element);\n");
        b.Put(indentation); b.Put("            }\n");
        b.Put(indentation); b.Put("        }\n");
        b.Put(indentation); b.Put("        for (int i = list.size() - 1; i > k; i--) // remove extraneous elements\n");
        b.Put(indentation); b.Put("           list.erase(list.begin()+i);\n");
        b.Put(indentation); b.Put("        return list;\n");
        b.Put(indentation); b.Put("    }\n\n");

        b.Put(indentation); b.Put("    /**\n");
        b.Put(indentation); b.Put("     * A list of all children of this node, including the nullptr ones.\n");
        b.Put(indentation); b.Put("     */\n");
        b.Put(indentation); b.Put("    virtual std::vector<IAst*> getAllChildren()=0;\n");
    }
    else
    {
        b.Put(indentation); b.Put("    std::vector<IAst*> getChildren()\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        throw UnsupportedOperationException(\"noparent-saved option in effect\");\n");
        b.Put(indentation); b.Put("    }\n");
        b.Put(indentation); b.Put("    std::vector<IAst*> getAllChildren() { return getChildren(); }\n");
    }

    b.Put("\n");


    GenerateVisitorHeaders(b, indentation, "     ");

    //
    // Not Preorder visitor? generate dummy accept method to satisfy IAst abstract declaration of accept(IAstVisitor);
    // TODO: Should IAstVisitor be used for default visitors also? If (when) yes then we should remove it from the test below
    //
    if (option -> visitor == Option::NONE || option -> visitor == Option::DEFAULT) // ??? Don't need this for DEFAULT case after upgrade
    {
        b.Put(indentation); b.Put("    void accept(IAstVisitor* v) {}\n");
    }
    b.Put(indentation); b.Put("};\n\n");

    return;
}

typedef std::map<std::string, std::string> Substitutions;


//
// Generate the the Ast list class
//
void CppAction2::GenerateAbstractAstListType(ActionFileSymbol* ast_filename_symbol,
                                             const char *indentation,
                                             const char *classname)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    add_forward_class_def(ast_filename_symbol, this->abstract_ast_list_classname);
    /*
     * Generate the List root class
     */
    b.Put(indentation); 								
                                 b.Put("struct ");
                                 b.Put(this -> abstract_ast_list_classname);
                                 b.Put(" :public ");
                                 b.Put(option -> ast_type);
                                 b.Put(" ,public IAbstractArrayList<");
                                 b.Put(option -> ast_type);
                                 b.Put("*>\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("    bool leftRecursive;\n");
    //b.Put(indentation); b.Put("    java.util.ArrayList list;\n");
    b.Put(indentation); b.Put("    int size() { return list.size(); }\n");
    b.Put(indentation); b.Put("   std::vector<");
								 b.Put(option->ast_type);
    b.Put(indentation); b.Put("   *> getList() { return list; }\n");
    b.Put(indentation); b.Put("    ");
                                 b.Put(option -> ast_type);
                                 b.Put(" *getElementAt(int i) { return (");
                                 b.Put(option -> ast_type);
                                 b.Put("*) list.at(leftRecursive ? i : list.size() - 1 - i); }\n");

	 b.Put(indentation); b.Put("   std::vector<");
	// b.Put(indentation); b.Put(option->ast_type);
     b.Put("IAst");
    b.Put(indentation); b.Put("    *> getArrayList()\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        if (! leftRecursive) // reverse the list \n");
    b.Put(indentation); b.Put("        {\n");
    b.Put(indentation); b.Put("            for (int i = 0, n = list.size() - 1; i < n; i++, n--)\n");
    b.Put(indentation); b.Put("            {\n");
    b.Put(indentation); b.Put("                auto ith = list[i];\n");
    b.Put(indentation); b.Put("                 auto nth = list[n];\n");
    b.Put(indentation); b.Put("                list[i]=(nth);\n");
    b.Put(indentation); b.Put("                list[n]=(ith);\n");
    b.Put(indentation); b.Put("            }\n");
    b.Put(indentation); b.Put("            leftRecursive = true;\n");
    b.Put(indentation); b.Put("        }\n");
    b.Put(indentation); b.Put("         return  std::vector<IAst*>(list.begin(), list.end());\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    /**\n");
    b.Put(indentation); b.Put("     * @deprecated replaced by {@link #addElement()}\n");
    b.Put(indentation); b.Put("     *\n");
    b.Put(indentation); b.Put("     */\n");
    b.Put(indentation); b.Put("    bool add(");
                                 b.Put(option -> ast_type);
                                 b.Put(" *element)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        addElement(element);\n");
    b.Put(indentation); b.Put("        return true;\n");
    b.Put(indentation); b.Put("    }\n\n");
    b.Put(indentation); b.Put("    void addElement(");
                                 b.Put(option -> ast_type);
                                 b.Put(" *element)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        list.push_back(element);\n");
    b.Put(indentation); b.Put("        if (leftRecursive)\n");
    b.Put(indentation); b.Put("             rightIToken = element->getRightIToken();\n");
    b.Put(indentation); b.Put("        else leftIToken =  element->getLeftIToken();\n");
    b.Put(indentation); b.Put("    }\n\n");

    // generate constructors for list class
    b.Put(indentation); b.Put("    ");
                                 b.Put(classname);
                                 b.Put("(IToken* leftIToken, IToken* rightIToken, bool leftRecursive):");
                                 b.Put(option->ast_type);
                                 b.Put("(leftIToken, rightIToken)\n");
	
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        this->leftRecursive = leftRecursive;\n");
    b.Put(indentation); b.Put("    }\n\n");

    b.Put(indentation); b.Put("    ");
                                 b.Put(classname);
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" *element, bool leftRecursive):");
                                 b.Put(option->ast_type);
                                 b.Put("(element->getLeftIToken(), element->getRightIToken())\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("         this->leftRecursive = leftRecursive;\n");
    b.Put(indentation); b.Put("        list.push_back(element);\n");
    b.Put(indentation); b.Put("    }\n\n");

    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("    /**\n");
        b.Put(indentation); b.Put("     * Make a copy of the list and return it. Note that we obtain the local list by\n");
        b.Put(indentation); b.Put("     * invoking getArrayList so as to make sure that the list we return is in proper order.\n");
        b.Put(indentation); b.Put("     */\n");

        b.Put(indentation); b.Put("   std::vector<");
       // b.Put(option->ast_type);
       b.Put("IAst");
        b.Put(indentation); b.Put("    *> getAllChildren()\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        auto list_= getArrayList();\n return   std::vector<IAst*>(list_.begin(),list_.end());\n");
        b.Put(indentation); b.Put("    }\n\n");
    }

    //
    // Implementation for functions in java.util.List
    //

    Substitutions subs;
    subs["%%AstType%%"] = option->ast_type;
    subs["%%ListClassName%%"] = classname;
  
	b.Put(indentation); b.Put("};\n\n");
    return;
}


//
// Generate the the Ast token class
//
void CppAction2::GenerateAstTokenType(NTC &ntc, ActionFileSymbol* ast_filename_symbol,
                                      const char *indentation,
                                      const char *classname)
{
    add_forward_class_def(ast_filename_symbol, classname);
    TextBuffer & b = *(ast_filename_symbol->BodyBuffer());
    /*
     * Generate the Token root class
     */
    b.Put(indentation); 
                                 b.Put("struct ");
                                 b.Put(classname);
                                 b.Put(" :public  ");
                                 b.Put(option -> ast_type);
                           /*      b.Put(" ,public I");
                                 b.Put(classname);*/
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    b.Put(indentation); b.Put("    ");
                                 b.Put(classname);
                                 b.Put("(IToken* token):");
                                 b.Put(option->ast_type);
                                 b.Put("(token){ }\n");
	
    b.Put(indentation); b.Put("    IToken* getIToken() { return leftIToken; }\n");
    b.Put(indentation); b.Put("    std::wstring toString() { return leftIToken->toString(); }\n\n");

    ClassnameElement element; // generate a temporary element with no symbols in its symbol set.
    element.real_name = (char *) classname;
    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);

    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("    /**\n");
        b.Put(indentation); b.Put("     * A token class has no children. So, we return the empty list.\n");
        b.Put(indentation); b.Put("     */\n");
        b.Put(indentation); b.Put("   std::vector<IAst*> getAllChildren() { return {}; }\n\n");
    }

    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set);

    b.Put(indentation); b.Put("};\n\n");

    return;
}


//
//
//
void CppAction2::GenerateCommentHeader(TextBuffer &b,
                                       const char *indentation,
                                       Tuple<int> &ungenerated_rule,
                                       Tuple<int> &generated_rule)
{
  

    const char* rule_info = rule_info_holder.c_str(); /*" *<li>Rule $rule_number:  $rule_text";*/
    BlockSymbol* scope_block = nullptr;
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


void CppAction2::GenerateListMethods(CTC &ctc,
                                     NTC &ntc,
                                     TextBuffer &b,
                                     const char *indentation,
                                     const char *,
                                     ClassnameElement &element,
                                     Array<const char *> &)
{
    const char *element_name = element.array_element_type_symbol -> Name(),
             //  *element_type = ctc.FindBestTypeFor(element.array_element_type_symbol -> SymbolIndex());
        * element_type = ctc.FindUniqueTypeFor(element.array_element_type_symbol->SymbolIndex(), option->ast_type);
    //
    // Generate ADD method
    //
    b.Put(indentation); b.Put("    ");
                                 b.Put("void addElement(");
                                 b.Put(element_type);                        
                                 b.Put(" *");
                                 b.Put(ast_member_prefix.c_str());
                                 b.Put(element_name);
                                 b.Put(")\n");
    b.Put(indentation); b.Put("    {\n");
  
    b.Put(indentation);
    b.Put("        ");
	b.Put(this->abstract_ast_list_classname);
	b.Put("::addElement((");
                                 b.Put(option -> ast_type);
                                 b.Put("*) ");
                                 b.Put(ast_member_prefix.c_str());
                                 b.Put(element_name);
                                 b.Put(");\n");
    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("        ");
        if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
        {
            b.Put("if (");
            b.Put(ast_member_prefix.c_str());
            b.Put(element_name);
            b.Put(" != nullptr) ");
        }
        b.Put("((");
        b.Put(option -> ast_type);
        b.Put("*) ");
        b.Put(ast_member_prefix.c_str());
        b.Put(element_name);
        b.Put(")->setParent(this);\n");
    }
    b.Put(indentation); b.Put("    }\n");

    b.Put("\n");

    //
    // Generate visitor methods.
    //
    if (option -> visitor == Option::DEFAULT)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("    void accept(");
                                     b.Put(option -> visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
            b.Put(" *v) { for (int i = 0; i < size(); i++) v->visit"
                           "("
                           "get");
            b.Put(element_name);
            b.Put("At(i)"
                           "); }\n");
        }
        else
        {
            b.Put(" *v) { for (int i = 0; i < size(); i++) get");
            b.Put(element_name);
            b.Put("At(i)->accept(v); }\n");
        }

        b.Put(indentation); b.Put("    void accept(Argument");
                                     b.Put(option -> visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
            b.Put(" *v, Object *o) { for (int i = 0; i < size(); i++) v->visit"
                           "("
                           "get");
            b.Put(element_name);
            b.Put("At(i), o");
            b.Put("); }\n");
        }
        else
        {
            b.Put(" *v, Object* o) { for (int i = 0; i < size(); i++) get");
            b.Put(element_name);
            b.Put("At(i)->accept(v, o); }\n");
        }

        //
        // Code cannot be generated to automatically visit a node that
        // can return a value. These cases are left up to the user.
        //
        b.Put(indentation); b.Put("    lpg::Any accept(Result");
                                     b.Put(option -> visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
                                         b.Put(" *v)\n");
            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        std::vector<lpg::Any> result;\n");
            b.Put(indentation); b.Put("        for (int i = 0; i < size(); i++)\n");
            b.Put(indentation); b.Put("            result.push_back(v->visit(get");
                                         b.Put(element_name);
                                         b.Put("At(i)));\n");
            b.Put(indentation); b.Put("        return result;\n");
            b.Put(indentation); b.Put("    }\n");
        }
        else
        {
                                         b.Put(" *v)\n");
            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        std::vector<lpg::Any> result;\n");
            b.Put(indentation); b.Put("        for (int i = 0; i < size(); i++)\n");
            b.Put(indentation); b.Put("            result.push_back(get");
                                         b.Put(element_name);
                                         b.Put("At(i)->accept(v));\n");
            b.Put(indentation); b.Put("        return result;\n");
            b.Put(indentation); b.Put("    }\n");
        }

        b.Put(indentation); b.Put("    lpg::Any accept(ResultArgument");
                                     b.Put(option -> visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
                                         b.Put(" *v, Object* o)\n");
            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        std::vector<lpg::Any> result;\n");
            b.Put(indentation); b.Put("        for (int i = 0; i < size(); i++)\n");
            b.Put(indentation); b.Put("            result.push_back(v->visit(get");
                                         b.Put(element_name);
                                         b.Put("At(i), o));\n");
            b.Put(indentation); b.Put("        return result;\n");
            b.Put(indentation); b.Put("    }\n");
        }
        else
        {
                                         b.Put(" *v, Object *o)\n");
            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        std::vector<lpg::Any> result;\n");
            b.Put(indentation); b.Put("        for (int i = 0; i < size(); i++)\n");
            b.Put(indentation); b.Put("            result.push_back(get");
                                         b.Put(element_name);
                                         b.Put("At(i)->accept(v, o));\n");
            b.Put(indentation); b.Put("        return result;\n");
            b.Put(indentation); b.Put("    }\n");
        }
    }
    else if (option -> visitor == Option::PREORDER)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("    void accept(IAstVisitor* v)\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        if (! v->preVisit(this)) return;\n");
        b.Put(indentation); b.Put("        enter((");
                                     b.Put(option -> visitor_type);
                                     b.Put("*) v);\n");
        b.Put(indentation); b.Put("        v->postVisit(this);\n");
        b.Put(indentation); b.Put("    }\n");
        b.Put(indentation); b.Put("    void enter(");
                                     b.Put(option -> visitor_type);
                                     b.Put(" *v)\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        bool checkChildren = v->visit(this);\n");
        b.Put(indentation); b.Put("        if (checkChildren)\n");
        b.Put(indentation); b.Put("        {\n");
        b.Put(indentation); b.Put("            for (int i = 0; i < size(); i++)\n");
        b.Put(indentation); b.Put("            {\n");
        b.Put(indentation); b.Put("                ");

        const char *element_typename = ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex());
        if (element_typename != NULL)
        {
           // b.Put(element_typename);
           // b.Put("* element = get");
            b.Put(element_typename);
            b.Put("* element = (");
            b.Put(element_typename);
            b.Put("*)get");
            b.Put(element_name);
            b.Put("At(i);\n");
            if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
            {
                b.Put(indentation); b.Put("                if (element != nullptr)");
                b.Put(indentation); b.Put("                {\n");
                b.Put(indentation); b.Put("                    if (! v->preVisit(element)) continue;\n");
                b.Put(indentation); b.Put("                    element->enter(v);\n");
                b.Put(indentation); b.Put("                    v->postVisit(element);\n");
                b.Put(indentation); b.Put("                }\n");
            }
            else
            {
                b.Put(indentation); b.Put("                if (! v->preVisit(element)) continue;\n");
                b.Put(indentation); b.Put("                element->enter(v);\n");
                b.Put(indentation); b.Put("                v->postVisit(element);\n");
            }
        }
        else
        {
           // element_typename = typestring[element.array_element_type_symbol->SymbolIndex()];
            element_typename = "IAst";
            b.Put(element_typename);
           // b.Put("* element = get");
           // b.Put(element_name);

           
            b.Put("* element = (");
            b.Put(element_typename);
            b.Put("*)get");
            b.Put(element_name);
            b.Put("At(i);\n");
            b.Put(indentation); b.Put("                ");
            if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
                b.Put("if (element != nullptr) ");
            b.Put("element->accept(v);\n");
        }
        b.Put(indentation); b.Put("            }\n");
        b.Put(indentation); b.Put("        }\n");
        b.Put(indentation); b.Put("        v->endVisit(this);\n");
        b.Put(indentation); b.Put("    }\n");
    }

    return;
}


//
//
//
void CppAction2::GenerateListClass(CTC &ctc,
                                   NTC &ntc,
                                   ActionFileSymbol* ast_filename_symbol,
                                   const char *indentation,
                                   ClassnameElement &element,
                                   Array<const char *> &typestring)
{

    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    //Tuple<int> &interface = element.interface_;
    assert(element.array_element_type_symbol != NULL);
    const char *classname = element.real_name,
               *element_name = element.array_element_type_symbol -> Name(),
             //  *element_type = ctc.FindBestTypeFor(element.array_element_type_symbol -> SymbolIndex());
        * element_type = ctc.FindUniqueTypeFor(element.array_element_type_symbol->SymbolIndex(), option->ast_type);
  
	
    add_forward_class_def(ast_filename_symbol, classname);
    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);

    b.Put(indentation);
                                 b.Put("struct ");
                                 b.Put(classname);
                                 b.Put(" :public ");
                                 b.Put(this -> abstract_ast_list_classname);
    /*                             b.Put(" ,public ");
    for (int i = 0; i < interface.Length() - 1; i++)
    {
        b.Put(typestring[element.interface[i]]);
        b.Put(", ");
    }
    b.Put(typestring[element.interface[interface.Length() - 1]]);
    b.Put("\n");*/
    b.Put(indentation); b.Put("{\n");

    if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
    {
        b.Put(indentation); b.Put("    /**\n");
        b.Put(indentation); b.Put("     * The value returned by <b>get");
                                     b.Put(element_name);
                                     b.Put("At</b> may be <b>nullptr</b>\n");
        b.Put(indentation); b.Put("     */\n");
    }
    b.Put(indentation); b.Put("    ");
                                 b.Put(element_type);
								
                                 b.Put("* get");
                                 b.Put(element_name);
                                 b.Put("At(int i) { return (");
                                 b.Put(element_type);
                                 b.Put("*) getElementAt(i); }\n\n");

    //
    // generate constructors
    //
    b.Put(indentation); b.Put("    ");
                                 b.Put(classname);
                                 b.Put("(");
                                 b.Put("IToken* leftIToken, IToken* rightIToken, bool leftRecursive):");
                                 b.Put(this->abstract_ast_list_classname);
    b.Put(indentation); b.Put("(leftIToken, rightIToken, leftRecursive)\n{\n");
    b.Put(indentation); b.Put("    }\n\n");

    b.Put(indentation); b.Put("    ");
                                 b.Put(classname);
                                 b.Put("(");
                                b.Put(element_type);
                               
                                 b.Put("* ");
                                 b.Put(ast_member_prefix.c_str());
                                 b.Put(element_name);
                                 b.Put(", bool leftRecursive):");
                                 b.Put(this->abstract_ast_list_classname);
                                b.Put("        ((");
                                 b.Put(option->ast_type);
                                 b.Put("*) ");
                                 b.Put(ast_member_prefix.c_str());
                                 b.Put(element_name);
                                 b.Put(", leftRecursive)\n");
    b.Put(indentation); b.Put("    {\n");

    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("        ");
        if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
        {
            b.Put("if (");
            b.Put(ast_member_prefix.c_str());
            b.Put(element_name);
            b.Put(" != nullptr) ");
        }
        b.Put("((");
        b.Put(option -> ast_type);
        b.Put("*) ");
        b.Put(ast_member_prefix.c_str());
        b.Put(element_name);
        b.Put(")->setParent(this);\n");
    }
    b.Put(indentation); b.Put("    }\n");
    b.Put("\n");

    GenerateListMethods(ctc, ntc, b, indentation, classname, element, typestring);

    return;
}


//
// Generate a class that extends a basic list class. This is necessary when the user
// specifies action blocks to be associated with a generic list class - in which case,
// we have to generate a (new) unique class (that extends the generic class) to hold the content
// of the action blocks.
//
void CppAction2::GenerateListExtensionClass(CTC &ctc,
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
            //   *element_type = ctc.FindBestTypeFor(element.array_element_type_symbol -> SymbolIndex());
    * element_type = ctc.FindUniqueTypeFor(element.array_element_type_symbol->SymbolIndex(), option->ast_type);
	
    GenerateCommentHeader(b, indentation, element.ungenerated_rule, special_array.rules);

    add_forward_class_def(ast_filename_symbol, classname);
    b.Put(indentation); 
                                 b.Put("struct ");
                                 b.Put(special_array.name);
                                 b.Put(" :public ");
                                 b.Put(classname);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    GenerateEnvironmentDeclaration(b, indentation);

    b.Put(indentation); b.Put("    ");
                                 b.Put(special_array.name);
                                 b.Put("(");
                                 b.Put(option -> action_type);
                                 b.Put(" *environment, ");
                                 b.Put("IToken* leftIToken, IToken* rightIToken, bool leftRecursive):");
                                 b.Put(classname);
                                 b.Put("(leftIToken, rightIToken, leftRecursive)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation);
    b.Put(indentation); b.Put("        this->environment = environment;\n");
    b.Put(indentation); b.Put("        initialize();\n");
    b.Put(indentation); b.Put("    }\n\n");

    b.Put(indentation); b.Put("    ");
                                 b.Put(special_array.name);
                                 b.Put("(");
                                 b.Put(option -> action_type);
                                 b.Put(" *environment, ");
                                 b.Put(element_type);
                                 b.Put(" * ");
                                 b.Put(ast_member_prefix.c_str());
                                 b.Put(element_name);
                                 b.Put(", bool leftRecursive):");
                                 b.Put(classname);
                                 b.Put("(");
                                 b.Put(ast_member_prefix.c_str());
                                 b.Put(element_name);
                                 b.Put(", leftRecursive)\n");
    b.Put(indentation); b.Put("    {\n");

    b.Put(indentation); b.Put("        this->environment = environment;\n");
    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("        ");
        if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
        {
            b.Put("if (");
            b.Put(ast_member_prefix.c_str());
            b.Put(element_name);
            b.Put(" != nullptr) ");
        }
        b.Put("((");
        b.Put(option -> ast_type);
        b.Put("*) ");
        b.Put(ast_member_prefix.c_str());
        b.Put(element_name);
        b.Put(")->setParent(this);\n");
    }
    b.Put(indentation); b.Put("        initialize();\n");
    b.Put(indentation); b.Put("    }\n\n");

    GenerateListMethods(ctc, ntc, b, indentation, special_array.name, element, typestring);

    return;
}


//
// Generate a generic rule class
//
void CppAction2::GenerateRuleClass(CTC &ctc,
                                   NTC &ntc,
                                   ActionFileSymbol* ast_filename_symbol,
                                   const char *indentation,
                                   ClassnameElement &element,
                                   Array<const char *> &)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    char *classname = element.real_name;
    SymbolLookupTable &symbol_set = element.symbol_set;
    Tuple<int> &rhs_type_index = element.rhs_type_index;

    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);

    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);

    assert(element.rule.Length() == 1);
    //int rule_no = element.rule[0];
	
    add_forward_class_def(ast_filename_symbol, classname);
	
    b.Put(indentation);
                                 b.Put("struct ");
                                 b.Put(classname);
                                 b.Put(" :public ");
    const     char* base_type ;
    if (element.is_terminal_class)
    {

        base_type = grammar->Get_ast_token_classname();
        b.Put(base_type);
     /*   b.Put(" ,public ");
        b.Put(typestring[grammar -> rules[rule_no].lhs]);*/
        b.Put("\n");
        b.Put(indentation); b.Put("{\n");
        if (element.needs_environment)
            GenerateEnvironmentDeclaration(b, indentation);
        if (symbol_set.Size() == 1) // if the right-hand side contains a symbol ...
        {
            b.Put(indentation); b.Put("    IToken* get");
                                         b.Put(symbol_set[0] -> Name());
                                         b.Put("() { return leftIToken; }\n\n");
        }
        b.Put(indentation); b.Put("    ");
                                     b.Put(classname);
                                     b.Put("(");
        if (element.needs_environment)
        {
            b.Put(option -> action_type);
            b.Put(" *environment, IToken* token):");
            b.Put(base_type);
            b.Put(indentation); b.Put("(token)\n");
            b.Put(indentation); b.Put("    {\n");
          
            b.Put(indentation); b.Put("        this->environment = environment;\n");
            b.Put(indentation); b.Put("        initialize();\n");
            b.Put(indentation); b.Put("    }\n");
        }
        else
        {
	        b.Put("IToken* token) :");
            b.Put(base_type);
            b.Put("(token)\n{\n initialize(); }\n");
        }
    }
    else 
    {
        base_type = option->ast_type;
        b.Put(base_type);
      /*  b.Put(" ,public ");
        b.Put(typestring[grammar -> rules[rule_no].lhs]);*/
        b.Put("\n");
        b.Put(indentation); b.Put("{\n");
        if (element.needs_environment)
            GenerateEnvironmentDeclaration(b, indentation);

        if (symbol_set.Size() > 0)
        {
            {
                for (int i = 0; i < symbol_set.Size(); i++)
                {
                   // auto temp_type = ctc.FindBestTypeFor(rhs_type_index[i]);
                    auto temp_type = ctc.FindUniqueTypeFor(rhs_type_index[i], option->ast_type);
                	
                    b.Put(indentation); b.Put("    ");
                                                
												 b.Put(temp_type);
                                                 b.Put(" *");
                                                 b.Put(ast_member_prefix.c_str());
                                                 b.Put(symbol_set[i] -> Name());
                                                 b.Put(";\n");
                }
            }
            b.Put("\n");

            {
                for (int i = 0; i < symbol_set.Size(); i++)
                {
                    const char *symbolName = symbol_set[i] -> Name();
                   // const char *bestType = ctc.FindBestTypeFor(rhs_type_index[i]);
                    const char* bestType = ctc.FindUniqueTypeFor(rhs_type_index[i], option->ast_type);
                	
                	
                    if (ntc.CanProduceNullAst(rhs_type_index[i]))
                    {
                        b.Put(indentation); b.Put("    /**\n");
                        b.Put(indentation); b.Put("     * The value returned by <b>get");
                                                     b.Put(symbolName);
                                                     b.Put("</b> may be <b>nullptr</b>\n");
                        b.Put(indentation); b.Put("     */\n");
                    }

                    // Generate getter method
                    b.Put(indentation); b.Put("    ");
                                                 b.Put(bestType);
												
                                                 b.Put(" *get");
                                                 b.Put(symbolName);
                                                 b.Put("() { return ");
                                                 b.Put(ast_member_prefix.c_str());
                                                 b.Put(symbolName);
                                                 b.Put("; };\n");

                    // Generate setter method
                    b.Put(indentation); b.Put("    void set");
                    b.Put(symbolName);
                    b.Put("(");
                    b.Put(bestType);
                
                    b.Put(" *"); // add "_" prefix to arg name in case symbol happens to be a Java keyword
                    b.Put(ast_member_prefix.c_str());
                    b.Put(symbolName);
                    b.Put(")");
                    b.Put(" { this->");
                    b.Put(ast_member_prefix.c_str());
                    b.Put(symbolName);
                    b.Put(" = ");
                    b.Put(ast_member_prefix.c_str());
                    b.Put(symbolName);
                    b.Put("; }\n");
                }
            }
            b.Put("\n");
        }

        //
        // generate constructor
        //
        const char *header = "    ";
        b.Put(indentation);
        b.Put(header);
        b.Put(classname);
        int length = strlen(indentation) + strlen(header) + strlen(classname);

        b.Put("(");
        if (element.needs_environment)
        {
            b.Put(option -> action_type);
            b.Put(" *environment, ");
        }
        b.Put("IToken* leftIToken, IToken* rightIToken");
        b.Put(symbol_set.Size() == 0 ? ")\n" : ",\n");
        {
            for (int i = 0; i < symbol_set.Size(); i++)
            {
                for (int k = 0; k <= length; k++)
                    b.PutChar(' ');
              //  auto temp_type = ctc.FindBestTypeFor(rhs_type_index[i]);
                auto temp_type = ctc.FindUniqueTypeFor(rhs_type_index[i], option->ast_type);

                b.Put(temp_type);
                b.Put(" *");
                b.Put(ast_member_prefix.c_str());
                b.Put(symbol_set[i] -> Name());
                b.Put(i == symbol_set.Size() - 1 ? "):" : ",\n");
            }
        }
        b.Put(base_type);
        b.Put("(leftIToken, rightIToken)    {\n");
        if (element.needs_environment)
        {
            b.Put(indentation);
            b.Put("        this->environment = environment;\n");
        }

        {
            for (int i = 0; i < symbol_set.Size(); i++)
            {
                b.Put(indentation); b.Put("        this->");
											b.Put(ast_member_prefix.c_str());
                                             b.Put(symbol_set[i] -> Name());
                                             b.Put(" = ");
                                             b.Put(ast_member_prefix.c_str());
                                             b.Put(symbol_set[i] -> Name());
                                             b.Put(";\n");

                if (option -> parent_saved)
                {
                    b.Put(indentation); b.Put("        ");
                    if ((! optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
                    {
                        b.Put("if (");
                        b.Put(ast_member_prefix.c_str());
                        b.Put(symbol_set[i] -> Name());
                        b.Put(" != nullptr) ");
                    }
    
                    b.Put("((");
                    b.Put(option -> ast_type);
                    b.Put("*) ");
                    b.Put(ast_member_prefix.c_str());
                    b.Put(symbol_set[i] -> Name());
                    b.Put(")->setParent(this);\n");
                }
            }
        }

        b.Put(indentation); b.Put("        initialize();\n");
        b.Put(indentation); b.Put("    }\n");
    }

    if (option -> parent_saved)
        GenerateGetAllChildrenMethod(b, indentation, element);
 
    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set);
    
    return;
}


//
// Generate Ast class
//
void CppAction2::GenerateTerminalMergedClass(NTC &ntc,
                                             ActionFileSymbol* ast_filename_symbol,
                                             const char *indentation,
                                             ClassnameElement &element,
                                             Array<const char *> &)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    char *classname = element.real_name;
    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);
    add_forward_class_def(ast_filename_symbol, classname);
    b.Put(indentation);
                                 b.Put("struct ");
                                 b.Put(classname);
                                 b.Put(" :public ");
   /*                              b.Put(grammar -> Get_ast_token_classname());
                                 b.Put(" ,public ");
    for (int i = 0; i < element.interface.Length() - 1; i++)
    {
        b.Put(typestring[element.interface[i]]);
        b.Put(",public ");
    }
    b.Put(typestring[element.interface[element.interface.Length() - 1]]);
    b.Put("\n");*/
    b.Put(indentation); b.Put("{\n");
    if (element.needs_environment)
        GenerateEnvironmentDeclaration(b, indentation);
    SymbolLookupTable &symbol_set = element.symbol_set;
    if (symbol_set.Size() == 1) // if the right-hand side contains a symbol ...
    {
        b.Put(indentation); b.Put("    IToken get");
                                     b.Put(symbol_set[0] -> Name());
                                     b.Put("() { return leftIToken; }\n\n");
    }
    b.Put(indentation); b.Put("    ");
                                 b.Put(classname);
                                 b.Put("(");
                                 if (element.needs_environment)
                                 {
                                     b.Put(option -> action_type);
                                     b.Put(" environment, IToken token):");
                                     b.Put(grammar->Get_ast_token_classname());
                                     b.Put("(token)\n");
                                     b.Put(indentation); b.Put("    {\n");
                                
                                     b.Put(indentation); b.Put("        this->environment = environment;\n");
                                     b.Put(indentation); b.Put("        initialize();\n");
                                     b.Put(indentation); b.Put("    }\n");
                                 }
                                 else
                                 {
	                                 b.Put("IToken* token):");
                                     b.Put(grammar->Get_ast_token_classname());
                                     b.Put("(token)\n");
                                     b.Put("{ initialize(); }\n");
                                 }

    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);
  
    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set);

    return;
}


//
// Generate Ast class
//
void CppAction2::GenerateMergedClass(CTC &ctc,
                                     NTC &ntc,
                                     ActionFileSymbol* ast_filename_symbol,
                                     const char *indentation,
                                     ClassnameElement &element,
                                     Tuple< Tuple<ProcessedRuleElement> > &processed_rule_map,
                                     Array<const char *> &)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    char *classname = element.real_name;
    SymbolLookupTable &symbol_set = element.symbol_set;
    Tuple<int> &rhs_type_index = element.rhs_type_index;

    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);
    add_forward_class_def(ast_filename_symbol, classname);
    b.Put(indentation); 
                                 b.Put("struct ");
                                 b.Put(classname);
                                 b.Put(" :public  ");
                                 b.Put(option -> ast_type);
 /*                                b.Put(",public ");
    {
        for (int i = 0; i < element.interface.Length() - 1; i++)
        {
 
            b.Put(typestring[element.interface[i]]);
            b.Put(",public ");
        }
    }
    b.Put(typestring[element.interface[element.interface.Length() - 1]]);
    b.Put("\n");*/
    b.Put(indentation); b.Put("{\n");
    if (element.needs_environment)
        GenerateEnvironmentDeclaration(b, indentation);
    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            b.Put(indentation); b.Put("    ");
           // auto temp_type = ctc.FindBestTypeFor(rhs_type_index[i]);
            auto temp_type = ctc.FindUniqueTypeFor(rhs_type_index[i], option->ast_type);
                         
			 b.Put(temp_type);
	         b.Put(" *");
	         b.Put(ast_member_prefix.c_str());
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
                                             b.Put("</b> may be <b>nullptr</b>\n");
                b.Put(indentation); b.Put("     */\n");
            }

            b.Put(indentation);
        	b.Put("    ");
                                        
          //  auto temp_type = ctc.FindBestTypeFor(rhs_type_index[i]);
            auto temp_type = ctc.FindUniqueTypeFor(rhs_type_index[i], option->ast_type);

			 b.Put(temp_type);
	         b.Put(" *get");
	         b.Put(symbol_set[i] -> Name());
	         b.Put("() { return ");
	         b.Put(ast_member_prefix.c_str());
	         b.Put(symbol_set[i] -> Name());
	         b.Put("; }\n");
	}
    }
    b.Put("\n");


    //
    // generate merged constructor
    //
    const char *header = "    ";
    b.Put(indentation);
    b.Put(header);
    b.Put(classname);
    int length = strlen(indentation) + strlen(header) + strlen(classname);

    b.Put("(");
    if (element.needs_environment)
    {
        b.Put(option -> action_type);
        b.Put(" *environment, ");
    }
    b.Put("IToken *leftIToken, IToken *rightIToken");
    b.Put(symbol_set.Size() == 0 ? ")\n" : ",\n");
    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            for (int k = 0; k <= length; k++)
                b.PutChar(' ');
            auto temp_type = ctc.FindUniqueTypeFor(rhs_type_index[i], option->ast_type);

            b.Put(temp_type);
            b.Put(" *");
            b.Put(ast_member_prefix.c_str());
            b.Put(symbol_set[i] -> Name());
            b.Put(i == symbol_set.Size() - 1 ? ")\n" : ",\n");
        }
    }
    b.Put(":");
    b.Put(option->ast_type);
    b.Put(indentation); b.Put("(leftIToken, rightIToken)\n");
    b.Put(indentation); b.Put("    {\n");
 
    if (element.needs_environment)
    {
        b.Put(indentation);
        b.Put("        this->environment = environment;\n");
    }

    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            b.Put(indentation); b.Put("        this->");
										 b.Put(ast_member_prefix.c_str());
                                         b.Put(symbol_set[i] -> Name());
                                         b.Put(" = ");
                                         b.Put(ast_member_prefix.c_str());
                                         b.Put(symbol_set[i] -> Name());
                                         b.Put(";\n");
    
            if (option -> parent_saved)
            {
                b.Put(indentation); b.Put("        ");
                if ((! optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
                {
                    b.Put("if (");
                    b.Put(ast_member_prefix.c_str());
                    b.Put(symbol_set[i] -> Name());
                    b.Put(" != nullptr) ");
                }
    
                b.Put("((");
                b.Put(option -> ast_type);
                b.Put("*) ");
                b.Put(ast_member_prefix.c_str());
                b.Put(symbol_set[i] -> Name());
                b.Put(")->setParent(this);\n");
            }
        }
    }

    b.Put(indentation); b.Put("        initialize();\n");
    b.Put(indentation); b.Put("    }\n");

    if (option -> parent_saved)
        GenerateGetAllChildrenMethod(b, indentation, element);
  
    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set);

    return;
}


void CppAction2::GenerateInterface(bool is_terminal,
                                   ActionFileSymbol* ast_filename_symbol,
                                   const char *indentation,
                                   const char *interface_name,
                                   Tuple<int> &extension,
                                   Tuple<int> &classes,
                                   Tuple<ClassnameElement> &classname)
{

	return;
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    add_forward_class_def(ast_filename_symbol, interface_name);
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

    b.Put(indentation); b.Put("struct ");
                                 b.Put(interface_name);
    if (extension.Length() > 0)
    {
        b.Put(" : ");
        for (int k = 0; k < extension.Length() - 1; k++)
        {
            b.Put("public I");
            b.Put(extension[k] == grammar -> Get_ast_token_interface()
                               ? grammar -> Get_ast_token_classname()
                               : grammar -> RetrieveString(extension[k]));
            b.Put(", ");
        }
        b.Put("public I");
        b.Put(extension[extension.Length() - 1] == grammar -> Get_ast_token_interface()
                               ? grammar -> Get_ast_token_classname()
                               : grammar -> RetrieveString(extension[extension.Length() - 1]));
        b.Put(" {};\n\n");
    }
    else
    {
        b.Put(" :virtual IGetToken");
       
        b.Put("\n");
        b.Put(indentation); b.Put("{\n");
      //  b.Put(indentation); b.Put(" virtual   IToken* getLeftIToken() = 0;\n");
       // b.Put(indentation); b.Put(" virtual   IToken* getRightIToken() =0;\n");
        b.Put("\n");
        GenerateVisitorHeaders(b, indentation, "    ");
        b.Put(indentation); b.Put("};\n\n");
    }

    return;
}


//
//
//
void CppAction2::GenerateNullAstAllocation(TextBuffer &b, int rule_no)
{
    const char *code = "\n                    setResult(nullptr);";
    GenerateCode(&b, code, rule_no);

    return;
}

//
//
//
void CppAction2::GenerateAstAllocation(CTC &ctc,
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
    //        GenerateCode(&b, index.String(), rule_no);
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
    
    b.Put(" _automatic_ast_pool << ");
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
        //        GenerateCode(&b, index.String(), rule_no);
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
                   // GenerateCode(&b, lparen, rule_no);
                   // GenerateCode(&b, ctc.FindBestTypeFor(type_index[i]), rule_no);
                  //  GenerateCode(&b, rparen, rule_no);
                    GenerateCode(&b, "nullptr", rule_no);
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
                        b.Put(" _automatic_ast_pool << ");
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
                     // auto type_desc =  ctc.FindBestTypeFor(type_index[i]);
                      auto type_desc = ctc.FindUniqueTypeFor(type_index[i], option->ast_type);
   
                        GenerateCode(&b, lparen, rule_no);
                        GenerateCode(&b, type_desc, rule_no);
                        GenerateCode(&b, "*)", rule_no);
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
void CppAction2::GenerateListAllocation(CTC &ctc,
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

        b.Put(" _automatic_ast_pool << ");
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
                b.Put(" _automatic_ast_pool << ");
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
              //  auto temp_type = ctc.FindBestTypeFor(allocation_element.element_type_symbol_index);
                auto temp_type = ctc.FindUniqueTypeFor(allocation_element.element_type_symbol_index, option->ast_type);

                GenerateCode(&b, temp_type, rule_no);
                GenerateCode(&b, "*)", rule_no);
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
            GenerateCode(&b, "*)", rule_no);
            GenerateCode(&b, "getRhsSym(", rule_no);
            IntToString index(allocation_element.list_position);
            GenerateCode(&b, index.String(), rule_no);
            GenerateCode(&b, "))->addElement(", rule_no);
            if (grammar -> IsTerminal(allocation_element.element_symbol))
            {
                b.Put(" _automatic_ast_pool << ");
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
                //auto temp_type = ctc.FindBestTypeFor(allocation_element.element_type_symbol_index);
                auto temp_type = ctc.FindUniqueTypeFor(allocation_element.element_type_symbol_index, option->ast_type);

                GenerateCode(&b, temp_type, rule_no);
            
                GenerateCode(&b, " *)getRhsSym(", rule_no);
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
            GenerateCode(&b, "*)", rule_no);
            GenerateCode(&b, "getRhsSym(", rule_no);
            IntToString index(allocation_element.list_position);
            GenerateCode(&b, index.String(), rule_no);
        }

        GenerateCode(&b, rparen, rule_no);
    }

    GenerateCode(&b, trailer, rule_no);
 
    return;
}
