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
void CppAction2::ProcessRuleActionBlock(ActionBlockElement &action)
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
        const char beginjava[]   = { option -> escape, 'B', 'e', 'g', 'i', 'n', 'J', 'a', 'v', 'a', '\0'},
                   endjava[]     = { option -> escape, 'E', 'n', 'd', 'J', 'a', 'v', 'a', '\0'},
                   beginaction[] = { option -> escape, 'B', 'e', 'g', 'i', 'n', 'A', 'c', 't', 'i', 'o', 'n', '\0'},
                   noaction[]    = { option -> escape, 'N', 'o', 'A', 'c', 't', 'i', 'o', 'n', '\0'},
                   nullaction[]  = { option -> escape, 'N', 'u', 'l', 'l', 'A', 'c', 't', 'i', 'o', 'n', '\0'},
                   badaction[]   = { option -> escape, 'B', 'a', 'd', 'A', 'c', 't', 'i', 'o', 'n', '\0'};
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
    TextBuffer& ast_buffer = *(ast_filename_symbol->BodyBuffer());
	
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
            if (option->automatic_ast == Option::NESTED)
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
            if (option->automatic_ast == Option::NESTED)
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
        if (option->automatic_ast == Option::NESTED)
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

        if (option->automatic_ast == Option::NESTED)
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
                              (option->automatic_ast == Option::NESTED
	                               ? ast_filename_symbol
	                               : top_level_ast_file_symbol),
                              (option->automatic_ast == Option::NESTED
	                               ? (char*)"    "
	                               : (char*)""),
                              classname[i],
                              typestring);

            for (int j = 0; j < classname[i].special_arrays.Length(); j++)
            {
                //
                // Finish up the previous class we were procesing
                //
                if (option->automatic_ast == Option::NESTED) // Generate Class Closer
                    ast_buffer.Put("    };\n\n");
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
                                           (option->automatic_ast == Option::NESTED
	                                            ? ast_filename_symbol
	                                            : top_level_ast_file_symbol),
                                           (option->automatic_ast == Option::NESTED
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
                                  (option->automatic_ast == Option::NESTED
	                                   ? ast_filename_symbol
	                                   : top_level_ast_file_symbol),
                                  (option->automatic_ast == Option::NESTED
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
                                                (option->automatic_ast == Option::NESTED
	                                                 ? ast_filename_symbol
	                                                 : top_level_ast_file_symbol),
                                                (option->automatic_ast == Option::NESTED
	                                                 ? (char*)"    "
	                                                 : (char*)""),
                                                classname[i],
                                                typestring);
                else GenerateMergedClass(ctc,
                                         ntc,
                                         (option->automatic_ast == Option::NESTED
	                                          ? ast_filename_symbol
	                                          : top_level_ast_file_symbol),
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
                    if (top_level_ast_file_symbol != NULL) // possible when option -> automatic_ast == Option::TOPLEVEL
                    {
                        for (int j = 0; j < actions.Length(); j++)
                            actions[j].buffer = top_level_ast_file_symbol->BodyBuffer();
                    }
                    ProcessCodeActions(actions, typestring, processed_rule_map);
                }
            }
        }

        if (option->automatic_ast == Option::NESTED) // Generate Class Closer
            ast_buffer.Put("    };\n\n");
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
            if (option->automatic_ast == Option::NESTED)
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
            if (option->automatic_ast == Option::NESTED)
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
                        ast_ast_Allocation_symbol_buffer,
                        rule_no,
                        rule_allocation_map[rule_no]);
                }
                else
                {
                    if (user_specified_null_ast[rule_no] || (grammar->RhsSize(rule_no) == 0 && rule_allocation_map[rule_no].name == NULL))
                        GenerateNullAstAllocation(ast_ast_Allocation_symbol_buffer, rule_no);
                    else GenerateAstAllocation(ctc,
                        ast_ast_Allocation_symbol_buffer,
                        rule_allocation_map[rule_no],
                        processed_rule_map[rule_no],
                        typestring,
                        rule_no);
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
void CppAction2::GenerateEnvironmentDeclaration(TextBuffer &ast_buffer, const char *indentation)
{
    ast_buffer.Put(indentation); ast_buffer.Put("    ");
                                 ast_buffer.Put(option -> action_type);
                                 ast_buffer.Put("* environment;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    ");
                                 ast_buffer.Put(option -> action_type);
                                 ast_buffer.Put("* getEnvironment() { return environment; }\n\n");
}

//
//
//
void CppAction2::GenerateVisitorHeaders(TextBuffer &ast_buffer, const char *indentation, const char *modifiers)
{
    if (option -> visitor != Option::NONE)
    {
        char *header = new char[strlen(indentation) + strlen(modifiers) + 9];
        strcpy(header, indentation);
        strcat(header, modifiers);

        ast_buffer.Put(header);
        if (option -> visitor == Option::PREORDER)
        {
            ast_buffer.Put("virtual void accept(IAstVisitor* v)=0;");
        }
        else if (option -> visitor == Option::DEFAULT)
        {
            ast_buffer.Put("virtual void accept(");
            ast_buffer.Put(option -> visitor_type);
            ast_buffer.Put("* v)=0;");

            ast_buffer.Put("\n");

            ast_buffer.Put(header);
            ast_buffer.Put("virtual void accept(Argument");
            ast_buffer.Put(option -> visitor_type);
            ast_buffer.Put(" *v, Object* o)=0;\n");

            ast_buffer.Put(header);
            ast_buffer.Put("virtual lpg::Any accept(Result");
            ast_buffer.Put(option -> visitor_type);
            ast_buffer.Put(" *v)=0;\n");

            ast_buffer.Put(header);
            ast_buffer.Put("virtual lpg::Any accept(ResultArgument");
            ast_buffer.Put(option -> visitor_type);
            ast_buffer.Put(" *v, Object* o)=0;");
        }
        ast_buffer.Put("\n");

        delete [] header;
    }

    return;
}


//
//
//
void CppAction2::GenerateVisitorMethods(NTC &ntc,
                                        TextBuffer &ast_buffer,
                                        const char *indentation,
                                        ClassnameElement &element,
                                        BitSet &optimizable_symbol_set)
{
    if (option -> visitor == Option::DEFAULT)
    {
        ast_buffer.Put("\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    void accept(");
                                     ast_buffer.Put(option -> visitor_type);
                                     ast_buffer.Put(" *v) { v->visit(this); }\n");

        ast_buffer.Put(indentation); ast_buffer.Put("    void accept(Argument");
                                     ast_buffer.Put(option -> visitor_type);
                                     ast_buffer.Put(" *v, Object* o) { v->visit(this, o); }\n");

        ast_buffer.Put(indentation); ast_buffer.Put("    lpg::Any accept(Result");
                                     ast_buffer.Put(option -> visitor_type);
                                     ast_buffer.Put(" *v) { return v->visit(this); }\n");

        ast_buffer.Put(indentation); ast_buffer.Put("    lpg::Any accept(ResultArgument");
                                     ast_buffer.Put(option -> visitor_type);
                                     ast_buffer.Put(" *v, Object* o) { return v->visit(this, o); }\n");
    }
    else if (option -> visitor == Option::PREORDER)
    {
        ast_buffer.Put("\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    void accept(IAstVisitor* v)\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        if (! v->preVisit(this)) return;\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        enter(("); 
                                     ast_buffer.Put(option -> visitor_type);
                                     ast_buffer.Put("*) v);\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        v->postVisit(this);\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    }\n\n");

        ast_buffer.Put(indentation); ast_buffer.Put("    void enter(");
                                     ast_buffer.Put(option -> visitor_type);
                                     ast_buffer.Put(" *v)\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
        SymbolLookupTable &symbol_set = element.symbol_set;
        Tuple<int> &rhs_type_index = element.rhs_type_index;
        if (element.is_terminal_class || symbol_set.Size() == 0)
        {
            ast_buffer.Put(indentation); ast_buffer.Put("        v->visit(this);\n");
        }
        else
        {
            ast_buffer.Put(indentation); ast_buffer.Put("        bool checkChildren = v->visit(this);\n");
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
                    ast_buffer.Put("if (");
                    ast_buffer.Put(ast_member_prefix.c_str());
                    ast_buffer.Put(symbol_set[i] -> Name());
                    ast_buffer.Put(" != nullptr) ");
                }
                ast_buffer.Put("((IAst*)");
                ast_buffer.Put(ast_member_prefix.c_str());
                ast_buffer.Put(symbol_set[i] -> Name());
                ast_buffer.Put(")->accept(v);\n");
            }

            if (symbol_set.Size() > 1)
            {
                ast_buffer.Put(indentation); ast_buffer.Put("        }\n");
            }
        }
        ast_buffer.Put(indentation); ast_buffer.Put("        v->endVisit(this);\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
    }

    return;
}


//
//
//
void CppAction2::GenerateGetAllChildrenMethod(TextBuffer &ast_buffer,
                                              const char *indentation,
                                              ClassnameElement &element)
{
    if (! element.is_terminal_class)
    {
        SymbolLookupTable &symbol_set = element.symbol_set;

        ast_buffer.Put("\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    /**\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     * A list of all children of this node, including the nullptr ones.\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     */\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    std::vector<IAst*> getAllChildren()\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        std::vector<IAst*> list;\n");
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            ast_buffer.Put(indentation); ast_buffer.Put("        list.push_back((IAst*)");
										 ast_buffer.Put(ast_member_prefix.c_str());
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
void CppAction2::GenerateSimpleVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                const char *indentation,
                                                const char *interface_name,
                                                SymbolLookupTable &type_set)
{
    add_forward_class_def(ast_filename_symbol, interface_name);
    TextBuffer& ast_buffer = *(ast_filename_symbol->BodyBuffer());
    ast_buffer.Put(indentation); ast_buffer.Put("struct ");
                                 ast_buffer.Put(interface_name);
                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        ast_buffer.Put(indentation); ast_buffer.Put("    virtual void visit");
                                     ast_buffer.Put("(");
                                     ast_buffer.Put(symbol -> Name());
                                     ast_buffer.Put(" *n)=0;\n");
    }

                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    virtual void visit");
                                 ast_buffer.Put("(");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(" *n)=0;\n");

    ast_buffer.Put(indentation); ast_buffer.Put("};\n");
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
    TextBuffer& ast_buffer = *(ast_filename_symbol->BodyBuffer());
    ast_buffer.Put(indentation); ast_buffer.Put("struct ");
                                 ast_buffer.Put(interface_name);
                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        ast_buffer.Put(indentation); ast_buffer.Put("    virtual void visit");
                                     ast_buffer.Put("(");
                                     ast_buffer.Put(symbol -> Name());
                                     ast_buffer.Put(" *n, Object* o)=0;\n");
    }

                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("  virtual  void visit");
                                 ast_buffer.Put("(");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(" *n, Object* o)=0;\n");

    ast_buffer.Put(indentation); ast_buffer.Put("};\n");
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
    TextBuffer& ast_buffer = *(ast_filename_symbol->BodyBuffer());
    ast_buffer.Put(indentation); ast_buffer.Put("struct ");
                                 ast_buffer.Put(interface_name);
                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        ast_buffer.Put(indentation); ast_buffer.Put("  virtual  lpg::Any visit");
                                     ast_buffer.Put("(");
                                     ast_buffer.Put(symbol -> Name());
                                     ast_buffer.Put(" *n)=0;\n");
    }

                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("  virtual  lpg::Any visit");
                                 ast_buffer.Put("(");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(" *n)=0;\n");

    ast_buffer.Put(indentation); ast_buffer.Put("};\n");
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
    TextBuffer& ast_buffer = *(ast_filename_symbol->BodyBuffer());
    ast_buffer.Put(indentation); ast_buffer.Put("struct ");
    ast_buffer.Put(interface_name);
    ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        ast_buffer.Put(indentation); ast_buffer.Put("  virtual  lpg::Any visit");
                                     ast_buffer.Put("(");
                                     ast_buffer.Put(symbol -> Name());
                                     ast_buffer.Put(" *n, Object* o)=0;\n");
    }

                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("  virtual  lpg::Any visit");
                                 ast_buffer.Put("(");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(" *n, Object* o)=0;\n");

    ast_buffer.Put(indentation); ast_buffer.Put("};\n");
}


