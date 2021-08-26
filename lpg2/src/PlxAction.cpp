#include "PlxAction.h"

//
//
//
void PlxAction::ProcessRuleActionBlock(ActionBlockElement &action)
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
void PlxAction::ExpandExportMacro(TextBuffer *buffer, SimpleMacroSymbol *simple_macro)
{
    buffer -> Put(option -> exp_type);
    buffer -> Put(".");
    buffer -> Put(option -> exp_prefix);
    buffer -> Put(simple_macro -> Name() + 2); // skip initial escape and '_' characters
    buffer -> Put(option -> exp_suffix);
}


void PlxAction::GenerateDefaultTitle(Tuple<ActionBlockElement> &) {}
ActionFileSymbol *PlxAction::GenerateTitle(ActionFileLookupTable &, Tuple<ActionBlockElement> &, const char *, bool) { return NULL;}
ActionFileSymbol *PlxAction::GenerateTitleAndGlobals(ActionFileLookupTable &, Tuple<ActionBlockElement> &, const char *, bool) { return NULL;}

void PlxAction::GenerateVisitorHeaders(TextBuffer &, const char *, const char *) {}
void PlxAction::GenerateVisitorMethods(NTC &, TextBuffer &, const char *, ClassnameElement &, BitSet &) {}
void PlxAction::GenerateGetAllChildrenMethod(TextBuffer &, const char *, ClassnameElement &) {}
void PlxAction::GenerateEqualsMethod(NTC &, TextBuffer &, const char *, ClassnameElement &, BitSet &) {}
void PlxAction::GenerateHashcodeMethod(NTC &, TextBuffer &, const char *, ClassnameElement &, BitSet &) {}

void PlxAction::GenerateSimpleVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void PlxAction::GenerateArgumentVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void PlxAction::GenerateResultVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void PlxAction::GenerateResultArgumentVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}

void PlxAction::GeneratePreorderVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void PlxAction::GeneratePreorderVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}

void PlxAction::GenerateNoResultVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}
void PlxAction::GenerateResultVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) {}

void PlxAction::GenerateAstType(ActionFileSymbol* ast_filename_symbol, const char *, const char *) {}
void PlxAction::GenerateAbstractAstListType(ActionFileSymbol* ast_filename_symbol, const char *, const char *) {}
void PlxAction::GenerateAstTokenType(NTC &, ActionFileSymbol*, const char *, const char *) {}
void PlxAction::GenerateInterface(bool, ActionFileSymbol*, const char *, const char *, Tuple<int> &, Tuple<int> &, Tuple<ClassnameElement> &) {}
void PlxAction::GenerateCommentHeader(TextBuffer &, const char *, Tuple<int> &, Tuple<int> &) {}
void PlxAction::GenerateListExtensionClass(CTC &, NTC &, ActionFileSymbol*, const char *, SpecialArrayElement &, ClassnameElement &, Array<const char *> &) {}
void PlxAction::GenerateListClass(CTC &, NTC &, ActionFileSymbol*, const char *, ClassnameElement &, Array<const char *> &) {}
void PlxAction::GenerateRuleClass(CTC &, NTC &, ActionFileSymbol*, const char *, ClassnameElement &, Array<const char *> &) {}
void PlxAction::GenerateMergedClass(CTC &,
                                    NTC &,
                                    ActionFileSymbol*,
                                    const char *,
                                    ClassnameElement &,
                                    Tuple< Tuple<ProcessedRuleElement> > &,
                                    Array<const char *> &) {}
void PlxAction::GenerateTerminalMergedClass(NTC &, ActionFileSymbol*, const char *, ClassnameElement &, Array<const char *> &) {}
void PlxAction::GenerateNullAstAllocation(TextBuffer &, int rule_no) {}
void PlxAction::GenerateEnvironmentDeclaration(TextBuffer &, const char *) {}
void PlxAction::GenerateListAllocation(CTC &ctc, NTC&, TextBuffer &, int, RuleAllocationElement &) {}
void PlxAction::GenerateAstAllocation(CTC &ctc,
                                      NTC&,
                                      TextBuffer &,
                                      RuleAllocationElement &,
                                      Tuple<ProcessedRuleElement> &, Array<const char *> &, int) {}
