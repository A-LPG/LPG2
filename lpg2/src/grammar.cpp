#include <sys/stat.h>
#include "control.h"
#include "grammar.h"
#include "Action.h"
#include "CAction.h"
#include "CppAction.h"
#include "PlxAction.h"
#include "PlxasmAction.h"
#include "JavaAction.h"
#include "MlAction.h"
#include "XmlAction.h"

#include <iostream>

#include "CppAction2.h"
#include "CSharpAction.h"
#include "TypeScriptAction.h"
using namespace std;

Grammar::Grammar(Control *control_,
                 Blocks *action_blocks_,
                 VariableLookupTable *variable_table_,
                 MacroLookupTable *macro_table_)
                                  : control(control_),
                                    option(control_ -> option),
                                    action_blocks(action_blocks_),
                                    lex_stream(control_ -> lex_stream),
                                    variable_table(variable_table_),
                                    macro_table(macro_table_),
                                    parser(control_, control_ -> lex_stream, variable_table_, macro_table_),

                                    symbol_index(4, variable_table_ -> Size()),
                                    rules(2, 2048),
                                    rhs_sym(4, 8192),
                                    name(2, variable_table_ -> Size())
{
    action = NULL;

    identifier_symbol = NULL;
    eol_symbol = NULL;
    eof_symbol = NULL;
    error_symbol = NULL;

    num_items = 0;
    num_symbols = 0;
    num_single_productions = 0;
    num_rules = 0;

    empty = 0;
    identifier_image = 0;
    eol_image = 0;
    eof_image = 0;
    error_image = 0;
    accept_image = 0;

    variable_symbol_pool.Next() = null_symbol = new VariableSymbol("", 0, 0, 0);

    empty_symbol = allocate_variable_symbol("empty");
    accept_symbol = allocate_variable_symbol("accept");

    return;
}

void Grammar::Process()
{
    parser.Parse();
    if (parser.rules.Length() == 0)
    {
        Tuple<const char *> msg;
        msg.Next() = option -> grm_file;
        msg.Next() = " is an empty grammar... Processing stopped";
        option -> EmitError(lex_stream -> NumTokens() - 2, msg); // point to the last token

        control -> Exit(4);
    }
    switch (option->programming_language)
    {
    case Option::C:
        this->action = new CAction(control, action_blocks, this, macro_table);
        break;
    case Option::CPP:
        this->action = new CppAction(control, action_blocks, this, macro_table);
        break;
    case Option::CPP2:
        this->action = new CppAction2(control, action_blocks, this, macro_table);
        break;
    case Option::CSHARP:
        this->action = new CSharpAction(control, action_blocks, this, macro_table);
        break;
    case Option::TSC:
        this->action = new TypeScriptAction(control, action_blocks, this, macro_table);
        break;
    case Option::JAVA:
        this->action = new JavaAction(control, action_blocks, this, macro_table);
        break;
    case Option::PLX:
        this->action = new PlxAction(control, action_blocks, this, macro_table);
        break;
    case Option::PLXASM:
        this->action = new PlxasmAction(control, action_blocks, this, macro_table);
        break;
    case Option::ML:
        this->action = new MlAction(control, action_blocks, this, macro_table);
        break;
    case Option::XML:
        this->action = new XmlAction(control, action_blocks, this, macro_table);
        break;
    default:
        assert(false);
        break;
    }
   
    //
    // Add all the exported symbols to the export_macro_table.
    //
    action -> InsertExportMacros(); // make sure that this is processed first
    action -> InsertImportedFilterMacros();
    action -> CheckMacrosForConsistency();

    //
    // If bad errors were detected, quit!
    //
    if (action -> return_code > 0)
        control -> Exit(action -> return_code);

    symbol_index.Next(); // skip the 0th element

    ProcessTerminals(declared_terminals_in_g);
    ProcessRules(declared_terminals_in_g);
    ProcessExportedTerminals();
    ProcessNames();
    if (option -> list)
        DisplayInput();

    //
    //
    //
    for (int i = 0; i < parser.predecessor_candidates.Length(); i++)
    {
        int lhs_index = parser.predecessor_candidates[i].lhs_index,
            rhs_index = parser.predecessor_candidates[i].rhs_index;
        this -> check_predecessor_sets_for.NextIndex(); // allocate next element.
        this -> check_predecessor_sets_for[i].left_symbol = lex_stream -> GetVariableSymbol(lhs_index) -> SymbolIndex();
        this -> check_predecessor_sets_for[i].right_symbol = lex_stream -> GetVariableSymbol(rhs_index) -> SymbolIndex();

        if (this -> check_predecessor_sets_for[i].left_symbol == 0)
            option -> EmitError(lhs_index, "This symbol was not defined in the grammar");
        if (this -> check_predecessor_sets_for[i].right_symbol == 0)
            option -> EmitError(rhs_index, "This symbol was not defined in the grammar");
    }

    option -> FlushReport();

    if (action -> return_code > 0)
        control -> Exit(action -> return_code);

    delete this -> action;
    this -> action = NULL;

    return;
}


VariableSymbol *Grammar::GetSymbol(int sym_index)
{
    VariableSymbol *symbol;

    switch(lex_stream -> Kind(sym_index))
    {
        case TK_EMPTY_KEY:
             symbol = empty_symbol;
             break;
        case TK_IDENTIFIER_KEY:
             symbol = identifier_symbol;
             break;
        case TK_ERROR_KEY:
             symbol = error_symbol;
             break;
        case TK_EOF_KEY:
             symbol = eof_symbol;
             break;
        case TK_EOL_KEY:
             symbol = eol_symbol;
             break;
        default:
            assert(lex_stream -> Kind(sym_index) == TK_SYMBOL ||
                   lex_stream -> Kind(sym_index) == TK_MACRO_NAME);
            symbol = lex_stream -> GetVariableSymbol(sym_index);
            break;
    }

    return symbol;
}


int Grammar::GetSymbolIndex(int index)
{
    VariableSymbol *symbol = GetSymbol(index);
    return (symbol ? symbol -> SymbolIndex() : 0);
}


int Grammar::AssignSymbolIndex(VariableSymbol *symbol)
{
    int index = symbol -> SymbolIndex();
    if (index == 0)
    {
        index = ++num_symbols;
        symbol -> SetSymbolIndex(index);

        int symno_index = symbol_index.NextIndex();
        assert(symno_index == index);
        symbol_index[symno_index].symbol = symbol;
        symbol_index[symno_index].external_name_index = symbol -> NameIndex();
    }

    return index;
}


//
// Construct set of exported symbols. Make sure that there are no duplicates.
//
void Grammar::ProcessExportedTerminals(void)
{
    Array<int> import_index(variable_table -> Size(), -1);

    //
    // Merge the exported symbols inherited from filter files
    // with the symbols that are to be locally exported.
    //
    Tuple<int> imports;
    {
        for (int i = 0; i < lex_stream -> NumImportedFilters(); i++)
            imports.Next() = lex_stream -> ImportedFilter(i);
    }
    {
        for (int i = 0; i < parser.exports.Length(); i++)
            imports.Next() = parser.exports[i];
    }

    //
    // Add all symbols from the imports list to the exported_symbols list.
    //
    for (int i = 0; i < imports.Length(); i++)
    {
        int import = imports[i];
        VariableSymbol *symbol = lex_stream -> GetVariableSymbol(import);
        int decl = import_index[symbol -> Index()];
        if (decl == -1)
        {
            import_index[symbol -> Index()] = import;
            exported_symbols.Next() = symbol;
        }
        else
        {
            Tuple<const char *> msg;
            msg.Next() = "Symbol \"";
            msg.Next() = symbol -> Name();
            msg.Next() = "\" was specified more than once in an Export section. Previous specification was located at ";
            msg.Next() = lex_stream -> FileName(decl);
            msg.Next() = ":";
            IntToString start_line(lex_stream -> Line(decl));
            msg.Next() = start_line.String();
            msg.Next() = ":";
            IntToString start_column(lex_stream -> Column(decl));
            msg.Next() = start_column.String();
            msg.Next() = ":";
            IntToString end_line(lex_stream -> EndLine(decl));
            msg.Next() = end_line.String();
            msg.Next() = ":";
            IntToString end_column(lex_stream -> EndColumn(decl));
            msg.Next() =  end_column.String();
            msg.Next() = ":";
            IntToString start_location(lex_stream -> StartLocation(decl));
            msg.Next() = start_location.String();
            msg.Next() = ":";
            IntToString end_location(lex_stream -> EndLocation(decl));
            msg.Next() = end_location.String();
            option -> EmitWarning(import, msg);
        }
    }

    return;
}


