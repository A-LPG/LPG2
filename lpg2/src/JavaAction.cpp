#include "CTC.h"
#include "NTC.h"
#include "JavaAction.h"

#include <map>
#include <string>

#include "LCA.h"
#include "TTC.h"
#include "VisitorStaffFactory.h"

//
//
//
void JavaAction::ExpandExportMacro(TextBuffer *buffer, SimpleMacroSymbol *simple_macro)
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
void JavaAction::GenerateDefaultTitle(Tuple<ActionBlockElement> &notice_actions)
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
    if (option -> IsPackage())
    {
        buffer -> Put("package ");
        buffer -> Put(option -> package);
        buffer -> Put(";\n\n");
    }
    if (option -> automatic_ast &&
        option -> IsPackage() &&
        option -> ast_package != NULL &&
        strcmp(option -> package, option -> ast_package) != 0 &&
        *option -> ast_package != '\0')
    {
        buffer -> Put("import ");
        buffer -> Put(option -> ast_package);
        buffer -> Put(".*;\n");
    }

    return;
}


//
// First construct a file for this type. Write the title information to its header
// buffer and return the file.
//
ActionFileSymbol *JavaAction::GenerateTitle(ActionFileLookupTable &ast_filename_table,
                                            Tuple<ActionBlockElement> &notice_actions,
                                            const char *type_name,
                                            bool needs_environment)
{
    const char *filetype = option->GetFileTypeWithLanguage();
	
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
    if (option -> ast_package != NULL && *option -> ast_package != '\0')
    {
        buffer -> Put("package ");
        buffer -> Put(option -> ast_package);
        buffer -> Put(";\n\n");
    }

    if (needs_environment &&
        option -> IsPackage() &&
        option -> ast_package != NULL &&
        strcmp(option -> ast_package, option -> package) != 0)
    {
        buffer -> Put("import ");
        buffer -> Put(option -> package);
        buffer -> Put(".*;\n");
    }

    delete [] filename;

    return file_symbol;
}


ActionFileSymbol *JavaAction::GenerateTitleAndGlobals(ActionFileLookupTable &ast_filename_table,
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
void JavaAction::GenerateEnvironmentDeclaration(TextBuffer &b, const char *indentation)
{
    b.Put(indentation); b.Put("    private ");
                                 b.Put(option -> action_type);
                                 b.Put(" environment;\n");
    b.Put(indentation); b.Put("    public ");
                                 b.Put(option -> action_type);
                                 b.Put(" getEnvironment() { return environment; }\n\n");
}






//
//
//
void JavaAction::GenerateVisitorHeaders(TextBuffer &b, const char *indentation, const char *modifiers)
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
            b.Put(visitorFactory -> visitor_type);
            b.Put(" v);");

            b.Put("\n");

            b.Put(header);
            b.Put("void accept(");
            b.Put(visitorFactory -> argument_visitor_type);
            b.Put(" v, Object o);\n");

            b.Put(header);
            b.Put("Object accept(");
            b.Put(visitorFactory -> result_visitor_type);
            b.Put(" v);\n");

            b.Put(header);
            b.Put("Object accept(");
            b.Put(visitorFactory -> result_argument_visitor_type);
            b.Put(" v, Object o);");
        }
        b.Put("\n");

        delete [] header;
    }

    return;
}


//
//
//
void JavaAction::GenerateVisitorMethods(NTC &ntc,
                                        TextBuffer &b,
                                        const char *indentation,
                                        ClassnameElement &element,
                                        BitSet &optimizable_symbol_set)
{
    if (option -> visitor & Option::DEFAULT)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("    public void accept(");
                                     b.Put(visitorFactory -> visitor_type);
                                     b.Put(" v) { v.visit(this); }\n");

        b.Put(indentation); b.Put("    public void accept(");
                                     b.Put(visitorFactory -> argument_visitor_type);
                                     b.Put(" v, Object o) { v.visit(this, o); }\n");

        b.Put(indentation); b.Put("    public Object accept(");
                                     b.Put(visitorFactory -> result_visitor_type);
                                     b.Put(" v) { return v.visit(this); }\n");

        b.Put(indentation); b.Put("    public Object accept(");
                                     b.Put(visitorFactory -> result_argument_visitor_type);
                                     b.Put(" v, Object o) { return v.visit(this, o); }\n");
    }
    if (option -> visitor & Option::PREORDER)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("    public void accept(IAstVisitor v)\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        if (! v.preVisit(this)) return;\n");
        b.Put(indentation); b.Put("        enter((");
                                     b.Put(visitorFactory->preorder_visitor_type);
                                     b.Put(") v);\n");
        b.Put(indentation); b.Put("        v.postVisit(this);\n");
        b.Put(indentation); b.Put("    }\n\n");

        b.Put(indentation); b.Put("    public void enter(");
                                     b.Put(visitorFactory->preorder_visitor_type);
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
            b.Put(indentation); b.Put("        boolean checkChildren = v.visit(this);\n");
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
void JavaAction::GenerateGetAllChildrenMethod(TextBuffer &b,
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
        b.Put(indentation); b.Put("    public java.util.ArrayList getAllChildren()\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        java.util.ArrayList list = new java.util.ArrayList();\n");
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            b.Put(indentation);
        	b.Put("        if(_");
            b.Put(symbol_set[i]->Name());
        	b.Put(" != null) ");
            b.Put("list.add(_");b.Put(symbol_set[i] -> Name());b.Put(");\n");
        }
        b.Put(indentation); b.Put("        return list;\n");
        b.Put(indentation); b.Put("    }\n");
    }

    return;
}


