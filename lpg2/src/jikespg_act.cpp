
#include "jikespg_act.h"
#include "control.h"
#include "parser.h"
#include "scanner.h"
#include "symbol.h"

//#line 345 "jikespg.g"

//
// Compare the right hand-side of two rules and return true if they are identical.
//
bool jikespg_act::Compare(RuleDefinition &rule1, RuleDefinition &rule2)
{
    //
    // Copy all symbols in the right-hand side of rule 1 to list1
    //
    Tuple<int> list1;
    for (int i = lex_stream -> Next(rule1.separator_index);
         i < rule1.end_rhs_index;
         i++)
    {
        if (lex_stream -> Kind(i) == TK_SYMBOL)
            list1.Next() = i;
    }

    //
    // Copy all symbols in the right-hand side of rule 2 to list2
    //
    Tuple<int> list2;
    {
        for (int i = lex_stream -> Next(rule2.separator_index);
             i < rule2.end_rhs_index;
             i++)
        {
            if (lex_stream -> Kind(i) == TK_SYMBOL)
                list2.Next() = i;
        }
    }

    //
    // Compare symbols in list1 to symbols in list2 and return true if they are the same.
    //
    if (list1.Length() == list2.Length())
    {
        int i;
        for (i = 0; i < list1.Length(); i++)
        {
            int j = list1[i],
                k = list2[i];
            if ((lex_stream -> NameStringLength(j) != lex_stream -> NameStringLength(k)) ||
                strcmp(lex_stream -> NameString(j), lex_stream -> NameString(k)) != 0)
                break;
        }
        return (i == list1.Length());
    }

    return false;
}