void Grammar::ProcessTerminals(Tuple<int> &declared_terminals)
{
    //
    // We MUST first declare the terminals that are markers for the start
    // symbols. By declaring them first, the indexes of these markers in
    // the declared_terminals will be the same as the corresponding indexed
    // of the start symbols in parser.start_indexes. Note that the main start
    // symbol has the "empty" symbol as its marker. I.e., no marker is required
    // for it when parsing.
    //
    empty = AssignSymbolIndex(empty_symbol);
    assert(declared_terminals.Length() == 0);
    declared_terminals.Next() = empty;
    for (int i = 1; i < parser.start_indexes.Length(); i++)     // Create the terminal markers for the extra entry points, if any.
    {
        const char *entry_name = lex_stream -> NameString(parser.start_indexes[i]),
                   *Marker = "Marker";
        int length = strlen(entry_name) + strlen(Marker) + 3;
        char *marker_name = new char[length];
        strcpy(marker_name, entry_name);
        strcat(marker_name, Marker);
        VariableSymbol *marker_symbol = variable_table -> FindName(marker_name, strlen(marker_name));
        for (int k = 0; marker_symbol != NULL && k < 100; k++) // keep searching until we find an undeclared symbol name
        {
            IntToString number(k);
            strcpy(marker_name, entry_name);
            strcat(marker_name, Marker);
            strcat(marker_name, number.String());
            marker_symbol = variable_table -> FindName(marker_name, strlen(marker_name));
        }

        if (marker_symbol == NULL)
             marker_symbol = variable_table -> InsertName(marker_name, strlen(marker_name));
        else
        {
            Tuple<const char *> msg;
            msg.Next() = "Unable to declare a marker for symbol \"";
            msg.Next() = entry_name;
            msg.Next() = "\". After trying the suffixes \"Marker\" and \"Marker\" + [\"00\"..\"99\"]";
            option -> EmitError(lex_stream -> GetVariableSymbol(parser.start_indexes[i]) -> Location(), msg);
            control -> Exit(12);
        }

        assert(declared_terminals.Length() == i);
        declared_terminals.Next() = AssignSymbolIndex(marker_symbol);

        delete [] marker_name;
    }

    //
    // Process the declared set of terminal symbols.
    // This is useful for upward compatibility with old versions where
    // the declaration of terminals was required. It will also pick up
    // terminal symbols that are declared but never used.
    //
    {
        for (int i = 0; i < parser.terminals.Length(); i++)
        {
            VariableSymbol *terminal = lex_stream -> GetVariableSymbol(parser.terminals[i]);
            assert(terminal);
            declared_terminals.Next() = AssignSymbolIndex(terminal);
        }
    }

    BitSet imported_term_set(variable_table -> Size(), BitSet::EMPTY);
    {
        for (int i = 0; i < lex_stream -> NumImportedTerminals(); i++) // imported from another grammar
        {
            int import = lex_stream -> ImportedTerminal(i);
            VariableSymbol *terminal = lex_stream -> GetVariableSymbol(import);
            declared_terminals.Next() = AssignSymbolIndex(terminal);

            imported_term_set.AddElement(terminal -> Index());
        }
    }

    {
        for (int i = 0; i < parser.keywords.Length(); i++)
        {
            VariableSymbol *keyword = lex_stream -> GetVariableSymbol(parser.keywords[i]);
            assert(keyword);
            declared_terminals.Next() = AssignSymbolIndex(keyword);
        }
    }

    //
    // Compute the set of terminals directly from the grammar, even
    // though the user may have declared them using a terminals and keywords section;
    // Traverse the right-hand side and initially, add all symbols found to the
    // term_set.
    //
    BitSet term_set(variable_table -> Size(), BitSet::EMPTY);
    for (int k = 0; k < parser.rules.Length(); k++)
    {
        for (int i = lex_stream -> Next(parser.rules[k].separator_index);
                 i < parser.rules[k].end_rhs_index;
                 i = lex_stream -> Next(i))
        {
            VariableSymbol *symbol = lex_stream -> GetVariableSymbol(i);
            if (symbol)
                term_set.AddElement(symbol -> Index());
        }
    }

    //
    // Add all symbols appearing on the right-hand side of an alias rule to the term_set.
    //
    for (int l = 0; l < parser.aliases.Length(); l++)
    {
        VariableSymbol *symbol = lex_stream -> GetVariableSymbol(parser.aliases[l].rhs_index); // the right-hand-side symbol
        if (symbol) // no symbol is defined for EMPTY
            term_set.AddElement(symbol -> Index());
    }

    //
    // Remove all symbols appearing on the left-hand side of an alias rule from the term_set.
    //
    for (int m = 0; m < parser.aliases.Length(); m++)
    {
        VariableSymbol *symbol = lex_stream -> GetVariableSymbol(parser.aliases[m].lhs_index); // the left-hand-side symbol
        assert(symbol);
        term_set.RemoveElement(symbol -> Index());
    }

    //
    // Remove all symbols appearing on the left-hand side of a grammar rule from the term_set.
    //
    BitSet nterm_set(variable_table -> Size(), BitSet::EMPTY);
    for (int n = 0; n < parser.rules.Length(); n++)
    {
        VariableSymbol *symbol = lex_stream -> GetVariableSymbol(parser.rules[n].lhs_index);
        assert(symbol);
        term_set.RemoveElement(symbol -> Index());
        nterm_set.AddElement(symbol -> Index());
    }

    //
    // Remove all symbols to whom a known non-terminal is aliased.
    // This iterative algorithm is in general INEFFICIENT. However, since
    // the Alias section is likely to be small, it does not matter.
    //
    // TODO: compute this more efficiently? ... first compute partial order
    //       of nonterminals based on Alias relation, etc...
    //
    bool changed = true;
    while (changed)
    {
        changed = false;

        //
        // Remove all symbols to whom a known non-terminal is aliased.
        // I.e., any symbol appearing on the right-hand side of an alias rule
        // where the left-hand side symbol was also used as the left-hand
        // side of a grammar rule.
        //
        for (int i = 0; i < parser.aliases.Length(); i++)
        {
            VariableSymbol *symbol = lex_stream -> GetVariableSymbol(parser.aliases[i].lhs_index);
            assert(symbol);
            if (nterm_set[symbol -> Index()]) // The lhs is a nonterminal?
            {
                symbol = lex_stream -> GetVariableSymbol(parser.aliases[i].rhs_index); // the right-hand-side symbol
                if (! nterm_set[symbol -> Index()]) // The rhs is NOT a nonterminal?
                {
                    assert(symbol);
                    changed = true;
                    term_set.RemoveElement(symbol -> Index());
                    nterm_set.AddElement(symbol -> Index());
               }
            }
        }
    }

    //
    // Add all recovery symbols that are not known to be nonterminals
    // to the term_set. Note that the recovery terminals are considered
    // to be implicitly imported.
    //
    for (int j = 0; j < parser.recovers.Length(); j++)
    {
        VariableSymbol *symbol = lex_stream -> GetVariableSymbol(parser.recovers[j]);
        if (! nterm_set[symbol -> Index()])
        {
            term_set.AddElement(symbol -> Index());
            declared_terminals.Next() = AssignSymbolIndex(symbol);

            imported_term_set.AddElement(symbol -> Index()); // considered implicitly imported.
        }
    }

    //
    // Now process the declared set of terminal symbols.
    //
    for (int o = 0; o < variable_table -> Size(); o++)
    {
        VariableSymbol *symbol = variable_table -> Element(o);
        if (term_set[symbol -> Index()])
            (void) AssignSymbolIndex(symbol);
    }

    //
    // Now take care of the terminal aliasses that are not aliassed to special symbols.
    //
    Tuple<int> remaining_aliases;
    ProcessInitialAliases(remaining_aliases);

    //
    // Process the special symbols.
    //
    if (parser.identifier_index != 0)
    {
         identifier_symbol = lex_stream -> GetVariableSymbol(parser.identifier_index);
         assert(identifier_symbol);
         identifier_image = AssignSymbolIndex(identifier_symbol);
         declared_terminals.Next() = identifier_image;
    }

    if (parser.eol_index != 0)
    {
         eol_symbol = lex_stream -> GetVariableSymbol(parser.eol_index);
         assert(eol_symbol);
         eol_image = AssignSymbolIndex(eol_symbol);
         declared_terminals.Next() = eol_image;
    }

    if (parser.eof_index != 0)
    {
         eof_symbol = lex_stream -> GetVariableSymbol(parser.eof_index);
         assert(eof_symbol);
         eof_image = AssignSymbolIndex(eof_symbol);
    }
    else
    {
        eof_symbol = allocate_variable_symbol("eof");
        eof_image = AssignSymbolIndex(eof_symbol);
    }
    declared_terminals.Next() = eof_image;

    if (parser.error_index != 0)
    {
         error_symbol = lex_stream -> GetVariableSymbol(parser.error_index);
         assert(error_symbol);
         error_image = AssignSymbolIndex(error_symbol);
         declared_terminals.Next() = error_image;
    }
    else if (option -> error_maps)
    {
         error_symbol = allocate_variable_symbol("error");
         error_image = AssignSymbolIndex(error_symbol);
         declared_terminals.Next() = error_image;
    }
    else error_image = DEFAULT_SYMBOL;

    //
    // Compute the set of keywords identified by the user.
    // Two representations are used: a bit set for ease of
    // queries and a tuple for ease of iteration.
    //
    keyword_set.Initialize(num_symbols + 1);
    for (int p = 0; p < parser.keywords.Length(); p++)
    {
        VariableSymbol *keyword = lex_stream -> GetVariableSymbol(parser.keywords[p]);
        if (! keyword_set[keyword -> SymbolIndex()])
        {
            keywords.Next() = keyword -> SymbolIndex();
            keyword_set.AddElement(keyword -> SymbolIndex());
        }
    }

    //
    // Compute the number of terminals in the language.
    // It is important that this operation be performed here!
    // Note that the next symbol following last terminal is the
    // Accept nonterminal.
    //
    num_terminals = num_symbols;
    accept_image = AssignSymbolIndex(accept_symbol);

    ProcessRemainingAliases(remaining_aliases);

    //
    // Check to see if the grammar contained undeclared terminals and if so, print them.
    //
    {
        BitSet declared_set(num_terminals + 1, BitSet::EMPTY);
        int num_undeclared_terminals = num_terminals; // Assume that all terminals are not declared
        for (int i = 0; i < declared_terminals.Length(); i++)
        {
            int symbol = declared_terminals[i];
            if (! declared_set[symbol])
            {
                declared_set.AddElement(symbol);
                num_undeclared_terminals--; // Each time we find a terminal declaration, decrease undeclared count
            }
        }
        if (num_undeclared_terminals > 0) // Some terminals were not declared?
        {
            char tok[Control::SYMBOL_SIZE + 1];
            for (int symbol = 1; symbol <= num_terminals; symbol++)
            {
                if (! declared_set[symbol])
                {
                    RestoreSymbol(tok, RetrieveString(symbol));

                    Tuple<const char *> msg;
                    msg.Next() = "The undeclared symbol \"";
                    msg.Next() = tok;
                    msg.Next() = "\" is assumed to be a terminal";
                    option ->EmitWarning(RetrieveTokenLocation(symbol), msg);
                }
            }
        }
    }

    //
    // Check to see if a terminal symbol that was used in this file
    // was not exported from one of the files imported by this grammar.
    //
    if (option -> warnings && option -> import_file.Length() > 0)
    {
        char tok[Control::SYMBOL_SIZE + 1];
        for (int o = 0; o < variable_table -> Size(); o++)
        {
            VariableSymbol *symbol = variable_table -> Element(o);
            if (term_set[symbol -> Index()] && (! imported_term_set[symbol -> Index()]))
            {
                RestoreSymbol(tok, RetrieveString(symbol -> SymbolIndex()));

                Tuple<const char *> msg;
                msg.Next() = "The terminal symbol \"";
                msg.Next() = tok;
                msg.Next() = "\" was not imported from";
                if (option -> import_file.Length() == 1)
                {
                    msg.Next() = " ";
                    msg.Next() = option -> import_file[0];
                }
                else
                {
                    msg.Next() = ": ";
                    for (int p = 0; p < option -> import_file.Length(); p++)
                    {
                        msg.Next() = option -> import_file[p];
                        if (p < option -> import_file.Length() - 1)
                            msg.Next() = ", ";
                    }
                }
                option -> EmitWarning(RetrieveTokenLocation(symbol -> SymbolIndex()), msg);
            }
        }
    }

    return;
}