//
//
//
void JavaAction::GenerateEqualsMethod(NTC &ntc,
                                      TextBuffer &b,
                                      const char *indentation,
                                      ClassnameElement &element,
                                      BitSet &optimizable_symbol_set)
{
    SymbolLookupTable &symbol_set = element.symbol_set;

    //
    // Note that if an AST node does not contain any field (symbol_set.Size() == 0),
    // we do not generate an "equals" function for it.
    //
    if ((! element.is_terminal_class) && symbol_set.Size() > 0) 
    {
        Tuple<int> &rhs_type_index = element.rhs_type_index;

        b.Put("\n");
        b.Put(indentation); b.Put("    public boolean equals(Object o)\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        if (o == this) return true;\n");
        b.Put(indentation); b.Put("        if (! (o instanceof ");
                                     b.Put(element.real_name);
                                     b.Put(")) return false;\n");
        b.Put(indentation); b.Put("        if (! super.equals(o)) return false;\n");
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
                b.Put(indentation); b.Put("            else; // continue\n");
                b.Put(indentation); b.Put("        else ");
            }
            b.Put("if (! _");
            b.Put(symbol_set[i] -> Name());
            b.Put(".equals(other._");
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
void JavaAction::GenerateHashcodeMethod(NTC &ntc,
                                        TextBuffer &b,
                                        const char *indentation,
                                        ClassnameElement &element,
                                        BitSet &optimizable_symbol_set)
{
    SymbolLookupTable &symbol_set = element.symbol_set;

    //
    // Note that if an AST node does not contain any field (symbol_set.Size() == 0),
    // we do not generate an "equals" function for it.
    //
    if ((! element.is_terminal_class) && symbol_set.Size() > 0) 
    {
        Tuple<int> &rhs_type_index = element.rhs_type_index;

        b.Put("\n");
        b.Put(indentation); b.Put("    public int hashCode()\n");
        b.Put(indentation); b.Put("    {\n");

        b.Put(indentation); b.Put("        int hash = super.hashCode();\n");
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            b.Put(indentation); b.Put("        hash = hash * 31 + (_");
            b.Put(symbol_set[i] -> Name());
            if ((! optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
            {
                b.Put(" == null ? 0 : _");
                b.Put(symbol_set[i] -> Name());
            }
            b.Put(".hashCode());\n");
        }

        b.Put(indentation); b.Put("        return hash;\n");
        b.Put(indentation); b.Put("    }\n");
    }

    return;
}


//
//
//
void JavaAction::GenerateSimpleVisitorInterface(ActionFileSymbol* ast_filename_symbol,
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
}

//
//
//
void JavaAction::GenerateArgumentVisitorInterface(ActionFileSymbol* ast_filename_symbol,
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
                                     b.Put(" n, Object o);\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    void visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" n, Object o);\n");

    b.Put(indentation); b.Put("}\n");
}

//
//
//
void JavaAction::GenerateResultVisitorInterface(ActionFileSymbol* ast_filename_symbol,
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
        b.Put(indentation); b.Put("    Object visit");
                                     b.Put("(");
                                     b.Put(symbol -> Name());
                                     b.Put(" n);\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    Object visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" n);\n");

    b.Put(indentation); b.Put("}\n");
}

//
//
//
void JavaAction::GenerateResultArgumentVisitorInterface(ActionFileSymbol* ast_filename_symbol,
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
        b.Put(indentation); b.Put("    Object visit");
                                     b.Put("(");
                                     b.Put(symbol -> Name());
                                     b.Put(" n, Object o);\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    Object visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" n, Object o);\n");

    b.Put(indentation); b.Put("}\n");
}


//
//
//
void JavaAction::GeneratePreorderVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                  const char *indentation,
                                                  const char *interface_name,
                                                  SymbolLookupTable &type_set)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    assert(option -> visitor & Option::PREORDER);
    b.Put(indentation); b.Put("public interface ");
                                 b.Put(interface_name);
                                 b.Put(" extends IAstVisitor\n");
    b.Put(indentation); b.Put("{\n");

    //    b.Put(indentation); b.Put("    boolean preVisit(");
    //                                 b.Put(option -> ast_type);
    //                                 b.Put(" element);\n");
    //
    //    b.Put(indentation); b.Put("    void postVisit(");
    //                                 b.Put(option -> ast_type);
    //                                 b.Put(" element);\n\n");

    b.Put(indentation); b.Put("    boolean visit");
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
        b.Put(indentation); b.Put("    boolean visit");
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

    return;
}


//
//
//
void JavaAction::GenerateNoResultVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                      const char *indentation,
                                                      const char *classname,
                                                      SymbolLookupTable &type_set)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    b.Put(indentation); b.Put(option -> automatic_ast == Option::NESTED ? "static " : "");
                                 b.Put("public abstract class ");
                                 b.Put(classname);
                                 b.Put(" implements ");
                                 b.Put(visitorFactory -> visitor_type);
                                 b.Put(", ");
                                 b.Put(visitorFactory -> argument_visitor_type);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("    public abstract void unimplementedVisitor(String s);\n\n");
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
                                         b.Put(" n, Object o) { unimplementedVisitor(\"visit(");
                                         b.Put(symbol -> Name());
                                         b.Put(", Object)\"); }\n");
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
                                         b.Put("if (n instanceof ");
                                         b.Put(symbol -> Name());
                                         b.Put(") visit((");
                                         b.Put(symbol -> Name());
                                         b.Put(") n);\n");
        }
    }
    b.Put(indentation); b.Put("        else throw new UnsupportedOperationException(\"visit(\" + n.getClass().toString() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");


    b.Put(indentation); b.Put("    public void visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" n, Object o)\n");
    b.Put(indentation); b.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                         b.Put(i == 0 ? "" : "else ");
                                         b.Put("if (n instanceof ");
                                         b.Put(symbol -> Name());
                                         b.Put(") visit((");
                                         b.Put(symbol -> Name());
                                         b.Put(") n, o);\n");
        }
    }
    b.Put(indentation); b.Put("        else throw new UnsupportedOperationException(\"visit(\" + n.getClass().toString() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("}\n");
}

