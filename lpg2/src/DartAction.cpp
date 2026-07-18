#include "CTC.h"
#include "NTC.h"
#include "DartAction.h"

#include <map>
#include <string>

#include "LCA.h"
#include "TTC.h"
#include "VisitorStaffFactory.h"

TextBuffer* DartAction::GetBuffer(ActionFileSymbol* fileSymbol) const
{
    if (!option->IsNested()){
        return  (fileSymbol->BodyBuffer());
    }else{
        return (fileSymbol->BufferForNestAst());
    }
}
void DartAction::ProcessCodeActionEnd()
{

}

//
// Shared ProcessAstActions hooks. Dart emits interfaces before the root types
// and orders the nonterminal interfaces with extensions last.
//
TextBuffer *DartAction::AstCodeBuffer(ActionFileSymbol *file)
{
    return GetBuffer(file);
}

void DartAction::EmitAstClassCloser(TextBuffer &code_buf, ActionFileSymbol * /*top_level_file*/, bool /*list_extension_closer*/)
{
    code_buf.Put("    }\n\n");
}

void DartAction::MaybeEmitAstRootInterface(ActionFileLookupTable &ast_filename_table,
                                           ActionFileSymbol *default_file_symbol,
                                           Tuple<ActionBlockElement> &notice_actions)
{
    astRootInterfaceName.append("IRootFor");
    astRootInterfaceName += option->action_type;
    if (option->IsNested())
        GenerateAstRootInterface(default_file_symbol, (char*)"    ");
    else
    {
        ActionFileSymbol* file_symbol = GenerateTitleAndGlobals(ast_filename_table, notice_actions,
            astRootInterfaceName.c_str(), false);
        GenerateAstRootInterface(file_symbol, (char*)"    ");
        file_symbol->Flush();
    }
}

//
//
//
void DartAction::ExpandExportMacro(TextBuffer *buffer, SimpleMacroSymbol *simple_macro)
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
void DartAction::GenerateDefaultTitle(Tuple<ActionBlockElement> &notice_actions)
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
   // TextBuffer *buffer = (option -> DefaultBlock() -> Buffer()
   //                           ? option -> DefaultBlock() -> Buffer()
    //                          : option -> DefaultBlock() -> ActionfileSymbol() -> InitialHeadersBuffer());
   


    return;
}


