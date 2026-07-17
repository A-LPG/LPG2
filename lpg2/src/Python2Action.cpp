#include "CTC.h"
#include "NTC.h"
#include "Python2Action.h"
#include "VisitorStaffFactory.h"
#include <map>
#include <string>

#include "LCA.h"
#include "TTC.h"

TextBuffer* Python2Action::GetBuffer(ActionFileSymbol* ast_filename_symbol) const
{
    if (option->IsTopLevel())
    {
        return  (ast_filename_symbol->BodyBuffer());
    }
	return (ast_filename_symbol->BufferForNestAst());
    
}
void Python2Action::ProcessCodeActionEnd()
{

}

//
// Shared ProcessAstActions hooks. Python emits interfaces before the root types
// and orders the nonterminal interfaces with extensions last.
//
TextBuffer *Python2Action::AstCodeBuffer(ActionFileSymbol *file)
{
    return GetBuffer(file);
}

void Python2Action::EmitAstClassCloser(TextBuffer &code_buf, ActionFileSymbol * /*top_level_file*/, bool /*list_extension_closer*/)
{
    code_buf.Put("    \n\n");
}

void Python2Action::MaybeEmitAstRootInterface(ActionFileLookupTable &ast_filename_table,
                                              ActionFileSymbol *default_file_symbol,
                                              Tuple<ActionBlockElement> &notice_actions)
{
    const char *indentation = (option->IsNested() ? (char*)"    " : (char*)"");
    astRootInterfaceName.append("IRootFor");
    astRootInterfaceName += option->action_type;
    if (option->IsNested())
        GenerateAstRootInterface(default_file_symbol, indentation);
    else
    {
        ActionFileSymbol* file_symbol = GenerateTitleAndGlobals(ast_filename_table, notice_actions,
            astRootInterfaceName.c_str(), false);
        GenerateAstRootInterface(file_symbol, indentation);
        file_symbol->Flush();
    }
}

//
//
//
void Python2Action::ExpandExportMacro(TextBuffer *buffer, SimpleMacroSymbol *simple_macro)
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
void Python2Action::GenerateDefaultTitle(Tuple<ActionBlockElement> &notice_actions)
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
        buffer->Put(" from ");
        buffer->Put(option->ast_package);
        buffer->Put("import  * as ");
        buffer->Put(option->ast_package);
        buffer -> Put("\"\n");
    }

    return;
}


//
// First construct a file for this type. Write the title information to its header
// buffer and return the file.
//
ActionFileSymbol *Python2Action::GenerateTitle(ActionFileLookupTable &ast_filename_table,
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


ActionFileSymbol *Python2Action::GenerateTitleAndGlobals(ActionFileLookupTable &ast_filename_table,
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
void Python2Action::GenerateEnvironmentDeclaration(TextBuffer &b, const char *indentation)
{

    b.Put(indentation); b.Put("    ");
                                
                                 b.Put("def getEnvironment(self)");
                                 b.Put(": return self.environment\n\n");
}






//
//
//
void Python2Action::GenerateVisitorHeaders(TextBuffer &b, const char *indentation, const char *modifiers)
{
    if (option -> visitor != Option::NONE)
    {
        char *header = new char[strlen(indentation) + strlen(modifiers) + 9];
        strcpy(header, indentation);
        strcat(header, modifiers);


        if (option -> visitor & Option::PREORDER)
        {
            b.Put(header);
            b.Put("def  accept(self, v):pass");
        }

        if (option -> visitor & Option::DEFAULT)
        {
            b.Put(header);
            b.Put("def  acceptWithVisitor(self, v");
            b.Put(") : pass");

            b.Put("\n");

            b.Put(header);
            b.Put("def  acceptWithArg(self, v");
            b.Put(", o) : pass\n");

            b.Put(header);
            b.Put("def  acceptWithResult(self, v");
            b.Put("):pass \n");

            b.Put(header);
            b.Put("def  acceptWithResultArgument(self, v");
          
            b.Put(", o) : pass");
        }
        b.Put("\n");

        delete [] header;
    }

    return;
}


//
//
//
void Python2Action::GenerateVisitorMethods(NTC &ntc,
                                        TextBuffer &b,
                                        const char *indentation,
                                        ClassnameElement &element,
                                        BitSet &optimizable_symbol_set)
{
    if (option -> visitor & Option::DEFAULT)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("    def  acceptWithVisitor(self, v):  v.visit").Put(element.real_name); b.Put("(self)\n");;
                                    

        b.Put(indentation); b.Put("    def  acceptWithArg(self, v, o):  v.visit").Put(element.real_name); b.Put("(self, o)\n");;
                                  
 

        b.Put(indentation); b.Put("    def  acceptWithResult(self, v");
                                
                                     b.Put("):  return v.visit");b.Put(element.real_name); b.Put("(self)\n");

        b.Put(indentation); b.Put("    def  acceptWithResultArgument(self, v");
                                  
                                     b.Put(", o):  return v.visit"); b.Put(element.real_name); b.Put("(self, o)\n");
    }
    if (option -> visitor & Option::PREORDER)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("    def  accept(self, v ): \n");
        b.Put(indentation); b.Put("    \n");
        b.Put(indentation); b.Put("        if not v.preVisit(self): return\n");
        b.Put(indentation); b.Put("        self.enter("); 
                                     
                                     b.Put(" v)\n");
        b.Put(indentation); b.Put("        v.postVisit(self)\n");
        b.Put(indentation); b.Put("    \n\n");

        b.Put(indentation); b.Put("    def  enter(self, v");
                                    
                                     b.Put("): \n");
        b.Put(indentation); b.Put("    \n");
        SymbolLookupTable &symbol_set = element.symbol_set;
        Tuple<int> &rhs_type_index = element.rhs_type_index;
        if (element.is_terminal_class || symbol_set.Size() == 0)
        {
            b.Put(indentation); b.Put("        v.visit"); b.Put(element.real_name); b.Put("(self)\n");
          
        }
        else
        {
            b.Put(indentation); b.Put("        checkChildren = v.visit");b.Put(element.real_name); b.Put("(self)\n");
            b.Put(indentation); b.Put("        if checkChildren:\n");
            if (symbol_set.Size() > 1)
            {
                b.Put(indentation); b.Put("        \n");
            }

            for (int i = 0; i < symbol_set.Size(); i++)
            {
                b.Put(indentation); b.Put("            ");
                if ((! optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
                {
                    b.Put("if self._");
                    b.Put(symbol_set[i] -> Name());
                    b.Put(": ");
                }
                b.Put("self._");
                b.Put(symbol_set[i] -> Name());
                b.Put(".accept(v)\n");
            }

            if (symbol_set.Size() > 1)
            {
                b.Put(indentation); b.Put("        \n");
            }
        }
        b.Put(indentation); b.Put("        v.endVisit"); b.Put(element.real_name); b.Put("(self)\n");
        b.Put(indentation); b.Put("    \n");
    }

    return;
}


//
//
//
void Python2Action::GenerateGetAllChildrenMethod(TextBuffer &b,
                                              const char *indentation,
                                              ClassnameElement &element)
{
    if (! element.is_terminal_class)
    {
        SymbolLookupTable &symbol_set = element.symbol_set;

        b.Put("\n");
        b.Put(indentation); b.Put("    '''/**\n");
        b.Put(indentation); b.Put("     * A list of all children of this node, don't including the null ones.\n");
        b.Put(indentation); b.Put("     */'''\n");
        b.Put(indentation); b.Put("    def  getAllChildren(self)  :\n");
        b.Put(indentation); b.Put("    \n");
        b.Put(indentation); b.Put("        _content = ArrayList()\n");
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            b.Put(indentation);
        	b.Put("        if self._");b.Put(symbol_set[i]->Name());
        	b.Put(":  _content.add(self._");b.Put(symbol_set[i] -> Name());
        	b.Put(")\n");
        }
        b.Put(indentation); b.Put("        return _content\n");
        b.Put(indentation); b.Put("    \n");
    }

    return;
}


//
//
//
void Python2Action::GenerateSimpleVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                const char *indentation,
                                                const char *interface_name,
                                                SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);

    b.Put("class ");
                                 b.Put(interface_name);
                                 b.Put("(object):");
                                 b.Put("\n");
	b.Put(indentation); b.Put("    "); b.Put(EMPTY_SLOTS); b.Put("\n");
    b.Put(indentation); b.Put("\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("    def visit"); b.Put(symbol->Name());
                                     b.Put("(self, n");
                                   
                                     b.Put("): pass\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    def visit");
                                 b.Put("(self, n");
                               
                                 b.Put("): pass\n");

    b.Put(indentation); b.Put("\n");


}

