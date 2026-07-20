#include "ebnf.h"
#include "jikespg_sym.h"

#include <cstdio>
#include <cstring>

EbnfExpander::EbnfExpander(Option *option_,
                           LexStream *lex_stream_,
                           VariableLookupTable *variable_table_,
                           MacroLookupTable *macro_table_,
                           jikespg_act &parser_)
    : option(option_),
      lex_stream(lex_stream_),
      variable_table(variable_table_),
      macro_table(macro_table_),
      parser(parser_),
      aux_counter(0),
      current_rule_no(0),
      ok(true)
{
}

bool EbnfExpander::IsInternalAuxName(const char *name)
{
    return name != NULL && std::strncmp(name, "__ebnf_", 7) == 0;
}

bool EbnfExpander::IsQuotedSymbol(int token_index) const
{
    if (lex_stream -> Kind(token_index) != TK_SYMBOL)
        return false;
    unsigned start = lex_stream -> StartLocation(token_index);
    if (start == 0)
        return false;
    char *buf = lex_stream -> InputBuffer(token_index);
    if (! buf)
        return false;
    char prev = buf[start - 1];
    return prev == '\'' || prev == '\"';
}

EbnfExpander::MetaKind EbnfExpander::ClassifyMeta(int token_index) const
{
    if (lex_stream -> Kind(token_index) != TK_SYMBOL)
        return META_NONE;
    if (IsQuotedSymbol(token_index))
        return META_NONE;

    unsigned start = lex_stream -> StartLocation(token_index),
             end = lex_stream -> EndLocation(token_index);
    if (end != start)
        return META_NONE;
    char *buf = lex_stream -> InputBuffer(token_index);
    if (! buf)
        return META_NONE;

    // Symbol-less single-char tokens are EBNF meta (from ClassifyEbnfMeta).
    if (lex_stream -> GetVariableSymbol(token_index) != NULL)
        return META_NONE;

    char ch = buf[start];
    if (ch == '?') return META_OPT;
    if (ch == '*') return META_STAR;
    if (ch == '+') return META_PLUS;
    if (ch == '(') return META_LPAREN;
    if (ch == ')') return META_RPAREN;
    if (ch == option -> or_marker) return META_BAR;
    return META_NONE;
}

bool EbnfExpander::RuleNeedsExpansion(const jikespg_act::RuleDefinition &rule) const
{
    for (int i = lex_stream -> Next(rule.separator_index);
         i < rule.end_rhs_index;
         i = lex_stream -> Next(i))
    {
        if (ClassifyMeta(i) != META_NONE)
            return true;
    }
    return false;
}

void EbnfExpander::EmitError(int token_index, const char *msg)
{
    option -> EmitError(token_index, msg);
    ok = false;
}

int EbnfExpander::MakeSymbolToken(const char *name, int name_length,
                                  InputFileSymbol *file, unsigned location)
{
    int index = lex_stream -> GetNextToken(file, location);
    Token *token = lex_stream -> GetTokenReference(index);
    token -> SetKind(TK_SYMBOL);
    token -> SetEndLocation(location);
    token -> SetSymbol(variable_table -> FindOrInsertName(name, name_length));
    return index;
}

int EbnfExpander::MakeMacroToken(const char *name, int name_length,
                                 InputFileSymbol *file, unsigned location)
{
    int index = lex_stream -> GetNextToken(file, location);
    Token *token = lex_stream -> GetTokenReference(index);
    token -> SetKind(TK_MACRO_NAME);
    token -> SetEndLocation(location);
    token -> SetSymbol(macro_table -> FindOrInsertName(name, name_length));
    return index;
}

int EbnfExpander::MakeKindToken(unsigned kind, InputFileSymbol *file, unsigned location)
{
    int index = lex_stream -> GetNextToken(file, location);
    Token *token = lex_stream -> GetTokenReference(index);
    token -> SetKind(kind);
    token -> SetEndLocation(location);
    return index;
}

int EbnfExpander::MakeAuxName(const char *prefix, InputFileSymbol *file, unsigned location)
{
    char name[80];
    for (;;)
    {
        aux_counter++;
        // Stable: rule index + occurrence within the expand pass.
        std::snprintf(name, sizeof(name), "%s_r%d_%d", prefix, current_rule_no, aux_counter);
        int length = (int) std::strlen(name);
        if (variable_table -> FindName(name, length) == NULL)
            return MakeSymbolToken(name, length, file, location);
    }
}