//
// First construct a file for this type. Write the title information to its header
// buffer and return the file.
//
ActionFileSymbol *DartAction::GenerateTitle(ActionFileLookupTable &ast_filename_table,
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


ActionFileSymbol *DartAction::GenerateTitleAndGlobals(ActionFileLookupTable &ast_filename_table,
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
void DartAction::GenerateEnvironmentDeclaration(TextBuffer &b, const char *indentation)
{
    b.Put(indentation); b.Put("     late ").Put(option -> action_type);
								 b.Put(" environment").Put(";\n");

    b.Put(indentation); b.Put("     ").Put(option->action_type);
                                
                                 b.Put(" getEnvironment() ");
                                 b.Put("{ return environment; }\n\n");
}




//
//
//
void DartAction::GenerateVisitorHeaders(TextBuffer &b, const char *indentation, const char *modifiers)
{
    if (option -> visitor != Option::NONE)
    {
        char *header = new char[strlen(indentation) + strlen(modifiers) + 9];
        strcpy(header, indentation);
        strcat(header, modifiers);

        if (option -> visitor & Option::DEFAULT)
        {
            b.Put(header);
            b.Put(" void acceptWithVisitor(");
            b.Put(visitorFactory -> visitor_type);
            b.Put(" v);");
            b.Put("\n");

            b.Put(header);
            b.Put(" void acceptWithArg(");
            b.Put(visitorFactory -> argument_visitor_type);
            b.Put(" v, Object? o);\n");

            b.Put(header);
            b.Put("Object? acceptWithResult(");
            b.Put(visitorFactory -> result_visitor_type);
            b.Put(" v);\n");

            b.Put(header);
            b.Put("Object? acceptWithResultArgument(");
            b.Put(visitorFactory -> result_argument_visitor_type);
            b.Put(" v, Object? o);");
            b.Put("\n");
        }


        delete [] header;
    }

    return;
}


//
//
//
void DartAction::GenerateVisitorMethods(NTC &ntc,
                                        TextBuffer &b,
                                        const char *indentation,
                                        ClassnameElement &element,
                                        BitSet &optimizable_symbol_set)
{
    if (option -> visitor & Option::DEFAULT)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("      void acceptWithVisitor(");
                                     b.Put(visitorFactory -> visitor_type);
                                     b.Put(" v){ v.visit"); b.Put(element.real_name); b.Put("(this); }\n");

        b.Put(indentation); b.Put("     void acceptWithArg(");
                                     b.Put(visitorFactory -> argument_visitor_type);
                                     b.Put(" v, Object? o){ v.visit");b.Put(element.real_name); b.Put("(this, o); }\n");

        b.Put(indentation); b.Put("     Object? acceptWithResult(");
                                     b.Put(visitorFactory -> result_visitor_type);
                                     b.Put(" v) { return v.visit");b.Put(element.real_name); b.Put("(this); }\n");

        b.Put(indentation); b.Put("      Object? acceptWithResultArgument(");
                                     b.Put(visitorFactory -> result_argument_visitor_type);
                                     b.Put(" v, Object? o)  { return v.visit"); b.Put(element.real_name); b.Put("(this, o); }\n");
    }
    if (option -> visitor & Option::PREORDER)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("     void  accept(IAstVisitor v )\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        if (! v.preVisit(this)) return;\n");
        b.Put(indentation); b.Put("        enter(v as ");
                                     b.Put(visitorFactory->preorder_visitor_type).Put(");\n");

        b.Put(indentation); b.Put("        v.postVisit(this);\n");
        b.Put(indentation); b.Put("    }\n\n");

        b.Put(indentation); b.Put("      void enter(");
                                     b.Put(visitorFactory->preorder_visitor_type);
                                     b.Put(" v)\n");
        b.Put(indentation); b.Put("    {\n");
        SymbolLookupTable &symbol_set = element.symbol_set;
        Tuple<int> &rhs_type_index = element.rhs_type_index;
        if (element.is_terminal_class || symbol_set.Size() == 0)
        {
            b.Put(indentation); b.Put("        v.visit"); b.Put(element.real_name); b.Put("(this);\n");
          
        }
        else
        {
            b.Put(indentation); b.Put("        var checkChildren = v.visit");b.Put(element.real_name); b.Put("(this);\n");
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
                    b.Put("if (null != _");
                    b.Put(symbol_set[i] -> Name());
                    b.Put(") ");

                    b.Put("_");
                    b.Put(symbol_set[i]->Name());
                    b.Put("!.accept(v);\n");
                }
                else
                {
                    b.Put("_");
                    b.Put(symbol_set[i]->Name());
                    b.Put(".accept(v);\n");
                }
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
void DartAction::GenerateGetAllChildrenMethod(TextBuffer &b,
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
        b.Put(indentation); b.Put("        ArrayList getAllChildren() \n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        var list = new ArrayList();\n");
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            b.Put(indentation);
        	b.Put("        if(null != _");b.Put(symbol_set[i]->Name());
        	b.Put(")  list.add(_");b.Put(symbol_set[i] -> Name());
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
void DartAction::GenerateSimpleVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                const char *indentation,
                                                const char *interface_name,
                                                SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);

    b.Put("abstract class ");
                                 b.Put(interface_name);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("   void visit"); b.Put(symbol->Name());
                                     b.Put("(");
                                     b.Put(symbol -> Name());
                                     b.Put(" n);\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("   void visit(");
                               
                                 b.Put(option -> ast_type);
                                 b.Put(" n);\n");

    b.Put(indentation); b.Put("}\n");


}

//
//
//
void DartAction::GenerateArgumentVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                  const char *indentation,
                                                  const char *interface_name,
                                                  SymbolLookupTable &type_set)
{
   
    TextBuffer& b =*GetBuffer(ast_filename_symbol);

    b.Put("abstract class ");
                                 b.Put(interface_name);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("   void visit"); b.Put(symbol->Name());
                                     b.Put("(");
                                     b.Put(symbol -> Name());
                                     b.Put(" n, Object? o);\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("   void visit(");
                              
                                 b.Put(option -> ast_type);
                                 b.Put(" n, Object? o);\n");

    b.Put(indentation); b.Put("}\n");
    
}

//
//
//
void DartAction::GenerateResultVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                const char *indentation,
                                                const char *interface_name,
                                                SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);

	b.Put("abstract class ");
                                 b.Put(interface_name);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("    Object? visit"); b.Put(symbol->Name());
                                     b.Put("(");
                                     b.Put(symbol -> Name());
                                     b.Put( " n);\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("   Object? visit(");
                               
                                 b.Put(option -> ast_type);
                                 b.Put(" n);\n");

    b.Put(indentation); b.Put("}\n");
    
}

//
//
//
void DartAction::GenerateResultArgumentVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                        const char *indentation,
                                                        const char *interface_name,
                                                        SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
	b.Put("abstract class ");
    b.Put(interface_name);
    b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("   Object? visit"); b.Put(symbol->Name());
                                     b.Put("(");
                                     b.Put(symbol -> Name());
                                     b.Put(" n, Object? o);\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("   Object? visit(");
                               
                                 b.Put(option -> ast_type);
                                 b.Put(" n, Object? o);\n");

    b.Put(indentation); b.Put("}\n");
    
}


//
//
//
void DartAction::GeneratePreorderVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                  const char *indentation,
                                                  const char *interface_name,
                                                  SymbolLookupTable &type_set)
{
    TextBuffer& buf =*GetBuffer(ast_filename_symbol);
    assert(option -> visitor & Option::PREORDER);
	buf.Put("abstract class ");
                                 buf.Put(interface_name);
                                 buf.Put(" implements IAstVisitor\n");
    buf.Put(indentation); buf.Put("{\n");


    buf.Put(indentation); buf.Put("   bool visit(");
                              
                                 buf.Put(option -> ast_type);
                                 buf.Put(" n);\n");

    buf.Put(indentation); buf.Put("   void endVisit(");
                             
                                 buf.Put(option -> ast_type);
                                 buf.Put(" n);\n\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        buf.Put(indentation); buf + "   bool visit" + symbol->Name() + "(" + symbol -> Name()+ " n);\n";
   
        buf.Put(indentation); buf+  "   void endVisit"+symbol->Name() + "("+ symbol -> Name()+" n);\n\n";

    }

    buf.Put(indentation); buf.Put("}\n\n");
    
    return;
}


//
//
//
void DartAction::GenerateNoResultVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                      const char *indentation,
                                                      const char *classname,
                                                      SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);

                                 b.Put("abstract class ");
                                 b.Put(classname);
                                 b.Put(" implements ");
                                 b.Put(visitorFactory -> visitor_type);
                                 b.Put(", ");
                                 b.Put(visitorFactory -> argument_visitor_type);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("      void unimplementedVisitor(String s);\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];

            b.Put(indentation);
        	b+ "      void visit"+ symbol->Name()+ "("+ symbol->Name() + " n, [Object? o]) { unimplementedVisitor(\"visit" + symbol->Name() + "(";
                                         b.Put(symbol -> Name());
                                         b.Put(", Object)\"); }\n");
            b.Put("\n");
        }
    }

                                 b.Put("\n");

   

    b.Put(indentation); b.Put("      void visit(") + option -> ast_type+" n, [Object? o])\n";
          
    b.Put(indentation); b.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                         b.Put(i == 0 ? "" : "else ");
                                         b.Put("if (n is ");
                                         b.Put(symbol -> Name());
                                         b.Put(") visit");
                                         b.Put(symbol -> Name());
                                         b+ "(n, o);\n";

        }
    }
    b.Put(indentation); b.Put("        else throw  ArgumentError(\"visit(\" + n.toString() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("}\n");
    
}

//
//
//
void DartAction::GenerateResultVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                    const char *indentation,
                                                    const char *classname,
                                                    SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);

                                 b.Put("abstract class ");
                                 b.Put(classname);
                                 b.Put(" implements ");
                                 b.Put(visitorFactory -> result_visitor_type);
                                 b.Put(", ");
                                 b.Put(visitorFactory -> result_argument_visitor_type);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("     Object? unimplementedVisitor(String s);\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
       
            b.Put(indentation); b.Put("    Object? visit"); b.Put(symbol->Name()); b.Put("(");
                                         b.Put(symbol -> Name());
                                         b.Put(" n, [Object? o]){ return  unimplementedVisitor(\"visit"); b.Put(symbol->Name()); b.Put("(");
                                         b.Put(symbol -> Name());
                                         b.Put(", Object)\"); }\n");
            b.Put("\n");
        }
    }

                                 b.Put("\n");



    b.Put(indentation); b.Put("    Object? visit(");
                                
                                 b.Put(option -> ast_type);
                                 b.Put(" n, [Object? o])\n");
    b.Put(indentation); b.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                         b.Put(i == 0 ? "" : "else ");
                                         b.Put("if (n is ");
                                         b.Put(symbol -> Name());
                                         b.Put(") return visit");
                                         b.Put(symbol->Name());
                                         b.Put("(n, o);\n");

        }
    }
    b.Put(indentation); b.Put("        else throw  ArgumentError(\"visit(\" + n.toString() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("}\n");
    
}


