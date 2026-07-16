#include "CTC.h"
#include "NTC.h"
#include "RustAction.h"

#include <map>
#include <string>

#include "LCA.h"
#include "TTC.h"
#include "VisitorStaffFactory.h"
TextBuffer* RustAction::GetBuffer(ActionFileSymbol* ast_filename_symbol) const
{
    if (option->IsTopLevel())
    {
        return  (ast_filename_symbol->BodyBuffer());
    }
	return (ast_filename_symbol->BufferForNestAst());
    
}

RustAction::RustAction(Control* control_, Blocks* action_blocks_, Grammar* grammar_, MacroLookupTable* macro_table_): Action(control_, action_blocks_, grammar_, macro_table_)
{


}

namespace 
{
	std::string templateany_cast_to_Interface(const char* interfaceName)
	{
        char temp[2048] = {};
        sprintf(temp,
            "pub fn any_cast_to_%s(i: Option<Box<dyn std::any::Any>>) -> Option<Rc<dyn %s>> {\n"
            "    i.and_then(|boxed| boxed.downcast::<Rc<dyn %s>>().ok().map(|a| (*a).clone()))\n"
            "}\n",
            interfaceName, interfaceName, interfaceName);
        return temp;
	}
    std::string templateany_cast_to_Struct(const char* structName)
    {
        char temp[2048] = {};
        sprintf(temp,
            "pub fn any_cast_to_%s(i: Option<Box<dyn std::any::Any>>) -> Option<Rc<%s>> {\n"
            "    i.and_then(|boxed| {\n"
            "        boxed.downcast::<Rc<%s>>().ok().map(|a| (*a).clone()).or_else(|| None)\n"
            "    })\n"
            "}\n",
            structName, structName, structName);
        return temp;
    }

    //
    // Emit `impl IAst for Class` that delegates every trait method to an embedded
    // `base` field (Rc<Ast>, Rc<AstToken>, or Rc<AbstractAstList>).
    // Avoid calling trait methods from trait methods (no recursion).
    //
    // all_children_mode:
    //   0 = ArrayList::new()          (terminals / token wrappers)
    //   1 = ClassName::get_all_children(self)  (rules with parent_saved inherent)
    //   2 = self.base.get_all_children()      (delegate to base)
    //
    // accept_mode:
    //   0 = empty body
    //   1 = ClassName::accept(self, v)  (preorder inherent)
    //
    void EmitIAstImplDelegatingToBase(TextBuffer& b,
                                      const char* classname,
                                      int all_children_mode,
                                      int accept_mode)
    {
        b + "impl IAst for " + classname + " {\n";
        b.Put("    fn get_next_ast(&self) -> Option<Rc<dyn IAst>> { self.base.get_next_ast() }\n");
        b.Put("    fn set_parent(&self, parent: Option<Rc<dyn IAst>>) { self.base.set_parent(parent) }\n");
        b.Put("    fn get_parent(&self) -> Option<Rc<dyn IAst>> { self.base.get_parent() }\n");
        b.Put("    fn get_left_i_token(&self) -> Rc<dyn IToken> { self.base.get_left_i_token() }\n");
        b.Put("    fn get_right_i_token(&self) -> Rc<dyn IToken> { self.base.get_right_i_token() }\n");
        b.Put("    fn get_preceding_adjuncts(&self) -> Vec<Rc<dyn IToken>> { self.base.get_preceding_adjuncts() }\n");
        b.Put("    fn get_following_adjuncts(&self) -> Vec<Rc<dyn IToken>> { self.base.get_following_adjuncts() }\n");
        b.Put("    fn get_children(&self) -> ArrayList { self.base.get_children() }\n");
        if (all_children_mode == 1)
        {
            b + "    fn get_all_children(&self) -> ArrayList { " + classname + "::get_all_children(self) }\n";
        }
        else if (all_children_mode == 2)
        {
            b.Put("    fn get_all_children(&self) -> ArrayList { self.base.get_all_children() }\n");
        }
        else
        {
            b.Put("    fn get_all_children(&self) -> ArrayList { ArrayList::new() }\n");
        }
        if (accept_mode == 1)
        {
            b + "    fn accept(&self, v: &mut dyn IAstVisitor) { " + classname + "::accept(self, v) }\n";
        }
        else
        {
            b.Put("    fn accept(&self, _v: &mut dyn IAstVisitor) {}\n");
        }
        b + "    fn as_any(&self) -> &dyn std::any::Any { self }\n";
        b.Put("}\n\n");
    }

    // Emit `impl IAst for Ast` using UFCS / field access (no trait-method recursion).
    void EmitIAstImplForAst(TextBuffer& b, const char* classname, bool parent_saved, bool glr, bool preorder)
    {
        b + "impl IAst for " + classname + " {\n";
        if (glr)
        {
            b + "    fn get_next_ast(&self) -> Option<Rc<dyn IAst>> { " + classname + "::get_next_ast(self) }\n";
        }
        else
        {
            b.Put("    fn get_next_ast(&self) -> Option<Rc<dyn IAst>> { None }\n");
        }
        if (parent_saved)
        {
            b + "    fn set_parent(&self, parent: Option<Rc<dyn IAst>>) { " + classname + "::set_parent(self, parent) }\n";
            b + "    fn get_parent(&self) -> Option<Rc<dyn IAst>> { " + classname + "::get_parent(self) }\n";
            b + "    fn get_children(&self) -> ArrayList { " + classname + "::get_children(self) }\n";
            b.Put("    fn get_all_children(&self) -> ArrayList { ArrayList::new() }\n");
        }
        else
        {
            b.Put("    fn set_parent(&self, _parent: Option<Rc<dyn IAst>>) {}\n");
            b.Put("    fn get_parent(&self) -> Option<Rc<dyn IAst>> { None }\n");
            b + "    fn get_children(&self) -> ArrayList { " + classname + "::get_children(self) }\n";
            b + "    fn get_all_children(&self) -> ArrayList { " + classname + "::get_all_children(self) }\n";
        }
        b + "    fn get_left_i_token(&self) -> Rc<dyn IToken> { " + classname + "::get_left_i_token(self) }\n";
        b + "    fn get_right_i_token(&self) -> Rc<dyn IToken> { " + classname + "::get_right_i_token(self) }\n";
        b + "    fn get_preceding_adjuncts(&self) -> Vec<Rc<dyn IToken>> { " + classname + "::get_preceding_adjuncts(self) }\n";
        b + "    fn get_following_adjuncts(&self) -> Vec<Rc<dyn IToken>> { " + classname + "::get_following_adjuncts(self) }\n";
        if (preorder)
        {
            b + "    fn accept(&self, v: &mut dyn IAstVisitor) { " + classname + "::accept(self, v) }\n";
        }
        else
        {
            b.Put("    fn accept(&self, _v: &mut dyn IAstVisitor) {}\n");
        }
        b + "    fn as_any(&self) -> &dyn std::any::Any { self }\n";
        b.Put("}\n\n");
    }
}
void RustAction::ProcessCodeActionEnd()
{

}

//
//
//
void RustAction::ExpandExportMacro(TextBuffer *buffer, SimpleMacroSymbol *simple_macro)
{
    buffer -> Put("crate::");
    if (*option -> exp_type != '\0')
    {
        buffer -> Put(option -> exp_type);
        buffer -> Put("::");
    }
    buffer -> Put(option -> exp_prefix);
    buffer -> Put(simple_macro -> Name() + 2); // skip initial escape and '_' characters
    buffer -> Put(option -> exp_suffix);
}


//
//
//
void RustAction::GenerateDefaultTitle(Tuple<ActionBlockElement> &notice_actions)
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
    // Issue the module comment
    //
    TextBuffer *buffer = (option -> DefaultBlock() -> Buffer()
                              ? option -> DefaultBlock() -> Buffer()
                              : option -> DefaultBlock() -> ActionfileSymbol() -> InitialHeadersBuffer());
   
    if (*option->package != '\0')
    {
        buffer->Put("// mod ");
        buffer->Put(option->package);
        buffer->Put("\n\n");
    }

    return;
}


//
// First construct a file for my type. Write the title information to its header
// buffer and return the file.
//
ActionFileSymbol *RustAction::GenerateTitle(ActionFileLookupTable &ast_filename_table,
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
    buffer -> Put("// Generated by LPG2 ");
    buffer -> Put(Control::VERSION);
    buffer -> Put("; runtime ABI: lpg-rust-runtime-v1\n\n");
    if (*option->ast_package != '\0')
    {
        buffer->Put("// mod ");
        buffer->Put(option->ast_package);
        buffer->Put("\n\n");
    }


    delete [] filename;

    return file_symbol;
}