int EbnfExpander::CopyToken(int src, InputFileSymbol *file)
{
    Token *st = lex_stream -> GetTokenReference(src);
    int copy = MakeKindToken(st -> Kind(), file, st -> StartLocation());
    Token *ct = lex_stream -> GetTokenReference(copy);
    ct -> SetEndLocation(st -> EndLocation());
    if (st -> GetSymbol())
        ct -> SetSymbol(st -> GetSymbol());
    return copy;
}

int EbnfExpander::MakeEndMarker(InputFileSymbol *file, unsigned location)
{
    // ProcessRules walks i = Next(sep); i < end_rhs; i = Next(i).
    // LexStream::Next clamps at Length()-1, so end_rhs must be a real token
    // index strictly after the last RHS token.
    return lex_stream -> GetNextToken(file, location);
}

void EbnfExpander::AppendAuxRule(int lhs_token, int classname_index, int array_element_type_index,
                                 int separator_kind,
                                 const Tuple<RhsAtom> &rhs,
                                 Tuple<jikespg_act::RuleDefinition> &out,
                                 InputFileSymbol *file, unsigned location)
{
    int sep = MakeKindToken(separator_kind, file, location);
    for (int i = 0; i < rhs.Length(); i++)
    {
        CopyToken(rhs[i].token, file);
        if (rhs[i].macro != 0)
            CopyToken(rhs[i].macro, file);
    }
    int end_marker = MakeEndMarker(file, location);

    int index = out.NextIndex();
    out[index].lhs_index = lhs_token;
    out[index].classname_index = classname_index;
    out[index].array_element_type_index = array_element_type_index;
    out[index].separator_index = sep;
    out[index].end_rhs_index = end_marker;
}

void EbnfExpander::RewriteRule(const jikespg_act::RuleDefinition &src,
                               const Tuple<RhsAtom> &atoms,
                               Tuple<jikespg_act::RuleDefinition> &out)
{
    InputFileSymbol *file = lex_stream -> GetFileSymbol(src.separator_index);
    unsigned location = lex_stream -> StartLocation(src.separator_index);
    int sep_kind = lex_stream -> Kind(src.separator_index);

    int sep = MakeKindToken(sep_kind, file, location);
    for (int i = 0; i < atoms.Length(); i++)
    {
        CopyToken(atoms[i].token, file);
        if (atoms[i].macro != 0)
            CopyToken(atoms[i].macro, file);
    }
    int end_marker = MakeEndMarker(file, location);

    int index = out.NextIndex();
    out[index].lhs_index = src.lhs_index;
    out[index].classname_index = src.classname_index;
    out[index].array_element_type_index = src.array_element_type_index;
    out[index].separator_index = sep;
    out[index].end_rhs_index = end_marker;
}

int EbnfExpander::ApplyQuantifier(MetaKind q, int operand_token,
                                  Tuple<jikespg_act::RuleDefinition> &out,
                                  InputFileSymbol *file, unsigned location)
{
    const char *prefix = (q == META_OPT ? "__ebnf_opt"
                          : q == META_STAR ? "__ebnf_star"
                          : "__ebnf_plus");
    int lhs = MakeAuxName(prefix, file, location);

    // */+ use automatic_ast list shape equivalent to `List$$Elem ::= ...`:
    // classname "$" (empty) + array_element_type pointing at the element symbol.
    int classname_index = 0;
    int array_element_type_index = 0;
    if ((q == META_STAR || q == META_PLUS) &&
        lex_stream -> Kind(operand_token) == TK_SYMBOL &&
        lex_stream -> GetVariableSymbol(operand_token) != NULL)
    {
        char empty_macro[2] = { option -> macro_prefix, '\0' };
        classname_index = MakeMacroToken(empty_macro, 1, file, location);
        // ProcessRules uses GetVariableSymbol(array_element_type_index).
        array_element_type_index = CopyToken(operand_token, file);
    }

    Tuple<RhsAtom> empty_rhs;
    Tuple<RhsAtom> once_rhs;
    RhsAtom atom;
    atom.token = operand_token;
    atom.macro = 0;
    once_rhs.Next() = atom;

    if (q == META_OPT)
    {
        RhsAtom empty_atom;
        empty_atom.token = MakeKindToken(TK_EMPTY_KEY, file, location);
        empty_atom.macro = 0;
        empty_rhs.Next() = empty_atom;
        AppendAuxRule(lhs, 0, 0, TK_EQUIVALENCE, empty_rhs, out, file, location);
        AppendAuxRule(lhs, 0, 0, TK_OR_MARKER, once_rhs, out, file, location);
    }
    else if (q == META_STAR)
    {
        RhsAtom empty_atom;
        empty_atom.token = MakeKindToken(TK_EMPTY_KEY, file, location);
        empty_atom.macro = 0;
        empty_rhs.Next() = empty_atom;
        AppendAuxRule(lhs, classname_index, array_element_type_index,
                      TK_EQUIVALENCE, empty_rhs, out, file, location);

        Tuple<RhsAtom> rec;
        RhsAtom self;
        self.token = lhs;
        self.macro = 0;
        rec.Next() = self;
        rec.Next() = atom;
        AppendAuxRule(lhs, classname_index, array_element_type_index,
                      TK_OR_MARKER, rec, out, file, location);
    }
    else // META_PLUS
    {
        AppendAuxRule(lhs, classname_index, array_element_type_index,
                      TK_EQUIVALENCE, once_rhs, out, file, location);
        Tuple<RhsAtom> rec;
        RhsAtom self;
        self.token = lhs;
        self.macro = 0;
        rec.Next() = self;
        rec.Next() = atom;
        AppendAuxRule(lhs, classname_index, array_element_type_index,
                      TK_OR_MARKER, rec, out, file, location);
    }
    return lhs;
}