//
//
//
void DartAction::GeneratePreorderVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                      const char *indentation,
                                                      const char *classname,
                                                      SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    assert(option -> visitor & Option::PREORDER);
     b.Put("abstract class ");
     b.Put(classname);
     b.Put(" implements ");
     b.Put(visitorFactory->preorder_visitor_type);
     b.Put("\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("     void unimplementedVisitor(String s)  ;\n\n");
    b.Put(indentation); b.Put("     bool preVisit(IAst element) { return true; }\n\n");
    b.Put(indentation); b.Put("     void postVisit(IAst element) {}\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("    bool visit"); b.Put(symbol->Name());
                                         b.Put("(");
                                         b.Put(symbol -> Name());
                                         b.Put(" n){ unimplementedVisitor(\"visit(");
                                         b.Put(symbol -> Name());
                                         b.Put(")\"); return true; }\n");
            b.Put(indentation); b.Put("    void endVisit"); b.Put(symbol->Name());
                                         b.Put("(");
                                         b.Put(symbol -> Name());
                                         b.Put(" n)  { unimplementedVisitor(\"endVisit(");
                                         b.Put(symbol -> Name());
                                         b.Put(")\"); }\n");
            b.Put("\n");
        }
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    bool visit(");
                              
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
                                         b.Put(") return visit");
                                         b.Put(symbol->Name());
                                         b.Put("( n);\n");
  
        }
    }
    b.Put(indentation); b.Put("        throw  ArgumentError(\"visit(\" + n.toString() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("   void endVisit(");
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
                                         b.Put(") endVisit");
                                         b.Put(symbol -> Name());
                                         b.Put("(n);\n");

        }
    }
    b.Put(indentation); b.Put("        else throw  ArgumentError(\"visit(\" + n.toString() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("}\n");
    
    return;
}


//
// Generate the the Ast root classes
//
void DartAction::GenerateAstType(ActionFileSymbol* ast_filename_symbol,
                                 const char *indentation,
                                 const char *classname)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    /*
     * First, generate the main root class
     */

                                 b.Put("abstract class ");
                                 b.Put(classname);
                                 b.Put(" implements IAst\n");
    b.Put(indentation); b.Put("{\n");
    if (option -> glr)
    {
        b.Put(indentation); b.Put("    IAst? nextAst ;\n");
        b.Put(indentation); b.Put("    IAst? getNextAst(){ return nextAst; }\n");
        b.Put(indentation); b.Put("    void setNextAst(IAst n){ nextAst = n; }\n");
        b.Put(indentation); b.Put("    void resetNextAst(){ nextAst = null; }\n");
    }
    else
    {
	    b.Put(indentation); b.Put("    IAst? getNextAst(){ return null; }\n");
	    b.Put(indentation); b.Put("    void setNextAst(IAst n){ }\n");
	    b.Put(indentation); b.Put("    void resetNextAst(){ }\n");
    }

    b.Put(indentation); b.Put("     late IToken leftIToken ;\n");
    b.Put(indentation); b.Put("     late IToken rightIToken ;\n");
    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("     IAst? parent;\n");
        b.Put(indentation); b.Put("     void setParent(IAst p){ parent = p; }\n");
        b.Put(indentation); b.Put("     IAst? getParent(){ return parent; }\n");\
    }
    else
    {
        b.Put(indentation); b.Put("     IAst? getParent()\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        throw  ArgumentError(\"noparent-saved option in effect\");\n");
        b.Put(indentation); b.Put("    }\n");
    }
    b.Put(indentation); b.Put("    int getRuleIndex(){ return 0; }\n");
    b.Put("\n");
    b.Put(indentation); b.Put("     IToken getLeftIToken()  { return leftIToken; }\n");
    b.Put(indentation); b.Put("     IToken getRightIToken()  { return rightIToken; }\n");
    b.Put(indentation); b.Put("      List<IToken> getPrecedingAdjuncts() { return leftIToken.getPrecedingAdjuncts(); }\n");
    b.Put(indentation); b.Put("      List<IToken> getFollowingAdjuncts() { return rightIToken.getFollowingAdjuncts(); }\n\n");

    b.Put(indentation); b.Put("    String  toString()  \n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("      var  lex = leftIToken.getILexStream();\n");
    b.Put(indentation); b.Put("      if( lex != null)\n");
    b.Put(indentation); b.Put("        return lex.toStringWithOffset(leftIToken.getStartOffset(), rightIToken.getEndOffset());\n");
    b.Put(indentation); b.Put("      return  '';\n");
    b.Put(indentation); b.Put("    }\n\n");

   
    b.Put(indentation); b.Put(classname); b.Put("(IToken leftIToken ,[ IToken? rightIToken ])\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        this.leftIToken = leftIToken;\n");
    b.Put(indentation); b.Put("        if(rightIToken != null) this.rightIToken = rightIToken;\n");
    b.Put(indentation); b.Put("        else            this.rightIToken = leftIToken;\n");
    b.Put(indentation); b.Put("    }\n\n");
    b.Put(indentation); b.Put("    void initialize(){}\n");
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
        b.Put(indentation); b.Put("      ArrayList getChildren() \n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("         var list = getAllChildren() ;\n");
        b.Put(indentation); b.Put("        var k = -1;\n");
        b.Put(indentation); b.Put("        for (var i = 0; i < list.size(); i++)\n");
        b.Put(indentation); b.Put("        {\n");
        b.Put(indentation); b.Put("            var element = list.get(i);\n");
        b.Put(indentation); b.Put("            if (null==element)\n");
        b.Put(indentation); b.Put("            {\n");
        b.Put(indentation); b.Put("                if (++k != i)\n");
        b.Put(indentation); b.Put("                    list.set(k, element);\n");
        b.Put(indentation); b.Put("            }\n");
        b.Put(indentation); b.Put("        }\n");
        b.Put(indentation); b.Put("        for (var i = list.size() - 1; i > k; i--) // remove extraneous elements\n");
        b.Put(indentation); b.Put("            list.remove(i);\n");
        b.Put(indentation); b.Put("        return list;\n");
        b.Put(indentation); b.Put("    }\n\n");

        b.Put(indentation); b.Put("    /**\n");
        b.Put(indentation); b.Put("     * A list of all children of this node, don't including the null ones.\n");
        b.Put(indentation); b.Put("     */\n");
        b.Put(indentation); b.Put("     ArrayList getAllChildren() ;\n");
    }
    else
    {
        b.Put(indentation); b.Put("      ArrayList getChildren() \n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        throw  ArgumentError(\"noparent-saved option in effect\");\n");
        b.Put(indentation); b.Put("    }\n");
        b.Put(indentation); b.Put("        ArrayList getAllChildren()  { return getChildren(); }\n");
    }

    b.Put("\n");

    GenerateVisitorHeaders(b, indentation, "     ");

    //
    // Not Preorder visitor? generate dummy accept method to satisfy IAst abstract declaration of accept(IAstVisitor);

    if (!(option -> visitor & Option::PREORDER) )
    {
        b.Put(indentation); b.Put("     void accept(IAstVisitor v){}\n");
    }
    b.Put(indentation); b.Put("}\n\n");
    
    return;
}