ActionFileSymbol *RustAction::GenerateTitleAndGlobals(ActionFileLookupTable &ast_filename_table,
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
void RustAction::GenerateEnvironmentDeclaration(TextBuffer &b, const char *, const char* def_prefix)
{
    b.Put(def_prefix);
    b + "pub fn get_environment(&self) -> Rc<" + option->action_type + "> { self.environment.clone() }\n\n";
}


void RustAction::ProcessAstActions(Tuple<ActionBlockElement>& actions,
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
                    for (int l = 0; l < actions.Length(); l++)
                        actions[l].buffer = GetBuffer((option->IsNested()
                                                       ? default_file_symbol
                                                       : top_level_file_symbol));

                    rule_allocation_map[rule_no].needs_environment = true;
                    ProcessCodeActions(actions, typestring, processed_rule_map);
                }
                GetBuffer((option->IsNested()
                           ? default_file_symbol
                           : top_level_file_symbol))->Put("\n\n");// Generate Class Closer;
                if (!option->IsNested()){
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

                for (int j = 0; j < actions.Length(); j++)
                    actions[j].buffer = GetBuffer((option->IsNested()
                                                   ? default_file_symbol
                                                   : top_level_file_symbol));

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
                    for (int j = 0; j < actions.Length(); j++)
                        actions[j].buffer =  GetBuffer((option->IsNested()
                                                        ? default_file_symbol
                                                        : top_level_file_symbol));
                    ProcessCodeActions(actions, typestring, processed_rule_map);
                }
            }
            GetBuffer((option->IsNested()
                       ? default_file_symbol
                       : top_level_file_symbol))->Put("    \n\n");// Generate Class Closer
            if (!option->IsNested()){
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
    visitorFactory->GenerateVisitor(this, ast_filename_table, default_file_symbol, notice_actions, type_set);

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
void RustAction::GenerateVisitorHeaders(TextBuffer &b, const char *indentation, const char *modifiers, const char* def_prefix)
{
    (void)modifiers;
    if (option -> visitor != Option::NONE)
    {
        //
        // When `def_prefix` is set we are generating default method bodies inside an
        // `impl` block; otherwise we are generating trait method signatures.
        //
        std::string header = indentation;
        header += "    ";

        if (option -> visitor & Option::PREORDER)
        {
            b.Put(header.c_str());
            b.Put("fn accept(&self, v: &mut dyn IAstVisitor)");
            b.Put(def_prefix ? " {}" : ";");
            b.Put("\n");
        }
        if (option -> visitor & Option::DEFAULT)
        {
            b.Put(header.c_str());
            b.Put("fn accept_with_visitor(&self, v: &mut dyn ");
            b.Put(visitorFactory->visitor_type);
            b.Put(")");
            b.Put(def_prefix ? " {}" : ";");
            b.Put("\n");

            b.Put(header.c_str());
            b.Put("fn accept_with_arg(&self, v: &mut dyn ");
            b.Put(visitorFactory->argument_visitor_type);
            b.Put(", o: Option<Box<dyn std::any::Any>>)");
            b.Put(def_prefix ? " {}" : ";");
            b.Put("\n");

            b.Put(header.c_str());
            b.Put("fn accept_with_result(&self, v: &mut dyn ");
            b.Put(visitorFactory->result_visitor_type);
            b.Put(") -> Option<Box<dyn std::any::Any>>");
            b.Put(def_prefix ? " { None }" : ";");
            b.Put("\n");

            b.Put(header.c_str());
            b.Put("fn accept_with_result_argument(&self, v: &mut dyn ");
            b.Put(visitorFactory->result_argument_visitor_type);
            b.Put(", o: Option<Box<dyn std::any::Any>>) -> Option<Box<dyn std::any::Any>>");
            b.Put(def_prefix ? " { None }" : ";");
            b.Put("\n");
        }
        b.Put("\n");
    }

    return;
}


//
//
//
void RustAction::GenerateVisitorMethods(NTC &,
                                        TextBuffer &b,
                                        const char *,
                                        ClassnameElement &element,
                                        BitSet &, const char* def_prefix)
{
    if (option -> visitor & Option::DEFAULT)
    {
        b.Put("\n");
        b.Put(def_prefix); b + "pub fn accept_with_visitor(&self, v: &mut dyn " + visitorFactory->visitor_type + ") { v.Visit" + element.real_name + "(self); }\n";

        b.Put(def_prefix); b + "pub fn accept_with_arg(&self, v: &mut dyn " + visitorFactory->argument_visitor_type + ", o: Option<Box<dyn std::any::Any>>) { v.Visit" + element.real_name + "WithArg(self, o); }\n";

        b.Put(def_prefix); b + "pub fn accept_with_result(&self, v: &mut dyn " + visitorFactory->result_visitor_type + ") -> Option<Box<dyn std::any::Any>> { v.Visit" + element.real_name + "WithResult(self) }\n";

        b.Put(def_prefix); b + "pub fn accept_with_result_argument(&self, v: &mut dyn " + visitorFactory->result_argument_visitor_type + ", o: Option<Box<dyn std::any::Any>>) -> Option<Box<dyn std::any::Any>> { v.Visit" + element.real_name + "WithResultArgument(self, o) }\n";
    }
    if (option -> visitor & Option::PREORDER)
    {
        b.Put("\n");
        b.Put(def_prefix); b.Put("pub fn accept(&self, v: &mut dyn IAstVisitor) {\n");
         b.Put("        if !v.pre_visit(self as &dyn IAst) { return; }\n");
         b.Put("        self.enter(v);\n");
         b.Put("        v.post_visit(self as &dyn IAst);\n");
         b.Put("    }\n\n");

        b.Put(def_prefix); b + "pub fn enter(&self, v: &mut dyn " + visitorFactory->preorder_visitor_type + ") {\n";

        SymbolLookupTable &symbol_set = element.symbol_set;
        if (element.is_terminal_class || symbol_set.Size() == 0)
        {
             b + "        v.Visit" + element.real_name + "(self);\n";
        }
        else
        {
             b + "        let check_children = v.Visit" + element.real_name + "(self);\n";
             b.Put("        if check_children {\n");

            for (int i = 0; i < symbol_set.Size(); i++)
            {
                b + "            if let Some(child) = &self._" + symbol_set[i]->Name() + " { child.accept(v); }\n";
            }

	 b.Put("        }\n");
        }
         b + "        v.EndVisit" + element.real_name + "(self);\n";
         b.Put("    }\n");
    }

    return;
}


//
//
//
void RustAction::GenerateGetAllChildrenMethod(TextBuffer &b,
                                              const char *,
                                              ClassnameElement &element, const char* def_prefix)
{
    if (! element.is_terminal_class)
    {
        SymbolLookupTable &symbol_set = element.symbol_set;

        b.Put("\n");
         b.Put("    // A list of all children of this node, including the null ones.\n");
        b.Put(def_prefix); b.Put("pub fn get_all_children(&self) -> ArrayList {\n");
         b.Put("        let mut list = ArrayList::new();\n");
        for (int i = 0; i < symbol_set.Size(); i++)
        {
	b+"        if let Some(child) = &self._"+symbol_set[i]->Name()+" { list.add(box_ast(child.clone() as Rc<dyn IAst>)); }\n";

        }
         b.Put("        list\n");
         b.Put("    }\n");
    }

    return;
}


//
//
//
void RustAction::GenerateSimpleVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                const char *,
                                                const char *interface_name,
                                                SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);

     b+"pub trait "+ interface_name + " {\n";
                              
    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
	b+"    fn Visit"+symbol->Name()+"(&mut self, n: &"+symbol -> Name()+");\n";
    }

	b.Put("\n");
	b+"    fn Visit(&mut self, n: Rc<dyn IAst>);\n";

	b.Put("}\n");
	b + templateany_cast_to_Interface(interface_name).c_str();
}

//
//
//
void RustAction::GenerateArgumentVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                  const char *,
                                                  const char *interface_name,
                                                  SymbolLookupTable &type_set)
{
   
    TextBuffer& b =*GetBuffer(ast_filename_symbol);

     b + "pub trait " + interface_name + " {\n";

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
	b+"    fn Visit"+symbol->Name()+"WithArg(&mut self, n: &"+symbol -> Name()+", o: Option<Box<dyn std::any::Any>>);\n";
    }

	 b.Put("\n");

     b.Put("    fn VisitWithArg(&mut self, n: Rc<dyn IAst>, o: Option<Box<dyn std::any::Any>>);\n");

     b.Put("}\n");
     b + templateany_cast_to_Interface(interface_name).c_str();
}

//
//
//
void RustAction::GenerateResultVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                const char *,
                                                const char *interface_name,
                                                SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
     b + "pub trait " + interface_name + " {\n";

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
	b+"    fn Visit"+symbol->Name()+"WithResult(&mut self, n: &"+symbol -> Name()+") -> Option<Box<dyn std::any::Any>>;\n";
    }

	b.Put("\n");
	b.Put("    fn VisitWithResult(&mut self, n: Rc<dyn IAst>) -> Option<Box<dyn std::any::Any>>;\n");

	b.Put("}\n");
    b + templateany_cast_to_Interface(interface_name).c_str();
}

//
//
//
void RustAction::GenerateResultArgumentVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                        const char *,
                                                        const char *interface_name,
                                                        SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
     b + "pub trait " + interface_name + " {\n";

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b+"    fn Visit"+symbol->Name()+"WithResultArgument(&mut self, n: &"+symbol -> Name()+", o: Option<Box<dyn std::any::Any>>) -> Option<Box<dyn std::any::Any>>;\n";
    }

	b.Put("\n");
	b.Put("    fn VisitWithResultArgument(&mut self, n: Rc<dyn IAst>, o: Option<Box<dyn std::any::Any>>) -> Option<Box<dyn std::any::Any>>;\n");

	b.Put("}\n");
    b + templateany_cast_to_Interface(interface_name).c_str();
}


//
//
//
void RustAction::GeneratePreorderVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                  const char *,
                                                  const char *interface_name,
                                                  SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    assert(option -> visitor & Option::PREORDER);
     b + "pub trait " + interface_name + ": IAstVisitor {\n";

     b.Put("    fn Visit(&mut self, n: Rc<dyn IAst>) -> bool;\n");
     b.Put("    fn EndVisit(&mut self, n: Rc<dyn IAst>);\n\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];

	b+"    fn Visit"+symbol->Name()+"(&mut self, n: &"+symbol -> Name()+") -> bool;\n";

	b+"    fn EndVisit"+symbol->Name()+"(&mut self, n: &"+symbol -> Name()+");\n";

        b.Put("\n");
    }

     b.Put("}\n\n");
     b + templateany_cast_to_Interface(interface_name).c_str();
    return;
}


//
//
//
void RustAction::GenerateNoResultVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                      const char *,
                                                      const char *classname,
                                                      SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);

    std::string plus_interface= visitorFactory->visitor_type;
    plus_interface += visitorFactory->argument_visitor_type;

     b + "pub trait " + plus_interface.c_str() + ": " + visitorFactory->visitor_type + " + " + visitorFactory->argument_visitor_type + " {}\n\n";

     b + "pub struct " + classname + ";\n\n";

     b + "impl " + classname + " {\n";
     b + "    pub fn new() -> " + classname + " { " + classname + " }\n";
     b + "    pub fn unimplemented_visitor(&self, _s: &str) {}\n";
     b.Put("}\n\n");

     b + "impl " + visitorFactory->visitor_type + " for " + classname + " {\n";
    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b + "    fn Visit" + symbol->Name() + "(&mut self, _n: &" + symbol->Name()
          + ") { self.unimplemented_visitor(\"Visit" + symbol->Name() + "(" + symbol->Name() + ")\"); }\n";
    }
     b.Put("    fn Visit(&mut self, n: Rc<dyn IAst>) {\n");
    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b + "        if let Some(node) = downcast_ast::<" + symbol->Name() + ">(n.clone()) {\n";
        b + "            self.Visit" + symbol->Name() + "(&node);\n";
        b.Put("            return;\n");
        b.Put("        }\n");
    }
     b.Put("    }\n");
     b.Put("}\n\n");

     b + "impl " + visitorFactory->argument_visitor_type + " for " + classname + " {\n";
    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b + "    fn Visit" + symbol->Name() + "WithArg(&mut self, _n: &" + symbol->Name()
          + ", _o: Option<Box<dyn std::any::Any>>) { self.unimplemented_visitor(\"Visit"
          + symbol->Name() + "WithArg(" + symbol->Name() + ")\"); }\n";
    }
     b.Put("    fn VisitWithArg(&mut self, n: Rc<dyn IAst>, o: Option<Box<dyn std::any::Any>>) {\n");
    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b + "        if let Some(node) = downcast_ast::<" + symbol->Name() + ">(n.clone()) {\n";
        b + "            self.Visit" + symbol->Name() + "WithArg(&node, o);\n";
        b.Put("            return;\n");
        b.Put("        }\n");
    }
     b.Put("    }\n");
     b.Put("}\n\n");

     b + "impl " + plus_interface.c_str() + " for " + classname + " {}\n\n";

    b + templateany_cast_to_Struct(classname).c_str();
}

