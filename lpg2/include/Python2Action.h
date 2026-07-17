#pragma once

#include "Action.h"
#include "control.h"

class Python2Action : public Action
{
public:
    TextBuffer* GetBuffer(ActionFileSymbol*) const;
    Python2Action(Control *control_, Blocks *action_blocks_, Grammar *grammar_, MacroLookupTable *macro_table_)
              : Action(control_, action_blocks_, grammar_, macro_table_)
    {}
    virtual ~Python2Action() {}

   
    virtual void ExpandExportMacro(TextBuffer *, SimpleMacroSymbol *);

    virtual const char *GetDefaultTerminalType() { return "IToken"; }
    virtual const char *GetDefaultNonterminalType() { return "object"; }
    virtual void GenerateDefaultTitle(Tuple<ActionBlockElement> &);
    virtual ActionFileSymbol *GenerateTitle(ActionFileLookupTable &, Tuple<ActionBlockElement> &, const char *, bool);
    virtual ActionFileSymbol *GenerateTitleAndGlobals(ActionFileLookupTable &, Tuple<ActionBlockElement> &, const char *, bool);

    virtual void GenerateVisitorHeaders(TextBuffer &, const char *, const char *);
    virtual void GenerateVisitorMethods(NTC &, TextBuffer &, const char *, ClassnameElement &, BitSet &);
    virtual void GenerateGetAllChildrenMethod(TextBuffer &, const char *, ClassnameElement &);
   
    virtual void GenerateSimpleVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &);
    virtual void GenerateArgumentVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &);
    virtual void GenerateResultVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &);
    virtual void GenerateResultArgumentVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &);

    virtual void GeneratePreorderVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &);
    virtual void GeneratePreorderVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &);

    virtual void GenerateNoResultVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &);
    virtual void GenerateResultVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &);

    virtual void GenerateAstType(ActionFileSymbol* , const char *, const char *);
    virtual void GenerateAbstractAstListType(ActionFileSymbol* , const char *, const char *);
    virtual void GenerateAstTokenType(NTC &, ActionFileSymbol*, const char *, const char *);
    void GenerateAstRootInterface(ActionFileSymbol* ast_filename_symbol, const char* indentation);
    virtual void GenerateInterface(bool, ActionFileSymbol*, const char *, const char *, Tuple<int> &, Tuple<int> &, Tuple<ClassnameElement> &);
    virtual void GenerateCommentHeader(TextBuffer &, const char *, Tuple<int> &, Tuple<int> &);
    virtual void GenerateListExtensionClass(CTC &, NTC &, ActionFileSymbol*, const char *, SpecialArrayElement &, ClassnameElement &, Array<const char *> &);
    virtual void GenerateListClass(CTC &, NTC &, ActionFileSymbol*, const char *, ClassnameElement &, Array<const char *> &);
    virtual void GenerateRuleClass(CTC &, NTC &, ActionFileSymbol*, const char *, ClassnameElement &, Array<const char *> &);
    virtual void GenerateMergedClass(CTC &,
                                     NTC &,
                                     ActionFileSymbol*,
                                     const char *,
                                     ClassnameElement &,
                                     Tuple< Tuple<ProcessedRuleElement> > &,
                                     Array<const char *> &);
   
    virtual void GenerateTerminalMergedClass(NTC &, ActionFileSymbol*, const char *, ClassnameElement &, Array<const char *> &);
    virtual void GenerateNullAstAllocation(TextBuffer &, int rule_no);
    virtual void GenerateEnvironmentDeclaration(TextBuffer &, const char *);
    void ProcessCodeActionEnd();
    // ProcessAstActions is inherited from Action; behavior is expressed via the hooks below.
    virtual void GenerateListAllocation(CTC &ctc, NTC&, TextBuffer &, int, RuleAllocationElement &);
    virtual void GenerateAstAllocation(CTC &ctc,
                                       NTC&,
                                       TextBuffer &,
                                       RuleAllocationElement &,
                                       Tuple<ProcessedRuleElement> &, Array<const char *> &, int);

    void GenerateListMethods(CTC &, NTC &, TextBuffer &, const char *, const char *, ClassnameElement &, Array<const char *> &);
protected:
    // Shared ProcessAstActions hooks.
    TextBuffer *AstCodeBuffer(ActionFileSymbol *file) override;
    void EmitAstClassCloser(TextBuffer &code_buf, ActionFileSymbol *top_level_file, bool list_extension_closer) override;
    void MaybeEmitAstRootInterface(ActionFileLookupTable &ast_filename_table,
                                   ActionFileSymbol *default_file_symbol,
                                   Tuple<ActionBlockElement> &notice_actions) override;
    bool EmitInterfacesBeforeRootTypes() const override { return true; }
    AstInterfaceEmitOrder GetAstInterfaceEmitOrder() const override { return AstInterfaceEmitOrder::ExtensionsLast; }
    void EmitProstheticAstFactories(ActionFileSymbol *default_file_symbol) override;
private:
    std::string astRootInterfaceName;
    inline  static const char* EMPTY_SLOTS = "__slots__ = ()";
};