typedef std::map<std::string, std::string> Substitutions;



//
// Generate the the Ast list class
//
void DartAction::GenerateAbstractAstListType(ActionFileSymbol* ast_filename_symbol,
                                             const char *indentation,
                                             const char *classname)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    /*
     * Generate the List root class
     */

                                 b.Put("abstract class ");
                                 b.Put(this -> abstract_ast_list_classname);
                                 b.Put(" extends ");
                                 b.Put(option -> ast_type);
                                 b.Put(" implements IAbstractArrayList<");
                                 b.Put(option -> ast_type);
                                 b.Put(">\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("     late bool leftRecursive  ;\n");
    b.Put(indentation); b.Put("     "); 
	b.Put(" var list  =  ArrayList();\n");

    b.Put(indentation); b.Put("     int size()   { return list.size(); }\n");
    b.Put(indentation); b + "     ArrayList"  + " getList(){ return list; }\n";
    b.Put(indentation); b.Put("     ");
								 b.Put(option->ast_type);
                                 b.Put(" getElementAt(int i)");
                                 
                                 b.Put(" { return ");
                               
                                 b.Put("list.get(leftRecursive ? i : list.size() - 1 - i); }\n");

    b.Put(indentation);
	b + "     ArrayList" +" getArrayList()\n" ;
   
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        if (! leftRecursive) // reverse the list \n");
    b.Put(indentation); b.Put("        {\n");
    b.Put(indentation); b.Put("            for (var i = 0, n = list.size() - 1; i < n; i++, n--)\n");
    b.Put(indentation); b.Put("            {\n");
    b.Put(indentation); b.Put("                var ith = list.get(i),\n");
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

    b.Put(indentation); b+"     bool add("+ option -> ast_type + " element)\n";
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        addElement(element);\n");
    b.Put(indentation); b.Put("        return true;\n");
    b.Put(indentation); b.Put("    }\n\n");

    b.Put(indentation); b.Put("     void addElement(");
                                 b.Put(option -> ast_type);
                                 b.Put(" element)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        list.add(element);\n");
    b.Put(indentation); b.Put("        if (leftRecursive)\n");
    b.Put(indentation); b.Put("             rightIToken = element.getRightIToken();\n");
    b.Put(indentation); b.Put("        else leftIToken = element.getLeftIToken();\n");
    b.Put(indentation); b.Put("    }\n\n");





    // generate constructors for list class

    b.Put(indentation); b+ "      "+ this->abstract_ast_list_classname + 
        "(IToken leftToken, IToken rightToken , bool leftRecursive  ):super(leftToken, rightToken){\n";
    b.Put(indentation); b.Put("          this.leftRecursive = leftRecursive;\n");
    b.Put(indentation); b.Put("    }\n\n");

  
    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("    /**\n");
        b.Put(indentation); b.Put("     * Make a copy of the list and return it. Note that we obtain the local list by\n");
        b.Put(indentation); b.Put("     * invoking getArrayList so as to make sure that the list we return is in proper order.\n");
        b.Put(indentation); b.Put("     */\n");
        b.Put(indentation); b.Put("        ArrayList getAllChildren() \n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        return getArrayList().clone();\n");
        b.Put(indentation); b.Put("    }\n\n");
    }

    //
    // Implementation for functions in ArrayList
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
void DartAction::GenerateAstTokenType(NTC &ntc, ActionFileSymbol* ast_filename_symbol,
                                      const char *indentation,
                                      const char *classname)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    /*
     * Generate the Token root class
     */

                                 b.Put("class ");
                                 b.Put(classname);
                                 b.Put(" extends ");
                                 b.Put(option -> ast_type);
                                 b.Put(" implements I");
                                 b.Put(classname);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    b.Put(indentation); b.Put("    "); b+ classname + ("(IToken token   ):super(token){  }\n");
    b.Put(indentation); b.Put("     IToken getIToken()  { return leftIToken; }\n");
    b.Put(indentation); b.Put("     String toString(){ return leftIToken.toString(); }\n\n");

    ClassnameElement element; // generate a temporary element with no symbols in its symbol set.
    element.real_name = (char *) classname;
    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);

    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("    /**\n");
        b.Put(indentation); b.Put("     * A token class has no children. So, we return the empty list.\n");
        b.Put(indentation); b.Put("     */\n");
        b.Put(indentation); b.Put("        ArrayList getAllChildren()  { return  ArrayList(); }\n\n");
    }

    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set);

    b.Put(indentation); b.Put("}\n\n");
    
    return;
}


//
//
//
void DartAction::GenerateCommentHeader(TextBuffer& b,
    const char*,
    Tuple<int>& ungenerated_rule,
    Tuple<int>& generated_rule)
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

            LexStream::TokenIndex separator_token = grammar->parser.rules[grammar->rules[rule_no].source_index].separator_index;
            int line_no = lex_stream->Line(separator_token),
                start = lex_stream->StartLocation(separator_token),
                end = lex_stream->EndLocation(separator_token) + 1;
            const char* start_cursor_location = &(lex_stream->InputBuffer(separator_token)[start]),
                * end_cursor_location = &(lex_stream->InputBuffer(separator_token)[end]);

            b.Put("\n");

            ProcessActionLine(scope_block, ActionBlockElement::BODY,
                &b,
                lex_stream->FileName(separator_token),
                rule_info,
                &rule_info[strlen(rule_info)],
                rule_no,
                lex_stream->FileName(separator_token),
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

        LexStream::TokenIndex separator_token = grammar->parser.rules[grammar->rules[rule_no].source_index].separator_index;
        int line_no = lex_stream->Line(separator_token),
            start = lex_stream->StartLocation(separator_token),
            end = lex_stream->EndLocation(separator_token) + 1;
        const char* start_cursor_location = &(lex_stream->InputBuffer(separator_token)[start]),
            * end_cursor_location = &(lex_stream->InputBuffer(separator_token)[end]);

        b.Put("\n");

        ProcessActionLine(scope_block, ActionBlockElement::BODY,
            &b,
            lex_stream->FileName(separator_token), // option -> DefaultBlock() -> ActionfileSymbol() -> Name(),
            rule_info,
            &rule_info[strlen(rule_info)],
            rule_no,
            lex_stream->FileName(separator_token),
            line_no,
            start_cursor_location,
            end_cursor_location);
    }

    b.Put("\n");

    b.Put(" *</b>\n");

    b.Put(" */\n");
}