//
// Merge the information from an imported grammar with the
// information for this grammar.
//
void jikespg_act::Merge(int import_file_index, Parser &import)
{
    //
    // Process drop list before adding rules
    //
    RuleLookupTable symbol_table,
                    rule_table;
    {
        for (int i = 0; i < dropped_rules.Length(); i++)
        {
            //
            // If the separator index is 0, it is an indication
            // that all rules with this lhs_index should be dropped.
            //
            if (dropped_rules[i].separator_index == 0)
                (void) symbol_table.FindOrInsertName(lex_stream -> NameString(dropped_rules[i].lhs_index),
                                                     lex_stream -> NameStringLength(dropped_rules[i].lhs_index));
            else
            {
                RuleSymbol *rule_symbol = rule_table.FindOrInsertName(lex_stream -> NameString(dropped_rules[i].lhs_index),
                                                                      lex_stream -> NameStringLength(dropped_rules[i].lhs_index));
                rule_symbol -> AddRule(i);
            }
        }
    }

    //
    // Process special symbols.
    //
    if (import.start_indexes.Length() > 0)
    {
        ImportedStartIndexes &element = imported_start_indexes.Next();
        element.import_file_index = import_file_index;
        for (int i = 0; i < import.start_indexes.Length(); i++)
            element.start_indexes.Next() = import.start_indexes[i];
    }

    if (import.identifier_index != 0)
        SetIdentifierIndex(import.identifier_index);
    if (import.eol_index != 0)
        SetEolIndex(import.eol_index);
    if (import.eof_index != 0)
        SetEofIndex(import.eof_index);
    if (import.error_index != 0)
        SetErrorIndex(import.error_index);

    {
        for (int i = 0; i < import.terminals.Length(); i++)
            terminals.Next() = import.terminals[i];
    }

    {
        for (int i = 0; i < import.keywords.Length(); i++)
            keywords.Next() = import.keywords[i];
    }

    {
        for (int i = 0; i < import.exports.Length(); i++)
            exports.Next() = import.exports[i];
    }

    {
        for (int i = 0; i < import.aliases.Length(); i++)
        {
            //
            // Allow aliasing only for names that have not been dropped.
            //
            if (! symbol_table.FindName(lex_stream -> NameString(import.aliases[i].lhs_index),
                                        lex_stream -> NameStringLength(import.aliases[i].lhs_index)))
            {
                aliases.Next() = import.aliases[i];
            }
        }
    }

    {
        for (int i = 0; i < import.names.Length(); i++)
        {
            //
            // Assign name to all symbols that have not been dropped.
            //
            if (! symbol_table.FindName(lex_stream -> NameString(import.names[i].lhs_index),
                                        lex_stream -> NameStringLength(import.names[i].lhs_index)))
            {
                names.Next() = import.names[i];
            }
        }
    }

    {
        for (int i = 0; i < import.notice_blocks.Length(); i++)
            notice_blocks.Next() = import.notice_blocks[i];
    }

    {
        for (int i = 0; i < import.global_blocks.Length(); i++)
            global_blocks.Next() = import.global_blocks[i];
    }

    {
        for (int i = 0; i < import.header_blocks.Length(); i++)
            header_blocks.Next() = import.header_blocks[i];
    }

    {
        for (int i = 0; i < import.initial_blocks.Length(); i++)
            initial_blocks.Next() = import.initial_blocks[i];
    }

    {
        for (int i = 0; i < import.trailer_blocks.Length(); i++)
            trailer_blocks.Next() = import.trailer_blocks[i];
    }

    {
        for (int i = 0; i < import.types.Length(); i++)
        {
            //
            // Add type declaration for all symbols that have not been dropped.
            //
            if (! symbol_table.FindName(lex_stream -> NameString(import.types[i].symbol_index),
                                        lex_stream -> NameStringLength(import.types[i].symbol_index)))
                types.Next() = import.types[i];
        }
    }

    //
    // Add all imported rules that have not been dropped.
    //
    // TODO: NOTE THAT THE "DropActions" feature has not yet been IMPLEMENTED !!!
    //
    for (int i = 0; i < import.rules.Length(); i++)
    {
         if (! symbol_table.FindName(lex_stream -> NameString(import.rules[i].lhs_index),
                                     lex_stream -> NameStringLength(import.rules[i].lhs_index)))
         {
             RuleSymbol *rule_symbol = rule_table.FindName(lex_stream -> NameString(import.rules[i].lhs_index),
                                                           lex_stream -> NameStringLength(import.rules[i].lhs_index));
             if (! rule_symbol)
                 rules.Next() = import.rules[i];
             else
             {
                 Tuple<int> &dr = rule_symbol -> Rules();
                 int k;
                 for (k = 0; k < dr.Length(); k++)
                 {
                     int dropped_rule_no = dr[k];
                     if (Compare(dropped_rules[dropped_rule_no], import.rules[i]))
                         break;
                 }

                 if (k == dr.Length()) // not a dropped rule
                 {
                     int current_rule = rules.Length(),
                         preceding_rule = current_rule - 1;

                     rules.Next() = import.rules[i];

                     //
                     // If the first rule in a sequence of rules containing
                     // alternations is dropped, we have to replace the alternate
                     // symbol by a production symbol in the first alternate rule in 
                     // the sequence that was not dropped.
                     //
                     if (lex_stream -> Kind(rules[current_rule].separator_index) == TK_OR_MARKER &&
                         (preceding_rule < 0 || rules[preceding_rule].lhs_index != rules[current_rule].lhs_index))
                     {
                         class Token *separator = lex_stream -> GetTokenReference(rules[current_rule].separator_index);
                         separator -> SetKind(lex_stream -> Kind(lex_stream -> Next(rules[current_rule].lhs_index)));
                     }
                 }
             }
         }
    }
}

  

//
// Rule 1:  JikesPG_INPUT ::= Grammar
//
////#line 585 "jikespg.g" 
void jikespg_act::Act1()
{
    //
    // If start symbols are specified in this source file, then
    // they override all imported start symbols, if any.
    //
    if (start_indexes.Length() == 0)
    {
        //
        // If no start symbol is specified in this source file but this source
        // file contains more than one imported file with one or more start symbols
        // specified in them then it's an error.
        // If one and only one of the imported files contain start symbol
        // specification(s) then we inherit these start symbols.
        //
        if (imported_start_indexes.Length() > 1)
        {
            for (int i = 0; i < imported_start_indexes.Length(); i++)
                 option -> EmitError(imported_start_indexes[i].import_file_index,
                                     "Conflicting start symbol(s) specified in this imported file");
            control -> Exit(12);
        }
        else if (imported_start_indexes.Length() == 1)
        {
            ImportedStartIndexes &element = imported_start_indexes[0];
            for (int i = 0; i < element.start_indexes.Length(); i++)
                start_indexes.Next() = element.start_indexes[i];
        }
    }

    imported_start_indexes.Resize(0); // free up space.
}