void Grammar::ProcessInitialAliases(Tuple<int> &remaining_aliases)
{
    int return_code = 0;

    for (int i = 0; i < parser.aliases.Length(); i++)
    {
        int rhs_kind = lex_stream -> Kind(parser.aliases[i].rhs_index);

        if (rhs_kind == TK_EMPTY_KEY ||
            rhs_kind == TK_IDENTIFIER_KEY ||
            rhs_kind == TK_ERROR_KEY ||
            rhs_kind == TK_EOF_KEY ||
            rhs_kind == TK_EOL_KEY)
            remaining_aliases.Next() = i;
        else
        {
            int image = GetSymbolIndex(parser.aliases[i].rhs_index); // the right-hand-side symbol
            if (image == 0)
                remaining_aliases.Next() = i;
            else // If the right-hand side symbol is a declared symbol
            {
                //
                // Note that when one of the special symbols such as %eof,
                // or %eol etc.. are used on the left side of an alias, they
                // are preprocessed in the grammar as if they were specified
                // in a separate section headed by the keyword in question.
                // The use of these keywords on the left-hand side in the
                // Alias section has been deprecated.
                //
                assert(lex_stream -> Kind(parser.aliases[i].lhs_index) == TK_SYMBOL ||
                       lex_stream -> Kind(parser.aliases[i].lhs_index) == TK_MACRO_NAME);

                VariableSymbol *symbol = lex_stream -> GetVariableSymbol(parser.aliases[i].lhs_index);
                assert(symbol);
                if (symbol -> SymbolIndex() != 0)
                {
                    option -> EmitError(parser.aliases[i].lhs_index, "This symbol was previously defined");
                    return_code = 12;
                }
                symbol -> SetSymbolIndex(image);
            }
        }
    }

    //
    // If bad errors were detected, quit!
    //
    if (return_code > 0)
        control -> Exit(return_code);

    return;
}


void Grammar::ProcessRemainingAliases(Tuple<int> &remaining_aliases)
{
    int return_code = 0;

    for (int k = 0; k < remaining_aliases.Length(); k++)
    {
        int i = remaining_aliases[k];

        if ((lex_stream -> Kind(parser.aliases[i].rhs_index) == TK_IDENTIFIER_KEY && identifier_symbol == NULL) ||
            (lex_stream -> Kind(parser.aliases[i].rhs_index) == TK_EOL_KEY && eol_symbol == NULL) ||
            (lex_stream -> Kind(parser.aliases[i].rhs_index) == TK_ERROR_KEY && error_symbol == NULL))
        {
            Tuple<const char *> msg;
            msg.Next() = lex_stream -> NameString(parser.aliases[i].rhs_index);
            msg.Next() = " can\'t be used as an alias for \"";
            msg.Next() = lex_stream -> NameString(parser.aliases[i].lhs_index);
            msg.Next() = "\" because it was not defined.\n";
            option -> EmitError(parser.aliases[i].rhs_index, msg);

            return_code = 12;
        }
        else
        {
            int image = GetSymbolIndex(parser.aliases[i].rhs_index); // the right-hand-side symbol
            if (image == 0) // an as yet undefined symbol
            {
                VariableSymbol *symbol = lex_stream -> GetVariableSymbol(parser.aliases[i].rhs_index);
                assert(symbol);
                image = AssignSymbolIndex(symbol);
            }

            //
            // Note that when one of the special symbols such as %eof,
            // or %eol etc.. are used on the left side of an alias, they
            // are preprocessed in the grammar as if they were specified
            // in a separate section headed by the keyword in question.
            // The use of these keywords on the left-hand side in the
            // Alias section has been deprecated.
            //
            assert(lex_stream -> Kind(parser.aliases[i].lhs_index) == TK_SYMBOL ||
                   lex_stream -> Kind(parser.aliases[i].lhs_index) == TK_MACRO_NAME);

            VariableSymbol *symbol = lex_stream -> GetVariableSymbol(parser.aliases[i].lhs_index);
            assert(symbol);
            if (symbol -> SymbolIndex() != 0)
            {
                option -> EmitError(parser.aliases[i].lhs_index, "This symbol was previously defined");
                return_code = 12;
            }
            symbol -> SetSymbolIndex(image);
        }
    }

    if (eol_image == 0)
        eol_image = eof_image;

    //
    // If bad errors were detected, quit!
    //
    if (return_code > 0)
        control -> Exit(return_code);

    return;
}


// void Grammar::ProcessTitleOrGlobalBlock(int block_token, ActionBlockElement &action)
// {
//     BlockSymbol *block = lex_stream -> GetBlockSymbol(block_token);
//     if (! option -> ActionBlocks.IsIgnoredBlock(block -> BlockBegin(), block -> BlockBeginLength()))
//     {
//         TextBuffer *buffer = block -> Buffer();
//
//         if (block != option -> DefaultBlock())
//         {
//             option -> EmitError(block_token, "Only default blocks may appear in a Title or Global segment");
//             control -> Exit(12);
//         }
//
//         action.rule_number = 0;
//         action.location = ActionBlockElement::INITIALIZE; // does not matter - block must be default block...
//         action.block_token = block_token;
//         action.buffer = (buffer ? buffer : block -> ActionfileSymbol() -> InitialHeadersBuffer());
//     }
//
//     return;
// }


char *Grammar::InsertInterface(SymbolLookupTable &symbol_set, char *name)
{
    int length = strlen(name) + 1;
    char *type_name = new char[length + 1];
    strcpy(type_name, "I");
    strcat(type_name, name);
    Symbol *symbol = symbol_set.FindOrInsertName(type_name, length);
    delete [] type_name;

    return symbol -> Name();
}