//
//
//
void JavaAction::GenerateResultVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                    const char *indentation,
                                                    const char *classname,
                                                    SymbolLookupTable &type_set)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    b.Put(indentation); b.Put(option -> automatic_ast == Option::NESTED ? "static " : "");
                                 b.Put("public abstract class ");
                                 b.Put(classname);
                                 b.Put(" implements ");
                                 b.Put(visitorFactory -> result_visitor_type);
                                 b.Put(", ");
                                 b.Put(visitorFactory -> result_argument_visitor_type);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("    public abstract Object unimplementedVisitor(String s);\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("    public Object visit(");
                                         b.Put(symbol -> Name());
                                         b.Put(" n) { return unimplementedVisitor(\"visit(");
                                         b.Put(symbol -> Name());
                                         b.Put(")\"); }\n");
            b.Put(indentation); b.Put("    public Object visit(");
                                         b.Put(symbol -> Name());
                                         b.Put(" n, Object o) { return  unimplementedVisitor(\"visit(");
                                         b.Put(symbol -> Name());
                                         b.Put(", Object)\"); }\n");
            b.Put("\n");
        }
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    public Object visit");
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
                                         b.Put("if (n instanceof ");
                                         b.Put(symbol -> Name());
                                         b.Put(") return visit((");
                                         b.Put(symbol -> Name());
                                         b.Put(") n);\n");
        }
    }
    b.Put(indentation); b.Put("        else throw new UnsupportedOperationException(\"visit(\" + n.getClass().toString() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");


    b.Put(indentation); b.Put("    public Object visit");
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" n, Object o)\n");
    b.Put(indentation); b.Put("    {\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                         b.Put(i == 0 ? "" : "else ");
                                         b.Put("if (n instanceof ");
                                         b.Put(symbol -> Name());
                                         b.Put(") return visit((");
                                         b.Put(symbol -> Name());
                                         b.Put(") n, o);\n");
        }
    }
    b.Put(indentation); b.Put("        else throw new UnsupportedOperationException(\"visit(\" + n.getClass().toString() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("}\n");
}


//
//
//
void JavaAction::GeneratePreorderVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                      const char *indentation,
                                                      const char *classname,
                                                      SymbolLookupTable &type_set)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    assert(option -> visitor & Option::PREORDER);
    b.Put(indentation); b.Put(option -> automatic_ast == Option::NESTED ? "static " : "");
                                 b.Put("public abstract class ");
                                 b.Put(classname);
                                 b.Put(" implements ");
                                 b.Put(visitorFactory->preorder_visitor_type);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("    public abstract void unimplementedVisitor(String s);\n\n");
    b.Put(indentation); b.Put("    public boolean preVisit(IAst element) { return true; }\n\n");
    b.Put(indentation); b.Put("    public void postVisit(IAst element) {}\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("    public boolean visit");
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
    b.Put(indentation); b.Put("    public boolean visit");
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
                                         b.Put("if (n instanceof ");
                                         b.Put(symbol -> Name());
                                         b.Put(") return visit((");
                                         b.Put(symbol -> Name());
                                         b.Put(") n);\n");
        }
    }
    b.Put(indentation); b.Put("        else throw new UnsupportedOperationException(\"visit(\" + n.getClass().toString() + \")\");\n");
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
                                         b.Put("if (n instanceof ");
                                         b.Put(symbol -> Name());
                                         b.Put(") endVisit((");
                                         b.Put(symbol -> Name());
                                         b.Put(") n);\n");
        }
    }
    b.Put(indentation); b.Put("        else throw new UnsupportedOperationException(\"visit(\" + n.getClass().toString() + \")\");\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("}\n");

    return;
}


//
// Generate the the Ast root classes
//
void JavaAction::GenerateAstType(ActionFileSymbol* ast_filename_symbol,
                                 const char *indentation,
                                 const char *classname)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    /*
     * First, generate the main root class
     */
    b.Put(indentation); b.Put(option -> automatic_ast == Option::NESTED ? "static " : "");
                                 b.Put("public abstract class ");
                                 b.Put(classname);
                                 b.Put(" implements IAst\n");
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
	    b.Put(indentation).Put("    public IAst getNextAst() { return null; }\n");
    }

    b.Put(indentation); b.Put("    protected IToken leftIToken,\n");
    b.Put(indentation); b.Put("                     rightIToken;\n");
    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("    protected IAst parent = null;\n");
        b.Put(indentation); b.Put("    protected void setParent(IAst parent) { this.parent = parent; }\n");
        b.Put(indentation); b.Put("    public IAst getParent() { return parent; }\n");\
    }
    else
    {
        b.Put(indentation); b.Put("    public IAst getParent()\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        throw new UnsupportedOperationException(\"noparent-saved option in effect\");\n");
        b.Put(indentation); b.Put("    }\n");
    }

    b.Put("\n");
    b.Put(indentation); b.Put("    public IToken getLeftIToken() { return leftIToken; }\n");
    b.Put(indentation); b.Put("    public IToken getRightIToken() { return rightIToken; }\n");
    b.Put(indentation); b.Put("    public IToken[] getPrecedingAdjuncts() { return leftIToken.getPrecedingAdjuncts(); }\n");
    b.Put(indentation); b.Put("    public IToken[] getFollowingAdjuncts() { return rightIToken.getFollowingAdjuncts(); }\n\n");

    b.Put(indentation); b.Put("    public String toString()\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        return leftIToken.getILexStream().toString(leftIToken.getStartOffset(), rightIToken.getEndOffset());\n");
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
        b.Put(indentation); b.Put("     * A list of all children of this node, excluding the null ones.\n");
        b.Put(indentation); b.Put("     */\n");
        b.Put(indentation); b.Put("    public java.util.ArrayList getChildren()\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        java.util.ArrayList list = getAllChildren();\n");
        b.Put(indentation); b.Put("        int k = -1;\n");
        b.Put(indentation); b.Put("        for (int i = 0; i < list.size(); i++)\n");
        b.Put(indentation); b.Put("        {\n");
        b.Put(indentation); b.Put("            Object element = list.get(i);\n");
        b.Put(indentation); b.Put("            if (element != null)\n");
        b.Put(indentation); b.Put("            {\n");
        b.Put(indentation); b.Put("                if (++k != i)\n");
        b.Put(indentation); b.Put("                    list.set(k, element);\n");
        b.Put(indentation); b.Put("            }\n");
        b.Put(indentation); b.Put("        }\n");
        b.Put(indentation); b.Put("        for (int i = list.size() - 1; i > k; i--) // remove extraneous elements\n");
        b.Put(indentation); b.Put("            list.remove(i);\n");
        b.Put(indentation); b.Put("        return list;\n");
        b.Put(indentation); b.Put("    }\n\n");

        b.Put(indentation); b.Put("    /**\n");
        b.Put(indentation); b.Put("     * A list of all children of this node, including the null ones.\n");
        b.Put(indentation); b.Put("     */\n");
        b.Put(indentation); b.Put("    public abstract java.util.ArrayList getAllChildren();\n");
    }
    else
    {
        b.Put(indentation); b.Put("    public java.util.ArrayList getChildren()\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        throw new UnsupportedOperationException(\"noparent-saved option in effect\");\n");
        b.Put(indentation); b.Put("    }\n");
        b.Put(indentation); b.Put("    public java.util.ArrayList getAllChildren() { return getChildren(); }\n");
    }

    b.Put("\n");

    b.Put(indentation); b.Put("    public boolean equals(Object o)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        if (o == this) return true;\n");
    b.Put(indentation); b.Put("        if (! (o instanceof ");
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

    b.Put(indentation); b.Put("    public int hashCode()\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        int hash = 7;\n");
    b.Put(indentation); b.Put("        if (getLeftIToken().getILexStream() != null) hash = hash * 31 + getLeftIToken().getILexStream().hashCode();\n");
    b.Put(indentation); b.Put("        hash = hash * 31 + getLeftIToken().getTokenIndex();\n");
    b.Put(indentation); b.Put("        if (getRightIToken().getILexStream() != null) hash = hash * 31 + getRightIToken().getILexStream().hashCode();\n");
    b.Put(indentation); b.Put("        hash = hash * 31 + getRightIToken().getTokenIndex();\n");
    b.Put(indentation); b.Put("        return hash;\n");
    b.Put(indentation); b.Put("    }\n");

    GenerateVisitorHeaders(b, indentation, "    public abstract ");

    b.Put(indentation); b.Put("    public int getRuleIndex(){ return 0; }\n");
    //
    // Not Preorder visitor? generate dummy accept method to satisfy IAst abstract declaration of accept(IAstVisitor);

    if (!(option -> visitor & Option::PREORDER) )
    {
        b.Put(indentation); b.Put("    public void accept(IAstVisitor v) {}\n");
    }
    b.Put(indentation); b.Put("}\n\n");

    return;
}