//
// Rule 2:  Grammar ::= $Empty
//
// void NoAction(void);
//


//
// Rule 3:  Grammar ::= Grammar include_segment END_KEY_opt
//
// void NoAction(void);
//


//
// Rule 4:  Grammar ::= Grammar notice_segment END_KEY_opt
//
// void NoAction(void);
//


//
// Rule 5:  Grammar ::= Grammar define_segment END_KEY_opt
//
// void NoAction(void);
//


//
// Rule 6:  Grammar ::= Grammar terminals_segment END_KEY_opt
//
// void NoAction(void);
//


//
// Rule 7:  Grammar ::= Grammar export_segment END_KEY_opt
//
// void NoAction(void);
//


//
// Rule 8:  Grammar ::= Grammar import_segment END_KEY_opt
//
// void NoAction(void);
//


//
// Rule 9:  Grammar ::= Grammar softkeywords_segment END_KEY_opt
//
// void NoAction(void);
//


//
// Rule 10:  Grammar ::= Grammar eof_segment END_KEY_opt
//
// void NoAction(void);
//


//
// Rule 11:  Grammar ::= Grammar eol_segment END_KEY_opt
//
// void NoAction(void);
//


//
// Rule 12:  Grammar ::= Grammar error_segment END_KEY_opt
//
// void NoAction(void);
//


//
// Rule 13:  Grammar ::= Grammar recover_segment END_KEY_opt
//
// void NoAction(void);
//


//
// Rule 14:  Grammar ::= Grammar identifier_segment END_KEY_opt
//
// void NoAction(void);
//


//
// Rule 15:  Grammar ::= Grammar start_segment END_KEY_opt
//
// void NoAction(void);
//


//
// Rule 16:  Grammar ::= Grammar alias_segment END_KEY_opt
//
// void NoAction(void);
//


//
// Rule 17:  Grammar ::= Grammar names_segment END_KEY_opt
//
// void NoAction(void);
//


//
// Rule 18:  Grammar ::= Grammar headers_segment END_KEY_opt
//
// void NoAction(void);
//


//
// Rule 19:  Grammar ::= Grammar ast_segment END_KEY_opt
//
// void NoAction(void);
//


//
// Rule 20:  Grammar ::= Grammar globals_segment END_KEY_opt
//
// void NoAction(void);
//


//
// Rule 21:  Grammar ::= Grammar trailers_segment END_KEY_opt
//
// void NoAction(void);
//


//
// Rule 22:  Grammar ::= Grammar rules_segment END_KEY_opt
//
// void NoAction(void);
//


//
// Rule 23:  Grammar ::= Grammar types_segment END_KEY_opt
//
// void NoAction(void);
//


//
// Rule 24:  Grammar ::= Grammar dps_segment END_KEY_opt
//
// void NoAction(void);
//


//
// Rule 25:  include_segment ::= INCLUDE_KEY
//
// void NoAction(void);
//


//
// Rule 26:  include_segment ::= INCLUDE_KEY SYMBOL
//
// void NoAction(void);
//


//
// Rule 27:  notice_segment ::= NOTICE_KEY
//
// void NoAction(void);
//
  

//
// Rule 28:  notice_segment ::= notice_segment action_segment
//
////#line 699 "jikespg.g" 
void jikespg_act::Act28()
{
    notice_blocks.Next() = Token(2);
}


//
// Rule 29:  define_segment ::= DEFINE_KEY
//
// void NoAction(void);
//
  

//
// Rule 30:  define_segment ::= define_segment macro_name_symbol macro_segment
//
////#line 710 "jikespg.g" 
void jikespg_act::Act30()
{
    MacroSymbol *macro_symbol = lex_stream -> GetMacroSymbol(Token(2));
    assert(macro_symbol);
    macro_symbol -> SetBlock(Token(3));
}


//
// Rule 31:  macro_name_symbol ::= MACRO_NAME
//
// void NoAction(void);
//
  