//
//
//
void RustAction::GenerateResultVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                    const char *,
                                                    const char *classname,
                                                    SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    std::string plus_interface;

	plus_interface += visitorFactory->result_visitor_type;
    plus_interface += visitorFactory->result_argument_visitor_type;

     b + "pub trait " + plus_interface.c_str() + ": " + visitorFactory->result_visitor_type + " + " + visitorFactory->result_argument_visitor_type + " {}\n\n";

     b + "pub struct " + classname + ";\n\n";

     b + "impl " + classname + " {\n";
     b + "    pub fn new() -> " + classname + " { " + classname + " }\n";
     b + "    pub fn unimplemented_visitor(&self, _s: &str) -> Option<Box<dyn std::any::Any>> { None }\n";
     b.Put("}\n\n");

     b + "impl " + visitorFactory->result_visitor_type + " for " + classname + " {\n";
    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b + "    fn Visit" + symbol->Name() + "WithResult(&mut self, _n: &" + symbol->Name()
          + ") -> Option<Box<dyn std::any::Any>> { self.unimplemented_visitor(\"Visit"
          + symbol->Name() + "WithResult(" + symbol->Name() + ")\") }\n";
    }
     b.Put("    fn VisitWithResult(&mut self, n: Rc<dyn IAst>) -> Option<Box<dyn std::any::Any>> {\n");
    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b + "        if let Some(node) = downcast_ast::<" + symbol->Name() + ">(n.clone()) {\n";
        b + "            return self.Visit" + symbol->Name() + "WithResult(&node);\n";
        b.Put("        }\n");
    }
     b.Put("        None\n");
     b.Put("    }\n");
     b.Put("}\n\n");

     b + "impl " + visitorFactory->result_argument_visitor_type + " for " + classname + " {\n";
    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b + "    fn Visit" + symbol->Name() + "WithResultArgument(&mut self, _n: &" + symbol->Name()
          + ", _o: Option<Box<dyn std::any::Any>>) -> Option<Box<dyn std::any::Any>> { self.unimplemented_visitor(\"Visit"
          + symbol->Name() + "WithResultArgument(" + symbol->Name() + ")\") }\n";
    }
     b.Put("    fn VisitWithResultArgument(&mut self, n: Rc<dyn IAst>, o: Option<Box<dyn std::any::Any>>) -> Option<Box<dyn std::any::Any>> {\n");
    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b + "        if let Some(node) = downcast_ast::<" + symbol->Name() + ">(n.clone()) {\n";
        b + "            return self.Visit" + symbol->Name() + "WithResultArgument(&node, o);\n";
        b.Put("        }\n");
    }
     b.Put("        None\n");
     b.Put("    }\n");
     b.Put("}\n\n");

     b + "impl " + plus_interface.c_str() + " for " + classname + " {}\n\n";

    b + templateany_cast_to_Struct(classname).c_str();
}


//
//
//
void RustAction::GeneratePreorderVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                      const char */*indentation*/,
                                                      const char *classname,
                                                      SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    assert(option -> visitor & Option::PREORDER);

	  b + "pub struct " + classname + ";\n\n";

      b + "impl " + classname + " {\n";
      b + "    pub fn new() -> " + classname + " { " + classname + " }\n";
      b + "    pub fn unimplemented_visitor(&self, _s: &str) -> bool { true }\n";
      b.Put("}\n\n");

      b.Put("impl IAstVisitor for ");
      b.Put(classname);
      b.Put(" {\n");
      b.Put("    fn pre_visit(&mut self, _element: &dyn IAst) -> bool { true }\n");
      b.Put("    fn post_visit(&mut self, _element: &dyn IAst) {}\n");
      b.Put("}\n\n");

      b + "impl " + visitorFactory->preorder_visitor_type + " for " + classname + " {\n";
    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b + "    fn Visit" + symbol->Name() + "(&mut self, _n: &" + symbol->Name()
          + ") -> bool { self.unimplemented_visitor(\"Visit(" + symbol->Name() + ")\") }\n";
        b + "    fn EndVisit" + symbol->Name() + "(&mut self, _n: &" + symbol->Name()
          + ") { let _ = self.unimplemented_visitor(\"EndVisit(" + symbol->Name() + ")\"); }\n";
    }
     b.Put("    fn Visit(&mut self, n: Rc<dyn IAst>) -> bool {\n");
    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b + "        if let Some(node) = downcast_ast::<" + symbol->Name() + ">(n.clone()) {\n";
        b + "            return self.Visit" + symbol->Name() + "(&node);\n";
        b.Put("        }\n");
    }
     b.Put("        false\n");
     b.Put("    }\n");
     b.Put("    fn EndVisit(&mut self, n: Rc<dyn IAst>) {\n");
    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b + "        if let Some(node) = downcast_ast::<" + symbol->Name() + ">(n.clone()) {\n";
        b + "            self.EndVisit" + symbol->Name() + "(&node);\n";
        b.Put("            return;\n");
        b.Put("        }\n");
    }
     b.Put("    }\n");
     b.Put("}\n\n");

     b + templateany_cast_to_Struct(classname).c_str();
    return;
}


//
// Generate the the Ast root classes
//
void RustAction::GenerateAstType(ActionFileSymbol* ast_filename_symbol,
                                 const char *,
                                 const char *classname)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    /*
     * First, generate the main root class
     */

	b+"pub struct " + classname + " {\n";
     b.Put("    left_i_token: std::cell::RefCell<Rc<dyn IToken>>,\n");
     b.Put("    right_i_token: std::cell::RefCell<Rc<dyn IToken>>,\n");
    if (option->glr)
    {
	      b.Put("    next_ast: std::cell::RefCell<Option<Rc<dyn IAst>>>,\n");
    }
    if (option->parent_saved)
    {
	       b.Put("    parent: std::cell::RefCell<Option<Rc<dyn IAst>>>,\n");
    }
    b.Put("}\n\n");

    //
    // The def_prefix is now simply the indentation used inside the `impl` block.
    //
    const char* def_prefix = "    ";

    b + "impl " + classname + " {\n";

     b.Put("    pub fn new2(left_i_token: Rc<dyn IToken>, right_i_token: Rc<dyn IToken>) -> Rc<").Put(classname).Put("> {\n");
     b.Put("        Rc::new(").Put(classname).Put(" {\n");
     b.Put("            left_i_token: std::cell::RefCell::new(left_i_token),\n");
     b.Put("            right_i_token: std::cell::RefCell::new(right_i_token),\n");
    if (option->glr)
    {
         b.Put("            next_ast: std::cell::RefCell::new(None),\n");
    }
    if (option->parent_saved)
    {
         b.Put("            parent: std::cell::RefCell::new(None),\n");
    }
     b.Put("        })\n");
     b.Put("    }\n\n");

     b.Put("    pub fn new(token: Rc<dyn IToken>) -> Rc<").Put(classname).Put("> {\n");
     b.Put("        ").Put(classname).Put("::new2(token.clone(), token)\n");
     b.Put("    }\n\n");

    if (option -> glr)
    {
         b.Put(def_prefix); b.Put("pub fn get_next_ast(&self) -> Option<Rc<dyn IAst>> { self.next_ast.borrow().clone() }\n");
         b.Put(def_prefix); b.Put("pub fn set_next_ast(&self, n: Option<Rc<dyn IAst>>) { *self.next_ast.borrow_mut() = n; }\n");
         b.Put(def_prefix); b.Put("pub fn reset_next_ast(&self) { *self.next_ast.borrow_mut() = None; }\n");
    }
    else
    {
	     b.Put(def_prefix); b.Put("pub fn get_next_ast(&self) -> Option<Rc<dyn IAst>> { None }\n");
    }


    if (option -> parent_saved)
    {
         b.Put(def_prefix); b.Put("pub fn set_parent(&self, parent: Option<Rc<dyn IAst>>) { *self.parent.borrow_mut() = parent; }\n");
         b.Put(def_prefix); b.Put("pub fn get_parent(&self) -> Option<Rc<dyn IAst>> { self.parent.borrow().clone() }\n");
    }
    else
    {
         b.Put(def_prefix); b.Put("pub fn set_parent(&self, _parent: Option<Rc<dyn IAst>>) {}\n");
         b.Put(def_prefix); b.Put("pub fn get_parent(&self) -> Option<Rc<dyn IAst>> { None }\n");
    }

    b.Put("\n");
     b.Put(def_prefix); b.Put("pub fn get_left_i_token(&self) -> Rc<dyn IToken> { self.left_i_token.borrow().clone() }\n");
     b.Put(def_prefix); b.Put("pub fn get_right_i_token(&self) -> Rc<dyn IToken> { self.right_i_token.borrow().clone() }\n");
     b.Put(def_prefix); b.Put("pub fn set_left_i_token(&self, token: Rc<dyn IToken>) { *self.left_i_token.borrow_mut() = token; }\n");
     b.Put(def_prefix); b.Put("pub fn set_right_i_token(&self, token: Rc<dyn IToken>) { *self.right_i_token.borrow_mut() = token; }\n");
     b.Put(def_prefix); b.Put("pub fn get_preceding_adjuncts(&self) -> Vec<Rc<dyn IToken>> { self.get_left_i_token().get_preceding_adjuncts() }\n");
     b.Put(def_prefix); b.Put("pub fn get_following_adjuncts(&self) -> Vec<Rc<dyn IToken>> { self.get_right_i_token().get_following_adjuncts() }\n\n");

     b.Put(def_prefix); b.Put("pub fn to_string(&self) -> String {\n");
     b.Put("        let left = self.get_left_i_token();\n");
     b.Put("        let right = self.get_right_i_token();\n");
     b.Put("        match left.get_i_lex_stream() {\n");
     b.Put("            Some(stream) => stream.borrow().to_string_range(left.get_start_offset(), right.get_end_offset()),\n");
     b.Put("            None => String::new(),\n");
     b.Put("        }\n");
     b.Put("    }\n\n");


     b.Put(def_prefix); b.Put("pub fn initialize(&self) {}\n");
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
         b.Put("    // A list of all children of this node, excluding the null ones.\n");
         b.Put(def_prefix);  b.Put("pub fn get_children(&self) -> ArrayList {\n");
         b.Put("        let list = self.get_all_children();\n");
         b.Put("        let mut result = ArrayList::new();\n");
         b.Put("        let mut i = 0;\n");
         b.Put("        while i < list.size() {\n");
         b.Put("            if let Some(child) = list.get(i).and_then(|x| unbox_ast(x)) {\n");
         b.Put("                result.add(box_ast(child));\n");
         b.Put("            }\n");
         b.Put("            i += 1;\n");
         b.Put("        }\n");
         b.Put("        result\n");
         b.Put("    }\n\n");

         b.Put("    // A list of all children of this node, including the null ones.\n");
         b.Put(def_prefix); b.Put("pub fn get_all_children(&self) -> ArrayList { ArrayList::new() }\n");
    }
    else
    {
         b.Put(def_prefix); b.Put("pub fn get_children(&self) -> ArrayList {\n");
         b.Put("        panic!(\"no parent-saved option in effect\")\n");
         b.Put("    }\n");
         b.Put(def_prefix); b.Put("pub fn get_all_children(&self) -> ArrayList { self.get_children() }\n");
    }

    b.Put("\n");

    GenerateVisitorHeaders(b, "", "", def_prefix);


    b+ def_prefix + "pub fn get_rule_index(&self) -> i32 { 0 }\n";
    //
    // Not Preorder visitor? generate dummy accept method.
    //
    if (! (option -> visitor & Option::PREORDER))
    {
         b.Put(def_prefix); b.Put("pub fn accept(&self, _v: &mut dyn IAstVisitor) {}\n");
    }
     b.Put("}\n\n");

     EmitIAstImplForAst(b, classname, option->parent_saved, option->glr,
                        (option->visitor & Option::PREORDER) != 0);

     b + templateany_cast_to_Struct(classname).c_str();
    return;
}