static std::string replaceAll(const std::string& s, const std::string& var, const std::string& subst)
{
    std::string result;
    int pos = 0;
    int varLen = var.length();

    do {
        size_t idx = s.find(var, pos);
        if (idx == std::string::npos) {
            result += s.substr(pos);
            break;
        } else {
            result += s.substr(pos, idx - pos);
            result += subst;
            pos = idx + varLen;
        }
    } while (true);
    return result;
}

typedef std::map<std::string, std::string> Substitutions;

static std::string doReplacements(const std::string& s, const Substitutions& replacements)
{
    std::string result= s;

    for (Substitutions::const_iterator i = replacements.begin(); i != replacements.end(); i++) {
        std::string k = (*i).first;
        std::string v = (*i).second;
        result = replaceAll(result, k, v);
    }
    return result;
}

static void PutWithIndentation(TextBuffer& buffer, const std::string& s, const char *indentation)
{
    for (size_t pos=0; pos < s.length(); ) {
        size_t idx = s.find('\n', pos);
        size_t end = (idx == std::string::npos) ? s.length() : idx + 1;

        buffer.Put(indentation);
        buffer.Put(s.c_str() + pos, end - pos);
        pos = end;
    }
}

//
// Generate the the Ast list class
//
void JavaAction::GenerateAbstractAstListType(ActionFileSymbol* ast_filename_symbol,
                                             const char *indentation,
                                             const char *classname)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    /*
     * Generate the List root class
     */
    b.Put(indentation); b.Put(option -> automatic_ast == Option::NESTED ? "static " : "");
                                 b.Put("public abstract class ");
                                 b.Put(this -> abstract_ast_list_classname);
                                 b.Put(" extends ");
                                 b.Put(option -> ast_type);
                                 b.Put(" implements IAbstractArrayList<");
                                 b.Put(option -> ast_type);
                                 b.Put(">\n");
    b.Put(indentation); b.Put("{\n");
    b.Put(indentation); b.Put("    private boolean leftRecursive;\n");
    b.Put(indentation); b.Put("    private java.util.ArrayList list;\n");
    b.Put(indentation); b.Put("    public int size() { return list.size(); }\n");
    b.Put(indentation); b.Put("    public java.util.List getList() { return list; }\n");
    b.Put(indentation); b.Put("    public ");
                                 b.Put(option -> ast_type);
                                 b.Put(" getElementAt(int i) { return (");
                                 b.Put(option -> ast_type);
                                 b.Put(") list.get(leftRecursive ? i : list.size() - 1 - i); }\n");
    b.Put(indentation); b.Put("    public java.util.ArrayList getArrayList()\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        if (! leftRecursive) // reverse the list \n");
    b.Put(indentation); b.Put("        {\n");
    b.Put(indentation); b.Put("            for (int i = 0, n = list.size() - 1; i < n; i++, n--)\n");
    b.Put(indentation); b.Put("            {\n");
    b.Put(indentation); b.Put("                Object ith = list.get(i),\n");
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
    b.Put(indentation); b.Put("    public boolean add(");
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
    b.Put(indentation); b.Put("        list.add(element);\n");
    b.Put(indentation); b.Put("        if (leftRecursive)\n");
    b.Put(indentation); b.Put("             rightIToken = element.getRightIToken();\n");
    b.Put(indentation); b.Put("        else leftIToken = element.getLeftIToken();\n");
    b.Put(indentation); b.Put("    }\n\n");

    // generate constructors for list class
    b.Put(indentation); b.Put("    public ");
                                 b.Put(classname);
                                 b.Put("(IToken leftIToken, IToken rightIToken, boolean leftRecursive)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        super(leftIToken, rightIToken);\n");
    b.Put(indentation); b.Put("        this.leftRecursive = leftRecursive;\n");
    b.Put(indentation); b.Put("        list = new java.util.ArrayList();\n");
    b.Put(indentation); b.Put("    }\n\n");

    b.Put(indentation); b.Put("    public ");
                                 b.Put(classname);
                                 b.Put("(");
                                 b.Put(option -> ast_type);
                                 b.Put(" element, boolean leftRecursive)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        this(element.getLeftIToken(), element.getRightIToken(), leftRecursive);\n");
    b.Put(indentation); b.Put("        list.add(element);\n");
    b.Put(indentation); b.Put("    }\n\n");

    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("    /**\n");
        b.Put(indentation); b.Put("     * Make a copy of the list and return it. Note that we obtain the local list by\n");
        b.Put(indentation); b.Put("     * invoking getArrayList so as to make sure that the list we return is in proper order.\n");
        b.Put(indentation); b.Put("     */\n");
        b.Put(indentation); b.Put("    public java.util.ArrayList getAllChildren()\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        return (java.util.ArrayList) getArrayList().clone();\n");
        b.Put(indentation); b.Put("    }\n\n");
    }

    //
    // Implementation for functions in java.util.List
    //

    Substitutions subs;
    subs["%%AstType%%"] = option->ast_type;
    subs["%%ListClassName%%"] = classname;
    
    const char *iterDeclTemplate =
    "    private class Itr implements java.util.Iterator<%%AstType%%> {\n"
    "        java.util.Iterator<%%AstType%%> itr = list.iterator();\n"
    "        public boolean hasNext() {\n"
    "            return itr.hasNext();\n"
    "        }\n"
    "        public %%AstType%% next() {\n"
    "            return itr.next();\n"
    "        }\n"
    "        public void remove() {\n"
    "            throw new UnsupportedOperationException();\n"
    "        }\n"
    "    }\n";

    std::string iterDecl = doReplacements(iterDeclTemplate, subs);

    PutWithIndentation(b, iterDecl, indentation);
    