//
// Rule 32:  macro_name_symbol ::= SYMBOL
//
////#line 723 "jikespg.g" 
void jikespg_act::Act32()
{
    int length = lex_stream -> NameStringLength(Token(1)) + 1;
    char *macro_name = new char[length + 1];
    macro_name[0] = option -> macro_prefix;
    strcpy(macro_name + 1, lex_stream -> NameString(Token(1)));

    MacroSymbol *macro_symbol = macro_table -> FindName(macro_name, length);
    if (macro_symbol == NULL)
    {
        macro_symbol = macro_table -> InsertName(macro_name, length);
        ReportError(MACRO_EXPECTED_INSTEAD_OF_SYMBOL, Token(1));
    }

    lex_stream -> GetTokenReference(Token(1)) -> SetSymbol(macro_symbol);

    delete [] macro_name;
}


//
// Rule 33:  macro_segment ::= BLOCK
//
// void NoAction(void);
//


//
// Rule 34:  terminals_segment ::= TERMINALS_KEY
//
// void NoAction(void);
//
  

//
// Rule 35:  terminals_segment ::= terminals_segment terminal_symbol
//
////#line 751 "jikespg.g" 
void jikespg_act::Act35()
{
    terminals.Next() = Token(2);
}
  

//
// Rule 36:  terminals_segment ::= terminals_segment terminal_symbol produces name
//
////#line 759 "jikespg.g" 
void jikespg_act::Act36()
{
    terminals.Next() = Token(2);

    int index = names.NextIndex();
    names[index].lhs_index = Token(2);
    names[index].rhs_index = Token(4);

    index = aliases.NextIndex();
    aliases[index].lhs_index = Token(4);
    aliases[index].rhs_index = Token(2);
}


//
// Rule 37:  export_segment ::= EXPORT_KEY
//
// void NoAction(void);
//
  

//
// Rule 38:  export_segment ::= export_segment terminal_symbol
//
////#line 778 "jikespg.g" 
void jikespg_act::Act38()
{
    exports.Next() = Token(2);
}


//
// Rule 39:  import_segment ::= IMPORT_KEY
//
// void NoAction(void);
//
  

//
// Rule 40:  import_segment ::= IMPORT_KEY SYMBOL {drop_command}
//
////#line 789 "jikespg.g" 
void jikespg_act::Act40()
{
    int current_index = lex_stream -> Peek(),
        start_index = lex_stream -> NumTokens();

    Scanner scanner(option, lex_stream, variable_table, macro_table);
    scanner.Scan(Token(2));

    if (scanner.NumErrorTokens() > 0)
        control -> Exit(12);
    else // file found and scanned with no errors?
    {
        //
        // If the file is already in the process of being included, issue an error and stop.
        //
        InputFileSymbol *import_file = lex_stream -> FindOrInsertFile(option -> include_search_directory,
                                                                      lex_stream -> NameString(Token(2)));
        assert(import_file);

        if (import_file -> IsLocked())
        {
            ReportError(RECURSIVE_IMPORT, Token(2));
            control -> Exit(12);
        }
        else
        {
            //
            // Lock the include_file to avoid looping...
            //
            import_file -> Lock();

            Parser parser(control, lex_stream, variable_table, macro_table);
            parser.Parse(start_index);

            Merge(Token(2), parser);

            import_file -> Unlock();
        }
    }

    dropped_rules.Reset();
    lex_stream -> Reset(current_index);
}


//
// Rule 41:  drop_command ::= drop_symbols
//
// void NoAction(void);
//


//
// Rule 42:  drop_command ::= drop_rules
//
// void NoAction(void);
//


//
// Rule 43:  drop_command ::= DROPACTIONS_KEY
//
// void NoAction(void);
//


//
// Rule 44:  drop_symbols ::= DROPSYMBOLS_KEY
//
// void NoAction(void);
//
  

//
// Rule 45:  drop_symbols ::= drop_symbols SYMBOL
//
////#line 848 "jikespg.g" 
void jikespg_act::Act45()
{
    int index = dropped_rules.NextIndex();
    dropped_rules[index].lhs_index = Token(2);
    dropped_rules[index].separator_index = 0;
    dropped_rules[index].end_rhs_index = 0;
}


//
// Rule 46:  drop_rules ::= DROPRULES_KEY
//
// void NoAction(void);
//


//
// Rule 47:  drop_rules ::= drop_rules drop_rule
//
// void NoAction(void);
//
  

