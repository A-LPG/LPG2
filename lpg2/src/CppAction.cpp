#include "CppAction.h"

//
//
//
void CppAction::ProcessRuleActionBlock(ActionBlockElement &action)
{
    //
    // TODO: Do whatever preprocessing that is required here!
    //

    ProcessActionBlock(action);

    //
    // TODO: Do whatever postprocessing that is required here!
    //
}

//
//
//
void CppAction::ExpandExportMacro(TextBuffer *buffer, SimpleMacroSymbol *simple_macro)
{
    buffer -> Put(option -> exp_prefix);
    buffer -> Put(simple_macro -> Name() + 2); // skip initial escape and '_' characters
    buffer -> Put(option -> exp_suffix);
}


void CppAction::GenerateDefaultTitle(Tuple<ActionBlockElement> &) {}
ActionFileSymbol *CppAction::GenerateTitle(ActionFileLookupTable &, Tuple<ActionBlockElement> &, const char *, bool) { return NULL;}
ActionFileSymbol *CppAction::GenerateTitleAndGlobals(ActionFileLookupTable &, Tuple<ActionBlockElement> &, const char *, bool) { return NULL;}

void CppAction::GenerateVisitorHeaders(TextBuffer &, const char *, const char *) {}
void CppAction::GenerateVisitorMethods(NTC &, TextBuffer &, const char *, ClassnameElement &, BitSet &) {}
void CppAction::GenerateGetAllChildrenMethod(TextBuffer &, const char *, ClassnameElement &) {}
void CppAction::GenerateEqualsMethod(NTC &, TextBuffer &, const char *, ClassnameElement &, BitSet &) {}
void CppAction::GenerateHashcodeMethod(NTC &, TextBuffer &, const char *, ClassnameElement &, BitSet &) {}

void CppAction::GenerateSimpleVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void CppAction::GenerateArgumentVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void CppAction::GenerateResultVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void CppAction::GenerateResultArgumentVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}

void CppAction::GeneratePreorderVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void CppAction::GeneratePreorderVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}

void CppAction::GenerateNoResultVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void CppAction::GenerateResultVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}

void CppAction::GenerateAstType(ActionFileSymbol* ast_filename_symbol, const char *, const char *) {}
void CppAction::GenerateAbstractAstListType(ActionFileSymbol* ast_filename_symbol, const char *, const char *) {}
void CppAction::GenerateAstTokenType(NTC &, ActionFileSymbol*, const char *, const char *) {}
void CppAction::GenerateInterface(bool, ActionFileSymbol*, const char *, const char *, Tuple<int> &, Tuple<int> &, Tuple<ClassnameElement> &) {}
void CppAction::GenerateCommentHeader(TextBuffer &, const char *, Tuple<int> &, Tuple<int> &) {}
void CppAction::GenerateListExtensionClass(CTC &, NTC &, ActionFileSymbol*, const char *, SpecialArrayElement &, ClassnameElement &, Array<const char *> &) {}
void CppAction::GenerateListClass(CTC &, NTC &, ActionFileSymbol*, const char *, ClassnameElement &, Array<const char *> &) {}
void CppAction::GenerateRuleClass(CTC &, NTC &, ActionFileSymbol*, const char *, ClassnameElement &, Array<const char *> &) {}
void CppAction::GenerateMergedClass(CTC &,
                                    NTC &,
                                    ActionFileSymbol*,
                                    const char *,
                                    ClassnameElement &,
                                    Tuple< Tuple<ProcessedRuleElement> > &,
                                    Array<const char *> &) {}
void CppAction::GenerateTerminalMergedClass(NTC &, ActionFileSymbol*, const char *, ClassnameElement &, Array<const char *> &) {}
void CppAction::GenerateNullAstAllocation(TextBuffer &, int rule_no) {}
void CppAction::GenerateEnvironmentDeclaration(TextBuffer &, const char *) {}
void CppAction::GenerateListAllocation(CTC &ctc, NTC&, TextBuffer &, int, RuleAllocationElement &) {}
void CppAction::GenerateAstAllocation(CTC &ctc,
                                      NTC&,
                                      TextBuffer &,
                                      RuleAllocationElement &,
                                      Tuple<ProcessedRuleElement> &, Array<const char *> &, int) {}