void Grammar::ProcessRules(Tuple<int> &declared_terminals)
{
    Tuple<ActionBlockElement> notice_actions,
                              header_actions,
                              initial_actions,
                              code_actions,
                              ast_actions,
                              trailer_actions;
    Tuple<AttributeElement> attribute_actions;

    //
    // Add starting rule.
    //
    int start_index = (parser.start_indexes.Length() > 0 ? parser.start_indexes[0] : parser.rules[0].lhs_index);
    VariableSymbol *&start = start_symbol.Next();
    start = lex_stream -> GetVariableSymbol(start_index);
    AssignSymbolIndex(start);
    start_image = start -> SymbolIndex(); // save the image of the root nonterminal
    int rule_index = rules.NextIndex();
    rules[rule_index].first_token_index = start_index;
    rules[rule_index].last_token_index = start_index;
    rules[rule_index].lhs = accept_image;
    rules[rule_index].separator_token_kind = TK_EQUIVALENCE;
    rules[rule_index].produces_token_kind = TK_EQUIVALENCE;
    rules[rule_index].rhs_index = rhs_sym.Length();
    rules[rule_index].source_index = 0;

    if (start -> SymbolIndex() != empty)
        rhs_sym.Next() = start -> SymbolIndex();
    else
    {
        Tuple <const char *> msg;
        msg.Next() = "The start symbol, \"";
        msg.Next() = start -> Name();
        msg.Next() = "\", is aliased to the Empty symbol";

        option -> EmitError(start_index, msg);
        control -> Exit(12);
    }

    //
    // Now process the other entry points (extra start symbols), if any.
    //
    for (int i = 1; i < parser.start_indexes.Length(); i++)
    {
        int entry_index = parser.start_indexes[i];
        VariableSymbol *&entry = start_symbol.Next();
        entry = lex_stream -> GetVariableSymbol(entry_index);
        AssignSymbolIndex(entry);

        rule_index = rules.NextIndex();
        rules[rule_index].first_token_index = entry_index;
        rules[rule_index].last_token_index = entry_index;
        rules[rule_index].lhs = start -> SymbolIndex();
        rules[rule_index].separator_token_kind = TK_EQUIVALENCE;
        rules[rule_index].produces_token_kind = TK_EQUIVALENCE;
        rules[rule_index].rhs_index = rhs_sym.Length();
        rules[rule_index].source_index = 0;

        if (entry -> SymbolIndex() != empty)
        {
            rhs_sym.Next() = declared_terminals[i];
            rhs_sym.Next() = entry -> SymbolIndex();
        }
        else
        {
            Tuple <const char *> msg;
            msg.Next() = "The start symbol, \"";
            msg.Next() = entry -> Name();
            msg.Next() = "\", is aliased to the Empty symbol";

            option -> EmitError(entry_index, msg);
            control -> Exit(12);
        }
    }

    //
    // Are there any notice action blocks?
    //
    int return_code = 0;
    for (int j = 0; j < parser.notice_blocks.Length(); j++)
    {
        LexStream::TokenIndex block_token = parser.notice_blocks[j];
        BlockSymbol *block = lex_stream -> GetBlockSymbol(block_token);
        if (! option -> ActionBlocks().IsIgnoredBlock(block -> BlockBegin(), block -> BlockBeginLength()))
        {
           /* if (block != option -> DefaultBlock())
            {
                option -> EmitError(block_token, "Only default blocks may appear in a notice segment");
                return_code = 12;
            }*/

            ActionBlockElement &action = notice_actions.Next();
            action.rule_number = 0;
            action.location = ActionBlockElement::INITIALIZE; // does not matter - block must be default block...
            action.block_token = block_token;
            action.buffer = &notice_buffer;
        }
    }

    //
    // Now, process the title block, if present.
    // Then, process all global blocks specified.
    //
    {
        for (int j = 0; j < parser.global_blocks.Length(); j++)
        {
            LexStream::TokenIndex block_token = parser.global_blocks[j];
            BlockSymbol *block = lex_stream -> GetBlockSymbol(block_token);
            if (! option -> ActionBlocks().IsIgnoredBlock(block -> BlockBegin(), block -> BlockBeginLength()))
            {
                TextBuffer *buffer = block -> Buffer();

               /* if (block != option -> DefaultBlock())
                {
                    option -> EmitError(block_token, "Only default blocks may appear in a Title or Global segment");
                    control -> Exit(12);
                }*/

                ActionBlockElement &action = header_actions.Next();
                action.rule_number = 0;
                action.location = ActionBlockElement::INITIALIZE; // does not matter - block must be default block...
                action.block_token = block_token;
                action.buffer = (buffer ? buffer : block -> ActionfileSymbol() -> InitialHeadersBuffer());
            }

	    //            ProcessTitleOrGlobalBlock(parser.global_blocks[j], header_actions.Next());
        }
    }

    //
    // Are there any action blocks preceding the first rule?
    // These can be either specified in a Header segment or
    // in a Rule segment (legacy). The Header blocks come first.
    //
    {
        for (int j = 0; j < parser.header_blocks.Length(); j++)
        {
            LexStream::TokenIndex block_token = parser.header_blocks[j];
            BlockSymbol *block = lex_stream -> GetBlockSymbol(block_token);
            if (! option -> ActionBlocks().IsIgnoredBlock(block -> BlockBegin(), block -> BlockBeginLength()))
            {
                TextBuffer *buffer = block -> Buffer();

                ActionBlockElement &action = header_actions.Next();
                action.rule_number = 0;
                action.location = ActionBlockElement::INITIALIZE;
                action.block_token = block_token;
                action.buffer = (buffer ? buffer : block -> ActionfileSymbol() -> InitialHeadersBuffer());
            }
        }
    }

    for (int n = 0; n < parser.initial_blocks.Length(); n++)
    {
        LexStream::TokenIndex block_token = parser.initial_blocks[n];
        BlockSymbol *block = lex_stream -> GetBlockSymbol(block_token);
        if (! option -> ActionBlocks().IsIgnoredBlock(block -> BlockBegin(), block -> BlockBeginLength()))
        {
            TextBuffer *buffer = block -> Buffer();

            ActionBlockElement &action = initial_actions.Next();
            action.rule_number = 0;
            action.location = ActionBlockElement::BODY;
            action.block_token = block_token;
            action.buffer = (buffer ? buffer : block -> ActionfileSymbol() -> BodyBuffer());
        }
    }

    //
    // In this loop, the grammar is placed in the rule table structure and the
    // right-hand sides are placed in the RHS table.  A check is made to prevent
    // terminals from being used as left hand sides.
    // Also, if macro symbols are associated with symbols in the right-hand
    // side of certain rules, keep track of such macro names and their position
    // in processed_rule_map.
    //
    Array< Tuple<int> > special_nonterminal_array(variable_table -> Size());
    Tuple< Tuple<ProcessedRuleElement> > processed_rule_map;
    {
        //
        // We now skip the elements of procesed_rule_map that correspond to the
        // "Start" rules as no macros can be associated with them and no AST is
        // allocated for them.
        //
        for (int i = 0; i < start_symbol.Length(); i++)
            (void) processed_rule_map.Next();
    }
    SymbolLookupTable classname_set,
                      special_array_classname_set;
    Tuple<ClassnameElement> classname;

    //
    // Process the Token classname.
    //
    {
        int length = strlen(option -> ast_type) + strlen("Token");
        char *name = new char[length + 1];
        strcpy(name, option -> ast_type);
        strcat(name, "Token");
        this -> ast_token_classname = classname_set.FindOrInsertName(name, length) -> Name();
        delete [] name;

        ClassnameElement &element = classname.Next();
        element.specified_name = ast_token_classname;
        element.real_name = ast_token_classname;
        element.is_terminal_class = true;
        element.array_element_type_symbol = NULL;
    }

    for (int k = 0; k < parser.rules.Length(); k++)
    {
        Tuple<ProcessedRuleElement> &processed_rule_elements = processed_rule_map.Next();
        VariableSymbol *lhs_symbol = lex_stream -> GetVariableSymbol(parser.rules[k].lhs_index);
        int lhs_image = AssignSymbolIndex(lhs_symbol),
            separator_token_kind = lex_stream -> Kind(parser.rules[k].separator_index);

        if (IsTerminal(lhs_image))
        {
            option -> EmitError(parser.rules[k].lhs_index, "Terminal symbol used as left hand side");
            return_code = 12;
        }

        //
        // Find the last token in the right-hand side of the rule that is
        // a grammar symbol (not a block).
        //
        int last_symbol_index = parser.rules[k].separator_index;
        for (int j = parser.rules[k].end_rhs_index - 1;
                 j > parser.rules[k].separator_index;
                 j = lex_stream -> Previous(j))
        {
            if (lex_stream -> Kind(j) == TK_SYMBOL || lex_stream -> Kind(j) == TK_EMPTY_KEY)
            {
                last_symbol_index = j;
                break;
            }
        }

        //
        // Add info for this rule to rules (of RuleElement) array.
        //
        int rule_index = rules.NextIndex();
        rules[rule_index].first_token_index = parser.rules[k].separator_index;
        rules[rule_index].last_token_index = last_symbol_index;
        rules[rule_index].lhs = lhs_image;
        rules[rule_index].separator_token_kind = separator_token_kind;
        rules[rule_index].rhs_index = rhs_sym.Length();
        rules[rule_index].source_index = k;
        rules[rule_index].produces_token_kind = (separator_token_kind != TK_OR_MARKER
                                                          ? separator_token_kind
                                                          : rules[rule_index - 1].produces_token_kind);

        bool rule_contains_action_block = false;
        for (int i = lex_stream -> Next(parser.rules[k].separator_index);
                 i < parser.rules[k].end_rhs_index;
                 i = lex_stream -> Next(i))
        {
            if (lex_stream -> Kind(i) == TK_BLOCK)
            {
                BlockSymbol *block = lex_stream -> GetBlockSymbol(i);
                if (! option -> ActionBlocks().IsIgnoredBlock(block -> BlockBegin(), block -> BlockBeginLength()))
                {
                    rule_contains_action_block = true;

                    //
                    // Check whether or not the rule is a single production
                    // and if so, issue an error and stop.
                    //
                    if (rules[rule_index].IsArrowProduction())
                        option -> EmitError(i, "This action is associated with a single production");

                    if ((! option -> attributes) || i > last_symbol_index)
                    {
                        TextBuffer *buffer = block -> Buffer();

                        ActionBlockElement &action = (option -> automatic_ast != Option::NONE ? ast_actions.Next() : code_actions.Next());
                        action.rule_number = rule_index;
                        action.location = ActionBlockElement::BODY;
                        action.block_token = i;
                        action.buffer = (buffer ? buffer : block -> ActionfileSymbol() -> BodyBuffer());
                    }
                    else
                    {
                        IntToString number(attribute_actions.Length());
                        int length = number.Length() + 1; // +1 for escape character
                        char *name = new char[length + 1]; // +1 for \0
                        name[0] = option -> macro_prefix;
                        strcpy(&(name[1]), number.String());

                        VariableSymbol *lhs_symbol = variable_table -> FindName(name, length);
                        if (lhs_symbol)
                        {
                            // TODO: GENERATE ERROR MESSAGE !!!
                            assert(0);
                        }
                        else lhs_symbol = variable_table -> InsertName(name, length);

                        int index = attribute_actions.NextIndex();
                        attribute_actions[index].lhs_image = AssignSymbolIndex(lhs_symbol);
                        attribute_actions[index].block_token = i;

                        rhs_sym.Next() = attribute_actions[index].lhs_image;

                        delete [] name;
                    }
                }
            }
            else if (lex_stream -> Kind(i) == TK_MACRO_NAME)
            {
                int image = GetSymbolIndex(i - 1); // preceding symbol
                assert(image != 0);

                //
                // This may occur if the first symbol appearing in a rule was aliased to
                // %empty.
                //
                if (image == empty)
                {
                    option -> EmitError(i, "Misplaced rule macro associated with Empty symbol");
                    return_code = 12;
                }

                //
                // We have two cases to consider here. Either the empty macro, $,
                // was specified or a named macro was specified.
                //
                // 1. A named macro was specified:
                //    a. If there was a definition already generated automatically for
                //       that symbol, we simply need to override it with the user's
                //       definition.
                //
                //    b. otherwise, we create a new definition for the macro
                //
                // 2. The empty macro ($) was specified:
                //    if there was a definition already generated for that symbol,
                //    remove it.
                //
                int position = rhs_sym.Length() - rules[rule_index].rhs_index;
                if (lex_stream -> NameStringLength(i) > 1)
                {
                    if ((option -> variables == Option::BOTH) ||
                        (option -> variables == Option::TERMINALS && IsTerminal(image)) ||
                        (option -> variables == Option::NON_TERMINALS && (! IsTerminal(image)))) // override previous definition
                    {
                        ProcessedRuleElement &processed_rule_element = processed_rule_elements[processed_rule_elements.Length() - 1];
                        processed_rule_element.image = image;
                        processed_rule_element.token_index = i;
                        assert(processed_rule_element.position == position);
                    }
                    else // add new definition
                    {
                        ProcessedRuleElement &processed_rule_element = processed_rule_elements.Next();
                        processed_rule_element.image = image;
                        processed_rule_element.token_index = i;
                        processed_rule_element.position = position;
                    }
                }
                else if ((option -> variables == Option::BOTH) ||
                         (option -> variables == Option::TERMINALS && IsTerminal(image)) ||
                         (option -> variables == Option::NON_TERMINALS && (! IsTerminal(image)))) // remove previous definition
                {
                    assert(processed_rule_elements[processed_rule_elements.Length() - 1].position == position);
                    processed_rule_elements.Reset(processed_rule_elements.Length() - 1);
                }
            }
            else
            {
                VariableSymbol *rhs_symbol = GetSymbol(i);
                int image = (rhs_symbol ? AssignSymbolIndex(rhs_symbol) : 0);
                if (image == 0)
                {
                     option -> EmitError(i, "This symbol was neither declared nor aliased to a terminal symbol");
                     return_code = 12;
                }
                else if (image == eof_image)
                {
                     option -> EmitError(i, "End-of-file symbol cannot be used in a rule");
                     return_code = 12;
                }
                else if (image != empty)
                {
                    rhs_sym.Next() = image;

                    if ((option -> variables == Option::BOTH) ||
                        (option -> variables == Option::TERMINALS && IsTerminal(image)) ||
                        (option -> variables == Option::NON_TERMINALS && (!IsTerminal(image))))
                    {
                        int position = rhs_sym.Length() - rules[rule_index].rhs_index;
                        ProcessedRuleElement &processed_rule_element = processed_rule_elements.Next();
                        processed_rule_element.image = image;
                        processed_rule_element.token_index = i;
                        processed_rule_element.position = position;
                    }
                }
            }
        }

        if (rules[rule_index].IsArrowProduction() && (rules[rule_index].rhs_index + 1 == rhs_sym.Length()))
            num_single_productions++;

        //
        // Keep track of all pairs (nonterminal X array_type) that are associated with
        // a rule that contain actions.
        //
        int array_element_type_index = parser.rules[k].array_element_type_index;
        if (array_element_type_index != 0 && rule_contains_action_block)
        {
            VariableSymbol *array_element_type_symbol = lex_stream -> GetVariableSymbol(array_element_type_index);
            int i;
            for (i = 0; i < special_nonterminal_array[array_element_type_symbol -> Index()].Length(); i++)
            {
                if (special_nonterminal_array[array_element_type_symbol -> Index()][i] == lhs_image)
                    break;
            }
            if (i == special_nonterminal_array[array_element_type_symbol -> Index()].Length())
                special_nonterminal_array[array_element_type_symbol -> Index()].Next() = lhs_image;
        }
    }

    //
    // If we have to generate extra rules because of the presence of
    // of "attributes", we do so here. Note that we don't have to check
    // here whether or not these attribute actions are to be ignored because
    // that check has already been made and if they were to be ignored they
    // would not have been added to this list in the first place.
    //
    for (int l = 0; l < attribute_actions.Length(); l++)
    {
        int rule_index = rules.NextIndex();
        (void) processed_rule_map.Next(); // allocate an empty macro table for rule_index;
        rules[rule_index].lhs = attribute_actions[l].lhs_image;
        rules[rule_index].separator_token_kind = TK_EQUIVALENCE;
        rules[rule_index].rhs_index = rhs_sym.Length();

        LexStream::TokenIndex block_token = attribute_actions[l].block_token;
        BlockSymbol *block = lex_stream -> GetBlockSymbol(block_token);
        TextBuffer *buffer = block -> Buffer();
        ActionBlockElement &action = code_actions.Next();
        action.rule_number = rule_index;
        action.location = ActionBlockElement::BODY;
        action.block_token = block_token;
        action.buffer = (buffer ? buffer : block -> ActionfileSymbol() -> BodyBuffer());
    }

    //
    // add a gate marker for the last rule.
    //
    rules.Next().rhs_index = rhs_sym.Length();

    //
    // Are there any trailing action blocks following the last rule?
    //
    for (int a = 0; a < parser.trailer_blocks.Length(); a++)
    {
        LexStream::TokenIndex block_token = parser.trailer_blocks[a];
        BlockSymbol *block = lex_stream -> GetBlockSymbol(block_token);
        if (! option -> ActionBlocks().IsIgnoredBlock(block -> BlockBegin(), block -> BlockBeginLength()))
        {
            TextBuffer *buffer = block -> Buffer();

            //
            // Note that trailer actions are associated with rule 0 instead of
            // the last rule (parser.rules.Length()). This is because we do not
            // want to have the variable name macros defined for the last rule
            // to be visible when processing trailer actions. (See ProcessCodeActions).
            //
            ActionBlockElement &action = trailer_actions.Next();
            action.rule_number = 0;
            action.location = ActionBlockElement::FINALIZE;
            action.block_token = block_token;
            action.buffer = (buffer ? buffer : block -> ActionfileSymbol() -> FinalTrailersBuffer());
        }
    }

    num_nonterminals = num_symbols - num_terminals;
    num_rules = start_symbol.Length() + (parser.rules.Length() - 1) + attribute_actions.Length();
    num_items = rhs_sym.Length() + (num_rules + 1);

    assert(num_rules == rules.Length() - 2);

    //
    // If we will be generating Ast classes, we update the classname info.
    //
    if (option -> automatic_ast != Option::NONE)
    {
        //
        // Note that we skip the start rules as No AST is generated for them.
        //
        for (int rule_index = start_symbol.Length(); rule_index <= num_rules; rule_index++)
        {
            int k = rules[rule_index].source_index,
                lhs_image = rules[rule_index].lhs,
                classname_index = parser.rules[k].classname_index,
                array_element_type_index = parser.rules[k].array_element_type_index;
            Symbol *symbol;
            {
                const char *name = (classname_index > 0
                                       ? lex_stream -> NameString(classname_index)
                                       : RetrieveString(rules[rule_index].lhs));
                if (! Code::IsValidVariableName(name))
                {
                    Tuple <const char *> msg;
                    msg.Next() = "\"";
                    msg.Next() = name;
                    msg.Next() = "\" is an illegal variable name";
                    option -> EmitError((classname_index > 0 ? classname_index
                                                             : parser.rules[k].lhs_index), msg);
                    return_code = 12;
                }

                //
                // If we are dealing with an array (typed or untyped) without an explicit
                // classname specified for it, we compose its name here. Otherwise, we accept
                // "name" as the name of the class.
                //
                if (array_element_type_index != 0 && lex_stream -> NameStringLength(classname_index) == 1) // The classname is the null macro?
                {
                    const char *array_element_name = (lex_stream -> GetVariableSymbol(array_element_type_index) -> NameLength() > 0
                                                                  ? lex_stream -> GetVariableSymbol(array_element_type_index) -> Name()
                                                                  : option -> ast_type);
                    int length = 1 + strlen(array_element_name) + strlen("List");
                    char *list_name = new char[length + 1];
                         list_name[0] = option ->macro_prefix;
                         list_name[1] = '\0';
                         strcat(list_name, array_element_name);
                         strcat(list_name, "List");
                         symbol = classname_set.FindOrInsertName(list_name, length);
                    delete [] list_name;
                }
                else symbol = classname_set.FindOrInsertName(name, strlen(name));
            }

            int index = symbol -> Index();
            if (index == classname.Length())
            {
                ClassnameElement &element = classname.Next();
                element.specified_name = symbol -> Name();
                element.real_name = (*(symbol -> Name()) == option -> macro_prefix
                                               ? symbol -> Name() + 1
                                               : symbol -> Name());
                element.array_element_type_symbol = (array_element_type_index == 0
                                                          ? NULL
                                                          : lex_stream -> GetVariableSymbol(array_element_type_index));
            }
            else if (array_element_type_index != 0 &&
                     lex_stream -> GetVariableSymbol(array_element_type_index) -> NameLength() > 0)
            {
                if (classname[index].array_element_type_symbol != lex_stream -> GetVariableSymbol(array_element_type_index))
                {
                    option -> EmitError(classname_index, "Respecification of an array with a different element type");
                    return_code = 12;
                }
            }

            //
            // If we are dealing with an array (typed or untyped) without an explicit
            // classname specified for the nonterminal in question and an action block
            // is present then we will be creating a unique subclass of the generic list
            // for this nonterminal. Here, we keep track of such instances.
            //
            VariableSymbol *array_element_type_symbol = lex_stream -> GetVariableSymbol(array_element_type_index);
            int i;
            for (i = 0; i < special_nonterminal_array[array_element_type_symbol -> Index()].Length(); i++)
            {
                if (special_nonterminal_array[array_element_type_symbol -> Index()][i] == lhs_image)
                    break;
            }
            if (i < special_nonterminal_array[array_element_type_symbol -> Index()].Length()) // add nonterminal name to make class unique
            {
                Tuple<SpecialArrayElement> &special_arrays = classname[index].special_arrays;
                int i;
                for (i = 0; i < special_arrays.Length(); i++)
                {
                    if (special_arrays[i].lhs_image == lhs_image)
                        break;
                }

                if (i < special_arrays.Length())
                    special_arrays[i].rules.Next() = rule_index;
                else // first encounter of lhs_image
                {
                    const char *lhs_name = RetrieveString(lhs_image);
                    char *extended_classname = new char[strlen(classname[index].real_name) + strlen(lhs_name) + 2];
                    strcpy(extended_classname, lhs_name);
                    strcat(extended_classname, "_");
                    strcat(extended_classname, classname[index].real_name);

                    SpecialArrayElement &special_array = special_arrays.Next();
                    special_array.lhs_image = lhs_image;
                    special_array.name = extended_classname;
                    special_array.rules.Next() = rule_index;


                }
            }

            //
            // Process rules that are explicitly associated with a non-null classname
            // or are not empty rules or are not single productions with a nonterminal
            // on the right-hand side.
            //
            int rule_size = RhsSize(rule_index); // rhs_sym.Length() - FirstRhsIndex(rule_index);
            if (rule_size > 1 ||
                (classname[index].specified_name != classname[index].real_name) ||
                (rule_size == 1 && (IsTerminal(rhs_sym[FirstRhsIndex(rule_index)]) ||
                                    lex_stream -> Kind(lex_stream -> Previous(parser.rules[rules[rule_index].source_index].end_rhs_index)) == TK_BLOCK)))
                 classname[index].rule.Next() = rule_index;
            else classname[index].ungenerated_rule.Next() = rule_index;
        }
    }

    //
    // Compute the set of "recover" symbols identified by the user.
    // Two representations are used: a bit set for ease of
    // queries and a tuple for ease of iteration.
    //
    // TODO: save the block (containing the allocation expression) for
    // each recover symbol, if it was specified.
    //
    // TODO: We will need to generate the following abstract declaration
    // in the Ast node:
    //
    //    abstract Ast create(Token);
    //
    // We will allocate an array of Ast elements called prostheticAst that is
    // indexable by the nonterminals.
    // For each nonterminal "i" for which an action block $allocation is defined,
    // (Note that $allocation is a not a macro but a real expression that may refer
    // to the variable error_token.)
    //
    //     prostheticAst[i] = new ProstheticAst() { Ast create(Token error_token) { return $allocation; } }
    //
    // otherwise,
    //
    //     prostheticAst[i] = new ProstheticAst() { Ast create(Token error_token) { return null; } }
    //
    // A prosthesis is created by invoking:
    //
    //     prostheticAst[i].create(the_error_token);
    //
    recover_set.Initialize(num_symbols + 1);
    for (int p = 0; p < parser.recovers.Length(); p++)
    {
        VariableSymbol *recover = lex_stream -> GetVariableSymbol(parser.recovers[p]);
        if (! recover_set[recover -> SymbolIndex()])
        {
            recovers.Next() = recover -> SymbolIndex();
            recover_set.AddElement(recover -> SymbolIndex());
        }
    }

    //
    // If automatic generation of the AST nodes is requested, we generate
    // the typestring map. Otherwise, we construct the typestring map based
    // on information provided by the user in the Type section of his grammar.
    // Note that this array has an upper bound of "num_symbols + 1" instead
    // of "num_symbols". This is because the index "num_symbols + 1" will be
    // used to represent the interface associated with Tokens.
    //
    SymbolLookupTable symbol_set;
    this -> ast_token_interface = num_symbols + 1;
    Array<const char *> typestring(ast_token_interface + 1, NULL);
    if (option -> automatic_ast != Option::NONE)
    {
        typestring[0] = option -> ast_type;

        typestring[ast_token_interface] = InsertInterface(symbol_set, ast_token_classname);
        {
            for (int symbol = 1; symbol <= num_terminals; symbol++)
                typestring[symbol] = typestring[ast_token_interface];
        }

        {
            for (int symbol = num_terminals + 1; symbol <= num_symbols; symbol++)
                typestring[symbol] = InsertInterface(symbol_set, RetrieveString(symbol));
        }
    }
    else if (parser.types.Length() > 0)
    {
        const char *terminal_default_type = action -> GetDefaultTerminalType(),
                   *nonterminal_default_type = action -> GetDefaultNonterminalType();
        if (terminal_default_type == NULL || nonterminal_default_type == NULL)
        {
            option -> EmitError(parser.types[0].type_index, "Type information cannot be processed for this programming language");
            return_code = 12;
        }

        typestring[0] = terminal_default_type;
        {
            for (int i = 1; i <= num_terminals; i++)
               typestring[i] = terminal_default_type;
        }

        typestring[accept_image] = nonterminal_default_type;
        for (int i = 0; i < parser.types.Length(); i++)
        {
            VariableSymbol *symbol = GetSymbol(parser.types[i].symbol_index);
            int image = (symbol ? symbol -> SymbolIndex() : 0);
            if (image == 0)
            {
                option -> EmitError(parser.types[i].symbol_index, "Undefined nonterminal symbol");
                return_code = 12;
            }

            if (option -> warnings && typestring[image] != NULL)
                option -> EmitWarning(parser.types[i].symbol_index, "Duplicate specification of this type");

            typestring[image] = lex_stream -> NameString(parser.types[i].type_index);
        }

        Tuple<int> untyped;
        for (int symbol = num_terminals + 1; symbol <= num_symbols; symbol++)
        {
            if (typestring[symbol] == NULL)
            {
                untyped.Next() = symbol;
                typestring[symbol] = nonterminal_default_type;
            }
        }

        if (option -> warnings && untyped.Length() > 0)
        {
            char tok[Control::SYMBOL_SIZE + 1];
            for (int i = 0; i < untyped.Length(); i++)
            {
                int symbol = untyped[i];

                RestoreSymbol(tok, RetrieveString(symbol));
                Tuple<const char *> msg;
                msg.Next() = "No type specified for nonterminal \"";
                msg.Next() = tok;
                msg.Next() = "\". ";
                msg.Next() = nonterminal_default_type;
                msg.Next() = " assumed.";
                option -> EmitWarning(RetrieveTokenLocation(symbol), msg);
            }
        }
    }

    //
    // If bad errors were detected, quit!
    //
    if (return_code > 0)
        control -> Exit(return_code);

    //
    // Generate the output.
    //
    action -> SetupBuiltinMacros();
    action -> GenerateDefaultTitle(notice_actions);
    action -> ProcessCodeActions(header_actions, typestring, processed_rule_map);
    if (option -> automatic_ast != Option::NONE)
    {
        assert(classname_set.Size() == classname.Length());
        action -> ProcessAstActions(ast_actions, notice_actions, initial_actions, typestring, processed_rule_map, classname_set, classname);
    }
    else
    {
        this->action -> ProcessCodeActions(initial_actions, typestring, processed_rule_map);
        this->action -> ProcessCodeActions(code_actions, typestring, processed_rule_map);
    }
    this->action -> ProcessCodeActions(trailer_actions, typestring, processed_rule_map);
	
    if (option -> warnings)
        action -> CheckExportMacros();
    action->ProcessCodeActionEnd();
	action_blocks -> Flush(); // Print out all action buffers and close action files.

    return;
}