void DartAction::GenerateListMethods(CTC &ctc,
                                     NTC &ntc,
                                     TextBuffer &b,
                                     const char *indentation,
                                     const char *classname,
                                     ClassnameElement &element,
                                     Array<const char *> &)
{
    const char* element_name = element.array_element_type_symbol->Name();

    //
    // Generate ADD method
    //
    b.Put(indentation); b.Put("     ");
                                 b.Put(" void addElement(");
                                 b.Put(option->ast_type);
                                 b.Put(" _");
                                 b.Put(element_name);
                                 b.Put(")\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation);
	b + "        super.addElement(_"+ element_name  + ");\n";

    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("        ");
        if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
        {
            b.Put("if (null != _");
            b.Put(element_name);
            b.Put(") ");
            b + "_" + element_name + "!.setParent(this);\n";
        }
        else
        {
            b + "_" + element_name + ".setParent(this);\n";
        }

    }
    b.Put(indentation); b.Put("    }\n");

    b.Put("\n");
   
    //
    // Generate visitor methods.
    //
    if (option -> visitor & Option::DEFAULT)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("    void acceptWithVisitor(");
                                     b.Put(visitorFactory -> visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
            b.Put(" v)  { for (var i = 0; i < size(); i++) v.visit"
                           "("
                           "get");
            b.Put(element_name);
            b.Put("At(i)"
                           "); }\n");
        }
        else
        {
            b.Put(" v){ for (var i = 0; i < size(); i++) get");
            b.Put(element_name);
            b.Put("At(i).acceptWithVisitor(v); }\n");
        }

        b.Put(indentation); b.Put("     void acceptWithArg(");
                                     b.Put(visitorFactory -> argument_visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
            b.Put(" v, Object? o){ for (var i = 0; i < size(); i++) v.visit"
                           "("
                           "get");
            b.Put(element_name);
            b.Put("At(i), o");
            b.Put("); }\n");
        }
        else
        {
            b.Put(" v, Object? o){ for (var i = 0; i < size(); i++) get");
            b.Put(element_name);
            b.Put("At(i).acceptWithArg(v, o); }\n");
        }

        //
        // Code cannot be generated to automatically visit a node that
        // can return a value. These cases are left up to the user.
        //
        b.Put(indentation); b.Put("     Object? acceptWithResult(");
                                     b.Put(visitorFactory -> result_visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
                                         b.Put(" v)\n");
            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        var result = ArrayList();\n");
            b.Put(indentation); b.Put("        for (var i = 0; i < size(); i++)\n");
            b.Put(indentation); b.Put("            result.add(v.visit(get");
                                         b.Put(element_name);
                                         b.Put("At(i)));\n");
            b.Put(indentation); b.Put("        return result;\n");
            b.Put(indentation); b.Put("    }\n");
        }
        else
        {
                                         b.Put(" v)\n");
            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        var result = ArrayList();\n");
            b.Put(indentation); b.Put("        for (var i = 0; i < size(); i++)\n");
            b.Put(indentation); b.Put("            result.add(get");
                                         b.Put(element_name);
                                         b.Put("At(i).acceptWithResult(v));\n");
            b.Put(indentation); b.Put("        return result;\n");
            b.Put(indentation); b.Put("    }\n");
        }

        b.Put(indentation); b.Put("     Object? acceptWithResultArgument(");
                                     b.Put(visitorFactory -> result_argument_visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
                                         b.Put(" v, Object? o) \n");
            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        var result = new ArrayList();\n");
            b.Put(indentation); b.Put("        for (var i = 0; i < size(); i++)\n");
            b.Put(indentation); b.Put("            result.add(v.visit(get");
                                         b.Put(element_name);
                                         b.Put("At(i), o));\n");
            b.Put(indentation); b.Put("        return result;\n");
            b.Put(indentation); b.Put("    }\n");
        }
        else
        {
                                         b.Put(" v, Object? o)\n");
            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        var result = new ArrayList();\n");
            b.Put(indentation); b.Put("        for (var i = 0; i < size(); i++)\n");
            b.Put(indentation); b.Put("            result.add(get");
                                         b.Put(element_name);
                                         b.Put("At(i).acceptWithResultArgument(v, o));\n");
            b.Put(indentation); b.Put("        return result;\n");
            b.Put(indentation); b.Put("    }\n");
        }
    }
    if (option -> visitor & Option::PREORDER)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("    void accept(IAstVisitor v )\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        if (! v.preVisit(this)) return;\n");
        b.Put(indentation); b.Put("        enter(v as ");
                                     b.Put(visitorFactory->preorder_visitor_type);
                                     b.Put(");\n");
        b.Put(indentation); b.Put("        v.postVisit(this);\n");
        b.Put(indentation); b.Put("    }\n");
        b.Put(indentation); b.Put("     void enter(");
                                     b.Put(visitorFactory->preorder_visitor_type);
                                     b.Put(" v)\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        var checkChildren = v.visit").Put(classname).Put("(this);\n");
        b.Put(indentation); b.Put("        if (checkChildren)\n");
        b.Put(indentation); b.Put("        {\n");
        b.Put(indentation); b.Put("            for (var i = 0; i < size(); i++)\n");
        b.Put(indentation); b.Put("            {\n");
        b.Put(indentation); b.Put("                ");

        const char *element_typename = ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex());
        if (element_typename != NULL)
        {
            //b.Put(element_typename);
            b.Put(" var element = get");
            b.Put(element_name);
            b.Put("At(i);\n");
            if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
            {
                b.Put(indentation); b.Put("                if (null != element)");
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
            b.Put("var element = get");
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
void DartAction::GenerateListClass(CTC &ctc,
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
               *element_type = ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex(),option->ast_type);

    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);


                                 b.Put("class ");
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
    b.Put(indentation); b.Put("     ");
								 b.Put(element_type);
                                 b.Put(" get");
                                 b.Put(element_name);
                                 b.Put("At(int i)");
                              
                                 b+ "{ return getElementAt(i) as "+ element_type+"; }\n\n";

    //
    // generate constructors
    //
	b.Put(indentation);
    b + "    " + classname + "(IToken leftToken, IToken rightToken , bool leftRecursive  ):super(leftToken, rightToken, leftRecursive)\n";
    b.Put(indentation); b.Put("    {}\n\n");

    b.Put(indentation);
	b+"    static " + classname + " "+  classname + "fromElement(" + element_type + " element,bool leftRecursive )\n";

    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        var obj = ");b.Put(classname);
	b.Put("(element.getLeftIToken(),element.getRightIToken(), leftRecursive);\n");
    b.Put(indentation); b.Put("        obj.list.add(element);\n");
    if (option->parent_saved)
    {
        b.Put(indentation); b.Put("        ");
        if (ntc.CanProduceNullAst(element.array_element_type_symbol->SymbolIndex()))
        {
            b.Put("if (null != element)");
        }

        b + "(element as "+option->ast_type + ").setParent(obj);\n";
    }
    b.Put(indentation); b.Put("        return obj;\n");
    b.Put(indentation); b.Put("    }\n");
    b.Put("\n");



    GenerateListMethods(ctc, ntc, b, indentation, classname, element, typestring);
    b.Put(indentation);
    b+ "    @override\n";
    b.Put(indentation);
    IntToString num(element.GetRuleNo());
    b+ "    int getRuleIndex() { return " + num.String() + " ;}\n";
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
void DartAction::GenerateListExtensionClass(CTC& ctc,
    NTC& ntc,
    ActionFileSymbol* ast_filename_symbol,
    const char* indentation,
    SpecialArrayElement& special_array,
    ClassnameElement& element,
    Array<const char*>&)

{
    TextBuffer& b = *GetBuffer(ast_filename_symbol);
    const char* classname = element.real_name,
        * element_type = ctc.FindUniqueTypeFor(element.array_element_type_symbol->SymbolIndex(), option->ast_type);

    GenerateCommentHeader(b, indentation, element.ungenerated_rule, special_array.rules);


    b.Put("class ");
    b.Put(special_array.name);
    b.Put(" extends ");
    b.Put(classname);
    b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    GenerateEnvironmentDeclaration(b, indentation);


    b.Put(indentation);
    b +"    "+ special_array.name + "("+ option->action_type+" environment, IToken leftIToken, IToken rightIToken, bool leftRecursive )\n";
    b.Put(indentation); b.Put("    :super(leftIToken, rightIToken, leftRecursive)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        this.environment = environment;\n");
    b.Put(indentation); b.Put("        initialize();\n");
    b.Put(indentation); b.Put("    }\n\n");


    b.Put(indentation);
    b + "    static  " + special_array.name + " " + special_array.name + "fromElement(" + option->action_type + " environment, "+
        element_type+" element,bool leftRecursive )\n";


    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        var obj = "); b.Put(special_array.name);
    b.Put("(environment,element.getLeftIToken(),element.getRightIToken(), leftRecursive);\n");
    b.Put(indentation); b.Put("        obj.list.add(element);\n");
    if (option->parent_saved)
    {
        b.Put(indentation); b.Put("        ");
        if (ntc.CanProduceNullAst(element.array_element_type_symbol->SymbolIndex()))
        {
            b.Put("if (null !=element)");
        }
     
        b + "(element as "+ option->ast_type+ ").setParent(obj);\n";
    }
    b.Put(indentation); b.Put("        return obj;\n");
    b.Put(indentation); b.Put("    }\n");
    b.Put("\n");


    b.Put(indentation);
    b+ "    @override\n";
    b.Put(indentation);
    IntToString num(element.GetRuleNo());
    b+ "    int getRuleIndex() { return " + num.String() + " ;}\n";
    //GenerateListMethods(ctc, ntc, b, indentation, classname, element, typestring);


}


//
// Generate a generic rule class
//
void DartAction::GenerateRuleClass(CTC &ctc,
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


                                 b.Put("class ");
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
            b.Put(indentation); b.Put("     IToken get");
                                         b.Put(symbol_set[0] -> Name());
                                         b.Put("(){ return leftIToken; }\n\n");
        }
        b.Put(indentation);
        b.Put(classname);
    	b.Put("(");
        if (element.needs_environment)
        {
            b.Put(option->action_type);
            b.Put(" environment");
         
            b.Put(", IToken token):super(token)");

            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        this.environment = environment;\n");
            b.Put(indentation); b.Put("        initialize();\n");
            b.Put(indentation); b.Put("    }\n");
        }
        else b.Put("IToken token) :super(token){ initialize(); }\n");
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
                 b.Put(indentation);
                 b.Put("     late ");
                 b.Put(ctc.FindUniqueTypeFor(rhs_type_index[i], option->ast_type));
                 if (ntc.CanProduceNullAst(rhs_type_index[i]))
                    b.Put("?");
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
                    const char *bestType = ctc.FindUniqueTypeFor(rhs_type_index[i], option->ast_type);
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
                    b.Put(indentation); b.Put("     ");
                    b.Put(bestType);
                    if (nullAst)  b.Put(" ? ");
                                                 b.Put(" get");
                                                 b.Put(symbolName);
                                                 b.Put("(){ return _");
          
                                                 b.Put(symbolName);
                                                 b.Put("; }\n");

                    // Generate setter method
                    b.Put(indentation); b.Put("     void set");
                    b.Put(symbolName);
                    b.Put("(");
                    b.Put(bestType);
                    b.Put(" _"); // add "_" prefix to arg name in case symbol happens to be a Java keyword
                    b.Put(symbolName);
  
                    b.Put("){ this._");
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
        const char* header = "    ";
        b.Put(indentation);
        b.Put(header);
        b.Put(classname);
        int length = strlen(indentation) + strlen(header) + strlen(classname);

        b.Put("(");
        if (element.needs_environment)
        {
            b.Put(option->action_type);
            b.Put(" environment, ");
        }
        b.Put("IToken leftIToken, IToken rightIToken");
        b.Put(symbol_set.Size() == 0 ? ")\n" : ",\n");
        {
            for (int i = 0; i < symbol_set.Size(); i++)
            {
                for (int k = 0; k <= length; k++)
                    b.PutChar(' ');

                b.Put(ctc.FindUniqueTypeFor(rhs_type_index[i], option->ast_type));
                if (ntc.CanProduceNullAst(rhs_type_index[i]))
                {
                    b.Put("?");
                }
                b.Put(" _");
                b.Put(symbol_set[i] -> Name());
                b.Put(i == symbol_set.Size() - 1 ? ")\n" : ",\n");
            }
        }

        b.Put(indentation); b.Put("        :super(leftIToken, rightIToken)\n\n");
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
                        b.Put("if (null != _");
                        b.Put(symbol_set[i] -> Name());
                        b.Put(") ");
                    }
    
                    b.Put("(_");
                    b.Put(symbol_set[i] -> Name());    b.Put(" as ");
                    b.Put(option->ast_type);
                    b.Put(").setParent(this);\n");
                }
            }
        }

        b.Put(indentation); b.Put("        initialize();\n");
        b.Put(indentation); b.Put("    }\n");

    }

    if (option -> parent_saved)
        GenerateGetAllChildrenMethod(b, indentation, element);
   
    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set);

    b.Put(indentation);
    b+ "    @override\n";
    b.Put(indentation);
    IntToString num(rule_no);
    b+ "    int getRuleIndex() { return " + num.String() + " ;}\n";
    return;
}