//
//
//
void Python2Action::GenerateArgumentVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                  const char *indentation,
                                                  const char *interface_name,
                                                  SymbolLookupTable &type_set)
{
   
    TextBuffer& b =*GetBuffer(ast_filename_symbol);

     b.Put("class ");
                                 b.Put(interface_name);
                                 b.Put("(object):");
                                 b.Put("\n");
                                 b.Put(indentation); b.Put("    "); b.Put(EMPTY_SLOTS); b.Put("\n");
    b.Put(indentation); b.Put("\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("    def visit"); b.Put(symbol->Name());
                                     b.Put("(self, n");
                                  
                                     b.Put(", o): pass\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    def visit");
                                 b.Put("(self, n");
                              
                                 b.Put(", o): pass\n");

    b.Put(indentation); b.Put("\n");
    
}

//
//
//
void Python2Action::GenerateResultVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                const char *indentation,
                                                const char *interface_name,
                                                SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    b.Put("class ");
                                 b.Put(interface_name);
                                 b.Put("(object):");
                                 b.Put("\n");
                                 b.Put(indentation); b.Put("    "); b.Put(EMPTY_SLOTS); b.Put("\n");
    b.Put(indentation); b.Put("\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("    def visit"); b.Put(symbol->Name());
                                     b.Put("(self, n");
                                  
                                     b.Put("): pass\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    def visit");
                                 b.Put("(self, n");
                             
                                 b.Put("): pass\n");

    b.Put(indentation); b.Put("\n");
    
}

//
//
//
void Python2Action::GenerateResultArgumentVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                        const char *indentation,
                                                        const char *interface_name,
                                                        SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    b.Put("class ");
    b.Put(interface_name);
    b.Put("(object):");
    b.Put("\n");
    b.Put(indentation); b.Put("    "); b.Put(EMPTY_SLOTS); b.Put("\n");
    b.Put(indentation); b.Put("\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("    def visit"); b.Put(symbol->Name());
                                     b.Put("(self, n");
                                  
                                     b.Put(", o): pass\n");
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    def visit");
                                 b.Put("(self, n");
                               
                                 b.Put(", o): pass\n");

    b.Put(indentation); b.Put("\n");
    
}


//
//
//
void Python2Action::GeneratePreorderVisitorInterface(ActionFileSymbol* ast_filename_symbol,
                                                  const char *indentation,
                                                  const char *interface_name,
                                                  SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    assert(option -> visitor & Option::PREORDER);
    b.Put("class ");
                                 b.Put(interface_name);
                                 b.Put(" (IAstVisitor):\n");
    b.Put(indentation); b.Put("\n");
    b.Put(indentation); b.Put("    "); b.Put(EMPTY_SLOTS); b.Put("\n");

    b.Put(indentation); b.Put("    def visit");
                                 b.Put("(self, n");
                              
                                 b.Put("): pass\n");
    b.Put(indentation); b.Put("    def endVisit");
                                 b.Put("(self, n");
                               
                                 b.Put(") : pass\n\n");

    for (int i = 0; i < type_set.Size(); i++)
    {
        Symbol *symbol = type_set[i];
        b.Put(indentation); b.Put("    def visit"); b.Put(symbol->Name());
                                     b.Put("(self, n");
                                  
                                     b.Put("):pass\n");
        b.Put(indentation); b.Put("    def endVisit"); b.Put(symbol->Name());
                                     b.Put("(self, n");
                                 
                                     b.Put("): pass\n");
        b.Put("\n");
    }

    b.Put(indentation); b.Put("\n\n");
    
    return;
}


//
//
//
void Python2Action::GenerateNoResultVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                      const char *indentation,
                                                      const char *classname,
                                                      SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
   
                                 b.Put("class ");
                                 b.Put(classname);
                                 b.Put(" ( ");
                                 b.Put(visitorFactory -> visitor_type);
                                 b.Put(",");
                                 b.Put(visitorFactory -> argument_visitor_type);
                                 b.Put("):\n");
	b.Put(indentation); b.Put("      "); b.Put(EMPTY_SLOTS); b.Put("\n");
    b.Put(indentation); b.Put("\n");
    b.Put(indentation); b.Put("      def unimplementedVisitor(self,s) :raise TypeError('Can not instantiate abstract class  with abstract methods') \n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];

            b.Put(indentation); b.Put("      def visit"); b.Put(symbol->Name());
                                         b.Put("(self, n");
                                       
                                         b.Put(", o = None): self.unimplementedVisitor(\"visit"); b.Put(symbol->Name()); b.Put("(");
                                         b.Put(symbol -> Name());
                                         b.Put(", any)\")\n");
            b.Put("\n");
        }
    }

                                 b.Put("\n");

   

    b.Put(indentation); b.Put("      def visit");
                                 b.Put("(self, n");
                                
                                 b.Put(", o = None): \n");
    b.Put(indentation); b.Put("    \n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                         b.Put(i == 0 ? "if " : "elif ");
                                         b.Put("isinstance(n, ");
                                         b.Put(symbol -> Name());
                                         b.Put("): self.visit");
                                         b.Put(symbol -> Name());
                                         b.Put("(");
                                        
                                         b.Put(" n, o)\n");
        }
    }
    b.Put(indentation); b.Put("        else: raise ValueError(\"visit(\" + n.toString() + \")\")\n");
    b.Put(indentation); b.Put("    \n");

    b.Put(indentation); b.Put("\n");
    
}

//
//
//
void Python2Action::GenerateResultVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                    const char *indentation,
                                                    const char *classname,
                                                    SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
  
                                 b.Put("class ");
                                 b.Put(classname);
                                 b.Put(" (");
                                 b.Put(visitorFactory -> result_visitor_type);
                                 b.Put(",");
                                 b.Put(visitorFactory -> result_argument_visitor_type);
                                 b.Put("):\n");
                                 b.Put(indentation); b.Put("    "); b.Put(EMPTY_SLOTS); b.Put("\n");
    b.Put(indentation); b.Put("\n");
    b.Put(indentation); b.Put("    def unimplementedVisitor(self,s ): raise TypeError('Can not instantiate abstract class  with abstract methods')\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
       
            b.Put(indentation); b.Put("    def visit"); b.Put(symbol->Name()); b.Put("(self, n");
                                        
                                         b.Put(", o = None): return  self.unimplementedVisitor(\"visit"); b.Put(symbol->Name()); b.Put("(");
                                         b.Put(symbol -> Name());
                                         b.Put(", any)\")\n");
            b.Put("\n");
        }
    }

                                 b.Put("\n");



    b.Put(indentation); b.Put("    def visit");
                                 b.Put("(self, n");
                              
                                 b.Put(", o = None):\n");
    b.Put(indentation); b.Put("    \n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                         b.Put(i == 0 ? "if " : "elif ");
                                         b.Put("isinstance(n, ");
                                         b.Put(symbol -> Name());
                                         b.Put("): return self.visit");
                                         b.Put(symbol->Name());
                                         b.Put("(");
                            
                                         b.Put("n, o)\n");
        }
    }
    b.Put(indentation); b.Put("        else: raise ValueError(\"visit(\" + n.toString() + \")\")\n");
    b.Put(indentation); b.Put("    \n");

    b.Put(indentation); b.Put("\n");
    
}