typedef std::map<std::string, std::string> Substitutions;



//
// Generate the the Ast list class
//
void RustAction::GenerateAbstractAstListType(ActionFileSymbol* ast_filename_symbol,
                                             const char *,
                                             const char *classname)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    /*
     * Generate the List root class
     *///IAbstractArrayList
     
	b+"pub struct "+ abstract_ast_list_classname + " {\n";
    b + "    base: Rc<" + option->ast_type + ">,\n";
    b + "    left_recursive: std::cell::Cell<bool>,\n";
    b + "    list: std::cell::RefCell<ArrayList>,\n}\n\n";

    const char* def_prefix = "    ";

    b + "impl " + abstract_ast_list_classname + " {\n";

    // generate constructor for list class
     b + "    pub fn new(left_token: Rc<dyn IToken>, right_token: Rc<dyn IToken>, left_recursive: bool) -> Rc<" + abstract_ast_list_classname + "> {\n";
     b + "        Rc::new(" + abstract_ast_list_classname + " {\n";
     b + "            base: " + option->ast_type + "::new2(left_token, right_token),\n";
     b + "            list: std::cell::RefCell::new(ArrayList::new()),\n";
     b + "            left_recursive: std::cell::Cell::new(left_recursive),\n";
     b + "        })\n";
     b + "    }\n\n";

     b.Put(def_prefix); b.Put("pub fn get_next_ast(&self) -> Option<Rc<dyn IAst>> { self.base.get_next_ast() }\n");
     b.Put(def_prefix); b.Put("pub fn set_parent(&self, parent: Option<Rc<dyn IAst>>) { self.base.set_parent(parent) }\n");
     b.Put(def_prefix); b.Put("pub fn get_parent(&self) -> Option<Rc<dyn IAst>> { self.base.get_parent() }\n");
     b.Put(def_prefix); b.Put("pub fn get_left_i_token(&self) -> Rc<dyn IToken> { self.base.get_left_i_token() }\n");
     b.Put(def_prefix); b.Put("pub fn get_right_i_token(&self) -> Rc<dyn IToken> { self.base.get_right_i_token() }\n");
     b.Put(def_prefix); b.Put("pub fn get_preceding_adjuncts(&self) -> Vec<Rc<dyn IToken>> { self.base.get_preceding_adjuncts() }\n");
     b.Put(def_prefix); b.Put("pub fn get_following_adjuncts(&self) -> Vec<Rc<dyn IToken>> { self.base.get_following_adjuncts() }\n");
     b.Put(def_prefix); b.Put("pub fn get_children(&self) -> ArrayList { self.base.get_children() }\n");
     b.Put(def_prefix); b.Put("pub fn size(&self) -> usize { self.list.borrow().size() }\n");
     b.Put(def_prefix); b.Put("pub fn get_list(&self) -> std::cell::Ref<'_, ArrayList> { self.list.borrow() }\n");

     b.Put(def_prefix); b.Put("pub fn get_element_at(&self, i: usize) -> Option<Rc<dyn IAst>> {\n");
     b.Put("        let list = self.list.borrow();\n");
     b.Put("        let k = if self.left_recursive.get() { i } else { list.size() - 1 - i };\n");
     b.Put("        list.get(k).and_then(|x| unbox_ast(x))\n");
     b.Put("    }\n");

     b.Put(def_prefix); b.Put("pub fn get_array_list(&self) -> ArrayList {\n");
     b.Put("        if !self.left_recursive.get() {\n");
     b.Put("            self.list.borrow_mut().reverse();\n");
     b.Put("            self.left_recursive.set(true);\n");
     b.Put("        }\n");
     b.Put("        self.list.borrow().clone_list()\n");
     b.Put("    }\n");

     b.Put(def_prefix); b.Put("pub fn add(&self, element: Rc<dyn IAst>) -> bool {\n");
     b.Put("        self.add_element(element);\n");
     b.Put("        true\n");
     b.Put("    }\n\n");

     b.Put(def_prefix);
	 b.Put("pub fn add_element(&self, element: Rc<dyn IAst>) {\n");
     b.Put("        self.list.borrow_mut().add(box_ast(element.clone()));\n");
     b.Put("        if self.left_recursive.get() {\n");
     b.Put("            self.base.set_right_i_token(element.get_right_i_token());\n");
     b.Put("        } else {\n");
     b.Put("            self.base.set_left_i_token(element.get_left_i_token());\n");
     b.Put("        }\n");
     b.Put("    }\n\n");


    if (option -> parent_saved)
    {
         b.Put("    // Make a copy of the list and return it in proper order.\n");
         b.Put(def_prefix); b.Put("pub fn get_all_children(&self) -> ArrayList {\n");
         b.Put("        self.get_array_list()\n");
         b.Put("    }\n\n");
    }
    else
    {
         b.Put(def_prefix); b.Put("pub fn get_all_children(&self) -> ArrayList { self.get_array_list() }\n\n");
    }

    b.Put("}\n\n");

    EmitIAstImplDelegatingToBase(b, abstract_ast_list_classname, 1, 0);

    Substitutions subs;
    subs["%%AstType%%"] = option->ast_type;
    subs["%%ListClassName%%"] = classname;

     b + templateany_cast_to_Struct(abstract_ast_list_classname).c_str();

    return;
}


//
// Generate the the Ast token class
//
void RustAction::GenerateAstTokenType(NTC &ntc, ActionFileSymbol* ast_filename_symbol,
                                      const char *indentation,
                                      const char *classname)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);

    const char* def_prefix = "    ";
    /*
     * Generate the Token root class
     */
     b + "pub struct " + classname + " {\n";
     b + "    base: Rc<" + option->ast_type + ">,\n";
     b + "}\n\n";

     b + "impl " + classname + " {\n";
     b + "    pub fn new(token: Rc<dyn IToken>) -> Rc<" + classname + "> {\n";
     b + "        Rc::new(" + classname + " { base: " + option->ast_type + "::new(token) })\n";
     b + "    }\n\n";

    b.Put(def_prefix); b.Put("pub fn initialize(&self) { self.base.initialize(); }\n");
    b.Put(def_prefix); b.Put("pub fn get_next_ast(&self) -> Option<Rc<dyn IAst>> { self.base.get_next_ast() }\n");
    b.Put(def_prefix); b.Put("pub fn set_parent(&self, parent: Option<Rc<dyn IAst>>) { self.base.set_parent(parent) }\n");
    b.Put(def_prefix); b.Put("pub fn get_parent(&self) -> Option<Rc<dyn IAst>> { self.base.get_parent() }\n");
    b.Put(def_prefix); b.Put("pub fn get_left_i_token(&self) -> Rc<dyn IToken> { self.base.get_left_i_token() }\n");
    b.Put(def_prefix); b.Put("pub fn get_right_i_token(&self) -> Rc<dyn IToken> { self.base.get_right_i_token() }\n");
    b.Put(def_prefix); b.Put("pub fn get_preceding_adjuncts(&self) -> Vec<Rc<dyn IToken>> { self.base.get_preceding_adjuncts() }\n");
    b.Put(def_prefix); b.Put("pub fn get_following_adjuncts(&self) -> Vec<Rc<dyn IToken>> { self.base.get_following_adjuncts() }\n");
    b.Put(def_prefix); b.Put("pub fn get_children(&self) -> ArrayList { self.base.get_children() }\n");
    b.Put(def_prefix); b.Put("pub fn get_i_token(&self) -> Rc<dyn IToken> { self.base.get_left_i_token() }\n");
    b.Put(def_prefix); b.Put("pub fn to_string(&self) -> String { self.base.get_left_i_token().to_string() }\n\n");

    ClassnameElement element; // generate a temporary element with no symbols in its symbol set.
    element.real_name = (char *) classname;
    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);

     b.Put("    // A token class has no children, so we return the empty list.\n");
    b.Put(def_prefix); b.Put("pub fn get_all_children(&self) -> ArrayList { ArrayList::new() }\n\n");

    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set, def_prefix);

    if (! (option -> visitor & Option::PREORDER))
    {
         b.Put(def_prefix); b.Put("pub fn accept(&self, _v: &mut dyn IAstVisitor) {}\n");
    }

     b.Put("}\n\n");

     EmitIAstImplDelegatingToBase(b, classname, 0,
                                  (option->visitor & Option::PREORDER) ? 1 : 0);

     b + templateany_cast_to_Struct(classname).c_str();
    return;
}