//    b.Put(indentation); b.Put("    private class Itr implements java.util.Iterator<");
//                                 b.Put(option -> ast_type);
//                                 b.Put("> {\n");
//    b.Put(indentation); b.Put("        java.util.Iterator<");
//                                 b.Put(option -> ast_type);
//                                 b.Put("> itr = list.iterator();\n");
//
//    b.Put(indentation); b.Put("        public boolean hasNext() {\n");
//    b.Put(indentation); b.Put("            return itr.hasNext();\n");
//    b.Put(indentation); b.Put("        }\n");
//
//    b.Put(indentation); b.Put("        public ");
//                                 b.Put(option -> ast_type);
//                                 b.Put(" next() {\n");
//    b.Put(indentation); b.Put("            return itr.next();\n");
//    b.Put(indentation); b.Put("        }\n");
//
//    b.Put(indentation); b.Put("        public void remove() {\n");
//    b.Put(indentation); b.Put("            throw new UnsupportedOperationException();\n");
//    b.Put(indentation); b.Put("         }\n");
//    b.Put(indentation); b.Put("    }\n\n");

    const char *listIterDeclTemplate =
    "    private class ListItr extends Itr implements java.util.ListIterator<%%AstType%%> {\n"
    "        java.util.ListIterator<%%AstType%%> list_itr;\n"
    "        ListItr(int index) {\n"
    "            list_itr = list.listIterator(index);\n"
    "        }\n"
    "        public boolean hasPrevious() {\n"
    "            return list_itr.hasPrevious();\n"
    "        }\n"
    "        public %%AstType%% previous() {\n"
    "            return list_itr.previous();\n"
    "        }\n"
    "        public int nextIndex() {\n"
    "            return list_itr.nextIndex();\n"
    "        }\n"
    "        public int previousIndex() {\n"
    "            return list_itr.previousIndex();\n"
    "        }\n"
    "        public void set(%%AstType%% o) {\n"
    "            throw new UnsupportedOperationException();\n"
    "        }\n"
    "        public void add(%%AstType%% o) {\n"
    "            throw new UnsupportedOperationException();\n"
    "        }\n"
    "    }\n";

    std::string listIterDecl = doReplacements(listIterDeclTemplate, subs);

    PutWithIndentation(b, listIterDecl, indentation);
    