//
//
//
void CppAction2::GeneratePreorderVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                  const char *indentation,
                                                  const char *interface_name,
                                                  SymbolLookupTable &type_set)
{
    TextBuffer& ast_buffer = *(ast_filename_symbol->BodyBuffer());

    add_forward_class_def(ast_filename_symbol, interface_name);
    assert(option -> visitor == Option::PREORDER);
    ast_buffer.Put(indentation); ast_buffer.Put("struct ");
                                 ast_buffer.Put(interface_name);
                                 ast_buffer.Put(" :public IAstVisitor\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");

    //    ast_buffer.Put(indentation); ast_buffer.Put("    bool preVisit(");
    //                                 ast_buffer.Put(option -> ast_type);
    //                                 ast_buffer.Put(" element);\n");
    //
    //    ast_buffer.Put(indentation); ast_buffer.Put("    void postVisit(");
    //                                 ast_buffer.Put(option -> ast_type);
    //                                 ast_buffer.Put(" element);\n\n");

    ast_buffer.Put(indentation); ast_buffer.Put("  virtual  bool visit");
                                 ast_buffer.Put("(");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(" *n)=0;\n");
    ast_buffer.Put(indentation); ast_buffer.Put(" virtual   void endVisit");
                                 ast_buffer.Put("(");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(" *n)=0;\n\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        ast_buffer.Put(indentation); ast_buffer.Put(" virtual   bool visit");
                                     ast_buffer.Put("(");
                                     ast_buffer.Put(symbol -> Name());
                                     ast_buffer.Put(" *n)=0;\n");
        ast_buffer.Put(indentation); ast_buffer.Put("  virtual  void endVisit");
                                     ast_buffer.Put("(");
                                     ast_buffer.Put(symbol -> Name());
                                     ast_buffer.Put(" *n)=0;\n");
        ast_buffer.Put("\n");
    }

    ast_buffer.Put(indentation); ast_buffer.Put("};\n\n");

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
    TextBuffer& ast_buffer = *(ast_filename_symbol->BodyBuffer());
    ast_buffer.Put(indentation); 
                                 ast_buffer.Put("struct ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put(" :public ");
                                 ast_buffer.Put(option -> visitor_type);
                                 ast_buffer.Put(", public Argument");
                                 ast_buffer.Put(option -> visitor_type);
                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    virtual void unimplementedVisitor(const std::string &s)=0;\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            ast_buffer.Put(indentation); ast_buffer.Put(" virtual   void visit");
                                         ast_buffer.Put("(");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(" *n) { unimplementedVisitor(\"visit(");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(")\"); }\n");
            ast_buffer.Put(indentation); ast_buffer.Put("  virtual  void visit");
                                         ast_buffer.Put("(");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(" *n, Object* o) { unimplementedVisitor(\"visit(");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(", Object)\"); }\n");
            ast_buffer.Put("\n");
        }
    }

                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("  virtual  void visit");
                                 ast_buffer.Put("(");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(" *n)\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            ast_buffer.Put(indentation); ast_buffer.Put("        ");
                                        // ast_buffer.Put(i == 0 ? "" : "else ");
                                         ast_buffer.Put("if (dynamic_cast< ");                        
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put("*>(n)");
                                         ast_buffer.Put("){ visit((");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put("*) n);return;}\n");
        }
    }
    ast_buffer.Put(indentation); ast_buffer.Put("        throw UnsupportedOperationException(\"visit(\" + n->to_utf8_string() + \")\");\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n");


    ast_buffer.Put(indentation); ast_buffer.Put("    void visit");
                                 ast_buffer.Put("(");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(" *n, Object* o)\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            ast_buffer.Put(indentation); ast_buffer.Put("        ");
                                        // ast_buffer.Put(i == 0 ? "" : "else ");
                                         ast_buffer.Put("if (dynamic_cast<");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put("*>(n)");
                                         ast_buffer.Put(") {visit((");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put("*) n, o);return;}\n");
        }
    }
    ast_buffer.Put(indentation); ast_buffer.Put("        throw UnsupportedOperationException(\"visit(\" + n->to_utf8_string() + \")\");\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n");

    ast_buffer.Put(indentation); ast_buffer.Put("};\n");
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
    TextBuffer& ast_buffer = *(ast_filename_symbol->BodyBuffer());
    ast_buffer.Put(indentation); 
                                 ast_buffer.Put("struct ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put(" :public Result");
                                 ast_buffer.Put(option -> visitor_type);
                                 ast_buffer.Put(",public ResultArgument");
                                 ast_buffer.Put(option -> visitor_type);
                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    virtual lpg::Any unimplementedVisitor(const std::string& s);\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            ast_buffer.Put(indentation); ast_buffer.Put("    lpg::Any visit(");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(" *n) { return unimplementedVisitor(\"visit(");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(")\"); }\n");
            ast_buffer.Put(indentation); ast_buffer.Put("    lpg::Any visit(");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(" *n, Object* o) { return  unimplementedVisitor(\"visit(");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(", Object)\"); }\n");
            ast_buffer.Put("\n");
        }
    }

                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    lpg::Any visit");
                                 ast_buffer.Put("(");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(" *n)\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            ast_buffer.Put(indentation); ast_buffer.Put("        ");
                                         //ast_buffer.Put(i == 0 ? "" : "else ");
                                         ast_buffer.Put("if (dynamic_cast<");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put("*>(n) ");                                    
                                         ast_buffer.Put(") return visit((");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put("*) n);\n");
        }
    }
    ast_buffer.Put(indentation); ast_buffer.Put("        throw UnsupportedOperationException(\"visit(\" + n->to_utf8_string() + \")\");\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n");


    ast_buffer.Put(indentation); ast_buffer.Put("    lpg::Any visit");
                                 ast_buffer.Put("(");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(" *n, Object *o)\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            ast_buffer.Put(indentation); ast_buffer.Put("        ");
                                        // ast_buffer.Put(i == 0 ? "" : "else ");
                                         ast_buffer.Put("if (dynamic_cast<");
                                         ast_buffer.Put(symbol->Name());
                                         ast_buffer.Put("*>(n) ");
                                         ast_buffer.Put(") return visit((");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put("*) n, o);\n");
        }
    }
    ast_buffer.Put(indentation); ast_buffer.Put("        throw UnsupportedOperationException(\"visit(\" + n->to_utf8_string() + \")\");\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n");

    ast_buffer.Put(indentation); ast_buffer.Put("};\n");
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
    TextBuffer& ast_buffer = *(ast_filename_symbol->BodyBuffer());
    assert(option -> visitor == Option::PREORDER);
    ast_buffer.Put(indentation);
                                 ast_buffer.Put("struct ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put(" :public ");
                                 ast_buffer.Put(option -> visitor_type);
                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    virtual void unimplementedVisitor(const std::string& s)=0;\n\n");
    ast_buffer.Put(indentation); ast_buffer.Put("   virtual bool preVisit(IAst* element) { return true; }\n\n");
    ast_buffer.Put(indentation); ast_buffer.Put("   virtual void postVisit(IAst* element) {}\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            ast_buffer.Put(indentation); ast_buffer.Put(" virtual   bool visit");
                                         ast_buffer.Put("(");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(" *n) { unimplementedVisitor(\"visit(");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(")\"); return true; }\n");
            ast_buffer.Put(indentation); ast_buffer.Put("  virtual  void endVisit");
                                         ast_buffer.Put("(");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(" *n) { unimplementedVisitor(\"endVisit(");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put(")\"); }\n");
            ast_buffer.Put("\n");
        }
    }

                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put(" virtual   bool visit");
                                 ast_buffer.Put("(");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(" *n)\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            ast_buffer.Put(indentation); ast_buffer.Put("        ");
                                        // ast_buffer.Put(i == 0 ? "" : "else ");
                                         ast_buffer.Put("if (dynamic_cast<");
                                         ast_buffer.Put(symbol->Name());
                                         ast_buffer.Put("*>(n) ");
                                         ast_buffer.Put(") return visit((");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put("*) n);\n");
        }
    }
    ast_buffer.Put(indentation); ast_buffer.Put("        throw UnsupportedOperationException(\"visit(\" + n->to_utf8_string() + \")\");\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n");

    ast_buffer.Put(indentation); ast_buffer.Put("  virtual  void endVisit");
                                 ast_buffer.Put("(");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(" *n)\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            ast_buffer.Put(indentation); ast_buffer.Put("        ");
                                        // ast_buffer.Put(i == 0 ? "" : "else ");
                                         ast_buffer.Put("if (dynamic_cast<");
                                         ast_buffer.Put(symbol->Name());
                                         ast_buffer.Put("*>(n) ");
                                         ast_buffer.Put("){ endVisit((");
                                         ast_buffer.Put(symbol -> Name());
                                         ast_buffer.Put("*) n);return;}\n");
        }
    }
    ast_buffer.Put(indentation); ast_buffer.Put("        throw UnsupportedOperationException(\"visit(\" + n->to_utf8_string() + \")\");\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n");

    ast_buffer.Put(indentation); ast_buffer.Put("};\n");

    return;
}