//
// Rule 48:  drop_rule ::= SYMBOL produces rhs
//
////#line 864 "jikespg.g" 
void jikespg_act::Act48()
{
    int index = dropped_rules.NextIndex();
    dropped_rules[index].lhs_index = Token(1);
    dropped_rules[index].separator_index = Token(2);
    dropped_rules[index].end_rhs_index = curtok;
}
  

//
// Rule 49:  drop_rule ::= SYMBOL MACRO_NAME produces rhs
//
////#line 874 "jikespg.g" 
void jikespg_act::Act49()
{
    int index = dropped_rules.NextIndex();
    dropped_rules[index].lhs_index = Token(1);
    dropped_rules[index].separator_index = Token(3);
    dropped_rules[index].end_rhs_index = curtok;
}
  

//
// Rule 50:  drop_rule ::= drop_rule | rhs
//
////#line 884 "jikespg.g" 
void jikespg_act::Act50()
{
    int index = dropped_rules.NextIndex();
    dropped_rules[index].lhs_index = dropped_rules[index - 1].lhs_index;
    dropped_rules[index].separator_index = Token(2);
    dropped_rules[index].end_rhs_index = curtok;
}


//
// Rule 51:  {drop_command} ::= $Empty
//
// void NoAction(void);
//


//
// Rule 52:  {drop_command} ::= {drop_command} drop_command
//
// void NoAction(void);
//


//
// Rule 53:  softkeywords_segment ::= SOFTKEYWORDS_KEY
//
// void NoAction(void);
//
  

//
// Rule 54:  softkeywords_segment ::= softkeywords_segment terminal_symbol
//
////#line 905 "jikespg.g" 
void jikespg_act::Act54()
{
    keywords.Next() = Token(2);
}
  

//
// Rule 55:  softkeywords_segment ::= softkeywords_segment terminal_symbol produces name
//
////#line 913 "jikespg.g" 
void jikespg_act::Act55()
{
    keywords.Next() = Token(2);

    int index = names.NextIndex();
    names[index].lhs_index = Token(2);
    names[index].rhs_index = Token(4);

    index = aliases.NextIndex();
    aliases[index].lhs_index = Token(4);
    aliases[index].rhs_index = Token(2);
}


//
// Rule 56:  error_segment ::= ERROR_KEY
//
// void NoAction(void);
//
  

//
// Rule 57:  error_segment ::= ERROR_KEY terminal_symbol
//
////#line 932 "jikespg.g" 
void jikespg_act::Act57()
{
    SetErrorIndex(Token(2));
}


//
// Rule 58:  recover_segment ::= RECOVER_KEY
//
// void NoAction(void);
//
  

//
// Rule 59:  recover_segment ::= recover_segment terminal_symbol
//
////#line 943 "jikespg.g" 
void jikespg_act::Act59()
{
    recovers.Next() = Token(2);
}


//
// Rule 60:  identifier_segment ::= IDENTIFIER_KEY
//
// void NoAction(void);
//
  

//
// Rule 61:  identifier_segment ::= IDENTIFIER_KEY terminal_symbol
//
////#line 954 "jikespg.g" 
void jikespg_act::Act61()
{
    SetIdentifierIndex(Token(2));
}


//
// Rule 62:  eol_segment ::= EOL_KEY
//
// void NoAction(void);
//
  

//
// Rule 63:  eol_segment ::= EOL_KEY terminal_symbol
//
////#line 965 "jikespg.g" 
void jikespg_act::Act63()
{
    SetEolIndex(Token(2));
}


//
// Rule 64:  eof_segment ::= EOF_KEY
//
// void NoAction(void);
//
  

//
// Rule 65:  eof_segment ::= EOF_KEY terminal_symbol
//
////#line 976 "jikespg.g" 
void jikespg_act::Act65()
{
    SetEofIndex(Token(2));
}


//
// Rule 66:  terminal_symbol ::= SYMBOL
//
// void NoAction(void);
//
  

//
// Rule 67:  terminal_symbol ::= MACRO_NAME
//
////#line 987 "jikespg.g" 
void jikespg_act::Act67() { ChangeMacroToVariable(Token(1)); }


//
// Rule 68:  alias_segment ::= ALIAS_KEY
//
// void NoAction(void);
//
  