void Grammar::SetName(VariableSymbol *symbol, VariableSymbol *symbol_name, bool negate)
{
    int index = name.NextIndex(),
        image = symbol -> SymbolIndex();
    name[index] = symbol_name -> Name();
    symbol -> SetNameIndex(index);
    symbol_index[image].external_name_index = (negate ? -index : index);

    return;
}


void Grammar::ProcessNames(void)
{
    if (option -> error_maps)
    {
        int return_code = 0;

        //
        // We take care of accept first to that he gets assigned #0
        // which we can ignore later.
        //
        SetName(accept_symbol, null_symbol);

        for (int i = 0; i < parser.names.Length(); i++)
        {
            VariableSymbol *symbol = GetSymbol(parser.names[i].lhs_index);
            int image = (symbol ? symbol -> SymbolIndex() : 0);
            if (image == 0)
            {
                option -> EmitError(parser.names[i].lhs_index, "Undefined symbol");
                return_code = 12;
            }
            else if (symbol_index[image].external_name_index != OMEGA)
            {
                option -> EmitError(parser.names[i].lhs_index, "This symbol has been named more than once");
                return_code = 12;
            }
            VariableSymbol *rhs_symbol = GetSymbol(parser.names[i].rhs_index);
            assert(rhs_symbol);
            SetName(symbol, rhs_symbol);
        }

        if (error_image == DEFAULT_SYMBOL) // some default, anything!
            symbol_index[DEFAULT_SYMBOL].external_name_index = symbol_index[accept_image].external_name_index;

        for (int j = FirstTerminal(); j <= LastTerminal(); j++)
        {
            if (symbol_index[j].external_name_index == OMEGA)
                SetName(symbol_index[j].symbol, symbol_index[j].symbol);
        }

        for (int k = FirstNonTerminal(); k <= LastNonTerminal(); k++)
        {
            if (symbol_index[k].external_name_index == OMEGA)
            {
                if (option -> names == Option::MAXIMUM)
                     SetName(symbol_index[k].symbol, symbol_index[k].symbol);
                else if (option -> names == Option::OPTIMIZED)
                     SetName(symbol_index[k].symbol, symbol_index[k].symbol, true);
                else
                {
                    assert(option -> names == Option::MINIMUM);
                    symbol_index[k].external_name_index = symbol_index[error_image].external_name_index;
                }
            }
        }

        //
        // If bad errors were detected, quit!
        //
        if (return_code > 0)
            control -> Exit(return_code);
    }

    num_names = name.Length() - 1;

    return;
}