//
//
//
void Python2Action::GeneratePreorderVisitorAbstractClass(ActionFileSymbol* ast_filename_symbol,
                                                      const char *indentation,
                                                      const char *classname,
                                                      SymbolLookupTable &type_set)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    assert(option -> visitor & Option::PREORDER);
    
                                 b.Put("class ");
                                 b.Put(classname);
                                 b.Put("(");
                                 b.Put(visitorFactory->preorder_visitor_type);
                                 b.Put("):\n");
                                 b.Put(indentation); b.Put("    "); b.Put(EMPTY_SLOTS); b.Put("\n");
    b.Put(indentation); b.Put("\n");
    b.Put(indentation); b.Put("    def  unimplementedVisitor(self,s ): raise TypeError('Can not instantiate abstract class  with abstract methods')\n\n");
    b.Put(indentation); b.Put("    def  preVisit(self, element ): return True\n\n");
    b.Put(indentation); b.Put("    def  postVisit(self,element ): pass\n\n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("    def visit"); b.Put(symbol->Name());
                                         b.Put("(self, n");
                                      
                                         b.Put("):\n");

        	b.Put(indentation); b.Put("        "); 
                                         b.Put("self.unimplementedVisitor(\"visit(");
                                         b.Put(symbol -> Name());
                                         b.Put(")\")\n");

        	b.Put(indentation); b.Put("        return True\n");

            b.Put(indentation); b.Put("    def endVisit"); b.Put(symbol->Name());
                                         b.Put("(self, n");
                                       
                                         b.Put("): self.unimplementedVisitor(\"endVisit(");
                                         b.Put(symbol -> Name());
                                         b.Put(")\")\n");
            b.Put("\n");
        }
    }

                                 b.Put("\n");
    b.Put(indentation); b.Put("    def visit");
                                 b.Put("(self, n");
                               
                                 b.Put("):\n");
    b.Put(indentation); b.Put("    \n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                         b.Put(i == 0 ? "if " : "elif ");
                                         b.Put("isinstance(n, ");
                                         b.Put(symbol -> Name());
                                         b.Put("): return self.visit");
                                         b.Put(symbol->Name());
                                         b.Put("(");
                                    
                                         b.Put(" n)\n");
        }
    }
    b.Put(indentation); b.Put("        raise ValueError(\"visit(\" + n.toString() + \")\")\n");
    b.Put(indentation); b.Put("    \n");

    b.Put(indentation); b.Put("    def endVisit");
                                 b.Put("(self, n");
                              
                                 b.Put(") : \n");
    b.Put(indentation); b.Put("    \n");
    {
        for (int i = 0; i < type_set.Size(); i++)
        {
            Symbol *symbol = type_set[i];
            b.Put(indentation); b.Put("        ");
                                         b.Put(i == 0 ? "if " : "elif ");
                                         b.Put("(isinstance(n, ");
                                         b.Put(symbol -> Name());
                                         b.Put(")): self.endVisit");
                                         b.Put(symbol -> Name());
                                         b.Put("(");
                                       
                                         b.Put(" n)\n");
        }
    }
    b.Put(indentation); b.Put("        else: raise ValueError(\"visit(\" + n.toString() + \")\")\n");
    b.Put(indentation); b.Put("    \n");

    b.Put(indentation); b.Put("\n");
    
    return;
}


//
// Generate the the Ast root classes
//
void Python2Action::GenerateAstType(ActionFileSymbol* ast_filename_symbol,
                                 const char *indentation,
                                 const char *classname)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    /*
     * First, generate the main rootclass
     */

                                 b.Put("class ");
                                 b.Put(classname);
                                 b.Put("(IAst):\n");
    b.Put(indentation); b.Put("\n");
    if (option -> glr)
    {

        b.Put(indentation); b.Put("    def getNextAst(self): return self.nextAst\n");
        b.Put(indentation); b.Put("    def setNextAst(self, n ): self.nextAst = n\n");
        b.Put(indentation); b.Put("    def resetNextAst(self):   self.nextAst = null\n");
    }
    else
    {
	    b.Put(indentation); b.Put("    def getNextAst(self): return None\n");
    }

    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("    def  setParent(self, parent):  self.parent = parent\n");
        b.Put(indentation); b.Put("    def  getParent(self): return self.parent\n");\
    }
    else
    {
        b.Put(indentation); b.Put("    def getParent(self): \n");
        b.Put(indentation); b.Put("    \n");
        b.Put(indentation); b.Put("        raise ValueError(\"noparent-saved option in effect\")\n");
        b.Put(indentation); b.Put("    \n");
    }

    b.Put("\n");
    b.Put(indentation); b.Put("    def getLeftIToken(self) :  return self.leftIToken\n");
    b.Put(indentation); b.Put("    def getRightIToken(self):  return self.rightIToken\n");
    b.Put(indentation); b.Put("    def getPrecedingAdjuncts(self)  : return self.leftIToken.getPrecedingAdjuncts()\n");
    b.Put(indentation); b.Put("    def getFollowingAdjuncts(self)  : return self.rightIToken.getFollowingAdjuncts()\n\n");

    b.Put(indentation); b.Put("    def  toString(self) :  \n");
    b.Put(indentation); b.Put("    \n");
    b.Put(indentation); b.Put("        info = self.leftIToken.getILexStream().toString(self.leftIToken.getStartOffset(), self.rightIToken.getEndOffset())\n");
    b.Put(indentation); b.Put("        return info if  info else  \"\"\n");
    b.Put(indentation); b.Put("    \n\n");

    b.Put(indentation); b.Put("    "); b.Put("__slots__ = ('parent', 'leftIToken', 'rightIToken', 'nextAst')"); b.Put("\n");
    b.Put(indentation);b.Put("    def __init__(self,leftIToken  , rightIToken  = None ):\n");
    b.Put(indentation); b.Put("    \n");
    b.Put(indentation); b.Put("        self.parent = None\n");
    b.Put(indentation); b.Put("        self.nextAst = None\n");
    b.Put(indentation); b.Put("        self.leftIToken = leftIToken\n");
    b.Put(indentation); b.Put("        if rightIToken: self.rightIToken = rightIToken\n");
    b.Put(indentation); b.Put("        else:            self.rightIToken = leftIToken\n");
    b.Put(indentation); b.Put("    \n\n");
    b.Put(indentation); b.Put("    def  initialize(self): pass\n");
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
        b.Put(indentation); b.Put("    '''/**\n");
        b.Put(indentation); b.Put("     * A list of all children of this node, excluding the null ones.\n");
        b.Put(indentation); b.Put("     */'''\n");
        b.Put(indentation); b.Put("    def  getChildren(self):\n");
        b.Put(indentation); b.Put("    \n");
        b.Put(indentation); b.Put("        _content = self.getAllChildren() \n");
        b.Put(indentation); b.Put("        k = -1\n");
        b.Put(indentation); b.Put("        for i in range(_content.size):\n");
        b.Put(indentation); b.Put("        \n");
        b.Put(indentation); b.Put("            element = _content.get(i)\n");
        b.Put(indentation); b.Put("            if element:\n");
        b.Put(indentation); b.Put("            \n");
        b.Put(indentation); b.Put("                k += 1\n");
        b.Put(indentation); b.Put("                if (k != i):\n");
        b.Put(indentation); b.Put("                    _content.set(k, element)\n");
        b.Put(indentation); b.Put("            \n");
        b.Put(indentation); b.Put("        \n");
        b.Put(indentation); b.Put("        i = _content.size() - 1\n");
        b.Put(indentation); b.Put("        while i > k : # remove extraneous elements\n");
        b.Put(indentation); b.Put("            i-=1\n");
        b.Put(indentation); b.Put("            _content.remove(i)\n");
        b.Put(indentation); b.Put("        return _content\n");
        b.Put(indentation); b.Put("    \n\n");

        b.Put(indentation); b.Put("    '''/**\n");
        b.Put(indentation); b.Put("     * A list of all children of this node, don't including the null ones.\n");
        b.Put(indentation); b.Put("     */'''\n");
        b.Put(indentation); b.Put("    def   getAllChildren(self): raise TypeError('Can not instantiate abstract class  with abstract methods')\n");
    }
    else
    {
        b.Put(indentation); b.Put("    def  getChildren(self):\n");
        b.Put(indentation); b.Put("    \n");
        b.Put(indentation); b.Put("        raise ValueError(\"noparent-saved option in effect\")\n");
        b.Put(indentation); b.Put("    \n");
        b.Put(indentation); b.Put("    def  getAllChildren(self):  return self.getChildren()\n");
    }

    b.Put("\n");

    GenerateVisitorHeaders(b, indentation, "    ");

    //
    // Not Preorder visitor? generate dummy accept method to satisfy IAst abstract declaration of accept(IAstVisitor);
    if (!(option -> visitor & Option::PREORDER))
    {
        b.Put(indentation); b.Put("    def  accept(self, v):  pass\n");
    }
    b.Put(indentation); b.Put("\n\n");
    
    return;
}



typedef std::map<std::string, std::string> Substitutions;