//
//
//
void RustAction::GenerateCommentHeader(TextBuffer &b,
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


void RustAction::GenerateListMethods(CTC &ctc,
                                     NTC &,
                                     TextBuffer &b,
                                     const char *,
                                     const char *classname,
                                     ClassnameElement &element,
    const char* super_prefix, const char* def_prefix)
{
    const char *element_name = element.array_element_type_symbol -> Name();

    //
    // Generate size/add methods that delegate to the embedded abstract list.
    //
    b.Put(def_prefix);
    b + "pub fn size(&self) -> usize { " + super_prefix + "size() }\n";

    b.Put(def_prefix);
	b + "pub fn add_element(self: &Rc<Self>, _" + element_name + ": Rc<dyn IAst>) {\n";
    b + "        " + super_prefix + "add_element(_" + element_name + ".clone());\n";
    if (option -> parent_saved)
    {
         b.Put("        _");
         b.Put(element_name);
         b.Put(".set_parent(Some(self.clone() as Rc<dyn IAst>));\n");
    }
    b.Put("    }\n");

    b.Put("\n");
   
    //
    // Generate visitor methods.
    //
    if (option -> visitor & Option::DEFAULT)
    {
        b.Put("\n");
        
	b.Put(def_prefix);
	b+"pub fn accept_with_visitor(&self, v: &mut dyn "+visitorFactory->visitor_type+") {\n";
        b + "        let mut i = 0;\n";
        b + "        while i < self.size() {\n";
        b + "            if let Some(element) = self.get_" + element_name + "_at(i) {\n";
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol->SymbolIndex()) != NULL)
        {
            b + "                v.Visit(element);\n";
        }
        else {
            b + "                element.accept_with_visitor(v);\n";
        }
        b + "            }\n";
        b + "            i += 1;\n";
        b + "        }\n";
        b.Put("    }\n");

        b.Put(def_prefix);
	b+"pub fn accept_with_arg(&self, v: &mut dyn "+visitorFactory-> argument_visitor_type + ", o: Option<Box<dyn std::any::Any>>) {\n";
        b + "        let mut i = 0;\n";
        b + "        while i < self.size() {\n";
        b + "            if let Some(element) = self.get_" + element_name + "_at(i) {\n";
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol->SymbolIndex()) != NULL)
        {
            b + "                v.VisitWithArg(element, o.as_ref().map(|_| unimplemented!()));\n";
        }
        else {
            b + "                element.accept_with_arg(v, None);\n";
        }
        b + "            }\n";
        b + "            i += 1;\n";
        b + "        }\n";
        b.Put("    }\n");

        //
        // Code cannot be generated to automatically visit a node that
        // can return a value. These cases are left up to the user.
        //
        b.Put(def_prefix);
	b+"pub fn accept_with_result(&self, v: &mut dyn "+visitorFactory-> result_visitor_type + ") -> Option<Box<dyn std::any::Any>> {\n";
        b + "        let mut result = ArrayList::new();\n";
        b + "        let mut i = 0;\n";
        b + "        while i < self.size() {\n";
        b + "            if let Some(element) = self.get_" + element_name + "_at(i) {\n";
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol->SymbolIndex()) != NULL)
        {
            b + "                if let Some(r) = v.VisitWithResult(element) { result.add(r); }\n";
        }
        else {
            b + "                if let Some(r) = element.accept_with_result(v) { result.add(r); }\n";
        }
        b + "            }\n";
        b + "            i += 1;\n";
        b + "        }\n";
	b.Put("        Some(Box::new(result))\n");
         b.Put("    }\n");


        b.Put(def_prefix);
	b+"pub fn accept_with_result_argument(&self, v: &mut dyn "+visitorFactory-> result_argument_visitor_type + ", o: Option<Box<dyn std::any::Any>>) -> Option<Box<dyn std::any::Any>> {\n";
        b + "        let mut result = ArrayList::new();\n";
        b + "        let mut i = 0;\n";
        b + "        while i < self.size() {\n";
        b + "            if let Some(element) = self.get_" + element_name + "_at(i) {\n";
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol->SymbolIndex()) != NULL)
        {
            b + "                if let Some(r) = v.VisitWithResultArgument(element, None) { result.add(r); }\n";
        }
        else {
            b + "                if let Some(r) = element.accept_with_result_argument(v, None) { result.add(r); }\n";
        }
        b + "            }\n";
        b + "            i += 1;\n";
        b + "        }\n";
	b.Put("        Some(Box::new(result))\n");
         b.Put("    }\n");

    }
    if (option -> visitor & Option::PREORDER)
    {
        b.Put("\n");
        b.Put(def_prefix); b.Put("pub fn accept(&self, v: &mut dyn IAstVisitor) {\n");
         b.Put("        if !v.pre_visit(self as &dyn IAst) { return; }\n");
         b.Put("        self.enter(v);\n");
         b.Put("        v.post_visit(self as &dyn IAst);\n");
         b.Put("    }\n");

        b.Put(def_prefix); b + "pub fn enter(&self, v: &mut dyn " + visitorFactory->preorder_visitor_type + ") {\n";
	b.Put("        let check_children = v.Visit").Put(classname).Put("(self);\n");
	b.Put("        if check_children {\n");
	b.Put("            let mut i = 0;\n");
	b.Put("            while i < self.size() {\n");
	        b + "                if let Some(element) = self.get_" + element_name + "_at(i) {\n";
	        const char *element_typename = ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex());
	        if (element_typename != NULL)
	        {
	             b.Put("                    if v.pre_visit(&*element) {\n");
	             b.Put("                        element.enter(v);\n");
	             b.Put("                        v.post_visit(&*element);\n");
	             b.Put("                    }\n");
	        }
	        else
	        {
	             b.Put("                    element.accept(v);\n");
	        }
		b+"                }\n";
	b.Put("                i += 1;\n");
	b.Put("            }\n");
	b.Put("        }\n");
	b.Put("        v.EndVisit").Put(classname).Put("(self);\n");
	b.Put("    }\n");
    }

    return;
}


//
//
//
void RustAction::GenerateListClass(CTC &ctc,
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
    std::string type_name = "Rc<";
    if(isInterface){
        type_name += "dyn ";
    }
    type_name  += ctc.FindBestTypeFor(element.array_element_type_symbol->SymbolIndex());
    type_name += ">";
    const auto element_type = type_name.c_str();

    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);

    const char* def_prefix = "    ";
     b + "pub struct " + classname + " {\n";
     b + "    base: Rc<" + abstract_ast_list_classname + ">,\n}\n\n";

     b + "impl " + classname + " {\n";

    if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
    {
         b.Put("    // The value returned by get_");
         b.Put(element_name);
         b.Put("_at may be None.\n");
    }
    b.Put(def_prefix);
	b+ "pub fn get_"+element_name+"_at(&self, i: usize) -> Option<Rc<dyn IAst>> {\n";
    b + "        self.base.get_element_at(i)\n";
     b + "    }\n\n";


    //
    // generate constructors
    //
	b + "    pub fn new(left_token: Rc<dyn IToken>, right_token: Rc<dyn IToken>, left_recursive: bool) -> Rc<" + classname + "> {\n";
	 b + "        Rc::new(" + classname + " { base: " + abstract_ast_list_classname + "::new(left_token, right_token, left_recursive) })\n";
	 b + "    }\n\n";


     b+"    pub fn from_element(element: "+ element_type+", left_recursive: bool) -> Rc<"+ classname+"> {\n";
    b + "        let obj = " + classname + "::new(element.get_left_i_token(), element.get_right_i_token(), left_recursive);\n";
     b + "        obj.add_element(element as Rc<dyn IAst>);\n";
     b.Put("        obj\n");
     b.Put("    }\n\n");

    b.Put(def_prefix); b.Put("pub fn get_next_ast(&self) -> Option<Rc<dyn IAst>> { self.base.get_next_ast() }\n");
    b.Put(def_prefix); b.Put("pub fn set_parent(&self, parent: Option<Rc<dyn IAst>>) { self.base.set_parent(parent) }\n");
    b.Put(def_prefix); b.Put("pub fn get_parent(&self) -> Option<Rc<dyn IAst>> { self.base.get_parent() }\n");
    b.Put(def_prefix); b.Put("pub fn get_left_i_token(&self) -> Rc<dyn IToken> { self.base.get_left_i_token() }\n");
    b.Put(def_prefix); b.Put("pub fn get_right_i_token(&self) -> Rc<dyn IToken> { self.base.get_right_i_token() }\n");
    b.Put(def_prefix); b.Put("pub fn get_preceding_adjuncts(&self) -> Vec<Rc<dyn IToken>> { self.base.get_preceding_adjuncts() }\n");
    b.Put(def_prefix); b.Put("pub fn get_following_adjuncts(&self) -> Vec<Rc<dyn IToken>> { self.base.get_following_adjuncts() }\n");
    b.Put(def_prefix); b.Put("pub fn get_children(&self) -> ArrayList { self.base.get_children() }\n");
    b.Put(def_prefix); b.Put("pub fn get_all_children(&self) -> ArrayList { self.base.get_all_children() }\n");

    std::string super_prefix = "self.base.";
    GenerateListMethods(ctc, ntc, b, indentation, classname, element, super_prefix.c_str(),def_prefix);

    b.Put(def_prefix);
    IntToString num(element.GetRuleNo());
    b+ "pub fn get_rule_index(&self) -> i32 { " + num.String() + " }\n";

    if (! (option -> visitor & Option::PREORDER))
    {
         b.Put(def_prefix); b.Put("pub fn accept(&self, _v: &mut dyn IAstVisitor) {}\n");
    }

    b.Put("}\n\n");// Generate Class Closer

    EmitIAstImplDelegatingToBase(b, classname, 2,
                                 (option->visitor & Option::PREORDER) ? 1 : 0);

    b + templateany_cast_to_Struct(classname).c_str();

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
void RustAction::GenerateListExtensionClass(CTC& ctc,
    NTC&,
    ActionFileSymbol* ast_filename_symbol,
    const char* indentation,
    SpecialArrayElement& special_array,
    ClassnameElement& element,
    Array<const char*>&)