//
// Generate Ast class
//
void DartAction::GenerateTerminalMergedClass(NTC &ntc,
                                             ActionFileSymbol* ast_filename_symbol,
                                             const char *indentation,
                                             ClassnameElement &element,
                                             Array<const char *> &typestring)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    char *classname = element.real_name;
    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);

    b.Put(indentation); 
                                 b.Put("class ");
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
        b.Put(indentation); b.Put("     IToken get");
                                     b.Put(symbol_set[0] -> Name());
                                     b.Put("(){ return leftIToken; }\n\n");
    }
    b.Put(indentation);
	b.Put("    ");
    b.Put(classname);
    b.Put("(");
    if (element.needs_environment)
    {
        b.Put(option->action_type);
        b.Put(" environment, IToken token):super(token)");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        this.environment = environment;\n");
        b.Put(indentation); b.Put("        initialize();\n");
        b.Put(indentation); b.Put("    }\n");
    }
    else b.Put("IToken token):super(token) { initialize(); }\n");

    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);
  
    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set);
    b.Put(indentation);
    b+ "    @override\n";
    b.Put(indentation);
    IntToString num(element.GetRuleNo());
    b+ "    int getRuleIndex() { return " + num.String() + " ;}\n";
    return;
}


//
// Generate Ast class
//
void DartAction::GenerateMergedClass(CTC &ctc,
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


                                 b.Put("class ");
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
            b.Put(indentation);

            if ((!optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
            {
                b.Put("     ");
                b.Put(ctc.FindUniqueTypeFor(rhs_type_index[i], option->ast_type));
	            b.Put(" ?");
            }
            else
            {
                b.Put("     late ");
                b.Put(ctc.FindUniqueTypeFor(rhs_type_index[i], option->ast_type));
            }
	         b.Put(" _");
	         b.Put(symbol_set[i] -> Name());

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

	        b.Put(indentation);
	        b.Put("     ");
	        b.Put(ctc.FindUniqueTypeFor(rhs_type_index[i], option->ast_type));
	        if (nullAst)
	            b.Put(" ? ");

	         b.Put(" get");
	         b.Put(symbol_set[i] -> Name());
	         b.Put("()");

	         b.Put("{ return _");
	         b.Put(symbol_set[i] -> Name());
	         b.Put("; }\n");
        }
    }
    b.Put("\n");


    //
    // generate merged constructor
    //
    const char* header = "     ";
    b.Put(indentation);
    b.Put(header);
    b.Put(classname);
    int length = strlen(indentation) + strlen(header) + strlen(classname);

    b.Put("(");
    if (element.needs_environment)
    {
        b.Put(option->action_type);
        b.Put(" environment, ");
    }
    b.Put("IToken leftIToken, IToken rightIToken");
    b.Put(symbol_set.Size() == 0 ? ")\n" : ",\n");
    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            for (int k = 0; k <= length; k++)
                b.PutChar(' ');

            b.Put(ctc.FindUniqueTypeFor(rhs_type_index[i], option->ast_type));
            if ((!optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
            {
                b.Put("?");
            }

            b.Put(" _");
            b.Put(symbol_set[i] -> Name());

            b.Put(i == symbol_set.Size() - 1 ? ")\n" : ",\n");
        }
    }

    b.Put(indentation); b.Put("        :super(leftIToken, rightIToken)\n\n");
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
                    b.Put("if(null != _");
                    b.Put(symbol_set[i] -> Name());
                    b.Put(") ");
                }
    
                b.Put("(_");
                b.Put(symbol_set[i] -> Name());
            	b.Put(" as ");
            	b.Put(option->ast_type);
                b.Put(").setParent(this);\n");
            }
        }
    }

    b.Put(indentation); b.Put("        initialize();\n");
    b.Put(indentation); b.Put("    }\n");

    if (option -> parent_saved)
        GenerateGetAllChildrenMethod(b, indentation, element);
  
    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set);
    b.Put(indentation);
    b+ "    @override\n";
    b.Put(indentation);
    IntToString num(element.GetRuleNo());
    b+ "    int getRuleIndex() { return " + num.String() + " ;}\n";
    return;
}