//
// Generate the the Ast list class
//
void Python2Action::GenerateAbstractAstListType(ActionFileSymbol* ast_filename_symbol,
                                             const char *indentation,
                                             const char *classname)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    /*
     * Generate the List root class
     */
     
	 b.Put("class ");
	 b.Put(this -> abstract_ast_list_classname);
	 b.Put(" ( ");
	 b.Put(option -> ast_type);
	 b.Put(" , IAbstractArrayList");
	 b.Put("):\n");
    b.Put(indentation); b.Put("\n");


    b.Put(indentation); b.Put("    def size(self): return self._content.size()\n");
    b.Put(indentation); b.Put("    def getList(self):"); b.Put(" return self._content\n");
    b.Put(indentation); b.Put("    def  getElementAt(self,i ) : ");

                                 b.Put("return self._content.get( i if self.left_recursive else  self._content.size() - 1 - i)\n");

    b.Put(indentation); b.Put("    def  getArrayList(self)  :\n"); 
   
    b.Put(indentation); b.Put("    \n");
    b.Put(indentation); b.Put("        if not self.left_recursive: # reverse the list \n");
    b.Put(indentation); b.Put("        \n");
    b.Put(indentation); b.Put("            i = 0\n");
    b.Put(indentation); b.Put("            n = self._content.size() - 1\n");
    b.Put(indentation); b.Put("            while i < n :\n");
    b.Put(indentation); b.Put("            \n");
    b.Put(indentation); b.Put("                ith = self._content.get(i)\n");
    b.Put(indentation); b.Put("                nth = self._content.get(n)\n");
    b.Put(indentation); b.Put("                self._content.set(i, nth)\n");
    b.Put(indentation); b.Put("                self._content.set(n, ith)\n");
    b.Put(indentation); b.Put("                i+=1\n");
    b.Put(indentation); b.Put("                n-=1\n");
    b.Put(indentation); b.Put("            \n");
    b.Put(indentation); b.Put("            self.left_recursive = True\n");
    b.Put(indentation); b.Put("        \n");
    b.Put(indentation); b.Put("        return self._content\n");
    b.Put(indentation); b.Put("    \n");

    b.Put(indentation); b.Put("    '''/**\n");
    b.Put(indentation); b.Put("     * @deprecated replaced by {@link #addElement()}\n");
    b.Put(indentation); b.Put("     *\n");
    b.Put(indentation); b.Put("     */'''\n");
    b.Put(indentation); b.Put("    def  add(self, element ");
                                 b.Put(")  :\n");
    b.Put(indentation); b.Put("    \n");
    b.Put(indentation); b.Put("        self.addElement(element)\n");
    b.Put(indentation); b.Put("        return True\n");
    b.Put(indentation); b.Put("    \n\n");

    b.Put(indentation); b.Put("    def  addElement(self,element");
                                
                                 b.Put(") : \n");
    b.Put(indentation); b.Put("    \n");
    b.Put(indentation); b.Put("        self._content.add(element)\n");
    b.Put(indentation); b.Put("        if self.left_recursive:\n");
    b.Put(indentation); b.Put("             self.rightIToken = element.getRightIToken()\n");
    b.Put(indentation); b.Put("        else :\n");
    b.Put(indentation); b.Put("             self.leftIToken = element.getLeftIToken()\n");
    b.Put(indentation); b.Put("    \n\n");




    b.Put(indentation); b.Put("    "); b.Put("__slots__ = ('left_recursive', '_content')"); b.Put("\n");
    // generate constructors for list class
    b.Put(indentation); b.Put("    def __init__(self,leftToken , rightToken  , left_recursive ):");
    b.Put(indentation); b.Put("    \n");
    b.Put(indentation);
	
    {
        char temp[1024] = {};
        sprintf(temp, "          super(%s, self).__init__(leftToken, rightToken)\n", classname);
        b.Put(temp);
    }
    b.Put(indentation); b.Put("          self.left_recursive = left_recursive\n");
    b.Put(indentation); b.Put("          self._content = ArrayList()\n");
    b.Put(indentation); b.Put("    \n\n");
    
  
    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("    '''/**\n");
        b.Put(indentation); b.Put("     * Make a copy of the list and return it. Note that we obtain the local list by\n");
        b.Put(indentation); b.Put("     * invoking getArrayList so as to make sure that the list we return is in proper order.\n");
        b.Put(indentation); b.Put("     */'''\n");
        b.Put(indentation); b.Put("    def  getAllChildren(self):\n");
        b.Put(indentation); b.Put("    \n");
        b.Put(indentation); b.Put("        return self.getArrayList().clone()\n");
        b.Put(indentation); b.Put("    \n\n");
    }

    //
    // Implementation for functions in ArrayList
    //

    Substitutions subs;
    subs["%%AstType%%"] = option->ast_type;
    subs["%%ListClassName%%"] = classname;
    b.Put(indentation); b.Put("\n\n");

    
  

    return;
}


//
// Generate the the Ast token class
//
void Python2Action::GenerateAstTokenType(NTC &ntc, ActionFileSymbol* ast_filename_symbol,
                                      const char *indentation,
                                      const char *classname)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    /*
     * Generate the Token root class
     */
   
                                 b.Put("class ");
                                 b.Put(classname);
                                 b.Put(" ( ");
                                 b.Put(option -> ast_type);
                                 b.Put(", I");
                                 b.Put(classname);
                                 b.Put("):\n");
    b.Put(indentation); b.Put("\n");
    b.Put(indentation); b.Put("    "); b.Put(EMPTY_SLOTS); b.Put("\n");
    b.Put(indentation); b.Put("    ");
    {
        char temp[1024] = {};
        sprintf(temp, "def __init__(self,token) : super(%s,self).__init__(token)\n", classname);
	    b.Put(temp);
    }
    b.Put(indentation); b.Put("    def  getIToken(self): return self.leftIToken\n");
    b.Put(indentation); b.Put("    def  toString(self):  return self.leftIToken.toString()\n\n");

    ClassnameElement element; // generate a temporary element with no symbols in its symbol set.
    element.real_name = (char *) classname;
    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);

    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("    '''/**\n");
        b.Put(indentation); b.Put("     * A token class has no children. So, we return the empty list.\n");
        b.Put(indentation); b.Put("     */'''\n");
        b.Put(indentation); b.Put("    def  getAllChildren(self)  :  return ArrayList()\n\n");
    }

    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set);

    b.Put(indentation); b.Put("\n\n");
    
    return;
}


//
//
//
void Python2Action::GenerateCommentHeader(TextBuffer &b,
                                       const char *,
                                       Tuple<int> &ungenerated_rule,
                                       Tuple<int> &generated_rule)
{
    BlockSymbol* scope_block = nullptr;
    const char* rule_info = rule_info_holder.c_str();

    b.Put("'''/**");
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

    b.Put(" */'''\n");
}