bool EbnfExpander::LowerGroup(int &pos, int end,
                              int &result_token,
                              Tuple<jikespg_act::RuleDefinition> &out)
{
    int lparen = pos;
    InputFileSymbol *file = lex_stream -> GetFileSymbol(lparen);
    unsigned location = lex_stream -> StartLocation(lparen);
    pos = lex_stream -> Next(pos);

    Tuple< Tuple<RhsAtom> > alternatives;
    alternatives.Next();

    while (pos < end)
    {
        MetaKind meta = ClassifyMeta(pos);
        if (meta == META_RPAREN)
        {
            pos = lex_stream -> Next(pos);
            if (alternatives.Length() == 1)
            {
                Tuple<RhsAtom> &alt = alternatives[0];
                if (alt.Length() == 0)
                {
                    result_token = MakeKindToken(TK_EMPTY_KEY, file, location);
                    return ok;
                }
                if (alt.Length() == 1 && alt[0].macro == 0)
                {
                    result_token = alt[0].token;
                    return ok;
                }
                int lhs = MakeAuxName("__ebnf_grp", file, location);
                AppendAuxRule(lhs, 0, 0, TK_EQUIVALENCE, alt, out, file, location);
                result_token = lhs;
                return ok;
            }

            int lhs = MakeAuxName("__ebnf_grp", file, location);
            for (int a = 0; a < alternatives.Length(); a++)
            {
                int sep = (a == 0 ? TK_EQUIVALENCE : TK_OR_MARKER);
                if (alternatives[a].Length() == 0)
                {
                    Tuple<RhsAtom> empty_rhs;
                    RhsAtom empty_atom;
                    empty_atom.token = MakeKindToken(TK_EMPTY_KEY, file, location);
                    empty_atom.macro = 0;
                    empty_rhs.Next() = empty_atom;
                    AppendAuxRule(lhs, 0, 0, sep, empty_rhs, out, file, location);
                }
                else
                    AppendAuxRule(lhs, 0, 0, sep, alternatives[a], out, file, location);
            }
            result_token = lhs;
            return ok;
        }
        if (meta == META_BAR)
        {
            alternatives.Next();
            pos = lex_stream -> Next(pos);
            continue;
        }

        int item_start = pos;
        if (lex_stream -> Kind(pos) == TK_BLOCK)
        {
            EmitError(pos, "Action blocks are not allowed inside EBNF groups");
            return false;
        }
        Tuple<RhsAtom> item_atoms;
        if (! LowerItem(pos, end, item_atoms, out))
            return false;
        if (item_atoms.Length() == 0)
        {
            EmitError(item_start, "Invalid EBNF item inside group");
            return false;
        }
        Tuple<RhsAtom> &cur = alternatives[alternatives.Length() - 1];
        for (int k = 0; k < item_atoms.Length(); k++)
            cur.Next() = item_atoms[k];
    }

    EmitError(lparen, "Unterminated EBNF group '('");
    return false;
}

