#include "PlxasmAction.h"

//
//
//
void PlxasmAction::ProcessRuleActionBlock(ActionBlockElement &action)
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
void PlxasmAction::ExpandExportMacro(TextBuffer *buffer, SimpleMacroSymbol *simple_macro)
{
    buffer -> Put(option -> exp_type);
    buffer -> Put(".");
    buffer -> Put(option -> exp_prefix);
    buffer -> Put(simple_macro -> Name() + 2); // skip initial escape and '_' characters
    buffer -> Put(option -> exp_suffix);
}


void PlxasmAction::GenerateDefaultTitle(Tuple<ActionBlockElement> &) {}
ActionFileSymbol *PlxasmAction::GenerateTitle(ActionFileLookupTable &, Tuple<ActionBlockElement> &, const char *, bool) { return NULL;}
ActionFileSymbol *PlxasmAction::GenerateTitleAndGlobals(ActionFileLookupTable &, Tuple<ActionBlockElement> &, const char *, bool) { return NULL;}

void PlxasmAction::GenerateVisitorHeaders(TextBuffer &, const char *, const char *) {}
void PlxasmAction::GenerateVisitorMethods(NTC &, TextBuffer &, const char *, ClassnameElement &, BitSet &) {}
void PlxasmAction::GenerateGetAllChildrenMethod(TextBuffer &, const char *, ClassnameElement &) {}
void PlxasmAction::GenerateEqualsMethod(NTC &, TextBuffer &, const char *, ClassnameElement &, BitSet &) {}
void PlxasmAction::GenerateHashcodeMethod(NTC &, TextBuffer &, const char *, ClassnameElement &, BitSet &) {}

void PlxasmAction::GenerateSimpleVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void PlxasmAction::GenerateArgumentVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void PlxasmAction::GenerateResultVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void PlxasmAction::GenerateResultArgumentVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}

void PlxasmAction::GeneratePreorderVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void PlxasmAction::GeneratePreorderVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}

void PlxasmAction::GenerateNoResultVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void PlxasmAction::GenerateResultVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}

void PlxasmAction::GenerateAstType(ActionFileSymbol*, const char *, const char *) {}
void PlxasmAction::GenerateAbstractAstListType(ActionFileSymbol*, const char *, const char *) {}
void PlxasmAction::GenerateAstTokenType(NTC &, ActionFileSymbol*, const char *, const char *) {}
void PlxasmAction::GenerateInterface(bool, ActionFileSymbol*, const char *, const char *, Tuple<int> &, Tuple<int> &, Tuple<ClassnameElement> &) {}
void PlxasmAction::GenerateCommentHeader(TextBuffer &, const char *, Tuple<int> &, Tuple<int> &) {}
void PlxasmAction::GenerateListExtensionClass(CTC &, NTC &, ActionFileSymbol*, const char *, SpecialArrayElement &, ClassnameElement &, Array<const char *> &) {}
void PlxasmAction::GenerateListClass(CTC &, NTC &, ActionFileSymbol*, const char *, ClassnameElement &, Array<const char *> &) {}
void PlxasmAction::GenerateRuleClass(CTC &, NTC &, ActionFileSymbol*, const char *, ClassnameElement &, Array<const char *> &) {}
void PlxasmAction::GenerateMergedClass(CTC &,
                                       NTC &,
                                       ActionFileSymbol*,
                                       const char *,
                                       ClassnameElement &,
                                       Tuple< Tuple<ProcessedRuleElement> > &,
                                       Array<const char *> &) {}
void PlxasmAction::GenerateTerminalMergedClass(NTC &, ActionFileSymbol*, const char *, ClassnameElement &, Array<const char *> &) {}
void PlxasmAction::GenerateNullAstAllocation(TextBuffer &, int) {}
void PlxasmAction::GenerateEnvironmentDeclaration(TextBuffer &, const char *) {}
void PlxasmAction::GenerateListAllocation(CTC &, NTC&, TextBuffer &, int, RuleAllocationElement &) {}
void PlxasmAction::GenerateAstAllocation(CTC &,
                                         NTC&,
                                         TextBuffer &,
                                         RuleAllocationElement &,
                                         Tuple<ProcessedRuleElement> &, Array<const char *> &, int) {}