void Python2Action::GenerateListMethods(CTC &ctc,
                                     NTC &ntc,
                                     TextBuffer &b,
                                     const char *indentation,
                                     const char *classname,
                                     ClassnameElement &element,
                                     Array<const char *> &)
{
    const char *element_name = element.array_element_type_symbol -> Name();

    //
    // Generate ADD method
    //
    b.Put(indentation); b.Put("    def ");
                                 b.Put(" addElement(self, ");
                                 b.Put(" _");
                                 b.Put(element_name);
                            
                                 b.Put("): \n");
    b.Put(indentation); b.Put("    \n");
    b.Put(indentation);
    {
        char temp[1024] = {};
        sprintf(temp, "        super(%s, self).addElement(", classname);
        b.Put(temp);
    }
	
                                
                                 b.Put(" _");
                                 b.Put(element_name);
                                 b.Put(")\n");
    if (option -> parent_saved)
    {
        b.Put(indentation); b.Put("        ");
        if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
        {
            b.Put("if self._");
            b.Put(element_name);
            b.Put(": ");
        }
   
        b.Put("_");
        b.Put(element_name);
        b.Put(".setParent(self)\n");
    }
    b.Put(indentation); b.Put("    \n");

    b.Put("\n");
   
    //
    // Generate visitor methods.
    //
    if (option -> visitor & Option::DEFAULT)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("    def  acceptWithVisitor(self, v");
                                  
                                     b.Put("):\n");
    	b.Put(indentation); b.Put("         "); b.Put("for i in range(self.size()):\n");

        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {

            b.Put(indentation); b.Put("         "); b.Put("    v.visit(self.get");
            b.Put(element_name);
            b.Put("At(i))\n");
        }
        else
        {
            b.Put(indentation); b.Put("         "); b.Put("    self.get");
            b.Put(element_name);
            b.Put("At(i).acceptWithVisitor(self, v)\n");
        }

        b.Put(indentation); b.Put("    def  acceptWithArg(self, v,o");
                                  
                                     b.Put("):\n");
    	b.Put(indentation); b.Put("         "); b.Put("for i in range(self.size()):\n");

        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
            b.Put(indentation); b.Put("         "); b.Put("    v.visit(self.get");
            b.Put(element_name);
            b.Put("At(i), o");
            b.Put(")\n");
        }
        else
        {
            b.Put(indentation); b.Put("         "); b.Put("    self.get");
            b.Put(element_name);
            b.Put("At(i).acceptWithArg(self, v, o)\n");
        }

        //
        // Code cannot be generated to automatically visit a node that
        // can return a value. These cases are left up to the user.
        //
        b.Put(indentation); b.Put("    def  acceptWithResult(self, v ");
                                    
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
                                         b.Put(") :\n");
            b.Put(indentation); b.Put("    \n");
            b.Put(indentation); b.Put("        result = ArrayList()\n");
            b.Put(indentation); b.Put("        for i in range(self.size()):\n");
            b.Put(indentation); b.Put("            result.add(v.visit(self.get");
                                         b.Put(element_name);
                                         b.Put("At(i)))\n");
            b.Put(indentation); b.Put("        return result\n");
            b.Put(indentation); b.Put("    \n");
        }
        else
        {
                                         b.Put(") :\n");
            b.Put(indentation); b.Put("    \n");
            b.Put(indentation); b.Put("        result = ArrayList()\n");
            b.Put(indentation); b.Put("        for i in range(self.size()):\n");
            b.Put(indentation); b.Put("            result.add(self.get");
                                         b.Put(element_name);
                                         b.Put("At(i).acceptWithResult(self, v))\n");
            b.Put(indentation); b.Put("        return result\n");
            b.Put(indentation); b.Put("    \n");
        }

        b.Put(indentation); b.Put("    def  acceptWithResultArgument(self, v");
                                 
        if (ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex()) != NULL)
        {
                                         b.Put(", o) :\n");
            b.Put(indentation); b.Put("    \n");
            b.Put(indentation); b.Put("        result = ArrayList()\n");
            b.Put(indentation); b.Put("        for i in range(self.size()):\n");
            b.Put(indentation); b.Put("            result.add(v.visit(self.get");
                                         b.Put(element_name);
                                         b.Put("At(i), o))\n");
            b.Put(indentation); b.Put("        return result\n");
            b.Put(indentation); b.Put("    \n");
        }
        else
        {
                                         b.Put(", o) : \n");
            b.Put(indentation); b.Put("    \n");
            b.Put(indentation); b.Put("        result = ArrayList()\n");
            b.Put(indentation); b.Put("        for i in range(self.size()):\n");
            b.Put(indentation); b.Put("            result.add(self.get");
                                         b.Put(element_name);
                                         b.Put("At(i).acceptWithResultArgument(v, o))\n");
            b.Put(indentation); b.Put("        return result\n");
            b.Put(indentation); b.Put("    \n");
        }
    }
    if (option -> visitor & Option::PREORDER)
    {
        b.Put("\n");
        b.Put(indentation); b.Put("    def  accept(self, v) : \n");
        b.Put(indentation); b.Put("    \n");
        b.Put(indentation); b.Put("        if (not v.preVisit(self)): return\n");
        b.Put(indentation); b.Put("        self.enter(");
                                    
                                     b.Put(" v)\n");
        b.Put(indentation); b.Put("        v.postVisit(self)\n");
        b.Put(indentation); b.Put("    \n");
        b.Put(indentation); b.Put("    def enter(self, v");
                                    
                                     b.Put(") : \n");
        b.Put(indentation); b.Put("    \n");
        b.Put(indentation); b.Put("        checkChildren = v.visit").Put(classname).Put("(self)\n");
        b.Put(indentation); b.Put("        if checkChildren:\n");
        b.Put(indentation); b.Put("        \n");
        b.Put(indentation); b.Put("            for i in range(self.size()):\n");
        b.Put(indentation); b.Put("            \n");
        b.Put(indentation); b.Put("                ");

        const char *element_typename = ctc.FindUniqueTypeFor(element.array_element_type_symbol -> SymbolIndex());
        if (element_typename != NULL)
        {
            //b.Put(element_typename);
            b.Put("element = self.get");
            b.Put(element_name);
            b.Put("At(i)\n");
            if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
            {
                b.Put(indentation); b.Put("                    if element:");
                b.Put(indentation); b.Put("                \n");
                b.Put(indentation); b.Put("                    if not v.preVisit(element): continue\n");
                b.Put(indentation); b.Put("                    element.enter(v)\n");
                b.Put(indentation); b.Put("                    v.postVisit(element)\n");
                b.Put(indentation); b.Put("                \n");
            }
            else
            {
                b.Put(indentation); b.Put("                if not v.preVisit(element): continue\n");
                b.Put(indentation); b.Put("                element.enter(v)\n");
                b.Put(indentation); b.Put("                v.postVisit(element)\n");
            }
        }
        else
        {
            //b.Put(typestring[element.array_element_type_symbol -> SymbolIndex()]);
            b.Put("element = self.get");
            b.Put(element_name);
            b.Put("At(i)\n");
            b.Put(indentation); b.Put("                ");
            if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
                b.Put("if element: ");
            b.Put("element.accept(v)\n");
        }
        b.Put(indentation); b.Put("            \n");
        b.Put(indentation); b.Put("        \n");
        b.Put(indentation); b.Put("        v.endVisit").Put(classname).Put("(self)\n");
        b.Put(indentation); b.Put("    \n");
    }

    return;
}


//
//
//
void Python2Action::GenerateListClass(CTC &ctc,
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
               *element_name = element.array_element_type_symbol -> Name();

    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);

   
                                 b.Put("class ");
                                 b.Put(classname);
                                 b.Put(" ( ");
                                 b.Put(abstract_ast_list_classname);
                                 b.Put(", ");
                                 for (int i = 0; i < interface.Length() - 1; i++)
                                 {
                                     b.Put(typestring[element.interface_[i]]);
                                     b.Put(", ");
                                 }
                                 b.Put(typestring[element.interface_[interface.Length() - 1]]);
                                 b.Put("):");
  
    b.Put("\n");
    b.Put(indentation); b.Put("\n");

    if (ntc.CanProduceNullAst(element.array_element_type_symbol -> SymbolIndex()))
    {
        b.Put(indentation); b.Put("    '''/**\n");
        b.Put(indentation); b.Put("     * The value returned by <b>get");
                                     b.Put(element_name);
                                     b.Put("At</b> may be <b>null</b>\n");
        b.Put(indentation); b.Put("     */'''\n");
    }
    b.Put(indentation); b.Put("    def ");
                                 b.Put(" get");
                                 b.Put(element_name);
                                 b.Put("At(self, i)");
                             
                                 b.Put(": return ");
                               
                                 b.Put(" self.getElementAt(i)\n\n");

	b.Put(indentation); b.Put("    "); b.Put(EMPTY_SLOTS); b.Put("\n");
    //
    // generate constructors
    //
	b.Put(indentation); b.Put("    def __init__(self,leftToken, rightToken, left_recursive):\n");
    b.Put(indentation); b.Put("    \n");
    b.Put(indentation);
    {
        char temp[1024] = {};
        sprintf(temp, "        super(%s, self).__init__(leftToken, rightToken, left_recursive)\n", classname);
        b.Put(temp);
    }

    b.Put(indentation); b.Put("    \n\n");

    b.Put(indentation); b.Put("    @staticmethod\n"); 
    b.Put(indentation); b.Put("    def ");b.Put(classname); b.Put("fromElement(");
    b.Put("element");
   
    b.Put(",left_recursive)  ");  b.Put(":\n");
    b.Put(indentation); b.Put("    \n");
    b.Put(indentation); b.Put("        obj = ");b.Put(classname);
	b.Put("(element.getLeftIToken(),element.getRightIToken(), left_recursive)\n");
    b.Put(indentation); b.Put("        obj._content.add(element)\n");
    if (option->parent_saved)
    {
        b.Put(indentation); b.Put("        ");
        if (ntc.CanProduceNullAst(element.array_element_type_symbol->SymbolIndex()))
        {
            b.Put("if element: ");
        }
        b.Put("element.setParent(obj)\n");
    }
    b.Put(indentation); b.Put("        return obj\n");
    b.Put(indentation); b.Put("    \n");
    b.Put("\n");



    GenerateListMethods(ctc, ntc, b, indentation, classname, element, typestring);

    IntToString num(element.GetRuleNo());
    b.Put(indentation); b.Put("    def getRuleIndex(self): return ") + num.String() + "\n";

    b.Put("    \n\n");// Generate Class Closer
    

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
void Python2Action::GenerateListExtensionClass(CTC& /*ctc*/,
    NTC& ntc,
    ActionFileSymbol* ast_filename_symbol,
    const char* indentation,
    SpecialArrayElement& special_array,
    ClassnameElement& element,
    Array<const char*>&)