bool EbnfExpander::LowerItem(int &pos, int end,
                             Tuple<RhsAtom> &atoms,
                             Tuple<jikespg_act::RuleDefinition> &out)
{
    if (pos >= end)
        return true;

    InputFileSymbol *file = lex_stream -> GetFileSymbol(pos);
    unsigned location = lex_stream -> StartLocation(pos);
    MetaKind meta = ClassifyMeta(pos);
    int operand = 0;
    int macro = 0;

    if (meta == META_LPAREN)
    {
        if (! LowerGroup(pos, end, operand, out))
            return false;
    }
    else if (meta == META_OPT || meta == META_STAR || meta == META_PLUS ||
             meta == META_RPAREN || meta == META_BAR)
    {
        EmitError(pos, "EBNF operator is not valid here");
        pos = lex_stream -> Next(pos);
        return false;
    }
    else if (lex_stream -> Kind(pos) == TK_BLOCK)
    {
        RhsAtom atom;
        atom.token = pos;
        atom.macro = 0;
        atoms.Next() = atom;
        pos = lex_stream -> Next(pos);
        return ok;
    }
    else if (lex_stream -> Kind(pos) == TK_EMPTY_KEY ||
             lex_stream -> Kind(pos) == TK_SYMBOL)
    {
        operand = pos;
        pos = lex_stream -> Next(pos);
        if (pos < end && lex_stream -> Kind(pos) == TK_MACRO_NAME)
        {
            macro = pos;
            pos = lex_stream -> Next(pos);
        }
    }
    else
    {
        EmitError(pos, "Unexpected token in EBNF right-hand side");
        pos = lex_stream -> Next(pos);
        return false;
    }

    if (operand != 0 && pos < end)
    {
        MetaKind q = ClassifyMeta(pos);
        if (q == META_OPT || q == META_STAR || q == META_PLUS)
        {
            if (macro != 0)
            {
                EmitError(macro, "RHS macro cannot be attached directly to an EBNF quantifier; "
                                 "write an explicit nonterminal instead");
                return false;
            }
            int qtok = pos;
            pos = lex_stream -> Next(pos);
            operand = ApplyQuantifier(q, operand, out, file,
                                      lex_stream -> StartLocation(qtok));
            macro = 0;
        }
    }

    if (operand != 0)
    {
        RhsAtom atom;
        atom.token = operand;
        atom.macro = macro;
        atoms.Next() = atom;
    }
    return ok;
}

bool EbnfExpander::LowerSequence(int start, int end,
                                 Tuple<RhsAtom> &atoms,
                                 Tuple<jikespg_act::RuleDefinition> &out)
{
    int pos = start;
    while (pos < end)
    {
        MetaKind meta = ClassifyMeta(pos);
        if (meta == META_BAR || meta == META_RPAREN)
        {
            EmitError(pos, "EBNF '|' and ')' are only valid inside a group '(...)'");
            return false;
        }
        if (! LowerItem(pos, end, atoms, out))
            return false;
    }
    return ok;
}

bool EbnfExpander::Expand(Tuple<jikespg_act::RuleDefinition> *save_original)
{
    if (! option -> ebnf)
        return true;

    bool any_ebnf = false;
    for (int r = 0; r < parser.rules.Length(); r++)
    {
        if (RuleNeedsExpansion(parser.rules[r]))
        {
            any_ebnf = true;
            break;
        }
    }
    if (! any_ebnf)
        return true;

    if (save_original != NULL)
    {
        save_original -> Reset();
        for (int r = 0; r < parser.rules.Length(); r++)
            save_original -> Next() = parser.rules[r];
    }

    Tuple<jikespg_act::RuleDefinition> expanded;
    Tuple<jikespg_act::RuleDefinition> aux_rules;

    for (int r = 0; r < parser.rules.Length(); r++)
    {
        current_rule_no = r;
        const jikespg_act::RuleDefinition &rule = parser.rules[r];
        if (! RuleNeedsExpansion(rule))
        {
            expanded.Next() = rule;
            continue;
        }

        int start = lex_stream -> Next(rule.separator_index);
        int end = rule.end_rhs_index;
        Tuple<RhsAtom> atoms;
        int aux_before = aux_rules.Length();
        if (! LowerSequence(start, end, atoms, aux_rules))
            return false;

        RewriteRule(rule, atoms, expanded);

        for (int a = aux_before; a < aux_rules.Length(); a++)
            expanded.Next() = aux_rules[a];
        if (aux_before == 0)
            aux_rules.Reset();
        else
        {
            Tuple<jikespg_act::RuleDefinition> keep;
            for (int a = 0; a < aux_before; a++)
                keep.Next() = aux_rules[a];
            aux_rules.Reset();
            for (int a = 0; a < keep.Length(); a++)
                aux_rules.Next() = keep[a];
        }
    }

    for (int a = 0; a < aux_rules.Length(); a++)
        expanded.Next() = aux_rules[a];

    if (! ok)
        return false;

    parser.rules.Reset();
    for (int i = 0; i < expanded.Length(); i++)
        parser.rules.Next() = expanded[i];

    return true;
}