void DartAction::GenerateAstRootInterface(
    ActionFileSymbol* ast_filename_symbol,
    const char* indentation)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    b.Put("abstract class ");
    b.Put(astRootInterfaceName.c_str());
    
    b.Put("\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("     IToken getLeftIToken() ;\n");
    b.Put(indentation); b.Put("    IToken  getRightIToken() ;\n");
    b.Put("\n");
    GenerateVisitorHeaders(b, indentation, "    ");
    b.Put(indentation); b.Put("}\n\n");
    
    
    return;
}
void DartAction::GenerateInterface(bool is_terminal,
                                   ActionFileSymbol* ast_filename_symbol,
                                   const char *indentation,
                                   const char *interface_name,
                                   Tuple<int> &,
                                   Tuple<int> &classes,
                                   Tuple<ClassnameElement> &classname)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    b.Put(indentation); b.Put("/***");
    if (is_terminal)
    {
        b.Put("\n");
        b.Put(indentation);  b.Put(" ** is always implemented by <b>");
                                      b.Put(grammar -> Get_ast_token_classname());
                                      b.Put("</b>. It is also implemented by");
    }
    else 
    {
        b.Put("\n");
        b.Put(indentation);
        b.Put(" ** is implemented by");
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
        b.Put(" **<b>\n");
        b.Put(indentation); b.Put(" **<ul>");
        for (int i = 0; i < classes.Length(); i++)
        {
            b.Put("\n");
            b.Put(indentation);
            b.Put(" **<li>");
            b.Put(classname[classes[i]].real_name);
        }
        b.Put("\n");
        b.Put(indentation);
        b.Put(" **</ul>\n");
        b.Put(indentation);
        b.Put(" **</b>");
    }

    b.Put("\n");
    b.Put(indentation);
    b.Put(" **/\n");

    b.Put("abstract class ");
	b.Put(interface_name);
    //if (extension.Length() > 0)
    //{
    //    b.Put(" implements ");
    //    for (int k = 0; k < extension.Length() - 1; k++)
    //    {
    //        b.PutChar('I');
    //        b.Put(extension[k] == grammar -> Get_ast_token_interface()
    //                           ? grammar -> Get_ast_token_classname()
    //                           : grammar -> RetrieveString(extension[k]));
    //        b.Put(", ");
    //    }
    //    b.PutChar('I');
    //    b.Put(extension[extension.Length() - 1] == grammar -> Get_ast_token_interface()
    //                           ? grammar -> Get_ast_token_classname()
    //                           : grammar -> RetrieveString(extension[extension.Length() - 1]));
    //    b.Put(" {}\n\n");
    //}
    //else
    {
        b.Put(" implements ");
        b.Put(astRootInterfaceName.c_str());
        b.Put(indentation); b.Put("{\n");
      
        b.Put(indentation); b.Put("}\n\n");
    }
	
    
    return;
}


//
//
//
void DartAction::GenerateNullAstAllocation(TextBuffer &b, int rule_no)
{
    const char *code = "\n                    setResult(null);";
    GenerateCode(&b, code, rule_no);

    return;
}