{
    TextBuffer& b = *GetBuffer(ast_filename_symbol);
    const char* classname = element.real_name;
        //* element_name = element.array_element_type_symbol->Name(),
       // * element_type = ctc.FindBestTypeFor(element.array_element_type_symbol->SymbolIndex());

    GenerateCommentHeader(b, indentation, element.ungenerated_rule, special_array.rules);

  
    b.Put("class ");
    b.Put(special_array.name);
    b.Put(" ( ");
    b.Put(classname);
    b.Put("):\n");
    b.Put(indentation); b.Put("\n");

    GenerateEnvironmentDeclaration(b, indentation);

    b.Put(indentation); b.Put("    "); b.Put("__slots__ = 'environment'"); b.Put("\n");
    b.Put(indentation); b.Put("    def __init__(self,");
    b.Put("environment");
    b.Put(", leftIToken ,  rightIToken , left_recursive  ):\n");

    b.Put(indentation); b.Put("    \n");
    b.Put(indentation);
    {
        char temp[1024] = {};
        sprintf(temp, "        super(%s, self).__init__(leftIToken, rightIToken, left_recursive)\n", classname);
        b.Put(temp);
    }
    b.Put(indentation); b.Put("        self.environment = environment\n");
    b.Put(indentation); b.Put("        self.initialize()\n");
    b.Put(indentation); b.Put("    \n\n");

    b.Put(indentation); b.Put("    @staticmethod"); 
    b.Put(indentation); b.Put("    def "); b.Put(special_array.name); b.Put("fromElement(environment ");
  
    b.Put(",element");
   
    b.Put(",left_recursive): ");  b.Put("\n");
    b.Put(indentation); b.Put("    \n");
    b.Put(indentation); b.Put("        obj =  "); b.Put(special_array.name);
    b.Put("(environment, element.getLeftIToken(), element.getRightIToken(), left_recursive)\n");
    b.Put(indentation); b.Put("        obj._content.add(element)\n");
    if (option->parent_saved)
    {
        b.Put(indentation); b.Put("        ");
        if (ntc.CanProduceNullAst(element.array_element_type_symbol->SymbolIndex()))
        {
            b.Put("if element:");
        }
        b.Put("element.setParent(obj)\n");
    }
    b.Put(indentation); b.Put("        return obj\n");
    b.Put(indentation); b.Put("    \n");
    b.Put("\n");

    IntToString num(element.GetRuleNo());
    b.Put(indentation); b.Put("    def getRuleIndex(self): return ") + num.String() + "\n";

}


//
// Generate a generic rule class
//
void Python2Action::GenerateRuleClass(CTC & /*ctc*/,
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
     b.Put(" ( ");
    if (element.is_terminal_class)
    {
        b.Put(grammar -> Get_ast_token_classname());
        b.Put(" ,");
        b.Put(typestring[grammar->rules[rule_no].lhs]);
        b.Put("):\n");
        b.Put(indentation); b.Put("\n");
        if (element.needs_environment)
            GenerateEnvironmentDeclaration(b, indentation);
        if (symbol_set.Size() == 1) // if the right-hand side contains a symbol ...
        {
            b.Put(indentation); b.Put("    def  get");
                                         b.Put(symbol_set[0] -> Name());
                                         b.Put("(self) :  return self.leftIToken \n\n");
        }

        b.Put(indentation); b.Put("    "); b.Put("__slots__ = 'environment'"); b.Put("\n\n");

        b.Put(indentation); b.Put("    "); b.Put("def __init__(self,");
        if (element.needs_environment)
        {
            b.Put("environment ");
            b.Put(", token  ):");

            b.Put(indentation); b.Put("    \n");
            b.Put(indentation);
        	
            {
                char temp[1024] = {};
                sprintf(temp,"        super(%s, self).__init__(token)\n" , classname);
                b.Put(temp);
            }
            b.Put(indentation); b.Put("        self.environment = environment\n");
            b.Put(indentation); b.Put("        self.initialize()\n");
            b.Put(indentation); b.Put("    \n");
        }
        else
        {
	        b.Put("token  ) : \n");
            b.Put(indentation); b.Put("    ");
        	
            {
                char temp[1024] = {};
                sprintf(temp, "    super(%s, self).__init__(token)\n", classname);
                b.Put(temp);
            }
            b.Put(indentation); b.Put("    "); b.Put("    self.initialize()\n");
        }

    }
    else 
    {
        b.Put(option -> ast_type);
        b.Put(" ,");
        b.Put(typestring[grammar->rules[rule_no].lhs]);
        b.Put("):\n");
        b.Put(indentation); b.Put("\n");
        std::vector<std::string> slots_infos;
        if (element.needs_environment)
        {
	        GenerateEnvironmentDeclaration(b, indentation);
        	slots_infos.emplace_back("environment");
        }
       
        if (symbol_set.Size() > 0)
        {

            b.Put("\n");

            {
                for (int i = 0; i < symbol_set.Size(); i++)
                {
                    const char *symbolName = symbol_set[i] -> Name();
                    //const char *bestType = ctc.FindBestTypeFor(rhs_type_index[i]);
                    //bool nullAst = false;
                    if (ntc.CanProduceNullAst(rhs_type_index[i]))
                    {
                        b.Put(indentation); b.Put("    '''/**\n");
                        b.Put(indentation); b.Put("     * The value returned by <b>get");
                                                     b.Put(symbolName);
                                                     b.Put("</b> may be <b>null</b>\n");
                        b.Put(indentation); b.Put("     */'''\n");
                        //nullAst = true;
                    }
                    std::string name = "_";
                    name += symbolName;
                    slots_infos.emplace_back(name);
                    // Generate getter method
                    b.Put(indentation); b.Put("    def ");
                                                 b.Put(" get");
                                                 b.Put(symbolName);
                                                 b.Put("(self) ");
                                              
                                                 b.Put(" : ");
                                                 b.Put(" return self._");
                                                 b.Put(symbolName);
                                                 b.Put("\n");

                    // Generate setter method
                    b.Put(indentation); b.Put("    def  set");
                    b.Put(symbolName);
                    b.Put("(self, ");
                    b.Put(" _"); // add "_" prefix to arg name in case symbol happens to be a Java keyword
                    b.Put(symbolName);
                   
                    b.Put(") : ");
                    b.Put("  self._");
                    b.Put(symbolName);
                    b.Put(" = _");
                    b.Put(symbolName);
                    b.Put("\n");
                }
            }
            b.Put("\n");
        }

        b.Put(indentation); b.Put("    "); b.Put("__slots__ = ");
        if (slots_infos.empty()){
            b.Put("()\n\n");
        }
        else
        {
            if (slots_infos.size() > 1){
                b.Put("(");
            }
            for (size_t i = 0; i < slots_infos.size(); i++){
                b.Put("'").Put(slots_infos[i].c_str());
                b.Put(i == slots_infos.size() - 1 ? "'" : "', ");
            }
            if (slots_infos.size() > 1){
                b.Put(")\n\n");
            }
            else{
                b.Put("\n\n");
            }
        }
        //
        // generate constructor
        //
        const char *header = "    def __init__(self, ";
        b.Put(indentation);
        b.Put(header);
       
        int length = strlen(indentation) + strlen(header);

        if (element.needs_environment)
        {
            b.Put("environment ");
            b.Put(",");
        }
        b.Put("leftIToken, rightIToken");
        b.Put(symbol_set.Size() == 0 ? "):\n" : ",\n");
        {
            for (int i = 0; i < symbol_set.Size(); i++)
            {
                for (int k = 0; k <= length; k++)
                    b.PutChar(' ');
             
                b.Put(" _");
                b.Put(symbol_set[i] -> Name());
             
                b.Put(i == symbol_set.Size() - 1 ? "):\n" : ",\n");
            }
        }

        b.Put(indentation); b.Put("    \n");
        b.Put(indentation);
        {
            char temp[1024] = {};
            sprintf(temp, "        super(%s, self).__init__(leftIToken, rightIToken)\n\n", classname);
            b.Put(temp);
        }
    
        if (element.needs_environment)
        {
            b.Put(indentation);
            b.Put("        self.environment = environment\n");
        }

        {
            for (int i = 0; i < symbol_set.Size(); i++)
            {
                b.Put(indentation); b.Put("        self._");
                                             b.Put(symbol_set[i] -> Name());
                                             b.Put(" = _");
                                             b.Put(symbol_set[i] -> Name());
                                             b.Put("\n");

                if (option -> parent_saved)
                {
                    b.Put(indentation); b.Put("        ");
                    if ((! optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
                    {
                        b.Put("if _");
                        b.Put(symbol_set[i] -> Name());
                        b.Put(": ");
                    }

                    b.Put("_");
                    b.Put(symbol_set[i] -> Name());
                    b.Put(".setParent(self)\n");
                }
            }
        }

        b.Put(indentation); b.Put("        self.initialize()\n");
        b.Put(indentation); b.Put("    \n");
    }

    if (option -> parent_saved)
        GenerateGetAllChildrenMethod(b, indentation, element);
   
    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set);

    IntToString num(element.GetRuleNo());
    b.Put(indentation); b.Put("    def getRuleIndex(self): return ") + num.String() + "\n";

    return;
}


//
// Generate Ast class
//
void Python2Action::GenerateTerminalMergedClass(NTC &ntc,
                                             ActionFileSymbol* ast_filename_symbol,
                                             const char *indentation,
                                             ClassnameElement &element,
                                             Array<const char *> &typestring)
{
    TextBuffer& b =*GetBuffer(ast_filename_symbol);
    char *classname = element.real_name;
    GenerateCommentHeader(b, indentation, element.ungenerated_rule, element.rule);

   
     b.Put("class ");
     b.Put(classname);
     b.Put(" ( ");
     b.Put(grammar -> Get_ast_token_classname());
     b.Put(", ");
     for (int i = 0; i < element.interface_.Length() - 1; i++)
     {
         b.Put(typestring[element.interface_[i]]);
         b.Put(", ");
     }
     b.Put(typestring[element.interface_[element.interface_.Length() - 1]]);
    b.Put("):\n");
    b.Put(indentation); b.Put("\n");
    if (element.needs_environment)
        GenerateEnvironmentDeclaration(b, indentation);
    SymbolLookupTable &symbol_set = element.symbol_set;
    if (symbol_set.Size() == 1) // if the right-hand side contains a symbol ...
    {
        b.Put(indentation); b.Put("    def  get");
                                     b.Put(symbol_set[0] -> Name());
                                     b.Put("(self) : return self.leftIToken\n\n");
    }

    b.Put(indentation); b.Put("    "); b.Put("__slots__ = 'environment'\n\n");

    b.Put(indentation); b.Put("    def __init__(self,");
                                 if (element.needs_environment)
                                 {
                                     b.Put("environment");
                                     b.Put(", token):");

                                     b.Put(indentation); b.Put("    \n");
                                     b.Put(indentation);
                                 
                                    {
                                        char temp[1024] = {};
                                        sprintf(temp,"        super(%s, self).__init__(token)\n" , classname);
                                        b.Put(temp);
                                    }
                                     b.Put(indentation); b.Put("        self.environment = environment\n");
                                     b.Put(indentation); b.Put("        self.initialize()\n");
                                     b.Put(indentation); b.Put("    \n");
                                 }
                                 else
                                 {

	                                 b.Put("token  ):\n");
                                     b.Put(indentation);
                                    {
                                        char temp[1024] = {};
                                        sprintf(temp,"    super(%s, self).__init__(token)\n" , classname);
                                        b.Put(temp);
                                    }
                                     b.Put(indentation); b.Put("    self.initialize()\n");
                                 }

    BitSet optimizable_symbol_set(element.symbol_set.Size(), BitSet::UNIVERSE);
  
    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set);

    IntToString num(element.GetRuleNo());
    b.Put(indentation); b.Put("    def getRuleIndex(self): return ") + num.String() + "\n";

    return;
}