{
    TextBuffer& b = *GetBuffer(ast_filename_symbol);

    const char* classname = element.real_name;
    bool isInterface = ctc.IsInterface(element.array_element_type_symbol->SymbolIndex());
    std::string type_name = "Rc<";
    if (isInterface)
        type_name += "dyn ";
    type_name += ctc.FindBestTypeFor(element.array_element_type_symbol->SymbolIndex());
    type_name += ">";
    const auto element_type = type_name.c_str();

    GenerateCommentHeader(b, indentation, element.ungenerated_rule, special_array.rules);

    const char* def_prefix = "    ";

     b + "pub struct " + special_array.name + " {\n";
     b + "    base: Rc<" + classname + ">,\n";
     b + "    environment: Rc<" + option->action_type + ">,\n}\n\n";

     b + "impl " + special_array.name + " {\n";

    GenerateEnvironmentDeclaration(b, indentation, def_prefix);

	b +  "    pub fn new(environment: Rc<" +option->action_type + ">, left_i_token: Rc<dyn IToken>, right_i_token: Rc<dyn IToken>, left_recursive: bool) -> Rc<"+ special_array.name+"> {\n";
     b + "        let node = Rc::new(" + special_array.name + " { base: " + classname + "::new(left_i_token, right_i_token, left_recursive), environment });\n";
     b + "        node.base.base.initialize();\n";
     b + "        node\n";
     b + "    }\n\n";

    b + "    pub fn from_element(environment: Rc<" + option->action_type +
        ">, element: " + element_type + ", left_recursive: bool) -> Rc<" + special_array.name + "> {\n";
	b+"        let obj = " + special_array.name + "::new(environment, element.get_left_i_token(), element.get_right_i_token(), left_recursive);\n";
     b + "        obj.base.add_element(element as Rc<dyn IAst>);\n";
     b.Put("        obj\n");
     b.Put("    }\n\n");

    b.Put(def_prefix); b.Put("pub fn get_next_ast(&self) -> Option<Rc<dyn IAst>> { self.base.get_next_ast() }\n");
    b.Put(def_prefix); b.Put("pub fn set_parent(&self, parent: Option<Rc<dyn IAst>>) { self.base.set_parent(parent) }\n");
    b.Put(def_prefix); b.Put("pub fn get_parent(&self) -> Option<Rc<dyn IAst>> { self.base.get_parent() }\n");
    b.Put(def_prefix); b.Put("pub fn get_left_i_token(&self) -> Rc<dyn IToken> { self.base.get_left_i_token() }\n");
    b.Put(def_prefix); b.Put("pub fn get_right_i_token(&self) -> Rc<dyn IToken> { self.base.get_right_i_token() }\n");
    b.Put(def_prefix); b.Put("pub fn get_preceding_adjuncts(&self) -> Vec<Rc<dyn IToken>> { self.base.get_preceding_adjuncts() }\n");
    b.Put(def_prefix); b.Put("pub fn get_following_adjuncts(&self) -> Vec<Rc<dyn IToken>> { self.base.get_following_adjuncts() }\n");
    b.Put(def_prefix); b.Put("pub fn get_children(&self) -> ArrayList { self.base.get_children() }\n");
    b.Put(def_prefix); b.Put("pub fn get_all_children(&self) -> ArrayList { self.base.get_all_children() }\n");

    b.Put(def_prefix);
    IntToString num(element.GetRuleNo());
    b+ "pub fn get_rule_index(&self) -> i32 { " + num.String() + " }\n";

    if (! (option -> visitor & Option::PREORDER))
    {
         b.Put(def_prefix); b.Put("pub fn accept(&self, _v: &mut dyn IAstVisitor) {}\n");
    }

    b.Put("}\n\n");

    EmitIAstImplDelegatingToBase(b, special_array.name, 2,
                                 (option->visitor & Option::PREORDER) ? 1 : 0);

    b + templateany_cast_to_Struct(special_array.name).c_str();

}


//
// Generate a generic rule class
//
void RustAction::GenerateRuleClass(CTC &ctc,
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

    const char* def_prefix = "    ";

    if (element.is_terminal_class)
    {
	     b + "pub struct " + classname + " {\n";
         b + "    base: Rc<" + grammar->Get_ast_token_classname() + ">,\n";
        if (element.needs_environment){
            b + "    environment: Rc<" + option->action_type + ">,\n";
        }
        b.Put("}\n\n");

        b + "impl " + classname + " {\n";

        if (element.needs_environment) {
            GenerateEnvironmentDeclaration(b, indentation, def_prefix);
        }
        if (symbol_set.Size() == 1) // if the right-hand side contains a symbol ...
        {
            b.Put(def_prefix); b.Put("pub fn get_");
                                         b.Put(symbol_set[0] -> Name());
                                         b.Put("(&self) -> Rc<dyn IToken> { self.base.get_left_i_token() }\n\n");
        }

        if (element.needs_environment)
        {
            b + "    pub fn new(environment: Rc<" + option->action_type + ">, token: Rc<dyn IToken>) -> Rc<" + classname + "> {\n";
            b + "        let node = Rc::new(" + classname + " { base: " + grammar->Get_ast_token_classname() + "::new(token), environment });\n";
        }
        else
        {
            b + "    pub fn new(token: Rc<dyn IToken>) -> Rc<" + classname + "> {\n";
            b + "        let node = Rc::new(" + classname + " { base: " + grammar->Get_ast_token_classname() + "::new(token) });\n";
        }
         b + "        node.base.initialize();\n";
         b + "        node\n";
         b + "    }\n\n";

         b.Put(def_prefix); b.Put("pub fn get_left_i_token(&self) -> Rc<dyn IToken> { self.base.get_left_i_token() }\n");
         b.Put(def_prefix); b.Put("pub fn get_right_i_token(&self) -> Rc<dyn IToken> { self.base.get_right_i_token() }\n");
    }
    else 
    {
	     b + "pub struct " + classname + " {\n";
         b + "    base: Rc<" + option->ast_type + ">,\n";
        if (element.needs_environment) {
            b + "    environment: Rc<" + option->action_type + ">,\n";
        }

        for (int i = 0; i < symbol_set.Size(); i++)
        {
             b+"    _"+symbol_set[i] -> Name()+": Option<Rc<";
             if(ctc.IsInterface(rhs_type_index[i]))
             {
                 b + "dyn ";
             }
	 b+ctc.FindBestTypeFor(rhs_type_index[i])+">>,\n";

        }
    
        b.Put("}\n\n");

        b + "impl " + classname + " {\n";

        for (int i = 0; i < symbol_set.Size(); i++)
        {
            const char* symbolName = symbol_set[i]->Name();
            const char* bestType = ctc.FindBestTypeFor(rhs_type_index[i]);
            auto is_interface = ctc.IsInterface(rhs_type_index[i]);
            if (ntc.CanProduceNullAst(rhs_type_index[i]))
            {
	b.Put("    // The value returned by get_");
                b.Put(symbolName);
                b.Put(" may be None.\n");
            }

            // Generate getter method
            b.Put(def_prefix); b.Put("pub fn get_");
            b.Put(symbolName);
            b.Put("(&self) -> Option<Rc<");
            if(is_interface){
                b.Put("dyn ");
            }
            b.Put(bestType);
            b.Put(">> { self._");
            b.Put(symbolName);
            b.Put(".clone() }\n");

            // Generate setter method
            b.Put(def_prefix); b.Put("pub fn set_");
            b.Put(symbolName);
            b.Put("(&mut self, _");
            b.Put(symbolName);
            b.Put(": Option<Rc<");
            if (is_interface) {
                b.Put("dyn ");
            }
            b.Put(bestType);
            b.Put(">>) { self._");
            b.Put(symbolName);
            b.Put(" = _");
            b.Put(symbolName);
            b.Put("; }\n");
        }
        b.Put("\n");
        

        if (element.needs_environment)
        {
            GenerateEnvironmentDeclaration(b, indentation, def_prefix);
        }
        //
        // generate constructor
        //
        b + "    pub fn new(";

        if (element.needs_environment)
        {
            b.Put("environment: Rc<");
            b.Put(option -> action_type);
            b.Put(">, ");
        }
        b.Put("left_i_token: Rc<dyn IToken>, right_i_token: Rc<dyn IToken>");
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            b.Put(", _");
            b.Put(symbol_set[i] -> Name());
            b.Put(": Option<Rc<");
            if(ctc.IsInterface(rhs_type_index[i])){
                b.Put("dyn ");
            }
            b.Put(ctc.FindBestTypeFor(rhs_type_index[i]));
            b.Put(">>");
        }
        b.Put(") -> Rc<").Put(classname).Put("> {\n");

         b + "        let node = Rc::new(" + classname + " {\n";
         b + "            base: " + option->ast_type + "::new2(left_i_token, right_i_token),\n";
        if (element.needs_environment)
        {
            b.Put("            environment,\n");
        }
        for (int i = 0; i < symbol_set.Size(); i++)
        {
             b.Put("            _");
             b.Put(symbol_set[i] -> Name());
             b.Put(",\n");
        }
         b.Put("        });\n");
        if (option -> parent_saved)
        {
            for (int i = 0; i < symbol_set.Size(); i++)
            {
                b.Put("        if let Some(ref child) = _");
                b.Put(symbol_set[i] -> Name());
                b.Put(" {\n");
                b.Put("            child.set_parent(Some(node.clone() as Rc<dyn IAst>));\n");
                b.Put("        }\n");
            }
        }
         b.Put("        node.base.initialize();\n");
         b.Put("        node\n");
         b.Put("    }\n\n");

         b.Put(def_prefix); b.Put("pub fn get_left_i_token(&self) -> Rc<dyn IToken> { self.base.get_left_i_token() }\n");
         b.Put(def_prefix); b.Put("pub fn get_right_i_token(&self) -> Rc<dyn IToken> { self.base.get_right_i_token() }\n");
    }

    if (option -> parent_saved)
    {
	    GenerateGetAllChildrenMethod(b, indentation, element,def_prefix);
    }

	GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set,def_prefix);

    b.Put(def_prefix);
    IntToString num(element.GetRuleNo());
    b+ "pub fn get_rule_index(&self) -> i32 { " + num.String() + " }\n";

    if (! (option -> visitor & Option::PREORDER))
    {
         b.Put(def_prefix); b.Put("pub fn accept(&self, _v: &mut dyn IAstVisitor) {}\n");
    }

    b.Put("}\n\n");

    {
        int all_children_mode;
        if (element.is_terminal_class)
            all_children_mode = 0;
        else if (option->parent_saved)
            all_children_mode = 1;
        else
            all_children_mode = 2;
        EmitIAstImplDelegatingToBase(b, classname, all_children_mode,
                                     (option->visitor & Option::PREORDER) ? 1 : 0);
    }

    b + templateany_cast_to_Struct(classname).c_str();

    return;
}