//
// Generate the the Ast root classes
//
void CppAction2::GenerateAstType(ActionFileSymbol* ast_filename_symbol,
                                 const char *indentation,
                                 const char *classname)
{
    TextBuffer& ast_buffer = *(ast_filename_symbol->BodyBuffer());
    /*
     * First, generate the main root class
     */
    add_forward_class_def(ast_filename_symbol, classname);
	
    ast_buffer.Put(indentation);
                                 ast_buffer.Put("struct ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put(" :public IAst\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");
    if (option -> glr)
    {
        ast_buffer.Put(indentation); ast_buffer.Put("    Ast* nextAst = nullptr;\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    IAst* getNextAst() { return nextAst; }\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    void setNextAst(IAst* n) { nextAst = n; }\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    void resetNextAst() { nextAst = nullptr; }\n");
    }
    else ast_buffer.Put(indentation); ast_buffer.Put("    IAst* getNextAst() { return nullptr; }\n");

    ast_buffer.Put(indentation); ast_buffer.Put("     IToken* leftIToken=nullptr;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        IToken*    rightIToken=nullptr;\n");
    if (option -> parent_saved)
    {
        ast_buffer.Put(indentation); ast_buffer.Put("     IAst* parent = nullptr;\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     void setParent(IAst* parent) { this->parent = parent; }\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    IAst* getParent() { return parent; }\n");\
    }
    else
    {
        ast_buffer.Put(indentation); ast_buffer.Put("    IAst* getParent()\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        throw UnsupportedOperationException(\"noparent-saved option in effect\");\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
    }

    ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    IToken* getLeftIToken() { return leftIToken; }\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    IToken* getRightIToken() { return rightIToken; }\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    std::vector<IToken*> getPrecedingAdjuncts() { return leftIToken->getPrecedingAdjuncts(); }\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    std::vector<IToken*> getFollowingAdjuncts() { return rightIToken->getFollowingAdjuncts(); }\n\n");

    ast_buffer.Put(indentation); ast_buffer.Put("    std::wstring toString()\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        return leftIToken->getILexStream()->toString(leftIToken->getStartOffset(), rightIToken->getEndOffset());\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n\n");

    ast_buffer.Put(indentation); ast_buffer.Put("    ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put("(IToken* token) { this->leftIToken = this->rightIToken = token; }\n");

    ast_buffer.Put(indentation); ast_buffer.Put("    ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put("(IToken* leftIToken, IToken* rightIToken)\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        this->leftIToken = leftIToken;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        this->rightIToken = rightIToken;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    void initialize() {}\n");
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
        ast_buffer.Put(indentation); ast_buffer.Put("     * A list of all children of this node, excluding the nullptr ones.\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     */\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    std::vector<IAst*> getChildren()\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        std::vector<IAst*> list = getAllChildren();\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        int k = -1;\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        for (int i = 0; i < list.size(); i++)\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("            IAst* element = list[i];\n");
        ast_buffer.Put(indentation); ast_buffer.Put("            if (element != nullptr)\n");
        ast_buffer.Put(indentation); ast_buffer.Put("            {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("                if (++k != i)\n");
        ast_buffer.Put(indentation); ast_buffer.Put("                    list[k]=(element);\n");
        ast_buffer.Put(indentation); ast_buffer.Put("            }\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        }\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        for (int i = list.size() - 1; i > k; i--) // remove extraneous elements\n");
        ast_buffer.Put(indentation); ast_buffer.Put("           list.erase(list.begin()+i);\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        return list;\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    }\n\n");

        ast_buffer.Put(indentation); ast_buffer.Put("    /**\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     * A list of all children of this node, including the nullptr ones.\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     */\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    virtual std::vector<IAst*> getAllChildren()=0;\n");
    }
    else
    {
        ast_buffer.Put(indentation); ast_buffer.Put("    std::vector<IAst*> getChildren()\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        throw UnsupportedOperationException(\"noparent-saved option in effect\");\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    std::vector<IAst*> getAllChildren() { return getChildren(); }\n");
    }

    ast_buffer.Put("\n");


    GenerateVisitorHeaders(ast_buffer, indentation, "     ");

    //
    // Not Preorder visitor? generate dummy accept method to satisfy IAst abstract declaration of accept(IAstVisitor);
    // TODO: Should IAstVisitor be used for default visitors also? If (when) yes then we should remove it from the test below
    //
    if (option -> visitor == Option::NONE || option -> visitor == Option::DEFAULT) // ??? Don't need this for DEFAULT case after upgrade
    {
        ast_buffer.Put(indentation); ast_buffer.Put("    void accept(IAstVisitor* v) {}\n");
    }
    ast_buffer.Put(indentation); ast_buffer.Put("};\n\n");

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
    TextBuffer& ast_buffer = *(ast_filename_symbol->BodyBuffer());
    add_forward_class_def(ast_filename_symbol, this->abstract_ast_list_classname);
    /*
     * Generate the List root class
     */
    ast_buffer.Put(indentation); 								
                                 ast_buffer.Put("struct ");
                                 ast_buffer.Put(this -> abstract_ast_list_classname);
                                 ast_buffer.Put(" :public ");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(" ,public IAbstractArrayList<");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put("*>\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    bool leftRecursive;\n");
    //ast_buffer.Put(indentation); ast_buffer.Put("    java.util.ArrayList list;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    int size() { return list.size(); }\n");
    ast_buffer.Put(indentation); ast_buffer.Put("   std::vector<");
								 ast_buffer.Put(option->ast_type);
    ast_buffer.Put(indentation); ast_buffer.Put("   *> getList() { return list; }\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    ");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(" *getElementAt(int i) { return (");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put("*) list.at(leftRecursive ? i : list.size() - 1 - i); }\n");

	 ast_buffer.Put(indentation); ast_buffer.Put("   std::vector<");
	// ast_buffer.Put(indentation); ast_buffer.Put(option->ast_type);
     ast_buffer.Put("IAst");
    ast_buffer.Put(indentation); ast_buffer.Put("    *> getArrayList()\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        if (! leftRecursive) // reverse the list \n");
    ast_buffer.Put(indentation); ast_buffer.Put("        {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("            for (int i = 0, n = list.size() - 1; i < n; i++, n--)\n");
    ast_buffer.Put(indentation); ast_buffer.Put("            {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("                auto ith = list[i];\n");
    ast_buffer.Put(indentation); ast_buffer.Put("                 auto nth = list[n];\n");
    ast_buffer.Put(indentation); ast_buffer.Put("                list[i]=(nth);\n");
    ast_buffer.Put(indentation); ast_buffer.Put("                list[n]=(ith);\n");
    ast_buffer.Put(indentation); ast_buffer.Put("            }\n");
    ast_buffer.Put(indentation); ast_buffer.Put("            leftRecursive = true;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        }\n");
    ast_buffer.Put(indentation); ast_buffer.Put("         return  std::vector<IAst*>(list.begin(), list.end());\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n");

    ast_buffer.Put(indentation); ast_buffer.Put("    /**\n");
    ast_buffer.Put(indentation); ast_buffer.Put("     * @deprecated replaced by {@link #addElement()}\n");
    ast_buffer.Put(indentation); ast_buffer.Put("     *\n");
    ast_buffer.Put(indentation); ast_buffer.Put("     */\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    bool add(");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(" *element)\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        addElement(element);\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        return true;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    void addElement(");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(" *element)\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        list.push_back(element);\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        if (leftRecursive)\n");
    ast_buffer.Put(indentation); ast_buffer.Put("             rightIToken = element->getRightIToken();\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        else leftIToken =  element->getLeftIToken();\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n\n");

    // generate constructors for list class
    ast_buffer.Put(indentation); ast_buffer.Put("    ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put("(IToken* leftIToken, IToken* rightIToken, bool leftRecursive):");
                                 ast_buffer.Put(option->ast_type);
                                 ast_buffer.Put("(leftIToken, rightIToken)\n");
	
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        this->leftRecursive = leftRecursive;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n\n");

    ast_buffer.Put(indentation); ast_buffer.Put("    ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put("(");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put(" *element, bool leftRecursive):");
                                 ast_buffer.Put(option->ast_type);
                                 ast_buffer.Put("(element->getLeftIToken(), element->getRightIToken())\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    ast_buffer.Put(indentation); ast_buffer.Put("         this->leftRecursive = leftRecursive;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        list.push_back(element);\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n\n");

    if (option -> parent_saved)
    {
        ast_buffer.Put(indentation); ast_buffer.Put("    /**\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     * Make a copy of the list and return it. Note that we obtain the local list by\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     * invoking getArrayList so as to make sure that the list we return is in proper order.\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     */\n");

        ast_buffer.Put(indentation); ast_buffer.Put("   std::vector<");
       // ast_buffer.Put(option->ast_type);
       ast_buffer.Put("IAst");
        ast_buffer.Put(indentation); ast_buffer.Put("    *> getAllChildren()\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        auto list_= getArrayList();\n return   std::vector<IAst*>(list_.begin(),list_.end());\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    }\n\n");
    }

    //
    // Implementation for functions in java.util.List
    //

    Substitutions subs;
    subs["%%AstType%%"] = option->ast_type;
    subs["%%ListClassName%%"] = classname;
  
	ast_buffer.Put(indentation); ast_buffer.Put("};\n\n");
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
    TextBuffer & ast_buffer = *(ast_filename_symbol->BodyBuffer());
    /*
     * Generate the Token root class
     */
    ast_buffer.Put(indentation); 
                                 ast_buffer.Put("struct ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put(" :public  ");
                                 ast_buffer.Put(option -> ast_type);
                           /*      ast_buffer.Put(" ,public I");
                                 ast_buffer.Put(classname);*/
                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");

    ast_buffer.Put(indentation); ast_buffer.Put("    ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put("(IToken* token):");
                                 ast_buffer.Put(option->ast_type);
                                 ast_buffer.Put("(token){ }\n");
	
    ast_buffer.Put(indentation); ast_buffer.Put("    IToken* getIToken() { return leftIToken; }\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    std::wstring toString() { return leftIToken->toString(); }\n\n");

    ClassnameElement element; // generate a temporary element with no symbols in its symbol set.
    element.real_name = (char *) classname;
    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);

    if (option -> parent_saved)
    {
        ast_buffer.Put(indentation); ast_buffer.Put("    /**\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     * A token class has no children. So, we return the empty list.\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     */\n");
        ast_buffer.Put(indentation); ast_buffer.Put("   std::vector<IAst*> getAllChildren() { return {}; }\n\n");
    }

    GenerateVisitorMethods(ntc, ast_buffer, indentation, element, optimizable_symbol_set);

    ast_buffer.Put(indentation); ast_buffer.Put("};\n\n");

    return;
}


//
//
//
void CppAction2::GenerateCommentHeader(TextBuffer &ast_buffer,
                                       const char *indentation,
                                       Tuple<int> &ungenerated_rule,
                                       Tuple<int> &generated_rule)
{
    const char *rule_info = " *<li>Rule $rule_number:  $rule_text";
    BlockSymbol* scope_block = nullptr;
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


void CppAction2::GenerateListMethods(CTC &ctc,
                                     NTC &ntc,
                                     TextBuffer &ast_buffer,
                                     const char *indentation,
                                     const char *classname,
                                     ClassnameElement &element,
                                     Array<const char *> &typestring)
{
    const char *element_name = element.array_element_type_symbol -> Name(),
             //  *element_type = ctc.FindBestTypeFor(element.array_element_type_symbol -> SymbolIndex());
        * element_type = ctc.FindUniqueTypeFor(element.array_element_type_symbol->SymbolIndex(), option->ast_type);
    //
    // Generate ADD method
    //
    ast_buffer.Put(indentation); ast_buffer.Put("    ");
                                 ast_buffer.Put("void addElement(");
                                 ast_buffer.Put(element_type);                        
                                 ast_buffer.Put(" *");
                                 ast_buffer.Put(ast_member_prefix.c_str());
                                 ast_buffer.Put(element_name);
                                 ast_buffer.Put(")\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
  
    ast_buffer.Put(indentation);
    ast_buffer.Put("        ");
	ast_buffer.Put(this->abstract_ast_list_classname);
	ast_buffer.Put("::addElement((");
                                 ast_buffer.Put(option -> ast_type);
                                 ast_buffer.Put("*) ");
                                 ast_buffer.Put(ast_member_prefix.c_str());
                                 ast_buffer.Put(element_name);
                                 ast_buffer.Put(");\n");
    if (option -> parent_saved)
    {
        ast_buffer.Put(indentation); ast_buffer.Put("        ");
        if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
        {
            ast_buffer.Put("if (");
            ast_buffer.Put(ast_member_prefix.c_str());
            ast_buffer.Put(element_name);
            ast_buffer.Put(" != nullptr) ");
        }
        ast_buffer.Put("((");
        ast_buffer.Put(option -> ast_type);
        ast_buffer.Put("*) ");
        ast_buffer.Put(ast_member_prefix.c_str());
        ast_buffer.Put(element_name);
        ast_buffer.Put(")->setParent(this);\n");
    }
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n");

    //
    // Generate the "equals" method for this list
    //
    ast_buffer.Put("\n");
   

    //
    // Generate visitor methods.
    //
    if (option -> visitor == Option::DEFAULT)
    {
        ast_buffer.Put("\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    void accept(");
                                     ast_buffer.Put(option -> visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
            ast_buffer.Put(" *v) { for (int i = 0; i < size(); i++) v->visit"
                           "("
                           "get");
            ast_buffer.Put(element_name);
            ast_buffer.Put("At(i)"
                           "); }\n");
        }
        else
        {
            ast_buffer.Put(" *v) { for (int i = 0; i < size(); i++) get");
            ast_buffer.Put(element_name);
            ast_buffer.Put("At(i)->accept(v); }\n");
        }

        ast_buffer.Put(indentation); ast_buffer.Put("    void accept(Argument");
                                     ast_buffer.Put(option -> visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
            ast_buffer.Put(" *v, Object *o) { for (int i = 0; i < size(); i++) v->visit"
                           "("
                           "get");
            ast_buffer.Put(element_name);
            ast_buffer.Put("At(i), o");
            ast_buffer.Put("); }\n");
        }
        else
        {
            ast_buffer.Put(" *v, Object* o) { for (int i = 0; i < size(); i++) get");
            ast_buffer.Put(element_name);
            ast_buffer.Put("At(i)->accept(v, o); }\n");
        }

        //
        // Code cannot be generated to automatically visit a node that
        // can return a value. These cases are left up to the user.
        //
        ast_buffer.Put(indentation); ast_buffer.Put("    lpg::Any accept(Result");
                                     ast_buffer.Put(option -> visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
                                         ast_buffer.Put(" *v)\n");
            ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        std::vector<lpg::Any> result;\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        for (int i = 0; i < size(); i++)\n");
            ast_buffer.Put(indentation); ast_buffer.Put("            result.push_back(v->visit(get");
                                         ast_buffer.Put(element_name);
                                         ast_buffer.Put("At(i)));\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        return result;\n");
            ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
        }
        else
        {
                                         ast_buffer.Put(" *v)\n");
            ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        std::vector<lpg::Any> result;\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        for (int i = 0; i < size(); i++)\n");
            ast_buffer.Put(indentation); ast_buffer.Put("            result.push_back(get");
                                         ast_buffer.Put(element_name);
                                         ast_buffer.Put("At(i)->accept(v));\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        return result;\n");
            ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
        }

        ast_buffer.Put(indentation); ast_buffer.Put("    lpg::Any accept(ResultArgument");
                                     ast_buffer.Put(option -> visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
                                         ast_buffer.Put(" *v, Object* o)\n");
            ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        std::vector<lpg::Any> result;\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        for (int i = 0; i < size(); i++)\n");
            ast_buffer.Put(indentation); ast_buffer.Put("            result.push_back(v->visit(get");
                                         ast_buffer.Put(element_name);
                                         ast_buffer.Put("At(i), o));\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        return result;\n");
            ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
        }
        else
        {
                                         ast_buffer.Put(" *v, Object *o)\n");
            ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        std::vector<lpg::Any> result;\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        for (int i = 0; i < size(); i++)\n");
            ast_buffer.Put(indentation); ast_buffer.Put("            result.push_back(get");
                                         ast_buffer.Put(element_name);
                                         ast_buffer.Put("At(i)->accept(v, o));\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        return result;\n");
            ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
        }
    }
    else if (option -> visitor == Option::PREORDER)
    {
        ast_buffer.Put("\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    void accept(IAstVisitor* v)\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        if (! v->preVisit(this)) return;\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        enter((");
                                     ast_buffer.Put(option -> visitor_type);
                                     ast_buffer.Put("*) v);\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        v->postVisit(this);\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    void enter(");
                                     ast_buffer.Put(option -> visitor_type);
                                     ast_buffer.Put(" *v)\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        bool checkChildren = v->visit(this);\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        if (checkChildren)\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("            for (int i = 0; i < size(); i++)\n");
        ast_buffer.Put(indentation); ast_buffer.Put("            {\n");
        ast_buffer.Put(indentation); ast_buffer.Put("                ");

        const char *element_typename = ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex());
        if (element_typename != NULL)
        {
           // ast_buffer.Put(element_typename);
           // ast_buffer.Put("* element = get");
            ast_buffer.Put(element_typename);
            ast_buffer.Put("* element = (");
            ast_buffer.Put(element_typename);
            ast_buffer.Put("*)get");
            ast_buffer.Put(element_name);
            ast_buffer.Put("At(i);\n");
            if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
            {
                ast_buffer.Put(indentation); ast_buffer.Put("                if (element != nullptr)");
                ast_buffer.Put(indentation); ast_buffer.Put("                {\n");
                ast_buffer.Put(indentation); ast_buffer.Put("                    if (! v->preVisit(element)) continue;\n");
                ast_buffer.Put(indentation); ast_buffer.Put("                    element->enter(v);\n");
                ast_buffer.Put(indentation); ast_buffer.Put("                    v->postVisit(element);\n");
                ast_buffer.Put(indentation); ast_buffer.Put("                }\n");
            }
            else
            {
                ast_buffer.Put(indentation); ast_buffer.Put("                if (! v->preVisit(element)) continue;\n");
                ast_buffer.Put(indentation); ast_buffer.Put("                element->enter(v);\n");
                ast_buffer.Put(indentation); ast_buffer.Put("                v->postVisit(element);\n");
            }
        }
        else
        {
           // element_typename = typestring[element.array_element_type_symbol->SymbolIndex()];
            element_typename = "IAst";
            ast_buffer.Put(element_typename);
           // ast_buffer.Put("* element = get");
           // ast_buffer.Put(element_name);

           
            ast_buffer.Put("* element = (");
            ast_buffer.Put(element_typename);
            ast_buffer.Put("*)get");
            ast_buffer.Put(element_name);
            ast_buffer.Put("At(i);\n");
            ast_buffer.Put(indentation); ast_buffer.Put("                ");
            if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
                ast_buffer.Put("if (element != nullptr) ");
            ast_buffer.Put("element->accept(v);\n");
        }
        ast_buffer.Put(indentation); ast_buffer.Put("            }\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        }\n");
        ast_buffer.Put(indentation); ast_buffer.Put("        v->endVisit(this);\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
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

    TextBuffer& ast_buffer = *(ast_filename_symbol->BodyBuffer());
    Tuple<int> &interface = element.interface_;
    assert(element.array_element_type_symbol != NULL);
    const char *classname = element.real_name,
               *element_name = element.array_element_type_symbol -> Name(),
             //  *element_type = ctc.FindBestTypeFor(element.array_element_type_symbol -> SymbolIndex());
        * element_type = ctc.FindUniqueTypeFor(element.array_element_type_symbol->SymbolIndex(), option->ast_type);
  
	
    add_forward_class_def(ast_filename_symbol, classname);
    GenerateCommentHeader(ast_buffer, indentation, element.ungenerated_rule, element.rule);

    ast_buffer.Put(indentation);
                                 ast_buffer.Put("struct ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put(" :public ");
                                 ast_buffer.Put(this -> abstract_ast_list_classname);
    /*                             ast_buffer.Put(" ,public ");
    for (int i = 0; i < interface.Length() - 1; i++)
    {
        ast_buffer.Put(typestring[element.interface[i]]);
        ast_buffer.Put(", ");
    }
    ast_buffer.Put(typestring[element.interface[interface.Length() - 1]]);
    ast_buffer.Put("\n");*/
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");

    if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
    {
        ast_buffer.Put(indentation); ast_buffer.Put("    /**\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     * The value returned by <b>get");
                                     ast_buffer.Put(element_name);
                                     ast_buffer.Put("At</b> may be <b>nullptr</b>\n");
        ast_buffer.Put(indentation); ast_buffer.Put("     */\n");
    }
    ast_buffer.Put(indentation); ast_buffer.Put("    ");
                                 ast_buffer.Put(element_type);
								
                                 ast_buffer.Put("* get");
                                 ast_buffer.Put(element_name);
                                 ast_buffer.Put("At(int i) { return (");
                                 ast_buffer.Put(element_type);
                                 ast_buffer.Put("*) getElementAt(i); }\n\n");

    //
    // generate constructors
    //
    ast_buffer.Put(indentation); ast_buffer.Put("    ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put("(");
                                 ast_buffer.Put("IToken* leftIToken, IToken* rightIToken, bool leftRecursive):");
                                 ast_buffer.Put(this->abstract_ast_list_classname);
    ast_buffer.Put(indentation); ast_buffer.Put("(leftIToken, rightIToken, leftRecursive)\n{\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n\n");

    ast_buffer.Put(indentation); ast_buffer.Put("    ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put("(");
                                ast_buffer.Put(element_type);
                               
                                 ast_buffer.Put("* ");
                                 ast_buffer.Put(ast_member_prefix.c_str());
                                 ast_buffer.Put(element_name);
                                 ast_buffer.Put(", bool leftRecursive):");
                                 ast_buffer.Put(this->abstract_ast_list_classname);
                                ast_buffer.Put("        ((");
                                 ast_buffer.Put(option->ast_type);
                                 ast_buffer.Put("*) ");
                                 ast_buffer.Put(ast_member_prefix.c_str());
                                 ast_buffer.Put(element_name);
                                 ast_buffer.Put(", leftRecursive)\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");

    if (option -> parent_saved)
    {
        ast_buffer.Put(indentation); ast_buffer.Put("        ");
        if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
        {
            ast_buffer.Put("if (");
            ast_buffer.Put(ast_member_prefix.c_str());
            ast_buffer.Put(element_name);
            ast_buffer.Put(" != nullptr) ");
        }
        ast_buffer.Put("((");
        ast_buffer.Put(option -> ast_type);
        ast_buffer.Put("*) ");
        ast_buffer.Put(ast_member_prefix.c_str());
        ast_buffer.Put(element_name);
        ast_buffer.Put(")->setParent(this);\n");
    }
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
    ast_buffer.Put("\n");

    GenerateListMethods(ctc, ntc, ast_buffer, indentation, classname, element, typestring);

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
    TextBuffer& ast_buffer = *(ast_filename_symbol->BodyBuffer());
    const char *classname = element.real_name,
               *element_name = element.array_element_type_symbol -> Name(),
            //   *element_type = ctc.FindBestTypeFor(element.array_element_type_symbol -> SymbolIndex());
    * element_type = ctc.FindUniqueTypeFor(element.array_element_type_symbol->SymbolIndex(), option->ast_type);
	
    GenerateCommentHeader(ast_buffer, indentation, element.ungenerated_rule, special_array.rules);

    add_forward_class_def(ast_filename_symbol, classname);
    ast_buffer.Put(indentation); 
                                 ast_buffer.Put("struct ");
                                 ast_buffer.Put(special_array.name);
                                 ast_buffer.Put(" :public ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put("\n");
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");

    GenerateEnvironmentDeclaration(ast_buffer, indentation);

    ast_buffer.Put(indentation); ast_buffer.Put("    ");
                                 ast_buffer.Put(special_array.name);
                                 ast_buffer.Put("(");
                                 ast_buffer.Put(option -> action_type);
                                 ast_buffer.Put(" *environment, ");
                                 ast_buffer.Put("IToken* leftIToken, IToken* rightIToken, bool leftRecursive):");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put("(leftIToken, rightIToken, leftRecursive)\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
    ast_buffer.Put(indentation);
    ast_buffer.Put(indentation); ast_buffer.Put("        this->environment = environment;\n");
    ast_buffer.Put(indentation); ast_buffer.Put("        initialize();\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n\n");

    ast_buffer.Put(indentation); ast_buffer.Put("    ");
                                 ast_buffer.Put(special_array.name);
                                 ast_buffer.Put("(");
                                 ast_buffer.Put(option -> action_type);
                                 ast_buffer.Put(" *environment, ");
                                 ast_buffer.Put(element_type);
                                 ast_buffer.Put(" * ");
                                 ast_buffer.Put(ast_member_prefix.c_str());
                                 ast_buffer.Put(element_name);
                                 ast_buffer.Put(", bool leftRecursive):");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put("(");
                                 ast_buffer.Put(ast_member_prefix.c_str());
                                 ast_buffer.Put(element_name);
                                 ast_buffer.Put(", leftRecursive)\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");

    ast_buffer.Put(indentation); ast_buffer.Put("        this->environment = environment;\n");
    if (option -> parent_saved)
    {
        ast_buffer.Put(indentation); ast_buffer.Put("        ");
        if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
        {
            ast_buffer.Put("if (");
            ast_buffer.Put(ast_member_prefix.c_str());
            ast_buffer.Put(element_name);
            ast_buffer.Put(" != nullptr) ");
        }
        ast_buffer.Put("((");
        ast_buffer.Put(option -> ast_type);
        ast_buffer.Put("*) ");
        ast_buffer.Put(ast_member_prefix.c_str());
        ast_buffer.Put(element_name);
        ast_buffer.Put(")->setParent(this);\n");
    }
    ast_buffer.Put(indentation); ast_buffer.Put("        initialize();\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n\n");

    GenerateListMethods(ctc, ntc, ast_buffer, indentation, special_array.name, element, typestring);

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
                                   Array<const char *> &typestring)
{
    TextBuffer& ast_buffer = *(ast_filename_symbol->BodyBuffer());
    char *classname = element.real_name;
    SymbolLookupTable &symbol_set = element.symbol_set;
    Tuple<int> &rhs_type_index = element.rhs_type_index;

    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);

    GenerateCommentHeader(ast_buffer, indentation, element.ungenerated_rule, element.rule);

    assert(element.rule.Length() == 1);
    int rule_no = element.rule[0];
	
    add_forward_class_def(ast_filename_symbol, classname);
	
    ast_buffer.Put(indentation);
                                 ast_buffer.Put("struct ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put(" :public ");
    const     char* base_type ;
    if (element.is_terminal_class)
    {

        base_type = grammar->Get_ast_token_classname();
        ast_buffer.Put(base_type);
     /*   ast_buffer.Put(" ,public ");
        ast_buffer.Put(typestring[grammar -> rules[rule_no].lhs]);*/
        ast_buffer.Put("\n");
        ast_buffer.Put(indentation); ast_buffer.Put("{\n");
        if (element.needs_environment)
            GenerateEnvironmentDeclaration(ast_buffer, indentation);
        if (symbol_set.Size() == 1) // if the right-hand side contains a symbol ...
        {
            ast_buffer.Put(indentation); ast_buffer.Put("    IToken* get");
                                         ast_buffer.Put(symbol_set[0] -> Name());
                                         ast_buffer.Put("() { return leftIToken; }\n\n");
        }
        ast_buffer.Put(indentation); ast_buffer.Put("    ");
                                     ast_buffer.Put(classname);
                                     ast_buffer.Put("(");
        if (element.needs_environment)
        {
            ast_buffer.Put(option -> action_type);
            ast_buffer.Put(" *environment, IToken* token):");
            ast_buffer.Put(base_type);
            ast_buffer.Put(indentation); ast_buffer.Put("(token)\n");
            ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
          
            ast_buffer.Put(indentation); ast_buffer.Put("        this->environment = environment;\n");
            ast_buffer.Put(indentation); ast_buffer.Put("        initialize();\n");
            ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
        }
        else
        {
	        ast_buffer.Put("IToken* token) :");
            ast_buffer.Put(base_type);
            ast_buffer.Put("(token)\n{\n initialize(); }\n");
        }
    }
    else 
    {
        base_type = option->ast_type;
        ast_buffer.Put(base_type);
      /*  ast_buffer.Put(" ,public ");
        ast_buffer.Put(typestring[grammar -> rules[rule_no].lhs]);*/
        ast_buffer.Put("\n");
        ast_buffer.Put(indentation); ast_buffer.Put("{\n");
        if (element.needs_environment)
            GenerateEnvironmentDeclaration(ast_buffer, indentation);

        if (symbol_set.Size() > 0)
        {
            {
                for (int i = 0; i < symbol_set.Size(); i++)
                {
                   // auto temp_type = ctc.FindBestTypeFor(rhs_type_index[i]);
                    auto temp_type = ctc.FindUniqueTypeFor(rhs_type_index[i], option->ast_type);
                	
                    ast_buffer.Put(indentation); ast_buffer.Put("    ");
                                                
												 ast_buffer.Put(temp_type);
                                                 ast_buffer.Put(" *");
                                                 ast_buffer.Put(ast_member_prefix.c_str());
                                                 ast_buffer.Put(symbol_set[i] -> Name());
                                                 ast_buffer.Put(";\n");
                }
            }
            ast_buffer.Put("\n");

            {
                for (int i = 0; i < symbol_set.Size(); i++)
                {
                    const char *symbolName = symbol_set[i] -> Name();
                   // const char *bestType = ctc.FindBestTypeFor(rhs_type_index[i]);
                    const char* bestType = ctc.FindUniqueTypeFor(rhs_type_index[i], option->ast_type);
                	
                	
                    if (ntc.CanProduceNullAst(rhs_type_index[i]))
                    {
                        ast_buffer.Put(indentation); ast_buffer.Put("    /**\n");
                        ast_buffer.Put(indentation); ast_buffer.Put("     * The value returned by <b>get");
                                                     ast_buffer.Put(symbolName);
                                                     ast_buffer.Put("</b> may be <b>nullptr</b>\n");
                        ast_buffer.Put(indentation); ast_buffer.Put("     */\n");
                    }

                    // Generate getter method
                    ast_buffer.Put(indentation); ast_buffer.Put("    ");
                                                 ast_buffer.Put(bestType);
												
                                                 ast_buffer.Put(" *get");
                                                 ast_buffer.Put(symbolName);
                                                 ast_buffer.Put("() { return ");
                                                 ast_buffer.Put(ast_member_prefix.c_str());
                                                 ast_buffer.Put(symbolName);
                                                 ast_buffer.Put("; };\n");

                    // Generate setter method
                    ast_buffer.Put(indentation); ast_buffer.Put("    void set");
                    ast_buffer.Put(symbolName);
                    ast_buffer.Put("(");
                    ast_buffer.Put(bestType);
                
                    ast_buffer.Put(" *"); // add "_" prefix to arg name in case symbol happens to be a Java keyword
                    ast_buffer.Put(ast_member_prefix.c_str());
                    ast_buffer.Put(symbolName);
                    ast_buffer.Put(")");
                    ast_buffer.Put(" { this->");
                    ast_buffer.Put(ast_member_prefix.c_str());
                    ast_buffer.Put(symbolName);
                    ast_buffer.Put(" = ");
                    ast_buffer.Put(ast_member_prefix.c_str());
                    ast_buffer.Put(symbolName);
                    ast_buffer.Put("; }\n");
                }
            }
            ast_buffer.Put("\n");
        }

        //
        // generate constructor
        //
        const char *header = "    ";
        ast_buffer.Put(indentation);
        ast_buffer.Put(header);
        ast_buffer.Put(classname);
        int length = strlen(indentation) + strlen(header) + strlen(classname);

        ast_buffer.Put("(");
        if (element.needs_environment)
        {
            ast_buffer.Put(option -> action_type);
            ast_buffer.Put(" *environment, ");
        }
        ast_buffer.Put("IToken* leftIToken, IToken* rightIToken");
        ast_buffer.Put(symbol_set.Size() == 0 ? ")\n" : ",\n");
        {
            for (int i = 0; i < symbol_set.Size(); i++)
            {
                for (int k = 0; k <= length; k++)
                    ast_buffer.PutChar(' ');
              //  auto temp_type = ctc.FindBestTypeFor(rhs_type_index[i]);
                auto temp_type = ctc.FindUniqueTypeFor(rhs_type_index[i], option->ast_type);

                ast_buffer.Put(temp_type);
                ast_buffer.Put(" *");
                ast_buffer.Put(ast_member_prefix.c_str());
                ast_buffer.Put(symbol_set[i] -> Name());
                ast_buffer.Put(i == symbol_set.Size() - 1 ? "):" : ",\n");
            }
        }
        ast_buffer.Put(base_type);
        ast_buffer.Put("(leftIToken, rightIToken)    {\n");
        if (element.needs_environment)
        {
            ast_buffer.Put(indentation);
            ast_buffer.Put("        this->environment = environment;\n");
        }

        {
            for (int i = 0; i < symbol_set.Size(); i++)
            {
                ast_buffer.Put(indentation); ast_buffer.Put("        this->");
											ast_buffer.Put(ast_member_prefix.c_str());
                                             ast_buffer.Put(symbol_set[i] -> Name());
                                             ast_buffer.Put(" = ");
                                             ast_buffer.Put(ast_member_prefix.c_str());
                                             ast_buffer.Put(symbol_set[i] -> Name());
                                             ast_buffer.Put(";\n");

                if (option -> parent_saved)
                {
                    ast_buffer.Put(indentation); ast_buffer.Put("        ");
                    if ((! optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
                    {
                        ast_buffer.Put("if (");
                        ast_buffer.Put(ast_member_prefix.c_str());
                        ast_buffer.Put(symbol_set[i] -> Name());
                        ast_buffer.Put(" != nullptr) ");
                    }
    
                    ast_buffer.Put("((");
                    ast_buffer.Put(option -> ast_type);
                    ast_buffer.Put("*) ");
                    ast_buffer.Put(ast_member_prefix.c_str());
                    ast_buffer.Put(symbol_set[i] -> Name());
                    ast_buffer.Put(")->setParent(this);\n");
                }
            }
        }

        ast_buffer.Put(indentation); ast_buffer.Put("        initialize();\n");
        ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
    }

    if (option -> parent_saved)
        GenerateGetAllChildrenMethod(ast_buffer, indentation, element);
 
    GenerateVisitorMethods(ntc, ast_buffer, indentation, element, optimizable_symbol_set);
    
    return;
}


//
// Generate Ast class
//
void CppAction2::GenerateTerminalMergedClass(NTC &ntc,
                                             ActionFileSymbol* ast_filename_symbol,
                                             const char *indentation,
                                             ClassnameElement &element,
                                             Array<const char *> &typestring)
{
    TextBuffer& ast_buffer = *(ast_filename_symbol->BodyBuffer());
    char *classname = element.real_name;
    GenerateCommentHeader(ast_buffer, indentation, element.ungenerated_rule, element.rule);
    add_forward_class_def(ast_filename_symbol, classname);
    ast_buffer.Put(indentation);
                                 ast_buffer.Put("struct ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put(" :public ");
   /*                              ast_buffer.Put(grammar -> Get_ast_token_classname());
                                 ast_buffer.Put(" ,public ");
    for (int i = 0; i < element.interface.Length() - 1; i++)
    {
        ast_buffer.Put(typestring[element.interface[i]]);
        ast_buffer.Put(",public ");
    }
    ast_buffer.Put(typestring[element.interface[element.interface.Length() - 1]]);
    ast_buffer.Put("\n");*/
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");
    if (element.needs_environment)
        GenerateEnvironmentDeclaration(ast_buffer, indentation);
    SymbolLookupTable &symbol_set = element.symbol_set;
    if (symbol_set.Size() == 1) // if the right-hand side contains a symbol ...
    {
        ast_buffer.Put(indentation); ast_buffer.Put("    IToken get");
                                     ast_buffer.Put(symbol_set[0] -> Name());
                                     ast_buffer.Put("() { return leftIToken; }\n\n");
    }
    ast_buffer.Put(indentation); ast_buffer.Put("    ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put("(");
                                 if (element.needs_environment)
                                 {
                                     ast_buffer.Put(option -> action_type);
                                     ast_buffer.Put(" environment, IToken token):");
                                     ast_buffer.Put(grammar->Get_ast_token_classname());
                                     ast_buffer.Put("(token)\n");
                                     ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
                                
                                     ast_buffer.Put(indentation); ast_buffer.Put("        this->environment = environment;\n");
                                     ast_buffer.Put(indentation); ast_buffer.Put("        initialize();\n");
                                     ast_buffer.Put(indentation); ast_buffer.Put("    }\n");
                                 }
                                 else
                                 {
	                                 ast_buffer.Put("IToken* token):");
                                     ast_buffer.Put(grammar->Get_ast_token_classname());
                                     ast_buffer.Put("(token)\n");
                                     ast_buffer.Put("{ initialize(); }\n");
                                 }

    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);
  
    GenerateVisitorMethods(ntc, ast_buffer, indentation, element, optimizable_symbol_set);

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
                                     Array<const char *> &typestring)
{
    TextBuffer& ast_buffer = *(ast_filename_symbol->BodyBuffer());
    char *classname = element.real_name;
    SymbolLookupTable &symbol_set = element.symbol_set;
    Tuple<int> &rhs_type_index = element.rhs_type_index;

    GenerateCommentHeader(ast_buffer, indentation, element.ungenerated_rule, element.rule);
    add_forward_class_def(ast_filename_symbol, classname);
    ast_buffer.Put(indentation); 
                                 ast_buffer.Put("struct ");
                                 ast_buffer.Put(classname);
                                 ast_buffer.Put(" :public  ");
                                 ast_buffer.Put(option -> ast_type);
 /*                                ast_buffer.Put(",public ");
    {
        for (int i = 0; i < element.interface.Length() - 1; i++)
        {
 
            ast_buffer.Put(typestring[element.interface[i]]);
            ast_buffer.Put(",public ");
        }
    }
    ast_buffer.Put(typestring[element.interface[element.interface.Length() - 1]]);
    ast_buffer.Put("\n");*/
    ast_buffer.Put(indentation); ast_buffer.Put("{\n");
    if (element.needs_environment)
        GenerateEnvironmentDeclaration(ast_buffer, indentation);
    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            ast_buffer.Put(indentation); ast_buffer.Put("    ");
           // auto temp_type = ctc.FindBestTypeFor(rhs_type_index[i]);
            auto temp_type = ctc.FindUniqueTypeFor(rhs_type_index[i], option->ast_type);
                         
			 ast_buffer.Put(temp_type);
	         ast_buffer.Put(" *");
	         ast_buffer.Put(ast_member_prefix.c_str());
	         ast_buffer.Put(symbol_set[i] -> Name());
	         ast_buffer.Put(";\n");
        }
    }
    ast_buffer.Put("\n");

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
                ast_buffer.Put(indentation); ast_buffer.Put("    /**\n");
                ast_buffer.Put(indentation); ast_buffer.Put("     * The value returned by <b>get");
                                             ast_buffer.Put(symbol_set[i] -> Name());
                                             ast_buffer.Put("</b> may be <b>nullptr</b>\n");
                ast_buffer.Put(indentation); ast_buffer.Put("     */\n");
            }

            ast_buffer.Put(indentation);
        	ast_buffer.Put("    ");
                                        
          //  auto temp_type = ctc.FindBestTypeFor(rhs_type_index[i]);
            auto temp_type = ctc.FindUniqueTypeFor(rhs_type_index[i], option->ast_type);

			 ast_buffer.Put(temp_type);
	         ast_buffer.Put(" *get");
	         ast_buffer.Put(symbol_set[i] -> Name());
	         ast_buffer.Put("() { return ");
	         ast_buffer.Put(ast_member_prefix.c_str());
	         ast_buffer.Put(symbol_set[i] -> Name());
	         ast_buffer.Put("; }\n");
	}
    }
    ast_buffer.Put("\n");


    //
    // generate merged constructor
    //
    const char *header = "    ";
    ast_buffer.Put(indentation);
    ast_buffer.Put(header);
    ast_buffer.Put(classname);
    int length = strlen(indentation) + strlen(header) + strlen(classname);

    ast_buffer.Put("(");
    if (element.needs_environment)
    {
        ast_buffer.Put(option -> action_type);
        ast_buffer.Put(" *environment, ");
    }
    ast_buffer.Put("IToken *leftIToken, IToken *rightIToken");
    ast_buffer.Put(symbol_set.Size() == 0 ? ")\n" : ",\n");
    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            for (int k = 0; k <= length; k++)
                ast_buffer.PutChar(' ');
            auto temp_type = ctc.FindUniqueTypeFor(rhs_type_index[i], option->ast_type);

            ast_buffer.Put(temp_type);
            ast_buffer.Put(" *");
            ast_buffer.Put(ast_member_prefix.c_str());
            ast_buffer.Put(symbol_set[i] -> Name());
            ast_buffer.Put(i == symbol_set.Size() - 1 ? ")\n" : ",\n");
        }
    }
    ast_buffer.Put(":");
    ast_buffer.Put(option->ast_type);
    ast_buffer.Put(indentation); ast_buffer.Put("(leftIToken, rightIToken)\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    {\n");
 
    if (element.needs_environment)
    {
        ast_buffer.Put(indentation);
        ast_buffer.Put("        this->environment = environment;\n");
    }

    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            ast_buffer.Put(indentation); ast_buffer.Put("        this->");
										 ast_buffer.Put(ast_member_prefix.c_str());
                                         ast_buffer.Put(symbol_set[i] -> Name());
                                         ast_buffer.Put(" = ");
                                         ast_buffer.Put(ast_member_prefix.c_str());
                                         ast_buffer.Put(symbol_set[i] -> Name());
                                         ast_buffer.Put(";\n");
    
            if (option -> parent_saved)
            {
                ast_buffer.Put(indentation); ast_buffer.Put("        ");
                if ((! optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
                {
                    ast_buffer.Put("if (");
                    ast_buffer.Put(ast_member_prefix.c_str());
                    ast_buffer.Put(symbol_set[i] -> Name());
                    ast_buffer.Put(" != nullptr) ");
                }
    
                ast_buffer.Put("((");
                ast_buffer.Put(option -> ast_type);
                ast_buffer.Put("*) ");
                ast_buffer.Put(ast_member_prefix.c_str());
                ast_buffer.Put(symbol_set[i] -> Name());
                ast_buffer.Put(")->setParent(this);\n");
            }
        }
    }

    ast_buffer.Put(indentation); ast_buffer.Put("        initialize();\n");
    ast_buffer.Put(indentation); ast_buffer.Put("    }\n");

    if (option -> parent_saved)
        GenerateGetAllChildrenMethod(ast_buffer, indentation, element);
  
    GenerateVisitorMethods(ntc, ast_buffer, indentation, element, optimizable_symbol_set);

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
    TextBuffer& ast_buffer = *(ast_filename_symbol->BodyBuffer());
    add_forward_class_def(ast_filename_symbol, interface_name);
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

    ast_buffer.Put(indentation); ast_buffer.Put("struct ");
                                 ast_buffer.Put(interface_name);
    if (extension.Length() > 0)
    {
        ast_buffer.Put(" : ");
        for (int k = 0; k < extension.Length() - 1; k++)
        {
            ast_buffer.Put("public I");
            ast_buffer.Put(extension[k] == grammar -> Get_ast_token_interface()
                               ? grammar -> Get_ast_token_classname()
                               : grammar -> RetrieveString(extension[k]));
            ast_buffer.Put(", ");
        }
        ast_buffer.Put("public I");
        ast_buffer.Put(extension[extension.Length() - 1] == grammar -> Get_ast_token_interface()
                               ? grammar -> Get_ast_token_classname()
                               : grammar -> RetrieveString(extension[extension.Length() - 1]));
        ast_buffer.Put(" {};\n\n");
    }
    else
    {
        ast_buffer.Put(" :virtual IGetToken");
       
        ast_buffer.Put("\n");
        ast_buffer.Put(indentation); ast_buffer.Put("{\n");
      //  ast_buffer.Put(indentation); ast_buffer.Put(" virtual   IToken* getLeftIToken() = 0;\n");
       // ast_buffer.Put(indentation); ast_buffer.Put(" virtual   IToken* getRightIToken() =0;\n");
        ast_buffer.Put("\n");
        GenerateVisitorHeaders(ast_buffer, indentation, "    ");
        ast_buffer.Put(indentation); ast_buffer.Put("};\n\n");
    }

    return;
}


//
//
//
void CppAction2::GenerateNullAstAllocation(TextBuffer &ast_buffer, int rule_no)
{
    const char *code = "\n                    setResult(nullptr);";
    GenerateCode(&ast_buffer, code, rule_no);

    return;
}

//
//
//
void CppAction2::GenerateAstAllocation(CTC &ctc,
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
    //        GenerateCode(&ast_buffer, index.String(), rule_no);
    //        GenerateCode(&ast_buffer, rparen, rule_no);
    //    }
    //
    if (allocation_element.is_terminal_class && (grammar -> RhsSize(rule_no) == 1 && grammar -> IsNonTerminal(grammar -> rhs_sym[grammar -> FirstRhsIndex(rule_no)])))
    {
        GenerateCode(&ast_buffer, space, rule_no);
        GenerateCode(&ast_buffer, "//", rule_no);
        GenerateCode(&ast_buffer, space, rule_no);
        GenerateCode(&ast_buffer, "// When garbage collection is not available, delete ", rule_no);
        GenerateCode(&ast_buffer, "getRhsSym(1)", rule_no);
        GenerateCode(&ast_buffer, space, rule_no);
        GenerateCode(&ast_buffer, "//", rule_no);
    }
    GenerateCode(&ast_buffer, space, rule_no);
    GenerateCode(&ast_buffer, "setResult(", rule_no);
    GenerateCode(&ast_buffer, space, rule_no);
    GenerateCode(&ast_buffer, space4, rule_no);
    GenerateCode(&ast_buffer, "//#line $current_line $input_file$", rule_no);
    GenerateCode(&ast_buffer, space, rule_no);
    GenerateCode(&ast_buffer, space4, rule_no);
    
    ast_buffer.Put(" _automatic_ast_pool << ");
    GenerateCode(&ast_buffer, newkey, rule_no);
    GenerateCode(&ast_buffer, classname, rule_no);
    GenerateCode(&ast_buffer, lparen, rule_no);
    if (allocation_element.needs_environment)
    {
        GenerateCode(&ast_buffer, "this, ", rule_no);
    }
    if (allocation_element.is_terminal_class)
    {
        GenerateCode(&ast_buffer, "getRhsIToken(1)", rule_no);
        //
        // TODO: Old bad idea. Remove at some point...
        //
        //
        //        assert(position.Length() <= 1);
        //
        //        GenerateCode(&ast_buffer, "getRhsIToken(", rule_no);
        //        IntToString index(position.Length() == 0 ? 1 : position[0]);
        //        GenerateCode(&ast_buffer, index.String(), rule_no);
        //        GenerateCode(&ast_buffer, rparen, rule_no);
        //
    }
    else
    {
        GenerateCode(&ast_buffer, "getLeftIToken()", rule_no);
        GenerateCode(&ast_buffer, ", ", rule_no);
        GenerateCode(&ast_buffer, "getRightIToken()", rule_no);
        if (position.Length() > 0)
        {
            GenerateCode(&ast_buffer, comma, rule_no);
            GenerateCode(&ast_buffer, extra_space, rule_no);
            GenerateCode(&ast_buffer, "//#line $current_line $input_file$", rule_no);
            GenerateCode(&ast_buffer, extra_space, rule_no);

            int offset = grammar -> FirstRhsIndex(rule_no) - 1;
            for (int i = 0; i < position.Length(); i++)
            {
                if (position[i] == 0)
                {
                   // GenerateCode(&ast_buffer, lparen, rule_no);
                   // GenerateCode(&ast_buffer, ctc.FindBestTypeFor(type_index[i]), rule_no);
                  //  GenerateCode(&ast_buffer, rparen, rule_no);
                    GenerateCode(&ast_buffer, "nullptr", rule_no);
                }
                else
                {
                    int symbol = grammar -> rhs_sym[offset + position[i]];
                    if (grammar -> IsTerminal(symbol))
                    {
                        const char *actual_type = ctc.FindBestTypeFor(type_index[i]);

                        if (strcmp(actual_type, grammar -> Get_ast_token_classname()) != 0)
                        {
                            GenerateCode(&ast_buffer, lparen, rule_no);
                            GenerateCode(&ast_buffer, actual_type, rule_no);
                            GenerateCode(&ast_buffer, rparen, rule_no);
                        }
                        ast_buffer.Put(" _automatic_ast_pool << ");
                        GenerateCode(&ast_buffer, newkey, rule_no);
                       
                        GenerateCode(&ast_buffer, grammar -> Get_ast_token_classname(), rule_no);
                      
                        GenerateCode(&ast_buffer, lparen, rule_no);
                        GenerateCode(&ast_buffer, "getRhsIToken(", rule_no);
                        IntToString index(position[i]);
                        GenerateCode(&ast_buffer, index.String(), rule_no);
                        GenerateCode(&ast_buffer, rparen, rule_no);
                    }
                    else
                    {
                     // auto type_desc =  ctc.FindBestTypeFor(type_index[i]);
                      auto type_desc = ctc.FindUniqueTypeFor(type_index[i], option->ast_type);
   
                        GenerateCode(&ast_buffer, lparen, rule_no);
                        GenerateCode(&ast_buffer, type_desc, rule_no);
                        GenerateCode(&ast_buffer, "*)", rule_no);
                        GenerateCode(&ast_buffer, "getRhsSym(", rule_no);
                        IntToString index(position[i]);
                        GenerateCode(&ast_buffer, index.String(), rule_no);
                    }
    
                    GenerateCode(&ast_buffer, rparen, rule_no);
                }
        
                if (i != position.Length() - 1)
                {
                    GenerateCode(&ast_buffer, comma, rule_no);
                    GenerateCode(&ast_buffer, extra_space, rule_no);
                    GenerateCode(&ast_buffer, "//#line $current_line $input_file$", rule_no);
                    GenerateCode(&ast_buffer, extra_space, rule_no);
                }
            }
        }
    }

    GenerateCode(&ast_buffer, rparen, rule_no);
    GenerateCode(&ast_buffer, space, rule_no);
    GenerateCode(&ast_buffer, "//#line $current_line $input_file$", rule_no);
    GenerateCode(&ast_buffer, space, rule_no);
    GenerateCode(&ast_buffer, trailer, rule_no);

    delete [] extra_space;

    return;
}

//
//
//
void CppAction2::GenerateListAllocation(CTC &ctc,
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
        GenerateCode(&ast_buffer, "setResult(", rule_no);
        GenerateCode(&ast_buffer, space, rule_no);
        GenerateCode(&ast_buffer, space4, rule_no);
        GenerateCode(&ast_buffer, "//#line $current_line $input_file$", rule_no);
        GenerateCode(&ast_buffer, space, rule_no);
        GenerateCode(&ast_buffer, space4, rule_no);

        ast_buffer.Put(" _automatic_ast_pool << ");
        GenerateCode(&ast_buffer, newkey, rule_no);
        GenerateCode(&ast_buffer, allocation_element.name, rule_no);
        GenerateCode(&ast_buffer, lparen, rule_no);
        if (allocation_element.needs_environment)
        {
        
            GenerateCode(&ast_buffer, "this, ", rule_no);
        }
        if (allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_EMPTY ||
            allocation_element.list_kind == RuleAllocationElement::RIGHT_RECURSIVE_EMPTY)
        {
            GenerateCode(&ast_buffer, "getLeftIToken()", rule_no);
            GenerateCode(&ast_buffer, ", ", rule_no);
            GenerateCode(&ast_buffer, "getRightIToken()", rule_no);
            GenerateCode(&ast_buffer, comma, rule_no);
            if (allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_EMPTY)
                 GenerateCode(&ast_buffer, " true /* left recursive */", rule_no);
            else GenerateCode(&ast_buffer, " false /* not left recursive */", rule_no);
        }
        else
        {
            assert(allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_SINGLETON ||
                   allocation_element.list_kind == RuleAllocationElement::RIGHT_RECURSIVE_SINGLETON);

            if (grammar -> IsTerminal(allocation_element.element_symbol))
            {
                ast_buffer.Put(" _automatic_ast_pool << ");
                GenerateCode(&ast_buffer, newkey, rule_no);
                GenerateCode(&ast_buffer, grammar -> Get_ast_token_classname(), rule_no);
                GenerateCode(&ast_buffer, lparen, rule_no);
                GenerateCode(&ast_buffer, "getRhsIToken(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&ast_buffer, index.String(), rule_no);
                GenerateCode(&ast_buffer, rparen, rule_no);
            }
            else
            {
                GenerateCode(&ast_buffer, lparen, rule_no);
              //  auto temp_type = ctc.FindBestTypeFor(allocation_element.element_type_symbol_index);
                auto temp_type = ctc.FindUniqueTypeFor(allocation_element.element_type_symbol_index, option->ast_type);

                GenerateCode(&ast_buffer, temp_type, rule_no);
                GenerateCode(&ast_buffer, "*)", rule_no);
                GenerateCode(&ast_buffer, "getRhsSym(", rule_no);
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
        GenerateCode(&ast_buffer, "//#line $current_line $input_file$", rule_no);
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
            GenerateCode(&ast_buffer, lparen, rule_no);
            GenerateCode(&ast_buffer, allocation_element.name, rule_no);
            GenerateCode(&ast_buffer, "*)", rule_no);
            GenerateCode(&ast_buffer, "getRhsSym(", rule_no);
            IntToString index(allocation_element.list_position);
            GenerateCode(&ast_buffer, index.String(), rule_no);
            GenerateCode(&ast_buffer, "))->addElement(", rule_no);
            if (grammar -> IsTerminal(allocation_element.element_symbol))
            {
                ast_buffer.Put(" _automatic_ast_pool << ");
                GenerateCode(&ast_buffer, newkey, rule_no);          
                GenerateCode(&ast_buffer, grammar -> Get_ast_token_classname(), rule_no);
                GenerateCode(&ast_buffer, lparen, rule_no);
                GenerateCode(&ast_buffer, "getRhsIToken(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&ast_buffer, index.String(), rule_no);
                GenerateCode(&ast_buffer, rparen, rule_no);
            }
            else
            {
                GenerateCode(&ast_buffer, lparen, rule_no);
                //auto temp_type = ctc.FindBestTypeFor(allocation_element.element_type_symbol_index);
                auto temp_type = ctc.FindUniqueTypeFor(allocation_element.element_type_symbol_index, option->ast_type);

                GenerateCode(&ast_buffer, temp_type, rule_no);
            
                GenerateCode(&ast_buffer, " *)getRhsSym(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&ast_buffer, index.String(), rule_no);
            }

            if (allocation_element.list_position != 1) // a right-recursive rule? set the list as result
            {
                GenerateCode(&ast_buffer, rparen, rule_no);
                GenerateCode(&ast_buffer, trailer, rule_no);

                GenerateCode(&ast_buffer, space, rule_no);
                GenerateCode(&ast_buffer, "setResult(", rule_no);
                GenerateCode(&ast_buffer, "getRhsSym(", rule_no);
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
            GenerateCode(&ast_buffer, "setResult(", rule_no);
            GenerateCode(&ast_buffer, lparen, rule_no);
            GenerateCode(&ast_buffer, allocation_element.name, rule_no);
            GenerateCode(&ast_buffer, "*)", rule_no);
            GenerateCode(&ast_buffer, "getRhsSym(", rule_no);
            IntToString index(allocation_element.list_position);
            GenerateCode(&ast_buffer, index.String(), rule_no);
        }

        GenerateCode(&ast_buffer, rparen, rule_no);
    }

    GenerateCode(&ast_buffer, trailer, rule_no);
 
    return;
}