//
// This procedure takes two character strings as arguments: IN and OUT.
// IN identifies a grammar symbol or name that is checked as to whether
// or not it needs to be quoted. If so, the necessary quotes are added
// as IN is copied into the space identified by OUT.
// NOTE that it is assumed that IN and OUT do not overlap each other.
//
void Grammar::RestoreSymbol(char *out, char *in)
{
    int  len = strlen(in);
    if (len > 0)
    {
        if ((len == 1 && in[0] == option -> or_marker) ||
            (in[0] == option -> escape)             ||
            (in[0] == option->macro_prefix) ||
            (in[0] == '\'')               ||
            (in[len - 1] == '\'')         ||
            (strchr(in, ' ') != NULL &&
            (in[0] != '<' || in[len - 1] != '>')))
        {
            *(out++) = '\'';
            while(*in != '\0')
            {
                if (*in == '\'')
                    *(out++) = *in;
                *(out++) = *(in++);
            }
            *(out++) = '\'';
            *out = '\0';

            return;
        }
    }

    strcpy(out, in);

    return;
}


//
// PRINT_LARGE_TOKEN generates code to print a token that may exceed the
// limit of its field.  The argument are LINE which is the symbol a varying
// length character string, TOKEN which is the symbol to be printed, INDENT
// which is a character string to be used as an initial prefix to indent the
// output line, and LEN which indicates the maximum number of characters that
// can be printed on a given line.  At the end of this process, LINE will
// have the value of the remaining substring that can fit on the output line.
// If a TOKEN is too large to be indented in a line, but not too large for
// the whole line, we forget the indentation, and printed it. Otherwise, it
// is "chapped up" and printed in pieces that are each indented.
//
void Grammar::PrintLargeToken(char *line, const char *token, const char *indent, int len)
{
    int toklen;

    char temp[Control::SYMBOL_SIZE + 1];

    toklen = strlen(token);

    if (toklen > len && toklen <= Control::PRINT_LINE_SIZE - 1)
    {
        fprintf(option -> syslis, "\n%s", token);
        strcpy(line, indent);
    }
    else
    {
        for (; toklen > len; toklen = strlen(temp))
        {
            memcpy(temp, token, len);
            temp[len] = '\0';
            fprintf(option -> syslis, "\n%s",temp);
            strcpy(temp, token+len + 1);
            token = temp;
        }
        strcpy(line, indent);
        strcat(line, token);
    }

    return;
}