//
// Generate Ast class
//
void RustAction::GenerateTerminalMergedClass(NTC &ntc,
                                             ActionFileSymbol* ast_filename_symbol,
                                             const char *indentation,
                                             ClassnameElement &element,
                                             Array<const char *> &)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    char *classname = element.real_name;
    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);

    const char* def_prefix = "    ";

    b + "pub struct " + classname + " {\n";
	b + "    base: Rc<" + grammar->Get_ast_token_classname() + ">,\n";
    if (element.needs_environment) {
        b + "    environment: Rc<" + option->action_type + ">,\n";
    }
    b.Put("}\n\n");

    b + "impl " + classname + " {\n";

    if (element.needs_environment){
	    GenerateEnvironmentDeclaration(b, indentation, def_prefix);
    }

    SymbolLookupTable &symbol_set = element.symbol_set;
    if (symbol_set.Size() == 1) // if the right-hand side contains a symbol ...
    {
        b.Put(def_prefix); b.Put("pub fn get_");
                                     b.Put(symbol_set[0] -> Name());
                                     b.Put("(&self) -> Rc<dyn IToken> { self.base.get_left_i_token() }\n\n");
    }

     if (element.needs_environment)
     {
         b + "    pub fn new(environment: Rc<" + option->action_type + ">, token: Rc<dyn IToken>) -> Rc<" + classname + "> {\n";
         b + "        let node = Rc::new(" + classname + " { base: " + grammar->Get_ast_token_classname() + "::new(token), environment });\n";
     }
     else
     {
         b + "    pub fn new(token: Rc<dyn IToken>) -> Rc<" + classname + "> {\n";
         b + "        let node = Rc::new(" + classname + " { base: " + grammar->Get_ast_token_classname() + "::new(token) });\n";
     }
      b + "        node.base.initialize();\n";
      b + "        node\n";
      b + "    }\n\n";

      b.Put(def_prefix); b.Put("pub fn get_left_i_token(&self) -> Rc<dyn IToken> { self.base.get_left_i_token() }\n");
      b.Put(def_prefix); b.Put("pub fn get_right_i_token(&self) -> Rc<dyn IToken> { self.base.get_right_i_token() }\n");

    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);
  
    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set, def_prefix);

    b.Put(def_prefix);
    IntToString num(element.GetRuleNo());
    b+ "pub fn get_rule_index(&self) -> i32 { " + num.String() + " }\n";

    if (! (option -> visitor & Option::PREORDER))
    {
         b.Put(def_prefix); b.Put("pub fn accept(&self, _v: &mut dyn IAstVisitor) {}\n");
    }

    b.Put("}\n\n");

    EmitIAstImplDelegatingToBase(b, classname, 0,
                                 (option->visitor & Option::PREORDER) ? 1 : 0);

    b + templateany_cast_to_Struct(classname).c_str();

}


//
// Generate Ast class
//
void RustAction::GenerateMergedClass(CTC &ctc,
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
    const char* def_prefix = "    ";
     b + "pub struct " + classname + " {\n";
     b + "    base: Rc<" + option->ast_type + ">,\n";
    if (element.needs_environment) {
        b + "    environment: Rc<" + option->action_type + ">,\n";
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
         b + "    _" + symbol_set[i]->Name() + ": Option<Rc<";
         if (ctc.IsInterface(rhs_type_index[i]))
         {
             b + "dyn ";
         }
         b + ctc.FindBestTypeFor(rhs_type_index[i]) + ">>,\n";
    }

    b.Put("}\n\n");

    b + "impl " + classname + " {\n";

    for (int i = 0; i < symbol_set.Size(); i++)
    {
        const char* symbolName = symbol_set[i]->Name();
        const char* bestType = ctc.FindBestTypeFor(rhs_type_index[i]);
        auto is_interface = ctc.IsInterface(rhs_type_index[i]);
        if ((! optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
        {
             b.Put("    // The value returned by get_");
             b.Put(symbolName);
             b.Put(" may be None.\n");
        }

        // Generate getter method
        b.Put(def_prefix); b.Put("pub fn get_");
        b.Put(symbolName);
        b.Put("(&self) -> Option<Rc<");
        if (is_interface) {
            b.Put("dyn ");
        }
        b.Put(bestType);
        b.Put(">> { self._");
        b.Put(symbolName);
        b.Put(".clone() }\n");
    }

    b.Put("\n");
    if (element.needs_environment) {
        GenerateEnvironmentDeclaration(b, indentation, def_prefix);
    }

    //
    // generate merged constructor
    //
     b + "    pub fn new(";

    if (element.needs_environment)
    {
        b.Put("environment: Rc<");
        b.Put(option->action_type);
        b.Put(">, ");
    }
    b.Put("left_i_token: Rc<dyn IToken>, right_i_token: Rc<dyn IToken>");

    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            b.Put(", _");
            b.Put(symbol_set[i] -> Name());
            b.Put(": Option<Rc<");
            if (ctc.IsInterface(rhs_type_index[i])) {
                b.Put("dyn ");
            }
            b.Put(ctc.FindBestTypeFor(rhs_type_index[i]));
            b.Put(">>");
        }
    }
    b.Put(") -> Rc<").Put(classname).Put("> {\n");

     b + "        let node = Rc::new(" + classname + " {\n";
     b + "            base: " + option->ast_type + "::new2(left_i_token, right_i_token),\n";
    if (element.needs_environment)
    {
        b.Put("            environment,\n");
    }

    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
             b.Put("            _");
             b.Put(symbol_set[i] -> Name());
             b.Put(",\n");
        }
    }
     b.Put("        });\n");
    if (option -> parent_saved)
    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            b.Put("        if let Some(ref child) = _");
            b.Put(symbol_set[i] -> Name());
            b.Put(" {\n");
            b.Put("            child.set_parent(Some(node.clone() as Rc<dyn IAst>));\n");
            b.Put("        }\n");
        }
    }
     b.Put("        node.base.initialize();\n");
     b.Put("        node\n");
     b.Put("    }\n\n");

     b.Put(def_prefix); b.Put("pub fn get_left_i_token(&self) -> Rc<dyn IToken> { self.base.get_left_i_token() }\n");
     b.Put(def_prefix); b.Put("pub fn get_right_i_token(&self) -> Rc<dyn IToken> { self.base.get_right_i_token() }\n");

    if (option -> parent_saved)
        GenerateGetAllChildrenMethod(b, indentation, element,def_prefix);
  
    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set,def_prefix);

    b.Put(def_prefix);
    IntToString num(element.GetRuleNo());
    b+ "pub fn get_rule_index(&self) -> i32 { " + num.String() + " }\n";

    if (! (option -> visitor & Option::PREORDER))
    {
         b.Put(def_prefix); b.Put("pub fn accept(&self, _v: &mut dyn IAstVisitor) {}\n");
    }

    b.Put("}\n\n");

    EmitIAstImplDelegatingToBase(b, classname, option->parent_saved ? 1 : 2,
                                 (option->visitor & Option::PREORDER) ? 1 : 0);

    b + templateany_cast_to_Struct(classname).c_str();

    return;
}

void RustAction::GenerateAstRootInterface(
    ActionFileSymbol* ast_filename_symbol,
    const char*)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    
	b+"pub trait "+ astRootInterfaceName.c_str() + " {\n";
     b.Put("    fn get_left_i_token(&self) -> Rc<dyn IToken>;\n");
     b.Put("    fn get_right_i_token(&self) -> Rc<dyn IToken>;\n");

    GenerateVisitorHeaders(b, "", "", nullptr);
	b.Put("}\n\n");

    b + "pub fn " + castToAny.c_str() + "(i: Box<dyn std::any::Any>) -> Box<dyn std::any::Any> { i }";
    b.Put("\n\n");
    //castToAny
    b + templateany_cast_to_Interface(astRootInterfaceName.c_str()).c_str();
    return;
}
void RustAction::GenerateInterface(bool is_terminal,
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
    b + "pub trait " + interface_name;
    if (extension.Length() > 0)
    {
        b.Put(": ");
        for (int k = 0; k < extension.Length(); k++)
        {
            if (k > 0)
                b.Put(" + ");
            b.PutChar('I');
            b.Put(extension[k] == grammar->Get_ast_token_interface()
                ? grammar->Get_ast_token_classname()
                : grammar->RetrieveString(extension[k]));
        }
        b.Put(" {}\n\n");
    }
    else
    {
        b.Put(": ").Put(astRootInterfaceName.c_str()).Put(" {}\n\n");
    }
    b + templateany_cast_to_Interface(interface_name).c_str();
}


//
//
//
void RustAction::GenerateNullAstAllocation(TextBuffer &b, int rule_no)
{
    const char *code = "\n                    self.set_result(None);";
    GenerateCode(&b, code, rule_no);

    return;
}