//    b.Put(indentation); b.Put("    private class ListItr extends Itr implements java.util.ListIterator<");
//                                 b.Put(option -> ast_type);
//                                 b.Put("> {\n");
//    b.Put(indentation); b.Put("        java.util.ListIterator<");
//                                 b.Put(option -> ast_type);
//                                 b.Put("> list_itr;\n");
//
//    b.Put(indentation); b.Put("        ListItr(int index) {\n");
//    b.Put(indentation); b.Put("            list_itr = list.listIterator(index);\n");
//    b.Put(indentation); b.Put("        }\n");
//
//    b.Put(indentation); b.Put("        public boolean hasPrevious() {\n");
//    b.Put(indentation); b.Put("            return list_itr.hasPrevious();\n");
//    b.Put(indentation); b.Put("        }\n");
//
//    b.Put(indentation); b.Put("        public ");
//                                 b.Put(option -> ast_type);
//                                 b.Put(" previous() {\n");
//    b.Put(indentation); b.Put("            return list_itr.previous();\n");
//    b.Put(indentation); b.Put("        }\n");
//
//    b.Put(indentation); b.Put("        public int nextIndex() {\n");
//    b.Put(indentation); b.Put("            return list_itr.nextIndex();\n");
//    b.Put(indentation); b.Put("        }\n\n");
//
//    b.Put(indentation); b.Put("        public int previousIndex() {\n");
//    b.Put(indentation); b.Put("            return list_itr.previousIndex();\n");
//    b.Put(indentation); b.Put("        }\n");
//
//    b.Put(indentation); b.Put("        public void set(");
//                                 b.Put(option -> ast_type);
//                                 b.Put(" o) {\n");
//    b.Put(indentation); b.Put("            throw new UnsupportedOperationException();\n");
//    b.Put(indentation); b.Put("        }\n");
//
//    b.Put(indentation); b.Put("        public void add(");
//                                 b.Put(option -> ast_type);
//                                 b.Put(" o) {\n");
//    b.Put(indentation); b.Put("            throw new UnsupportedOperationException();\n");
//    b.Put(indentation); b.Put("        }\n");
//    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    public boolean isEmpty() {\n");
    b.Put(indentation); b.Put("        return list.isEmpty();\n");
    b.Put(indentation); b.Put("    }\n");
        
    b.Put(indentation); b.Put("    public boolean contains(Object o) {\n");
    b.Put(indentation); b.Put("        return list.contains(o);\n");
    b.Put(indentation); b.Put("    }\n");
        
    b.Put(indentation); b.Put("    public java.util.Iterator<");
                                 b.Put(option -> ast_type);
                                 b.Put("> iterator() {\n");
    b.Put(indentation); b.Put("        return new Itr();\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    public Object[] toArray() {\n");
    b.Put(indentation); b.Put("        return getArrayList().toArray();\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    public <T> T[] toArray(T[] a) {\n");
    b.Put(indentation); b.Put("        return (T[]) getArrayList().toArray(a);\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    public boolean remove(Object o) {\n");
    b.Put(indentation); b.Put("        throw new UnsupportedOperationException();\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    public boolean containsAll(java.util.Collection<?> c) {\n");
    b.Put(indentation); b.Put("        return list.containsAll(c);\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    public boolean addAll(java.util.Collection<? extends ");
                                 b.Put(option -> ast_type);
                                 b.Put("> c) {\n");
    b.Put(indentation); b.Put("        throw new UnsupportedOperationException();\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    public boolean addAll(int index, java.util.Collection<? extends ");
                                 b.Put(option -> ast_type);
                                 b.Put("> c) {\n");
    b.Put(indentation); b.Put("        throw new UnsupportedOperationException();\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    public boolean removeAll(java.util.Collection<?> c) {\n");
    b.Put(indentation); b.Put("        throw new UnsupportedOperationException();\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    public boolean retainAll(java.util.Collection<?> c) {\n");
    b.Put(indentation); b.Put("        throw new UnsupportedOperationException();\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    public void clear() {\n");
    b.Put(indentation); b.Put("        throw new UnsupportedOperationException();\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    public ");
                                 b.Put(option -> ast_type);
                                 b.Put(" get(int index) {\n");
    b.Put(indentation); b.Put("        return getElementAt(index);\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    public ");
                                 b.Put(option -> ast_type);
                                 b.Put(" set(int index, ");
                                 b.Put(option -> ast_type);
                                 b.Put(" element) {\n");
    b.Put(indentation); b.Put("        throw new UnsupportedOperationException();\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    public void add(int index, ");
                                 b.Put(option -> ast_type);
                                 b.Put(" element) {\n");
    b.Put(indentation); b.Put("        throw new UnsupportedOperationException();\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    public ");
                                 b.Put(option -> ast_type);
                                 b.Put(" remove(int index) {\n");
    b.Put(indentation); b.Put("        throw new UnsupportedOperationException();\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    public int indexOf(Object o) {\n");
    b.Put(indentation); b.Put("        return getArrayList().indexOf(o);\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    public int lastIndexOf(Object o) {\n");
    b.Put(indentation); b.Put("        return getArrayList().lastIndexOf(o);\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    public java.util.ListIterator<");
                                 b.Put(option -> ast_type);
                                 b.Put("> listIterator() {\n");
    b.Put(indentation); b.Put("        return new ListItr(0);\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    public java.util.ListIterator<");
                                 b.Put(option -> ast_type);
                                 b.Put("> listIterator(int index) {\n");
    b.Put(indentation); b.Put("        return new ListItr(index);\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("    public java.util.List<");
                                 b.Put(option -> ast_type);
                                 b.Put("> subList(int fromIndex, int toIndex) {\n");
    b.Put(indentation); b.Put("        return getArrayList().subList(fromIndex, toIndex);\n");
    b.Put(indentation); b.Put("    }\n");

    b.Put(indentation); b.Put("}\n\n");

    return;
}


//
// Generate the the Ast token class
//
void JavaAction::GenerateAstTokenType(NTC &ntc, ActionFileSymbol* ast_filename_symbol,
                                      const char *indentation,
                                      const char *classname)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    /*
     * Generate the Token root class
     */
    b.Put(indentation); b.Put(option -> automatic_ast == Option::NESTED ? "static " : "");
                                 b.Put("public class ");
                                 b.Put(classname);
                                 b.Put(" extends ");
                                 b.Put(option -> ast_type);
                                 b.Put(" implements I");
                                 b.Put(classname);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    b.Put(indentation); b.Put("    public ");
                                 b.Put(classname);
                                 b.Put("(IToken token) { super(token); }\n");
    b.Put(indentation); b.Put("    public IToken getIToken() { return leftIToken; }\n");
    b.Put(indentation); b.Put("    public String toString() { return leftIToken.toString(); }\n\n");

    ClassnameElement element; // generate a temporary element with no symbols in its symbol set.
    element.real_name = (char *) classname;
    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);

    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("    /**\n");
        b.Put(indentation); b.Put("     * A token class has no children. So, we return the empty list.\n");
        b.Put(indentation); b.Put("     */\n");
        b.Put(indentation); b.Put("    public java.util.ArrayList getAllChildren() { return new java.util.ArrayList(); }\n\n");
    }

    b.Put(indentation); b.Put("    public boolean equals(Object o)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        if (o == this) return true;\n");
    b.Put(indentation); b.Put("        if (! (o instanceof ");
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

    b.Put(indentation); b.Put("    public int hashCode()\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        int hash = 7;\n");
    b.Put(indentation); b.Put("        if (getIToken().getILexStream() != null) hash = hash * 31 + getIToken().getILexStream().hashCode();\n");
    b.Put(indentation); b.Put("        hash = hash * 31 + getIToken().getTokenIndex();\n");
    b.Put(indentation); b.Put("        return hash;\n");
    b.Put(indentation); b.Put("    }\n");

    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set);

    b.Put(indentation); b.Put("}\n\n");

    return;
}