//
// Rule 69:  alias_segment ::= alias_segment ERROR_KEY produces alias_rhs
//
////#line 995 "jikespg.g" 
void jikespg_act::Act69()
{
    SetErrorIndex(Token(4));
}
  

//
// Rule 70:  alias_segment ::= alias_segment EOL_KEY produces alias_rhs
//
////#line 1003 "jikespg.g" 
void jikespg_act::Act70()
{
    SetEolIndex(Token(4));
}
  

//
// Rule 71:  alias_segment ::= alias_segment EOF_KEY produces alias_rhs
//
////#line 1011 "jikespg.g" 
void jikespg_act::Act71()
{
    SetEofIndex(Token(4));
}
  

//
// Rule 72:  alias_segment ::= alias_segment IDENTIFIER_KEY produces alias_rhs
//
////#line 1019 "jikespg.g" 
void jikespg_act::Act72()
{
    SetIdentifierIndex(Token(4));
}
  

//
// Rule 73:  alias_segment ::= alias_segment SYMBOL produces alias_rhs
//
////#line 1027 "jikespg.g" 
void jikespg_act::Act73()
{
    int index = aliases.NextIndex();
    aliases[index].lhs_index = Token(2);
    aliases[index].rhs_index = Token(4);
}
  

//
// Rule 74:  alias_segment ::= alias_segment alias_lhs_macro_name produces alias_rhs
//
////#line 1037 "jikespg.g" 
void jikespg_act::Act74()
{
    int index = aliases.NextIndex();
    aliases[index].lhs_index = Token(2);
    aliases[index].rhs_index = Token(4);
}
  

//
// Rule 75:  alias_lhs_macro_name ::= MACRO_NAME
//
////#line 1047 "jikespg.g" 
void jikespg_act::Act75() { ChangeMacroToVariable(Token(1)); }


//
// Rule 76:  alias_rhs ::= SYMBOL
//
// void NoAction(void);
//
  

//
// Rule 77:  alias_rhs ::= MACRO_NAME
//
////#line 1055 "jikespg.g" 
void jikespg_act::Act77() { ChangeMacroToVariable(Token(1)); }


//
// Rule 78:  alias_rhs ::= ERROR_KEY
//
// void NoAction(void);
//


//
// Rule 79:  alias_rhs ::= EOL_KEY
//
// void NoAction(void);
//


//
// Rule 80:  alias_rhs ::= EOF_KEY
//
// void NoAction(void);
//


//
// Rule 81:  alias_rhs ::= EMPTY_KEY
//
// void NoAction(void);
//


//
// Rule 82:  alias_rhs ::= IDENTIFIER_KEY
//
// void NoAction(void);
//


//
// Rule 83:  start_segment ::= START_KEY
//
// void NoAction(void);
//
  

//
// Rule 84:  start_segment ::= start_segment start_symbol
//
////#line 1078 "jikespg.g" 
void jikespg_act::Act84()
{
    start_indexes.Next() = Token(2);
}


//
// Rule 85:  headers_segment ::= HEADERS_KEY
//
// void NoAction(void);
//


//
// Rule 86:  headers_segment ::= headers_segment headers_action_segment_list
//
// void NoAction(void);
//
  

//
// Rule 87:  headers_action_segment_list ::= action_segment
//
////#line 1092 "jikespg.g" 
void jikespg_act::Act87()
{
header_blocks.Next() = Token(1);
}


//
// Rule 89:  ast_segment ::= AST_KEY
//
// void NoAction(void);
//
  

//
// Rule 90:  ast_segment ::= ast_segment action_segment
//
////#line 1105 "jikespg.g" 
void jikespg_act::Act90()
{
    ast_blocks.Next() = Token(2);
}


//
// Rule 91:  globals_segment ::= GLOBALS_KEY
//
// void NoAction(void);
//
  

//
// Rule 92:  globals_segment ::= globals_segment action_segment
//
////#line 1116 "jikespg.g" 
void jikespg_act::Act92()
{
    global_blocks.Next() = Token(2);
}


//
// Rule 93:  trailers_segment ::= TRAILERS_KEY
//
// void NoAction(void);
//
  

//
// Rule 94:  trailers_segment ::= trailers_segment action_segment
//
////#line 1127 "jikespg.g" 
void jikespg_act::Act94()
{
    trailer_blocks.Next() = Token(2);
}