//
//
//
void Grammar::DisplayString(const char *name, const char delimiter)
{
    fprintf(option -> syslis, " %c", delimiter);
    for (const char *p = name; *p != '\0'; p++)
    {
        switch(*p)
        {
            case '\b':
                fprintf(option -> syslis, "\\b");
                break;
            case '\t':
                fprintf(option -> syslis, "\\t");
                break;
            case '\n':
                fprintf(option -> syslis, "\\n");
                break;
            case '\f':
                fprintf(option -> syslis, "\\f");
                break;
            case '\r':
                fprintf(option -> syslis, "\\r");
                break;
            case '\\':
                fprintf(option -> syslis, "\\\\");
                break;
            case '\"':
                fprintf(option -> syslis, "\\\"");
                break;
            case '\'':
                fprintf(option -> syslis, "\\\'");
                break;
            default:
                putc(*p, option -> syslis);
                break;
        }
    }
    fprintf(option -> syslis, "%c", delimiter);
}

//
//
//
void Grammar::DisplaySymbol(const char *name)
{
    int length = strlen(name);
    if (strcmp(name, "\"")   == 0 ||
        strcmp(name, " ")    == 0 ||
        strcmp(name, "--")   == 0 ||
        strcmp(name, "|")    == 0 ||
        strcmp(name, "::=")  == 0 ||
        strcmp(name, "::=?") == 0 ||
        strcmp(name, "->")   == 0 ||
        strcmp(name, "->?")  == 0)
         fprintf(option -> syslis, " \'%s\'", name);
    else if (name[0] == '<' && name[length - 1] == '>')
         fprintf(option -> syslis, " %s", name);
    else if (strcmp(name, "\'") == 0) // a string consisting of 1 single quote
         fprintf(option -> syslis, " \"\'\"");
    else if (strpbrk(name, "\"") != NULL)
         DisplayString(name, '\'');
    else if (strpbrk(name, "\b\t\n\f\r\' ") != NULL)
         DisplayString(name, '\"');
    else if (name[0] == option->macro_prefix || name[0] == option -> escape || name[0] == '%') // does name start with user-escape or keyword-escape?
         fprintf(option -> syslis, " \'%s\'", name);
    else fprintf(option -> syslis, " %s", name);

    return;
}