//
//
//
void JavaAction::GenerateCommentHeader(TextBuffer &b,
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


void JavaAction::GenerateListMethods(CTC &ctc,
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
    b.Put(indentation); b.Put("        super.addElement((");
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
    // Generate the "equals" method for this list
    //
    b.Put("\n");
    b.Put(indentation); b.Put("    public boolean equals(Object o)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        if (o == this) return true;\n");
    b.Put(indentation); b.Put("        if (! (o instanceof ");
                                 b.Put(classname);
                                 b.Put(")) return false;\n");
    b.Put(indentation); b.Put("        if (! super.equals(o)) return false;\n");
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
    b.Put(indentation); b.Put("if (! element.equals(other.get");
                                 b.Put(element_name);
                                 b.Put("At(i))) return false;\n");
    b.Put(indentation); b.Put("        }\n");
    b.Put(indentation); b.Put("        return true;\n");
    b.Put(indentation); b.Put("    }\n");

    //
    // Generate the "hashCode" method for a list node
    //
    b.Put("\n");
    b.Put(indentation); b.Put("    public int hashCode()\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        int hash = super.hashCode();\n");
    b.Put(indentation); b.Put("        for (int i = 0; i < size(); i++)\n");
    b.Put(indentation); b.Put("            hash = hash * 31 + (get");
                                 b.Put(element_name);
    if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
    {
        b.Put("At(i) == null ? 0 : get");
        b.Put(element_name);
    }
    b.Put("At(i).hashCode());\n");
    b.Put(indentation); b.Put("        return hash;\n");
    b.Put(indentation); b.Put("    }\n");

    //
    // Generate visitor methods.
    //
    if (option -> visitor & Option::DEFAULT)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("    public void accept(");
                                     b.Put(visitorFactory -> visitor_type);
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

        b.Put(indentation); b.Put("    public void accept(");
                                     b.Put(visitorFactory -> argument_visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
            b.Put(" v, Object o) { for (int i = 0; i < size(); i++) v.visit"
                           "("
                           "get");
            b.Put(element_name);
            b.Put("At(i), o");
            b.Put("); }\n");
        }
        else
        {
            b.Put(" v, Object o) { for (int i = 0; i < size(); i++) get");
            b.Put(element_name);
            b.Put("At(i).accept(v, o); }\n");
        }

        //
        // Code cannot be generated to automatically visit a node that
        // can return a value. These cases are left up to the user.
        //
        b.Put(indentation); b.Put("    public Object accept(");
                                     b.Put(visitorFactory -> result_visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
                                         b.Put(" v)\n");
            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        java.util.ArrayList result = new java.util.ArrayList();\n");
            b.Put(indentation); b.Put("        for (int i = 0; i < size(); i++)\n");
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
            b.Put(indentation); b.Put("        java.util.ArrayList result = new java.util.ArrayList();\n");
            b.Put(indentation); b.Put("        for (int i = 0; i < size(); i++)\n");
            b.Put(indentation); b.Put("            result.add(get");
                                         b.Put(element_name);
                                         b.Put("At(i).accept(v));\n");
            b.Put(indentation); b.Put("        return result;\n");
            b.Put(indentation); b.Put("    }\n");
        }

        b.Put(indentation); b.Put("    public Object accept(");
                                     b.Put(visitorFactory -> result_argument_visitor_type);
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
                                         b.Put(" v, Object o)\n");
            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        java.util.ArrayList result = new java.util.ArrayList();\n");
            b.Put(indentation); b.Put("        for (int i = 0; i < size(); i++)\n");
            b.Put(indentation); b.Put("            result.add(v.visit(get");
                                         b.Put(element_name);
                                         b.Put("At(i), o));\n");
            b.Put(indentation); b.Put("        return result;\n");
            b.Put(indentation); b.Put("    }\n");
        }
        else
        {
                                         b.Put(" v, Object o)\n");
            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        java.util.ArrayList result = new java.util.ArrayList();\n");
            b.Put(indentation); b.Put("        for (int i = 0; i < size(); i++)\n");
            b.Put(indentation); b.Put("            result.add(get");
                                         b.Put(element_name);
                                         b.Put("At(i).accept(v, o));\n");
            b.Put(indentation); b.Put("        return result;\n");
            b.Put(indentation); b.Put("    }\n");
        }
    }
    if (option -> visitor & Option::PREORDER)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("    public void accept(IAstVisitor v)\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        if (! v.preVisit(this)) return;\n");
        b.Put(indentation); b.Put("        enter((");
                                     b.Put(visitorFactory->preorder_visitor_type);
                                     b.Put(") v);\n");
        b.Put(indentation); b.Put("        v.postVisit(this);\n");
        b.Put(indentation); b.Put("    }\n");
        b.Put(indentation); b.Put("    public void enter(");
                                     b.Put(visitorFactory->preorder_visitor_type);
                                     b.Put(" v)\n");
        b.Put(indentation); b.Put("    {\n");
        b.Put(indentation); b.Put("        boolean checkChildren = v.visit(this);\n");
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
void JavaAction::GenerateListClass(CTC &ctc,
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

    b.Put(indentation); b.Put(option -> automatic_ast == Option::NESTED ? "static " : "");
                                 b.Put("public class ");
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
                                 b.Put("IToken leftIToken, IToken rightIToken, boolean leftRecursive)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        super(leftIToken, rightIToken, leftRecursive);\n");
    b.Put(indentation); b.Put("    }\n\n");

    b.Put(indentation); b.Put("    public ");
                                 b.Put(classname);
                                 b.Put("(");
                                 b.Put(element_type);
                                 b.Put(" _");
                                 b.Put(element_name);
                                 b.Put(", boolean leftRecursive)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        super((");
                                 b.Put(option -> ast_type);
                                 b.Put(") _");
                                 b.Put(element_name);
                                 b.Put(", leftRecursive);\n");
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
    b+ "    @Override public  int getRuleIndex() { return " + num.String() + " ;}\n";

    b.Put("    }\n\n");// Generate Class Closer
    if (option->IsTopLevel())
    {
        ast_filename_symbol->Flush();
    }
    return;
}


//
// Generate a class that extends a basic list class. This is necessary when the user
// specifies action blocks to be associated with a generic list class - in which case,
// we have to generate a (new) unique class (that extends the generic class) to hold the content
// of the action blocks.
//
void JavaAction::GenerateListExtensionClass(CTC &ctc,
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

    b.Put(indentation); b.Put(option -> automatic_ast == Option::NESTED ? "static " : "");
                                 b.Put("public class ");
                                 b.Put(special_array.name);
                                 b.Put(" extends ");
                                 b.Put(classname);
                                 b.Put("\n");
    b.Put(indentation); b.Put("{\n");

    GenerateEnvironmentDeclaration(b, indentation);

    b.Put(indentation); b.Put("    public ");
                                 b.Put(special_array.name);
                                 b.Put("(");
                                 b.Put(option -> action_type);
                                 b.Put(" environment, ");
                                 b.Put("IToken leftIToken, IToken rightIToken, boolean leftRecursive)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        super(leftIToken, rightIToken, leftRecursive);\n");
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
                                 b.Put(", boolean leftRecursive)\n");
    b.Put(indentation); b.Put("    {\n");
    b.Put(indentation); b.Put("        super(_");
                                 b.Put(element_name);
                                 b.Put(", leftRecursive);\n");
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
    b+ "    @Override public  int getRuleIndex() { return " + num.String() + " ;}\n";
    return;
}