//
// Rule 95:  start_symbol ::= SYMBOL
//
// void NoAction(void);
//


//
// Rule 96:  start_symbol ::= MACRO_NAME
//
// void NoAction(void);
//


//
// Rule 97:  rules_segment ::= RULES_KEY {action_segment}
//
// void NoAction(void);
//


//
// Rule 98:  rules_segment ::= rules_segment rules
//
// void NoAction(void);
//
  

//
// Rule 99:  rules ::= SYMBOL produces rhs
//
////#line 1147 "jikespg.g" 
void jikespg_act::Act99()
{
    int index = rules.NextIndex();
    rules[index].lhs_index = Token(1);
    rules[index].classname_index = 0;
    rules[index].array_element_type_index = 0;
    rules[index].separator_index = Token(2);
    rules[index].end_rhs_index = curtok;
}
  

//
// Rule 100:  rules ::= SYMBOL MACRO_NAME produces rhs
//
////#line 1160 "jikespg.g" 
void jikespg_act::Act100()
{
    int index = rules.NextIndex();
    rules[index].lhs_index = Token(1);
    rules[index].classname_index = Token(2);
    rules[index].array_element_type_index = 0;
    rules[index].separator_index = Token(3);
    rules[index].end_rhs_index = curtok;
}
  

//
// Rule 101:  rules ::= SYMBOL MACRO_NAME MACRO_NAME produces rhs
//
////#line 1173 "jikespg.g" 
void jikespg_act::Act101()
{
    AddVariableName(Token(3));

    int index = rules.NextIndex();
    rules[index].lhs_index = Token(1);
    rules[index].classname_index = Token(2);
    rules[index].array_element_type_index = Token(3);
    rules[index].separator_index = Token(4);
    rules[index].end_rhs_index = curtok;
}
  

//
// Rule 102:  rules ::= rules | rhs
//
////#line 1188 "jikespg.g" 
void jikespg_act::Act102()
{
    int index = rules.NextIndex();
    rules[index].lhs_index = rules[index - 1].lhs_index;
    rules[index].classname_index = rules[index - 1].classname_index;
    rules[index].array_element_type_index = rules[index - 1].array_element_type_index;
    rules[index].separator_index = Token(2);
    rules[index].end_rhs_index = curtok;
}


//
// Rule 103:  produces ::= ::=
//
// void NoAction(void);
//


//
// Rule 104:  produces ::= ::=?
//
// void NoAction(void);
//


//
// Rule 105:  produces ::= ->
//
// void NoAction(void);
//


//
// Rule 106:  produces ::= ->?
//
// void NoAction(void);
//


//
// Rule 107:  rhs ::= $Empty
//
// void NoAction(void);
//


//
// Rule 108:  rhs ::= rhs SYMBOL
//
// void NoAction(void);
//


//
// Rule 109:  rhs ::= rhs SYMBOL MACRO_NAME
//
// void NoAction(void);
//


//
// Rule 110:  rhs ::= rhs EMPTY_KEY
//
// void NoAction(void);
//


//
// Rule 111:  rhs ::= rhs rhs_action_segment_list
//
// void NoAction(void);
//


//
// Rule 112:  rhs_action_segment_list ::= action_segment
//
// void NoAction(void);
//


//
// Rule 113:  rhs_action_segment_list ::= rhs_action_segment_list action_segment
//
// void NoAction(void);
//


//
// Rule 114:  action_segment ::= BLOCK
//
// void NoAction(void);
//


//
// Rule 115:  types_segment ::= TYPES_KEY
//
// void NoAction(void);
//


//
// Rule 116:  types_segment ::= types_segment type_declarationlist
//
// void NoAction(void);
//


//
// Rule 117:  type_declarationlist ::= type_declarations
//
// void NoAction(void);
//
  

//
// Rule 118:  type_declarationlist ::= type_declarations BLOCK
//
////#line 1244 "jikespg.g" 
void jikespg_act::Act118()
{
    int index = types.Length();
    do
    {
        types[--index].block_index = Token(2);
    } while(lex_stream -> Kind(types[index].separator_index) == TK_OR_MARKER);
}
  

