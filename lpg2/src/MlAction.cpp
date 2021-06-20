#include "MlAction.h"

//
//
//
void MlAction::ProcessRuleActionBlock(ActionBlockElement &action)
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
void MlAction::ExpandExportMacro(TextBuffer *buffer, SimpleMacroSymbol *simple_macro)
{
    buffer -> Put(option -> exp_type);
    buffer -> Put(".");
    buffer -> Put(option -> exp_prefix);
    buffer -> Put(simple_macro -> Name() + 2); // skip initial escape and '_' characters
    buffer -> Put(option -> exp_suffix);
}


void MlAction::GenerateDefaultTitle(Tuple<ActionBlockElement> &) {}
ActionFileSymbol *MlAction::GenerateTitle(ActionFileLookupTable &, Tuple<ActionBlockElement> &, const char *, bool) { return NULL;}
ActionFileSymbol *MlAction::GenerateTitleAndGlobals(ActionFileLookupTable &, Tuple<ActionBlockElement> &, const char *, bool) { return NULL;}

void MlAction::GenerateVisitorHeaders(TextBuffer &, const char *, const char *) {}
void MlAction::GenerateVisitorMethods(NTC &, TextBuffer &, const char *, ClassnameElement &, BitSet &) {}
void MlAction::GenerateGetAllChildrenMethod(TextBuffer &, const char *, ClassnameElement &) {}
void MlAction::GenerateEqualsMethod(NTC &, TextBuffer &, const char *, ClassnameElement &, BitSet &) {}
void MlAction::GenerateHashcodeMethod(NTC &, TextBuffer &, const char *, ClassnameElement &, BitSet &) {}

void MlAction::GenerateSimpleVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void MlAction::GenerateArgumentVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void MlAction::GenerateResultVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void MlAction::GenerateResultArgumentVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}

void MlAction::GeneratePreorderVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void MlAction::GeneratePreorderVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}

void MlAction::GenerateNoResultVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void MlAction::GenerateResultVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}

void MlAction::GenerateAstType(ActionFileSymbol* ast_filename_symbol, const char *, const char *) {}
void MlAction::GenerateAbstractAstListType(ActionFileSymbol* ast_filename_symbol, const char *, const char *) {}
void MlAction::GenerateAstTokenType(NTC &, ActionFileSymbol*, const char *, const char *) {}
void MlAction::GenerateInterface(bool, ActionFileSymbol*, const char *, const char *, Tuple<int> &, Tuple<int> &, Tuple<ClassnameElement> &) {}
void MlAction::GenerateCommentHeader(TextBuffer &, const char *, Tuple<int> &, Tuple<int> &) {}
void MlAction::GenerateListExtensionClass(CTC &, NTC &, ActionFileSymbol*, const char *, SpecialArrayElement &, ClassnameElement &, Array<const char *> &) {}
void MlAction::GenerateListClass(CTC &, NTC &, ActionFileSymbol*, const char *, ClassnameElement &, Array<const char *> &) {}
void MlAction::GenerateRuleClass(CTC &, NTC &, ActionFileSymbol*, const char *, ClassnameElement &, Array<const char *> &) {}
void MlAction::GenerateMergedClass(CTC &,
                                   NTC &,
                                   ActionFileSymbol*,
                                   const char *,
                                   ClassnameElement &,
                                   Tuple< Tuple<ProcessedRuleElement> > &,
                                   Array<const char *> &) {}
void MlAction::GenerateTerminalMergedClass(NTC &, ActionFileSymbol*, const char *, ClassnameElement &, Array<const char *> &) {}
void MlAction::GenerateNullAstAllocation(TextBuffer &, int rule_no) {}
void MlAction::GenerateEnvironmentDeclaration(TextBuffer &, const char *) {}
void MlAction::GenerateListAllocation(CTC &ctc, TextBuffer &, int, RuleAllocationElement &) {}
void MlAction::GenerateAstAllocation(CTC &ctc,
                                     TextBuffer &,
                                     RuleAllocationElement &,
                                     Tuple<ProcessedRuleElement> &,
                                     Array<const char *> &, int) {}