//
// Generate a generic rule class
//
void JavaAction::GenerateRuleClass(CTC &ctc,
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

    b.Put(indentation); b.Put(option -> automatic_ast == Option::NESTED ? "static " : "");
                                 b.Put("public class ");
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
            b.Put(indentation); b.Put("    {\n");
            b.Put(indentation); b.Put("        super(token);\n");
            b.Put(indentation); b.Put("        this.environment = environment;\n");
            b.Put(indentation); b.Put("        initialize();\n");
            b.Put(indentation); b.Put("    }\n");
        }
        else b.Put("IToken token) { super(token); initialize(); }\n");
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
    IntToString num(element.GetRuleNo());

    b+ "    @Override public  int getRuleIndex() { return " + num.String() + " ;}\n";
    return;
}


//
// Generate Ast class
//
void JavaAction::GenerateTerminalMergedClass(NTC &ntc,
                                             ActionFileSymbol* ast_filename_symbol,
                                             const char *indentation,
                                             ClassnameElement &element,
                                             Array<const char *> &typestring)
{
    TextBuffer& b = *(ast_filename_symbol->BodyBuffer());
    char *classname = element.real_name;
    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);

    b.Put(indentation); b.Put(option -> automatic_ast == Option::NESTED ? "static " : "");
                                 b.Put("public class ");
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
                                     b.Put(indentation); b.Put("    {\n");
                                     b.Put(indentation); b.Put("        super(token);\n");
                                     b.Put(indentation); b.Put("        this.environment = environment;\n");
                                     b.Put(indentation); b.Put("        initialize();\n");
                                     b.Put(indentation); b.Put("    }\n");
                                 }
                                 else b.Put("IToken token) { super(token); initialize(); }\n");

    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);
    GenerateHashcodeMethod(ntc, b, indentation, element, optimizable_symbol_set);
    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set);
    b.Put(indentation);
    IntToString num(element.GetRuleNo());

    b+ "    @Override public  int getRuleIndex() { return " + num.String() + " ;}\n";
    return;
}


//
// Generate Ast class
//
void JavaAction::GenerateMergedClass(CTC &ctc,
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

    b.Put(indentation); b.Put(option -> automatic_ast == Option::NESTED ? "static " : "");
                                 b.Put("public class ");
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
    b+ "    @Override public  int getRuleIndex() { return " + num.String() + " ;}\n";

    return;
}


void JavaAction::GenerateInterface(bool is_terminal,
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
        b.Put("\n");
        b.Put(indentation); b.Put("{\n");
        b.Put(indentation); b.Put("    public IToken getLeftIToken();\n");
        b.Put(indentation); b.Put("    public IToken getRightIToken();\n");
        b.Put("\n");
        GenerateVisitorHeaders(b, indentation, "    ");
        b.Put(indentation); b.Put("}\n\n");
    }

    return;
}


//
//
//
void JavaAction::GenerateNullAstAllocation(TextBuffer &b, int rule_no)
{
    const char *code = "\n                    setResult(null);";
    GenerateCode(&b, code, rule_no);

    return;
}


//
// Emit the prosthetic-AST factory array and the getProstheticAst() accessor.
// This closes the loop for Java backtracking recovery: when the backtracking
// parser replays a nonterminal ErrorToken (inserted by scope recovery for a
// %Recover symbol), it looks the factory up by getProsthesisIndex(kind) and
// calls create(error_token) to build a placeholder node instead of throwing.
//
void JavaAction::EmitProstheticAstFactories(ActionFileSymbol *default_file_symbol)
{
    //
    // Only wire this up when the grammar asks for automatic AST generation and
    // declares %Recover symbols; otherwise RuleAction.getProstheticAst() keeps
    // returning null and the parser retains its historical throw behavior.
    //
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
          "    // ParseTable.getProsthesisIndex(kind); unused slots stay null.\n"
          "    // Optional recover action blocks (/. ... ./) supply the create()\n"
          "    // expression and may reference the parameter error_token.\n"
          "    //\n");
    b.Put("    ProstheticAst prostheticAst[] = new ProstheticAst[");
    b.Put(array_size.String());
    b.Put("];\n");
    b.Put("    {\n");
    for (int i = 0; i < recover_nonterminals.Length(); i++)
    {
        int symbol = recover_nonterminals[i];
        IntToString slot(symbol - grammar -> num_terminals);
        b.Put("        prostheticAst[");
        b.Put(slot.String());
        b.Put("] = new ProstheticAst() { public IAst create(IToken error_token) { return ");

        int block_token = grammar -> RecoverAllocationBlock(symbol);
        if (block_token != 0)
        {
            BlockSymbol *block = lex_stream -> GetBlockSymbol(block_token);
            int start = lex_stream -> StartLocation(block_token) + block -> BlockBeginLength(),
                end = lex_stream -> EndLocation(block_token) - block -> BlockEndLength() + 1;
            const char *head = &(lex_stream -> InputBuffer(block_token)[start]),
                       *tail = &(lex_stream -> InputBuffer(block_token)[end]);
            while (head < tail && (*head == ' ' || *head == '\t' || *head == '\n' || *head == '\r'))
                head++;
            while (tail > head && (*(tail - 1) == ' ' || *(tail - 1) == '\t' ||
                                   *(tail - 1) == '\n' || *(tail - 1) == '\r'))
                tail--;
            b.Put(head, (int)(tail - head));
        }
        else
        {
            b.Put("new ");
            b.Put(grammar -> Get_ast_token_classname());
            b.Put("(error_token)");
        }
        b.Put("; } };\n");
    }
    b.Put("    }\n");
    b.Put("    public ProstheticAst[] getProstheticAst() { return prostheticAst; }\n");

    return;
}


//
//
//
void JavaAction::GenerateAstAllocation(CTC &ctc,
                                       NTC&,
                                       TextBuffer &b,
                                       RuleAllocationElement &allocation_element,
                                       Tuple<ProcessedRuleElement> &processed_rule_elements,
                                       Array<const char *> &/*typestring*/, int rule_no)
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
        GenerateCode(&b, option -> action_type, rule_no);
        GenerateCode(&b, ".this, ", rule_no);
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
void JavaAction::GenerateListAllocation(CTC &ctc,
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
            GenerateCode(&b, option -> action_type, rule_no);
            GenerateCode(&b, ".this, ", rule_no);
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
