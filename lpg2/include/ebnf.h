#ifndef ebnf_INCLUDED
#define ebnf_INCLUDED

#include "tuple.h"
#include "parser.h"
#include "LexStream.h"
#include "option.h"
#include "symbol.h"

//
// Expand postfix EBNF sugar in parser.rules into ordinary BNF RuleDefinitions
// before Grammar::ProcessRules. Requires Option::ebnf == true and that the
// scanner already split ? * + ( ) [ ] { } into single-character SYMBOL tokens.
//
class EbnfExpander
{
public:
    EbnfExpander(Option *option_,
                 LexStream *lex_stream_,
                 VariableLookupTable *variable_table_,
                 MacroLookupTable *macro_table_,
                 jikespg_act &parser_);

    // Returns false if a fatal diagnostic was emitted.
    // When save_original is non-null, copies pre-desugar rules into it for -list.
    bool Expand(Tuple<jikespg_act::RuleDefinition> *save_original);

    static bool IsInternalAuxName(const char *name);

private:
    enum MetaKind
    {
        META_NONE = 0,
        META_OPT,      // ?
        META_STAR,     // *
        META_PLUS,     // +
        META_LPAREN,   // (
        META_RPAREN,   // )
        META_LBRACKET, // [
        META_RBRACKET, // ]
        META_LBRACE,   // {
        META_RBRACE,   // }
        META_BAR       // | (inside groups)
    };

    struct RhsAtom
    {
        int token; // SYMBOL / EMPTY_KEY / BLOCK index
        int macro; // MACRO_NAME after SYMBOL, or 0
    };

    Option *option;
    LexStream *lex_stream;
    VariableLookupTable *variable_table;
    MacroLookupTable *macro_table;
    jikespg_act &parser;
    int aux_counter;
    int current_rule_no;
    bool ok;

    MetaKind ClassifyMeta(int token_index) const;
    bool IsQuotedSymbol(int token_index) const;
    bool RuleNeedsExpansion(const jikespg_act::RuleDefinition &rule) const;
    bool IsGroupCloser(MetaKind meta) const;
    const char *UnterminatedGroupMsg(MetaKind open_kind) const;

    int MakeSymbolToken(const char *name, int name_length,
                        InputFileSymbol *file, unsigned location);
    int MakeMacroToken(const char *name, int name_length,
                       InputFileSymbol *file, unsigned location);
    int MakeKindToken(unsigned kind, InputFileSymbol *file, unsigned location);
    int CopyToken(int src, InputFileSymbol *file);
    int MakeEndMarker(InputFileSymbol *file, unsigned location);
    int MakeAuxName(const char *prefix, InputFileSymbol *file, unsigned location);

    void EmitError(int token_index, const char *msg);

    bool LowerSequence(int start, int end,
                       Tuple<RhsAtom> &atoms,
                       Tuple<jikespg_act::RuleDefinition> &out);

    bool LowerItem(int &pos, int end,
                   Tuple<RhsAtom> &atoms,
                   Tuple<jikespg_act::RuleDefinition> &out);

    // Collect alternatives until close_kind; then optionally ApplyQuantifier(trailing_q).
    bool LowerGroup(int &pos, int end,
                    MetaKind open_kind,
                    MetaKind close_kind,
                    MetaKind trailing_q,
                    int &result_token,
                    Tuple<jikespg_act::RuleDefinition> &out);

    void AppendAuxRule(int lhs_token, int classname_index, int array_element_type_index,
                       int separator_kind,
                       const Tuple<RhsAtom> &rhs,
                       Tuple<jikespg_act::RuleDefinition> &out,
                       InputFileSymbol *file, unsigned location);

    int ApplyQuantifier(MetaKind q, int operand_token,
                        Tuple<jikespg_act::RuleDefinition> &out,
                        InputFileSymbol *file, unsigned location);

    void RewriteRule(const jikespg_act::RuleDefinition &src,
                     const Tuple<RhsAtom> &atoms,
                     Tuple<jikespg_act::RuleDefinition> &out);
};

#endif