//
// Generate Ast class
//
void Python2Action::GenerateMergedClass(CTC &,
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
     b.Put(" ( ");
     b.Put(option -> ast_type);
     b.Put(", ");
     {
         for (int i = 0; i < element.interface_.Length() - 1; i++)
         {
             b.Put(typestring[element.interface_[i]]);
             b.Put(", ");
         }
     }
     b.Put(typestring[element.interface_[element.interface_.Length() - 1]]);
    b.Put("):\n");
    b.Put(indentation); b.Put("\n");
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
    
    b.Put("\n");


    std::vector<std::string> slots_infos;
    if (element.needs_environment) {
        slots_infos.emplace_back("environment");
    }
    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            //bool nullAst = false;
            if ((! optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
            {
                b.Put(indentation); b.Put("    '''/**\n");
                b.Put(indentation); b.Put("     * The value returned by <b>get");
                                             b.Put(symbol_set[i] -> Name());
                                             b.Put("</b> may be <b>null</b>\n");
                b.Put(indentation); b.Put("     */'''\n");
                //nullAst = true;
            }
            std::string name = "_";
            name += symbol_set[i]->Name();
            slots_infos.emplace_back(name);
            b.Put(indentation); b.Put("    def ");
                                         b.Put(" get");
                                         b.Put(symbol_set[i] -> Name());
                                         b.Put("(self) ");
                                       
                                         b.Put(" : return self._");
                                         b.Put(symbol_set[i] -> Name());
                                         b.Put("\n");
        }
    }
    b.Put("\n");

    b.Put(indentation); b.Put("    "); b.Put("__slots__ = ");
    if (slots_infos.empty()) {
        b.Put("()\n\n");
    }
    else
    {
        if (slots_infos.size() > 1) {
            b.Put("(");
        }
        for (size_t i = 0; i < slots_infos.size(); i++) {
            b.Put("'").Put(slots_infos[i].c_str());
            b.Put(i == slots_infos.size() - 1 ? "'" : "', ");
        }
        if (slots_infos.size() > 1) {
            b.Put(")\n\n");
        }
        else {
            b.Put("\n\n");
        }
    }
    //
    // generate merged constructor
    //
    const char *header = "    def __init__(self,";
    b.Put(indentation);
    b.Put(header);
    int length = strlen(indentation) + strlen(header);

  
    if (element.needs_environment)
    {
        b.Put("environment");
        b.Put(", ");
    }
    b.Put("leftIToken  , rightIToken  ");
    b.Put(symbol_set.Size() == 0 ? "):\n" : ",\n");
    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            for (int k = 0; k <= length; k++)
                b.PutChar(' ');
            b.Put(" _");
            b.Put(symbol_set[i] -> Name());
          
         
            b.Put(i == symbol_set.Size() - 1 ? "):\n" : ",\n");
        }
    }

    b.Put(indentation); b.Put("    \n");
    b.Put(indentation);
	
    {
        char temp[1024] = {};
        sprintf(temp,"        super(%s, self).__init__(leftIToken, rightIToken)\n\n" , classname);
        b.Put(temp);
    }
    if (element.needs_environment)
    {
        b.Put(indentation);
        b.Put("        self.environment = environment\n");
    }

    {
        for (int i = 0; i < symbol_set.Size(); i++)
        {
            b.Put(indentation); b.Put("        self._");
                                         b.Put(symbol_set[i] -> Name());
                                         b.Put(" = _");
                                         b.Put(symbol_set[i] -> Name());
                                         b.Put("\n");
    
            if (option -> parent_saved)
            {
                b.Put(indentation); b.Put("        ");
                if ((! optimizable_symbol_set[i]) || ntc.CanProduceNullAst(rhs_type_index[i]))
                {
                    b.Put("if (_");
                    b.Put(symbol_set[i] -> Name());
                    b.Put("): ");
                }
    

                b.Put("_");
                b.Put(symbol_set[i] -> Name());
                b.Put(".setParent(self)\n");
            }
        }
    }

    b.Put(indentation); b.Put("        self.initialize()\n");
    b.Put(indentation); b.Put("    \n");

    if (option -> parent_saved)
        GenerateGetAllChildrenMethod(b, indentation, element);
  
    GenerateVisitorMethods(ntc, b, indentation, element, optimizable_symbol_set);

    IntToString num(element.GetRuleNo());
    b.Put(indentation); b.Put("    def getRuleIndex(self): return ") + num.String() + "\n";

    return;
}

void Python2Action::GenerateAstRootInterface(
    ActionFileSymbol* ast_filename_symbol,
    const char* indentation)
{
    TextBuffer& b = *GetBuffer(ast_filename_symbol);
    b.Put("class ");
    b.Put(astRootInterfaceName.c_str());

    b.Put("(object):\n");
    b.Put(indentation); b.Put("\n");
    b.Put(indentation); b.Put("    "); b.Put(EMPTY_SLOTS); b.Put("\n");
    b.Put(indentation); b.Put("    def  getLeftIToken(self)  : raise TypeError('Can not instantiate abstract class  with abstract methods')\n");
    b.Put(indentation); b.Put("    def  getRightIToken(self)  : raise TypeError('Can not instantiate abstract class  with abstract methods')\n");

    b.Put("\n");
    GenerateVisitorHeaders(b, indentation, "    ");
    b.Put(indentation); b.Put("\n\n");

    return;
}


