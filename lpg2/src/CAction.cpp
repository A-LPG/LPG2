#include "CAction.h"

//
//
//
void CAction::ProcessRuleActionBlock(ActionBlockElement &action)
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
void CAction::ExpandExportMacro(TextBuffer *buffer, SimpleMacroSymbol *simple_macro)
{
    buffer -> Put(option -> exp_type);
    buffer -> Put(".");
    buffer -> Put(option -> exp_prefix);
    buffer -> Put(simple_macro -> Name() + 2); // skip initial escape and '_' characters
    buffer -> Put(option -> exp_suffix);
}


void CAction::GenerateDefaultTitle(Tuple<ActionBlockElement> &) {}
ActionFileSymbol *CAction::GenerateTitle(ActionFileLookupTable &, Tuple<ActionBlockElement> &, const char *, bool) { return NULL;}
ActionFileSymbol *CAction::GenerateTitleAndGlobals(ActionFileLookupTable &, Tuple<ActionBlockElement> &, const char *, bool) { return NULL;}

void CAction::GenerateVisitorHeaders(TextBuffer &, const char *, const char *) {}
void CAction::GenerateVisitorMethods(NTC &, TextBuffer &, const char *, ClassnameElement &, BitSet &) {}
void CAction::GenerateGetAllChildrenMethod(TextBuffer &, const char *, ClassnameElement &) {}
void CAction::GenerateEqualsMethod(NTC &, TextBuffer &, const char *, ClassnameElement &, BitSet &) {}
void CAction::GenerateHashcodeMethod(NTC &, TextBuffer &, const char *, ClassnameElement &, BitSet &) {}

void CAction::GenerateSimpleVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void CAction::GenerateArgumentVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void CAction::GenerateResultVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void CAction::GenerateResultArgumentVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}

void CAction::GeneratePreorderVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void CAction::GeneratePreorderVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}

void CAction::GenerateNoResultVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void CAction::GenerateResultVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}

void CAction::GenerateAstType(ActionFileSymbol* ast_filename_symbol, const char *, const char *) {}
void CAction::GenerateAbstractAstListType(ActionFileSymbol* ast_filename_symbol, const char *, const char *) {}
void CAction::GenerateAstTokenType(NTC &, ActionFileSymbol*, const char *, const char *) {}
void CAction::GenerateInterface(bool, ActionFileSymbol*, const char *, const char *, Tuple<int> &, Tuple<int> &, Tuple<ClassnameElement> &) {}
void CAction::GenerateCommentHeader(TextBuffer &, const char *, Tuple<int> &, Tuple<int> &) {}
void CAction::GenerateListExtensionClass(CTC &, NTC &, ActionFileSymbol*, const char *, SpecialArrayElement &, ClassnameElement &, Array<const char *> &) {}
void CAction::GenerateListClass(CTC &, NTC &, ActionFileSymbol*, const char *, ClassnameElement &, Array<const char *> &) {}
void CAction::GenerateRuleClass(CTC &, NTC &, ActionFileSymbol*, const char *, ClassnameElement &, Array<const char *> &) {}
void CAction::GenerateMergedClass(CTC &,
                                  NTC &,
                                  ActionFileSymbol*,
                                  const char *,
                                  ClassnameElement &,
                                  Tuple< Tuple<ProcessedRuleElement> > &,
                                  Array<const char *> &) {}
void CAction::GenerateTerminalMergedClass(NTC &, ActionFileSymbol*, const char *, ClassnameElement &, Array<const char *> &) {}
void CAction::GenerateNullAstAllocation(TextBuffer &, int rule_no) {}
void CAction::GenerateEnvironmentDeclaration(TextBuffer &, const char *) {}
void CAction::GenerateListAllocation(CTC &ctc, NTC&, TextBuffer &, int, RuleAllocationElement &) {}
void CAction::GenerateAstAllocation(CTC &ctc,
                                    NTC&,
                                    TextBuffer &,
                                    RuleAllocationElement &,
                                    Tuple<ProcessedRuleElement> &, Array<const char *> &, int) {}