//
// Emit the getProstheticAst() accessor that returns prosthetic-AST factories.
// This closes the loop for Dart backtracking recovery: when the backtracking
// parser replays a nonterminal ErrorToken (inserted by scope recovery for a
// %Recover symbol), it looks the factory up by getProsthesisIndex(kind) and
// invokes it with the error token to build a placeholder node instead of
// throwing. The parser/action class mixes in ProstheticAstProvider (see the
// Dart parser templates), so grammars without %Recover keep the default null.
//
void DartAction::EmitProstheticAstFactories(ActionFileSymbol *default_file_symbol)
{
    if (option -> automatic_ast == Option::NONE || grammar -> recovers.Length() == 0)
        return;

    //
    // Only nonterminal recover symbols can be replayed as prosthetic tokens
    // (kind > NT_OFFSET); a terminal recover symbol has no prosthetic factory.
    //
    Tuple<int> recover_nonterminals;
    for (int i = 0; i < grammar -> recovers.Length(); i++)
    {
        int symbol = grammar -> recovers[i];
        if (grammar -> IsNonTerminal(symbol))
            recover_nonterminals.Next() = symbol;
    }
    if (recover_nonterminals.Length() == 0)
        return;

    TextBuffer &b = *(default_file_symbol -> BodyBuffer());

    IntToString array_size(grammar -> num_nonterminals + 1);
    b.Put("\n    //\n"
          "    // Prosthetic-AST factories for %Recover nonterminals. Indexed by\n"
          "    // getProsthesisIndex(kind); unused slots stay null. Optional recover\n"
          "    // action blocks (/. ... ./) supply the create expression and may\n"
          "    // reference the parameter error_token.\n"
          "    //\n");
    b.Put("    List<ProstheticAst?>? getProstheticAst() {\n");
    b.Put("        var prostheticAst = List<ProstheticAst?>.filled(");
    b.Put(array_size.String());
    b.Put(", null);\n");
    for (int i = 0; i < recover_nonterminals.Length(); i++)
    {
        int symbol = recover_nonterminals[i];
        IntToString slot(symbol - grammar -> num_terminals);
        b.Put("        prostheticAst[");
        b.Put(slot.String());
        b.Put("] = (error_token) => ");
        EmitRecoverAllocationOrDefault(b, symbol, "", "error_token");
        b.Put(";\n");
    }
    b.Put("        return prostheticAst;\n");
    b.Put("    }\n");

    return;
}


//
//
//
void DartAction::GenerateAstAllocation(CTC &ctc,
                                       NTC& ntc,
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
               *newkey = option->factory,
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

    GenerateTerminalGcDeleteReminder(b, space, rule_no, allocation_element);
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
                    GenerateCode(&b, "null", rule_no);
                }
                else
                {
                    int symbol = grammar -> rhs_sym[offset + position[i]];
                    if (grammar -> IsTerminal(symbol))
                    {
                        const char *actual_type = ctc.FindUniqueTypeFor(type_index[i], option->ast_type);

                        GenerateCode(&b, newkey, rule_no);
                        GenerateCode(&b, grammar -> Get_ast_token_classname(), rule_no);
                        GenerateCode(&b, lparen, rule_no);
                        GenerateCode(&b, "getRhsIToken(", rule_no);
                        IntToString index(position[i]);
                        GenerateCode(&b, index.String(), rule_no);
                        GenerateCode(&b, rparen, rule_no);

                        GenerateCode(&b, rparen, rule_no);

                        if (strcmp(actual_type, grammar->Get_ast_token_classname()) != 0)
                        {
                            GenerateCode(&b, "as ", rule_no);
                            GenerateCode(&b, actual_type, rule_no);
                            if(ntc.CanProduceNullAst(type_index[i]))
                            {
                                GenerateCode(&b, "?", rule_no);
                            }
                        }
 
                    }
                    else
                    {
                        GenerateCode(&b, "getRhsSym(", rule_no);
                        IntToString index(position[i]);
                        GenerateCode(&b, index.String(), rule_no);

                        GenerateCode(&b, rparen, rule_no);

                        GenerateCode(&b, " as ", rule_no);
                        GenerateCode(&b, ctc.FindUniqueTypeFor(type_index[i], option->ast_type), rule_no);
                        if (ntc.CanProduceNullAst(type_index[i]))
                        {
                            GenerateCode(&b, "?", rule_no);
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

    delete [] extra_space;

    return;
}

//
//
//
void DartAction::GenerateListAllocation(CTC &ctc,
                                        NTC&,
                                        TextBuffer &b,
                                        int rule_no, RuleAllocationElement &allocation_element)
{
    const char *space = "\n                    ",
               *space4 = "    ",
               *newkey = option->factory,
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
                GenerateCode(&b, "getRhsIToken(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&b, index.String(), rule_no);
                GenerateCode(&b, rparen, rule_no);
                GenerateCode(&b, rparen, rule_no);
            }
            else
            {
                GenerateCode(&b, "getRhsSym(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&b, index.String(), rule_no);
                GenerateCode(&b, rparen, rule_no);
                GenerateCode(&b, " as ", rule_no);
                GenerateCode(&b, ctc.FindUniqueTypeFor(allocation_element.element_type_symbol_index, option->ast_type), rule_no);

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
            GenerateCode(&b, "getRhsSym(", rule_no);
            IntToString index(allocation_element.list_position);
            GenerateCode(&b, index.String(), rule_no);
            GenerateCode(&b, ") as ", rule_no);
            GenerateCode(&b, allocation_element.name, rule_no);
            GenerateCode(&b, ").addElement(", rule_no);
            if (grammar->IsTerminal(allocation_element.element_symbol))
            {
                GenerateCode(&b, newkey, rule_no);
                GenerateCode(&b, grammar->Get_ast_token_classname(), rule_no);
                GenerateCode(&b, lparen, rule_no);
                GenerateCode(&b, "getRhsIToken(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&b, index.String(), rule_no);
                GenerateCode(&b, rparen, rule_no);
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
                GenerateCode(&b, rparen, rule_no);
            }
            else
            {
                GenerateCode(&b, "getRhsSym(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&b, index.String(), rule_no);


                if (allocation_element.list_position != 1) // a right-recursive rule? set the list as result
                {
                    GenerateCode(&b, rparen, rule_no);
                    GenerateCode(&b, "as ", rule_no);
                    GenerateCode(&b, option->ast_type, rule_no);
                    GenerateCode(&b, trailer, rule_no);

                    GenerateCode(&b, space, rule_no);
                    GenerateCode(&b, "setResult(", rule_no);
                    GenerateCode(&b, "getRhsSym(", rule_no);
                    IntToString index(allocation_element.list_position);
                    GenerateCode(&b, index.String(), rule_no);
                    GenerateCode(&b, rparen, rule_no);
                }
                else
                {
                    GenerateCode(&b, rparen, rule_no);
                    GenerateCode(&b, "as ", rule_no);
                    GenerateCode(&b, option->ast_type, rule_no);
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
            GenerateCode(&b, "setResult(", rule_no);
            GenerateCode(&b, "getRhsSym(", rule_no);
            IntToString index(allocation_element.list_position);
            GenerateCode(&b, index.String(), rule_no);
            GenerateCode(&b, rparen, rule_no);
            GenerateCode(&b, " as ", rule_no);
            GenerateCode(&b, allocation_element.name, rule_no);
        }


    }

    GenerateCode(&b, trailer, rule_no);
 
    return;
}
