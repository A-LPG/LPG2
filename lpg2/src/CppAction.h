#ifndef CppAction_INCLUDED
#define CppAction_INCLUDED

#include "tuple.h"
#include "CAction.h"

class CppAction : public CAction
{
public:

    CppAction(Control *control_, Blocks *action_blocks_, Grammar *grammar_, MacroLookupTable *macro_table_)
             : CAction(control_, action_blocks_, grammar_, macro_table_)
    {
        if (option -> automatic_ast != Option::NONE)
        {
            control_ -> option -> EmitError(0, "Cannot automatically generate AST for programming language Cpp");
            return_code = 12;
        }
    }
    virtual ~CppAction() {}

    virtual void ProcessRuleActionBlock(ActionBlockElement &);

    virtual void ExpandExportMacro(TextBuffer *, SimpleMacroSymbol *);

    virtual const char *GetDefaultTerminalType() { return "void *"; }
    virtual const char *GetDefaultNonterminalType() { return "void *"; }
    virtual void GenerateDefaultTitle(Tuple<ActionBlockElement> &);
    virtual ActionFileSymbol *GenerateTitle(ActionFileLookupTable &, Tuple<ActionBlockElement> &, const char *, bool);
    virtual ActionFileSymbol *GenerateTitleAndGlobals(ActionFileLookupTable &, Tuple<ActionBlockElement> &, const char *, bool);

    virtual void GenerateVisitorHeaders(TextBuffer &, const char *, const char *);
    virtual void GenerateVisitorMethods(NTC &, TextBuffer &, const char *, ClassnameElement &, BitSet &);
    virtual void GenerateGetAllChildrenMethod(TextBuffer &, const char *, ClassnameElement &);
    virtual void GenerateEqualsMethod(NTC &, TextBuffer &, const char *, ClassnameElement &, BitSet &);
    virtual void GenerateHashcodeMethod(NTC &, TextBuffer &, const char *, ClassnameElement &, BitSet &);

    virtual void GenerateSimpleVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &);
    virtual void GenerateArgumentVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &);
    virtual void GenerateResultVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &);
    virtual void GenerateResultArgumentVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &);

    virtual void GeneratePreorderVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &);
    virtual void GeneratePreorderVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &);

    virtual void GenerateNoResultVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &);
    virtual void GenerateResultVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &);

    virtual void GenerateAstType(ActionFileSymbol* ast_filename_symbol, const char *, const char *);
    virtual void GenerateAbstractAstListType(ActionFileSymbol* ast_filename_symbol, const char *, const char *);
    virtual void GenerateAstTokenType(NTC &, ActionFileSymbol*, const char *, const char *);
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
    virtual void GenerateListAllocation(CTC &ctc, TextBuffer &, int, RuleAllocationElement &);
    virtual void GenerateAstAllocation(CTC &ctc,
                                       TextBuffer &,
                                       RuleAllocationElement &,
                                       Tuple<ProcessedRuleElement> &,
                                       Array<const char *> &, int);
};

#endif /* CppAction_INCLUDED */