//
//
//
void RustAction::GenerateAstAllocation(CTC& ctc,
    NTC& ntc,
    TextBuffer& b,
    RuleAllocationElement& allocation_element,
    Tuple<ProcessedRuleElement>& processed_rule_elements,
    Array<const char*>&, int rule_no)
{
    (void)ntc;
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
        * lparen = "(",
        * comma = ",",
        * rparen = ")",
        // Closes ::new(, casts to Rc<dyn IAst>, closes box_ast( / Some( / set_result(.
        * trailer = " as Rc<dyn IAst>)));";
    int extra_space_length = strlen(space) + strlen(space4) + strlen(classname) + 8;
    char* extra_space = new char[extra_space_length + 1];
    extra_space[0] = '\n';
    {
        for (int i = 1; i < extra_space_length; i++)
            extra_space[i] = ' ';
    }
    extra_space[extra_space_length] = '\0';

    if (allocation_element.is_terminal_class && (grammar->RhsSize(rule_no) == 1 && grammar->IsNonTerminal(grammar->rhs_sym[grammar->FirstRhsIndex(rule_no)])))
    {
        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, "//", rule_no);
        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, "// When garbage collection is not available, delete ", rule_no);
        GenerateCode(&b, "self.get_rhs_sym(1)", rule_no);
        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, "//", rule_no);
    }
    GenerateCode(&b, space, rule_no);
    GenerateCode(&b, "self.set_result(Some(box_ast(", rule_no);
    GenerateCode(&b, space, rule_no);
    GenerateCode(&b, space4, rule_no);
    GenerateCode(&b, current_line_input_file_info.c_str(), rule_no);
    GenerateCode(&b, space, rule_no);
    GenerateCode(&b, space4, rule_no);

    //
    // AST nodes are built with the `ClassName::new(...)` associated function,
    // which returns an `Rc<ClassName>`.
    //
    GenerateCode(&b, classname, rule_no);
    GenerateCode(&b, "::new", rule_no);
    GenerateCode(&b, lparen, rule_no);
    if (allocation_element.needs_environment)
    {
        GenerateCode(&b, "self /* environment */, ", rule_no);
    }
    if (allocation_element.is_terminal_class)
    {
        GenerateCode(&b, "self.get_rhs_i_token(1).unwrap()", rule_no);
    }
    else
    {
        GenerateCode(&b, "self.get_left_i_token().unwrap()", rule_no);
        GenerateCode(&b, ", ", rule_no);
        GenerateCode(&b, "self.get_right_i_token().unwrap()", rule_no);
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
                    GenerateCode(&b, "None", rule_no);
                }
                else
                {
                    int symbol = grammar->rhs_sym[offset + position[i]];
                    const char* actual_type = ctc.FindBestTypeFor(type_index[i]);
                    IntToString index(position[i]);

                    if (grammar->IsTerminal(symbol))
                    {
                        //
                        // Terminals are wrapped in a fresh AstToken node inside Some(...).
                        //
                        GenerateCode(&b, "Some(", rule_no);
                        GenerateCode(&b, grammar->Get_ast_token_classname(), rule_no);
                        GenerateCode(&b, "::new(", rule_no);
                        GenerateCode(&b, "self.get_rhs_i_token(", rule_no);
                        GenerateCode(&b, index.String(), rule_no);
                        GenerateCode(&b, ").unwrap()", rule_no);
                        GenerateCode(&b, rparen, rule_no);
                        GenerateCode(&b, rparen, rule_no);
                    }
                    else
                    {
                        //
                        // Recover concrete Rc first; fall back to downcast_ast for
                        // box_ast(Rc<dyn IAst>) payloads.
                        //
                        GenerateCode(&b, "self.get_rhs_sym(", rule_no);
                        GenerateCode(&b, index.String(), rule_no);
                        GenerateCode(&b, ").and_then(|x| x.downcast_ref::<Rc<", rule_no);
                        if (ctc.IsInterface(type_index[i]))
                        {
                            GenerateCode(&b, "dyn ", rule_no);
                            GenerateCode(&b, actual_type, rule_no);
                            // Trait-object fields cannot be recovered from Rc<dyn IAst> alone.
                            GenerateCode(&b, ">>().cloned())", rule_no);
                        }
                        else
                        {
                            GenerateCode(&b, actual_type, rule_no);
                            GenerateCode(&b, ">>().cloned().or_else(|| unbox_ast(x).and_then(downcast_ast::<", rule_no);
                            GenerateCode(&b, actual_type, rule_no);
                            GenerateCode(&b, ">)))", rule_no);
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
void RustAction::GenerateListAllocation(CTC& ctc,
    NTC&,
    TextBuffer& b,
    int rule_no, RuleAllocationElement& allocation_element)
{
    const char* space = "\n                    ",
        * space4 = "    ",
        * lparen = "(",
        * comma = ",",
        * rparen = ")",
        // Closes ::new(/::from_element(, casts to Rc<dyn IAst>, closes box_ast/Some/set_result.
        * ctor_trailer = " as Rc<dyn IAst>)));";

    if (allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_EMPTY ||
        allocation_element.list_kind == RuleAllocationElement::RIGHT_RECURSIVE_EMPTY ||
        allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_SINGLETON ||
        allocation_element.list_kind == RuleAllocationElement::RIGHT_RECURSIVE_SINGLETON)
    {

        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, "self.set_result(Some(box_ast(", rule_no);
        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, space4, rule_no);
        GenerateCode(&b, current_line_input_file_info.c_str(), rule_no);
        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, space4, rule_no);


        if (allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_EMPTY ||
            allocation_element.list_kind == RuleAllocationElement::RIGHT_RECURSIVE_EMPTY)
        {
            GenerateCode(&b, allocation_element.name, rule_no);
            GenerateCode(&b, "::new", rule_no);
            GenerateCode(&b, lparen, rule_no);
            if (allocation_element.needs_environment)
            {
                GenerateCode(&b, "self /* environment */, ", rule_no);
            }

            GenerateCode(&b, "self.get_left_i_token().unwrap()", rule_no);
            GenerateCode(&b, ", ", rule_no);
            GenerateCode(&b, "self.get_right_i_token().unwrap()", rule_no);
            GenerateCode(&b, comma, rule_no);
            if (allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_EMPTY)
                GenerateCode(&b, " true /* left recursive */", rule_no);
            else GenerateCode(&b, " false /* not left recursive */", rule_no);
        }
        else
        {
            GenerateCode(&b, allocation_element.name, rule_no);
            GenerateCode(&b, "::from_element", rule_no);
            GenerateCode(&b, lparen, rule_no);
            if (allocation_element.needs_environment)
            {
                GenerateCode(&b, "self /* environment */, ", rule_no);
            }
            assert(allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_SINGLETON ||
                allocation_element.list_kind == RuleAllocationElement::RIGHT_RECURSIVE_SINGLETON);

            if (grammar->IsTerminal(allocation_element.element_symbol))
            {
                GenerateCode(&b, grammar->Get_ast_token_classname(), rule_no);
                GenerateCode(&b, "::new(", rule_no);
                GenerateCode(&b, "self.get_rhs_i_token(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&b, index.String(), rule_no);
                GenerateCode(&b, ").unwrap()", rule_no);
                GenerateCode(&b, rparen, rule_no);
            }
            else
            {
                GenerateCode(&b, "self.get_rhs_sym(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&b, index.String(), rule_no);
                GenerateCode(&b, ").and_then(|x| x.downcast_ref::<Rc<", rule_no);
                if (ctc.IsInterface(allocation_element.element_type_symbol_index))
                {
                    GenerateCode(&b, "dyn ", rule_no);
                    GenerateCode(&b, ctc.FindBestTypeFor(allocation_element.element_type_symbol_index), rule_no);
                    GenerateCode(&b, ">>().cloned())", rule_no);
                }
                else
                {
                    GenerateCode(&b, ctc.FindBestTypeFor(allocation_element.element_type_symbol_index), rule_no);
                    GenerateCode(&b, ">>().cloned().or_else(|| unbox_ast(x).and_then(downcast_ast::<", rule_no);
                    GenerateCode(&b, ctc.FindBestTypeFor(allocation_element.element_type_symbol_index), rule_no);
                    GenerateCode(&b, ">)))", rule_no);
                }
            }


            GenerateCode(&b, comma, rule_no);
            if (allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_SINGLETON)
                GenerateCode(&b, " true /* left recursive */", rule_no);
            else GenerateCode(&b, " false /* not left recursive */", rule_no);
        }

        GenerateCode(&b, rparen, rule_no);
        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, current_line_input_file_info.c_str(), rule_no);
        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, ctor_trailer, rule_no);

    }
    else
    {
        //
        // Add new element to list
        //
        if (allocation_element.list_kind == RuleAllocationElement::ADD_ELEMENT)
        {
            GenerateCode(&b, space, rule_no);

            GenerateCode(&b, "if let Some(_list) = self.get_rhs_sym(", rule_no);
            IntToString index(allocation_element.list_position);
            GenerateCode(&b, index.String(), rule_no);
            GenerateCode(&b, ").and_then(|x| x.downcast_ref::<Rc<", rule_no);
            GenerateCode(&b, allocation_element.name, rule_no);
            GenerateCode(&b, ">>().cloned().or_else(|| unbox_ast(x).and_then(downcast_ast::<", rule_no);
            GenerateCode(&b, allocation_element.name, rule_no);
            GenerateCode(&b, ">))) {", rule_no);
            GenerateCode(&b, space, rule_no);
            GenerateCode(&b, space4, rule_no);
            GenerateCode(&b, "_list.add_element(", rule_no);
            if (grammar->IsTerminal(allocation_element.element_symbol))
            {
                GenerateCode(&b, grammar->Get_ast_token_classname(), rule_no);
                GenerateCode(&b, "::new(", rule_no);
                GenerateCode(&b, "self.get_rhs_i_token(", rule_no);
                IntToString index2(allocation_element.element_position);
                GenerateCode(&b, index2.String(), rule_no);
                GenerateCode(&b, ").unwrap()", rule_no);
                GenerateCode(&b, rparen, rule_no);
                GenerateCode(&b, " as Rc<dyn IAst>", rule_no);
            }
            else
            {
                GenerateCode(&b, "self.get_rhs_sym(", rule_no);
                IntToString index2(allocation_element.element_position);
                GenerateCode(&b, index2.String(), rule_no);
                GenerateCode(&b, ").and_then(|x| unbox_ast(x)).expect(\"list element\")", rule_no);
            }
            GenerateCode(&b, ");", rule_no);
            GenerateCode(&b, space, rule_no);
            GenerateCode(&b, "}", rule_no);

            if (allocation_element.list_position != 1) // a right-recursive rule? set the list as result
            {
                GenerateCode(&b, space, rule_no);
                GenerateCode(&b, "self.set_result(self.get_rhs_sym(", rule_no);
                IntToString index3(allocation_element.list_position);
                GenerateCode(&b, index3.String(), rule_no);
                GenerateCode(&b, ").and_then(|x| x.downcast_ref::<Rc<", rule_no);
                GenerateCode(&b, allocation_element.name, rule_no);
                GenerateCode(&b, ">>().cloned().or_else(|| unbox_ast(x).and_then(downcast_ast::<", rule_no);
                GenerateCode(&b, allocation_element.name, rule_no);
                GenerateCode(&b, ">)).map(|a| box_ast(a as Rc<dyn IAst>)));", rule_no);
            }
        }

        //
        // Copy a list that is not the first element on the right-hand side of the rule
        //
        else
        {
            assert(allocation_element.list_kind == RuleAllocationElement::COPY_LIST);

            GenerateCode(&b, space, rule_no);
            GenerateCode(&b, "self.set_result(self.get_rhs_sym(", rule_no);
            IntToString index(allocation_element.list_position);
            GenerateCode(&b, index.String(), rule_no);
            GenerateCode(&b, ").and_then(|x| x.downcast_ref::<Rc<", rule_no);
            GenerateCode(&b, allocation_element.name, rule_no);
            GenerateCode(&b, ">>().map(|a| box_ast(a.clone() as Rc<dyn IAst>)));", rule_no);
        }


    }

    return;
}