//
// Rule 119:  type_declarations ::= SYMBOL produces SYMBOL
//
////#line 1255 "jikespg.g" 
void jikespg_act::Act119()
{
    int index = types.NextIndex();
    types[index].type_index = Token(1);
    types[index].separator_index = Token(2);
    types[index].symbol_index = Token(3);
    types[index].block_index = 0;
}
  

//
// Rule 120:  type_declarations ::= type_declarations | SYMBOL
//
////#line 1266 "jikespg.g" 
void jikespg_act::Act120()
{
    int index = types.NextIndex();
    types[index].type_index = types[index - 1].type_index;
    types[index].separator_index = Token(2);
    types[index].symbol_index = Token(3);
    types[index].block_index = 0;
}


//
// Rule 121:  dps_segment ::= DISJOINTPREDECESSORSETS_KEY
//
// void NoAction(void);
//
  

//
// Rule 122:  dps_segment ::= dps_segment SYMBOL SYMBOL
//
////#line 1281 "jikespg.g" 
void jikespg_act::Act122()
{
    int index = predecessor_candidates.NextIndex();
    predecessor_candidates[index].lhs_index = Token(2);
    predecessor_candidates[index].rhs_index = Token(3);
}


//
// Rule 123:  names_segment ::= NAMES_KEY
//
// void NoAction(void);
//
  

//
// Rule 124:  names_segment ::= names_segment name produces name
//
////#line 1294 "jikespg.g" 
void jikespg_act::Act124()
{
    int index = names.NextIndex();
    names[index].lhs_index = Token(2);
    names[index].rhs_index = Token(4);
}


//
// Rule 125:  name ::= SYMBOL
//
// void NoAction(void);
//
  

//
// Rule 126:  name ::= MACRO_NAME
//
////#line 1307 "jikespg.g" 
void jikespg_act::Act126() { ChangeMacroToVariable(Token(1)); }
  

//
// Rule 127:  name ::= EMPTY_KEY
//
////#line 1312 "jikespg.g" 
void jikespg_act::Act127()
{
    option -> EmitError(Token(1), "Illegal use of empty name or empty keyword");
    control -> Exit(12);
}


//
// Rule 128:  name ::= ERROR_KEY
//
// void NoAction(void);
//


//
// Rule 129:  name ::= EOL_KEY
//
// void NoAction(void);
//


//
// Rule 130:  name ::= IDENTIFIER_KEY
//
// void NoAction(void);
//


//
// Rule 131:  END_KEY_opt ::= $Empty
//
// void NoAction(void);
//


//
// Rule 132:  END_KEY_opt ::= END_KEY
//
// void NoAction(void);
//


//
// Rule 133:  {action_segment} ::= $Empty
//
// void NoAction(void);
//
  

//
// Rule 134:  {action_segment} ::= {action_segment} action_segment
//
////#line 1339 "jikespg.g" 
void jikespg_act::Act134()
{
    initial_blocks.Next() = Token(2);
}


//#line 1367 "jikespg.g"
#include <iostream>

using namespace std;

//
//
//
void jikespg_act::ReportError(int msg_code, int token_index)
{
    const char *msg = NULL;

    switch(msg_code)
    {
        case MACRO_EXPECTED_INSTEAD_OF_SYMBOL:
             msg = "A macro name was expected here instead of a grammar symbol name";
             break;
        case SYMBOL_EXPECTED_INSTEAD_OF_MACRO:
             msg = "A grammar symbol name was expected instead of a macro name";
             break;
        case RESPECIFICATION_OF_ERROR_SYMBOL:
             msg = "Respecification of the error symbol";
             break;
        case RESPECIFICATION_OF_IDENTIFIER_SYMBOL:
             msg = "Respecification of the identifier symbol";
             break;
        case RESPECIFICATION_OF_EOL_SYMBOL:
             msg = "Respecification of the eol symbol";
             break;
        case RESPECIFICATION_OF_EOF_SYMBOL:
             msg = "Respecification of the eof symbol";
             break;
        case RESPECIFICATION_OF_START_SYMBOL:
             msg = "Respecification of the start symbol";
             break;
        case RECURSIVE_IMPORT:
             msg = "Attempt to recursively include this file";
             break;
        default:
             assert(false);
    }

    option -> EmitWarning(token_index, msg);

    return;
}