//
//  If a listing is requested, this prints all the macros(if any), followed
// by the aliases(if any), followed by the terminal symbols, followed by the
// rules.
//   This grammar information is printed on lines no longer than
// PRINT_LINE_SIZE characters long.  If all the symbols in a rule cannot fit
// on one line, it is continued on a subsequent line beginning at the
// position after the equivalence symbol (::= or ->) or the middle of the
// print_line, whichever is smaller.  If a symbol cannot fit on a line
// beginning at the proper offset, it is laid out on successive lines,
// beginning at the proper offset.
//
void Grammar::DisplayInput(void)
{
    //
    // First, flush any data left in the report buffer.
    //
    option -> FlushReport();

    fprintf(option -> syslis, "\nPredefined Macros:\n\n");
    for (int i = 0; i < action -> LocalMacroTableSize(); i++)
        fprintf(option -> syslis, "    %s\n", action -> GetLocalMacro(i) -> Name());
    putc('\n', option -> syslis);

    //
    // Print the filter macros, if any.
    //
    if (action -> FilterMacroTableSize() > 0)
    {
        fprintf(option -> syslis, "\nFilter Macros:\n\n");
        for (int i = 0; i < action -> FilterMacroTableSize(); i++)
            fprintf(option -> syslis, "    %c%s\n", option -> macro_prefix, action -> GetFilterMacro(i) -> Name());
        putc('\n', option -> syslis);
    }

    //
    // Print the Exported symbols, if any.
    //
    if (parser.exports.Length() > 0)
    {
        fprintf(option -> syslis, "\nExported symbols:\n\n");
        for (int i = 0; i < parser.exports.Length(); i++)
        {
            fprintf(option -> syslis, "   ");
            DisplaySymbol(lex_stream -> NameString(parser.exports[i]));
            putc('\n', option -> syslis);
        }
    }

    //
    // Print the Macro definitions, if any.
    //
    if (macro_table -> Size() > 0)
    {
        fprintf(option -> syslis, "\n\nDefined Symbols:\n\n");
        for (int k = 0; k < macro_table -> Size(); k++)
        {
            MacroSymbol *macro = macro_table -> Element(k);
            int block_token = macro -> Block();
            if (block_token) // a defined macro
            {
                fprintf(option -> syslis, "\n\n    %s\n    ", macro -> Name());

                int start = lex_stream -> StartLocation(block_token),
                    end   = lex_stream -> EndLocation(block_token) + 1;
                for (char *head = &(lex_stream -> InputBuffer(block_token)[start]),
                          *tail = &(lex_stream -> InputBuffer(block_token)[end]);
                     head < tail;
                     head++)
                {
                    putc(*head, option -> syslis);
                    if (Code::IsNewline(*head))
                        fprintf(option -> syslis, "    ");
                }
                putc('\n', option -> syslis);
            }
        }
    }

    //
    //   Print the Aliases, if any.
    //
    if (parser.aliases.Length() > 0)
    {
        fprintf(option -> syslis, "\n\n%s\n\n", (parser.aliases.Length() == 1 ? "Alias:" : "Aliases:"));

        for (int i = 0; i < parser.aliases.Length(); i++)
        {
            fprintf( option -> syslis, "   ");
            int start = lex_stream -> StartLocation(parser.aliases[i].lhs_index),
                length = lex_stream -> EndLocation(parser.aliases[i].lhs_index) - start + 1;
            char *name = new char[length + 1];
            strncpy(name, &(lex_stream -> InputBuffer(parser.aliases[i].lhs_index)[start]), length);
            name[length] = '\0';
            DisplaySymbol(name);
            delete [] name;

            fprintf(option -> syslis, " ::=");

            start = lex_stream -> StartLocation(parser.aliases[i].rhs_index);
            length = lex_stream -> EndLocation(parser.aliases[i].rhs_index) - start + 1;
            name = new char[length + 1];
            strncpy(name, &(lex_stream -> InputBuffer(parser.aliases[i].rhs_index)[start]), length);
            name[length] = '\0';
            DisplaySymbol(name);
            delete [] name;

            putc('\n', option -> syslis);
        }
    }

    //
    // Print special symbols.
    //
    if (parser.start_indexes.Length() > 0)
    {
         fprintf(option -> syslis, "\n\nStart:\n\n");
         for (int i = 0; i < parser.start_indexes.Length(); i++)
         {
             fprintf(option -> syslis, "   ");
             DisplaySymbol(RetrieveString(start_symbol[i] -> SymbolIndex()));
         }
    }
    if (parser.identifier_index != 0)
    {
         fprintf(option -> syslis, "\n\nIdentifier:\n\n");
         fprintf(option -> syslis, "   ");
         DisplaySymbol(RetrieveString(identifier_image));
    }
    if (parser.eol_index != 0)
    {
         fprintf(option -> syslis, "\n\nEol:\n\n");
         fprintf(option -> syslis, "   ");
         DisplaySymbol(RetrieveString(eol_image));
    }
    if (parser.eof_index != 0)
    {
         fprintf(option -> syslis, "\n\nEof:\n\n");
         fprintf(option -> syslis, "   ");
         DisplaySymbol(RetrieveString(eof_image));
    }
    if (parser.error_index != 0)
    {
         fprintf(option -> syslis, "\n\nError:\n\n");
         fprintf(option -> syslis, "   ");
         DisplaySymbol(RetrieveString(error_image));
    }

    //
    // Print the terminals.
    //
    fprintf(option -> syslis, "\n\nTerminals:\n\n");
    {
        for (int i = FirstTerminal(); i <= LastTerminal(); i++)
        {
            if (i != empty)
            {
                if (! IsKeyword(i))
                {
                    fprintf(option -> syslis, "   ");
                    DisplaySymbol(RetrieveString(i));
                    putc('\n', option -> syslis);
                }
            }
        }
    }

    fprintf(option -> syslis, "\n\nSoft Keywords:\n\n");
    {
        for (int i = FirstTerminal(); i <= LastTerminal(); i++)
        {
            if (i != empty)
            {
                if (IsKeyword(i))
                {
                    fprintf(option -> syslis, "   ");
                    DisplaySymbol(RetrieveString(i));
                    putc('\n', option -> syslis);
                }
            }
        }
    }

    //
    //    Print the Rules
    //
    fprintf(option -> syslis, "\n\nRules:\n\n");
    {
        int alternate_space = 0;

        //
        // First, print the start rules.
        //
        fprintf(option -> syslis, "%-4d  ", 0);
        DisplaySymbol(RetrieveString(rules[0].lhs));
        fprintf(option -> syslis, " ::=");
        char tok[Control::SYMBOL_SIZE + 1];
        RestoreSymbol(tok, RetrieveString(rhs_sym[rules[0].rhs_index]));
        DisplaySymbol(tok);
        putc('\n', option -> syslis);
        for (int rule_no = 1; rule_no < start_symbol.Length(); rule_no++)
        {
            fprintf(option -> syslis, "%-4d  ", rule_no);
            if (rule_no > 1)
            {
                for (int i = 0; i < alternate_space; i++)
                    putc(' ', option -> syslis);
                putc(option -> or_marker, option -> syslis);
            }
            else
            {
                RestoreSymbol(tok, RetrieveString(rules[rule_no].lhs));
                alternate_space = strlen(tok) + 4;
                DisplaySymbol(tok);
                fprintf(option -> syslis, " ::=");
            }

            RestoreSymbol(tok, RetrieveString(rhs_sym[rules[rule_no].rhs_index]));
            DisplaySymbol(tok);

            RestoreSymbol(tok, RetrieveString(rhs_sym[rules[rule_no].rhs_index + 1]));
            DisplaySymbol(tok);

            putc('\n', option -> syslis);
        }

        putc('\n', option -> syslis); // leave a gap before listing the remaining rules.

        //
        // Print the user specified rules.
        //
        for (int rule_index = start_symbol.Length(); rule_index <= num_rules; rule_index++)
        {
            int source_index = rules[rule_index].source_index;
            fprintf(option -> syslis, "%-4d  ", rule_index);
            if (rules[rule_index].IsAlternateProduction())
            {
                for (int i = 0; i < alternate_space; i++)
                    putc(' ', option -> syslis);
                putc(option -> or_marker, option -> syslis);
            }
            else
            {
                const char *lhs_name = lex_stream -> NameString(parser.rules[source_index].lhs_index);
                alternate_space = strlen(lhs_name) + (rules[rule_index].IsArrowProduction() ? 3 : 4);
                DisplaySymbol(lhs_name);
                int classname_index = parser.rules[source_index].classname_index,
                    array_element_type_index = parser.rules[source_index].array_element_type_index;
                if (classname_index != 0 && array_element_type_index == 0)
                     fprintf(option -> syslis, "%s", lex_stream -> NameString(classname_index));
                else if (classname_index == 0 && array_element_type_index != 0)
                     fprintf(option -> syslis, "%c%c%s", option -> macro_prefix,
                                                         option ->macro_prefix,
                                                         lex_stream -> NameString(classname_index));
                else if (classname_index != 0 && array_element_type_index != 0)
                     fprintf(option -> syslis, "%s%c%s", lex_stream -> NameString(classname_index),
                                                         option ->macro_prefix,
                                                         lex_stream -> NameString(array_element_type_index));
                else assert (classname_index == 0 && array_element_type_index == 0);

                if (rules[rule_index].IsArrowProduction())
                     fprintf(option -> syslis, " ->");
                else fprintf(option -> syslis, " ::=");
            }

            for (int j = lex_stream -> Next(parser.rules[source_index].separator_index);
                     j < parser.rules[source_index].end_rhs_index;
                     j = lex_stream -> Next(j))
            {
                if (lex_stream -> Kind(j) == TK_SYMBOL)
                     DisplaySymbol(lex_stream -> NameString(j));
                else if (lex_stream -> Kind(j) == TK_MACRO_NAME)
                     fprintf(option -> syslis, " %s", lex_stream -> NameString(j));
                else if (lex_stream -> Kind(j) == TK_EMPTY_KEY)
                     fprintf(option -> syslis, " %c%s", option -> escape, "Empty");
            }

            putc('\n', option -> syslis);
        }
    }

    //
    //    Print the Types
    //
    if (parser.types.Length() > 0)
    {
        int alternate_space = 0;

        fprintf(option -> syslis, "\n\nTypes:\n\n");
        for (int k = 0; k < parser.types.Length(); k++)
        {
            fprintf(option -> syslis, "    ");
            if (lex_stream -> Kind(parser.types[k].separator_index) == TK_OR_MARKER)
            {
                for (int i = 0; i < alternate_space; i++)
                    putc(' ', option -> syslis);
                putc(option -> or_marker, option -> syslis);
            }
            else
            {
                alternate_space = lex_stream -> NameStringLength(parser.types[k].type_index) + 3;
                fprintf(option -> syslis, "%s ::=", lex_stream -> NameString(parser.types[k].type_index));
            }

            fprintf(option -> syslis, " %s", lex_stream -> NameString(parser.types[k].symbol_index));
            putc('\n', option -> syslis);
        }
        putc('\n', option -> syslis);
    }

    return;
}