void Python2Action::GenerateInterface(bool is_terminal,
    ActionFileSymbol* ast_filename_symbol,
    const char* indentation,
    const char* interface_name,
    Tuple<int>&,
    Tuple<int>& classes,
    Tuple<ClassnameElement>& classname)
{
    TextBuffer& b = *GetBuffer(ast_filename_symbol);
    b.Put("'''/**");
    if (is_terminal)
    {
        b.Put("\n");
        b.Put(" * is always implemented by <b>");
        b.Put(grammar->Get_ast_token_classname());
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

    b.Put(" */'''\n");

    b.Put("class ");
    b.Put(interface_name);
 /*   if (extension.Length() > 0)
    {
        b.Put(" ( ");
        for (int k = 0; k < extension.Length() - 1; k++)
        {
            b.PutChar('I');
            b.Put(extension[k] == grammar->Get_ast_token_interface()
                ? grammar->Get_ast_token_classname()
                : grammar->RetrieveString(extension[k]));
            b.Put(", ");
        }
        b.PutChar('I');
        b.Put(extension[extension.Length() - 1] == grammar->Get_ast_token_interface()
            ? grammar->Get_ast_token_classname()
            : grammar->RetrieveString(extension[extension.Length() - 1]));
        b.Put("):\n");
        b.Put(indentation); b.Put("    "); b.Put(EMPTY_SLOTS);
        b.Put("\n\n");
    }
    else*/
    {
        b.Put("(");
        b.Put(astRootInterfaceName.c_str());
        b.Put("):\n");
        b.Put(indentation); b.Put("    "); b.Put(EMPTY_SLOTS); 
        b.Put("\n\n");
    }

}



//
//
//
void Python2Action::GenerateNullAstAllocation(TextBuffer &b, int rule_no)
{
    const char *code = "\n                    self.setResult(None)";
    GenerateCode(&b, code, rule_no);

    return;
}


//
// Emit the getProstheticAst() accessor that returns prosthetic-AST factories.
// This closes the loop for Python backtracking recovery: when the backtracking
// parser replays a nonterminal ErrorToken (inserted by scope recovery for a
// %Recover symbol), it looks the factory up by getProsthesisIndex(kind) and
// invokes it with the error token to build a placeholder node instead of
// throwing. RuleAction supplies a default returning None so grammars without
// %Recover keep the historical throw behavior.
//
void Python2Action::EmitProstheticAstFactories(ActionFileSymbol *default_file_symbol)
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
    b.Put("\n    #\n"
          "    # Prosthetic-AST factories for %Recover nonterminals. Indexed by\n"
          "    # getProsthesisIndex(kind); unused slots stay None. Optional recover\n"
          "    # action blocks (/. ... ./) supply the create expression and may\n"
          "    # reference the parameter error_token.\n"
          "    #\n");
    b.Put("    def getProstheticAst(self):\n");
    b.Put("        prostheticAst = [None] * ");
    b.Put(array_size.String());
    b.Put("\n");
    for (int i = 0; i < recover_nonterminals.Length(); i++)
    {
        int symbol = recover_nonterminals[i];
        IntToString slot(symbol - grammar -> num_terminals);
        b.Put("        prostheticAst[");
        b.Put(slot.String());
        b.Put("] = lambda error_token: ");

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
            b.Put(grammar -> Get_ast_token_classname());
            b.Put("(error_token)");
        }
        b.Put("\n");
    }
    b.Put("        return prostheticAst\n");

    return;
}


//
//
//
void Python2Action::GenerateAstAllocation(CTC &,
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
               *newkey = "",
               *lparen = "(",
               *comma = ",",
               *rparen = ")",
               *trailer = ")";
    int extra_space_length = strlen(space) + strlen(space4) + strlen(newkey) + strlen(classname) + 1;
    char *extra_space = new char[extra_space_length + 1];
    extra_space[0] = '\n';
    {
        for (int i = 1; i < extra_space_length; i++)
            extra_space[i] = ' ';
    }
    extra_space[extra_space_length] = '\0';

    GenerateTerminalGcDeleteReminder(b, space, rule_no, allocation_element, "self.getRhsSym(1)", "#");
    GenerateCode(&b, space, rule_no);
    GenerateCode(&b, "self.setResult(", rule_no);
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
        GenerateCode(&b, "self, ", rule_no);
    }
    if (allocation_element.is_terminal_class)
    {
        GenerateCode(&b, "self.getRhsIToken(1)", rule_no);
    }
    else
    {
        GenerateCode(&b, "self.getLeftIToken()", rule_no);
        GenerateCode(&b, ", ", rule_no);
        GenerateCode(&b, "self.getRightIToken()", rule_no);
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
                    GenerateCode(&b, "None", rule_no);
                }
                else
                {
                    int symbol = grammar -> rhs_sym[offset + position[i]];
                    if (grammar -> IsTerminal(symbol))
                    {

                        GenerateCode(&b, newkey, rule_no);
                        GenerateCode(&b, grammar -> Get_ast_token_classname(), rule_no);
                        GenerateCode(&b, lparen, rule_no);
                        GenerateCode(&b, "self.getRhsIToken(", rule_no);
                        IntToString index(position[i]);
                        GenerateCode(&b, index.String(), rule_no);
                        GenerateCode(&b, rparen, rule_no);
                    }
                    else
                    {
                        GenerateCode(&b, "self.getRhsSym(", rule_no);
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
void Python2Action::GenerateListAllocation(CTC &,
                                           NTC&,
                                           TextBuffer &b,
                                           int rule_no, RuleAllocationElement &allocation_element)
{
    const char *space = "\n                    ",
               *space4 = "    ",
               *newkey = "",
               *lparen = "(",
               *comma = ",",
               *rparen = ")",
               *trailer = ")";

    if (allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_EMPTY ||
        allocation_element.list_kind == RuleAllocationElement::RIGHT_RECURSIVE_EMPTY ||
        allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_SINGLETON ||
        allocation_element.list_kind == RuleAllocationElement::RIGHT_RECURSIVE_SINGLETON)
    {

        GenerateCode(&b, space, rule_no);
        GenerateCode(&b, "self.setResult(", rule_no);
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
                GenerateCode(&b, "self, ", rule_no);
            }

            GenerateCode(&b, "self.getLeftIToken()", rule_no);
            GenerateCode(&b, ", ", rule_no);
            GenerateCode(&b, "self.getRightIToken()", rule_no);
            GenerateCode(&b, comma, rule_no);
            if (allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_EMPTY)
                 GenerateCode(&b, " True ", rule_no);
            else
                GenerateCode(&b, " False ", rule_no);
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
                GenerateCode(&b, "self, ", rule_no);
            }
            assert(allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_SINGLETON ||
                   allocation_element.list_kind == RuleAllocationElement::RIGHT_RECURSIVE_SINGLETON);

            if (grammar -> IsTerminal(allocation_element.element_symbol))
            {
                GenerateCode(&b, newkey, rule_no);
                GenerateCode(&b, grammar -> Get_ast_token_classname(), rule_no);
                GenerateCode(&b, lparen, rule_no);
                GenerateCode(&b, "self.getRhsIToken(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&b, index.String(), rule_no);
                GenerateCode(&b, rparen, rule_no);
            }
            else
            {
     
                GenerateCode(&b, "self.getRhsSym(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&b, index.String(), rule_no);
            }
    
            GenerateCode(&b, rparen, rule_no);
            GenerateCode(&b, comma, rule_no);
            if (allocation_element.list_kind == RuleAllocationElement::LEFT_RECURSIVE_SINGLETON)
                GenerateCode(&b, " True ", rule_no);
  
            else 
                GenerateCode(&b, " False ", rule_no);
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
         
            GenerateCode(&b, "self.getRhsSym(", rule_no);
            IntToString index(allocation_element.list_position);
            GenerateCode(&b, index.String(), rule_no);
            GenerateCode(&b, ")).addElement(", rule_no);
            if (grammar -> IsTerminal(allocation_element.element_symbol))
            {
                GenerateCode(&b, newkey, rule_no);
                GenerateCode(&b, grammar -> Get_ast_token_classname(), rule_no);
                GenerateCode(&b, lparen, rule_no);
                GenerateCode(&b, "self.getRhsIToken(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&b, index.String(), rule_no);
                GenerateCode(&b, rparen, rule_no);
            }
            else
            {
                GenerateCode(&b, "self.getRhsSym(", rule_no);
                IntToString index(allocation_element.element_position);
                GenerateCode(&b, index.String(), rule_no);
            }

            if (allocation_element.list_position != 1) // a right-recursive rule? set the list as result
            {
                GenerateCode(&b, rparen, rule_no);
                GenerateCode(&b, trailer, rule_no);

                GenerateCode(&b, space, rule_no);
                GenerateCode(&b, "self.setResult(", rule_no);
                GenerateCode(&b, "self.getRhsSym(", rule_no);
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
            GenerateCode(&b, "self.setResult(", rule_no);
            GenerateCode(&b, "self.getRhsSym(", rule_no);
            IntToString index(allocation_element.list_position);
            GenerateCode(&b, index.String(), rule_no);
        }

        GenerateCode(&b, rparen, rule_no);
    }

    GenerateCode(&b, trailer, rule_no);
 
    return;
}
